// Patch 2.1

#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_memory.h"
#include	"ai_route.h"
#include	"ai_motor.h"
#include	"hl1_monster_barney.h"
#include	"hl1_monster_barnacle.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"effect_dispatch_data.h"
#include	"te_effect_dispatch.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"ai_behavior_follow.h"
#include	"AI_Criteria.h"
#include	"SoundEmitterSystem/isoundemittersystembase.h"

#define BA_ATTACK	"BA_ATTACK"
#define BA_MAD		"BA_MAD"
#define BA_SHOT		"BA_SHOT"
#define BA_KILL		"BA_KILL"
#define BA_POK		"BA_POK"

ConVar	sk_barney_health("sk_barney_health", "35");

#define		BARNEY_DRAW		( 2 )
#define		BARNEY_SHOOT		( 3 )
#define		BARNEY_HOLSTER	( 4 )

#define		BARNEY_BODY_GUNHOLSTERED	0
#define		BARNEY_BODY_GUNDRAWN		1
#define		BARNEY_BODY_GUNGONE			2

BEGIN_DATADESC(CHL1Barney)
DEFINE_FIELD(m_fGunDrawn, FIELD_BOOLEAN),
DEFINE_FIELD(m_flPainTime, FIELD_TIME),
DEFINE_FIELD(m_flCheckAttackTime, FIELD_TIME),
DEFINE_FIELD(m_fLastAttackCheck, FIELD_BOOLEAN),
DEFINE_THINKFUNC(SUB_LVFadeOut),
DEFINE_USEFUNC(FollowerUse),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_barney, CHL1Barney);
PRECACHE_REGISTER(monster_barney);

static BOOL IsFacing(CBaseEntity *pevTest, const Vector &reference)
{
	Vector vecDir = (reference - pevTest->GetAbsOrigin());
	vecDir.z = 0;
	VectorNormalize(vecDir);
	Vector forward;
	QAngle angle;
	angle = pevTest->GetAbsAngles();
	angle.x = 0;
	AngleVectors(angle, &forward);
	if (DotProduct(forward, vecDir) > 0.96)
	{
		return TRUE;
	}
	return FALSE;
}

void CHL1Barney::ModifyOrAppendCriteria(AI_CriteriaSet& criteriaSet)
{
	BaseClass::ModifyOrAppendCriteria(criteriaSet);

	bool predisaster = FBitSet(m_spawnflags, SF_PRE_DISASTER) ? true : false;

	criteriaSet.AppendCriteria("disaster", predisaster ? "[disaster::pre]" : "[disaster::post]");
}

void CHL1Barney::Precache()
{
	m_iAmmoType = GetAmmoDef()->Index("9mmRound");

	PrecacheModel("models/barney.mdl");

	PrecacheScriptSound("Barney.FirePistol");
	PrecacheScriptSound("Barney.Pain");
	PrecacheScriptSound("Barney.Die");

	TalkInit();
	BaseClass::Precache();
}

void CHL1Barney::Spawn()
{
	Precache();

	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	SetModel("models/barney.mdl");

	SetRenderColor(255, 255, 255, 255);

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	m_iHealth = sk_barney_health.GetFloat();
	SetViewOffset(Vector(0, 0, 100));
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_NPCState = NPC_STATE_NONE;

	SetBodygroup(1, 0);

	m_fGunDrawn = false;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE | bits_CAP_DOORS_GROUP);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);

	NPCInit();

	CreateVPhysics();
	
	SetUse(&CHL1Barney::FollowerUse);
}

void CHL1Barney::Activate()
{
	SetUse(&CHL1Barney::FollowerUse);
	BaseClass::Activate();
}

void CHL1Barney::TalkInit()
{
	BaseClass::TalkInit();

	GetExpresser()->SetVoicePitch(100);
}

void CHL1Barney::StartTask(const Task_t *pTask)
{
	BaseClass::StartTask(pTask);
}

void CHL1Barney::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		if (GetEnemy() != NULL && (GetEnemy()->IsPlayer()))
		{
			m_flPlaybackRate = 1.5;
		}
		BaseClass::RunTask(pTask);
		break;
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

int CHL1Barney::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_CARCASS |
		SOUND_MEAT |
		SOUND_GARBAGE |
		SOUND_DANGER |
		SOUND_PLAYER;
}

Class_T	CHL1Barney::Classify(void)
{
	return	CLASS_PLAYER_ALLY;
}

void CHL1Barney::AlertSound(void)
{
	if (GetEnemy() != NULL)
	{
		if (IsOkToSpeak())
		{
			Speak(BA_ATTACK);
		}
	}

}

void CHL1Barney::SetYawSpeed(void)
{
	int ys;

	ys = 0;

	switch (GetActivity())
	{
	case ACT_IDLE:
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	GetMotor()->SetYawSpeed(ys);
}

bool CHL1Barney::CheckRangeAttack1(float flDot, float flDist)
{
	if (gpGlobals->curtime > m_flCheckAttackTime)
	{
		trace_t tr;

		Vector shootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
		CBaseEntity *pEnemy = GetEnemy();
		Vector shootTarget = ((pEnemy->BodyTarget(shootOrigin) - pEnemy->GetAbsOrigin()) + GetEnemyLKP());

		UTIL_TraceLine(shootOrigin, shootTarget, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		m_flCheckAttackTime = gpGlobals->curtime + 1;
		if (tr.fraction == 1.0 || (tr.m_pEnt != NULL && tr.m_pEnt == pEnemy))
			m_fLastAttackCheck = TRUE;
		else
			m_fLastAttackCheck = FALSE;

		m_flCheckAttackTime = gpGlobals->curtime + 1.5;
	}

	return m_fLastAttackCheck;
}

void CHL1Barney::BarneyFirePistol(void)
{
	Vector vecShootOrigin;

	vecShootOrigin = GetAbsOrigin() + Vector(0, 0, 55);
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);

	QAngle angDir;

	VectorAngles(vecShootDir, angDir);
	DoMuzzleFlash();

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, m_iAmmoType);

	int pitchShift = random->RandomInt(0, 20);

	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;

	CPASAttenuationFilter filter(this);

	EmitSound_t params;
	params.m_pSoundName = "Barney.FirePistol";
	params.m_flVolume = 1;
	params.m_nChannel = CHAN_WEAPON;
	params.m_SoundLevel = SNDLVL_NORM;
	params.m_nPitch = 100 + pitchShift;
	EmitSound(filter, entindex(), params);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 384, 0.3);
	
	m_cAmmoLoaded--;
}

int CHL1Barney::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	int ret = BaseClass::OnTakeDamage_Alive(inputInfo);

	if (!IsAlive() || m_lifeState == LIFE_DYING)
		return ret;

	if (m_NPCState != NPC_STATE_PRONE && (inputInfo.GetAttacker()->GetFlags() & FL_CLIENT))
	{
		if (GetEnemy() == NULL)
		{
			if (HasMemory(bits_MEMORY_SUSPICIOUS) || IsFacing(inputInfo.GetAttacker(), GetAbsOrigin()))
			{
				Speak(BA_MAD);

				Remember(bits_MEMORY_PROVOKED);
				StopFollowing();
			}
			else
			{
				Speak(BA_SHOT);
				Remember(bits_MEMORY_SUSPICIOUS);
			}
		}
		else
			Speak(BA_SHOT);
	}

	if (gpGlobals->curtime >= m_flPainTime)
	{
		m_flPainTime = gpGlobals->curtime + random->RandomFloat(0.5, 0.75);

		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Barney.Pain");
	}

	return ret;
}

void CHL1Barney::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	CTakeDamageInfo info = inputInfo;

	switch (ptr->hitgroup)
	{
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
		if (info.GetDamageType() & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
		{
			info.ScaleDamage(0.5f);
		}
		break;
	case HITGROUP_GEAR:
		if (info.GetDamageType() & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			info.SetDamage(info.GetDamage() - 20);
			if (info.GetDamage() <= 0)
			{
				g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
				info.SetDamage(0.01);
			}
		}
		ptr->hitgroup = HITGROUP_HEAD;
		break;
	}

	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

void CHL1Barney::Event_Killed(const CTakeDamageInfo &info)
{
	if (m_nBody < BARNEY_BODY_GUNGONE)
	{
		Vector vecGunPos;
		QAngle angGunAngles;
		CBaseEntity *pGun = NULL;

		SetBodygroup(1, BARNEY_BODY_GUNGONE);

		GetAttachment("0", vecGunPos, angGunAngles);

		angGunAngles.y += 180;
		pGun = DropItem("weapon_glock", vecGunPos, angGunAngles);
	}

	SetUse(NULL);
	BaseClass::Event_Killed(info);

	if (UTIL_IsLowViolence())
	{
		SUB_StartLVFadeOut(0.0f);
	}
}

void CHL1Barney::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);

	CSoundParameters params;
	if (GetParametersForSound("Barney.Die", params, NULL))
	{
		params.pitch = GetExpresser()->GetVoicePitch();

		EmitSound_t ep(params);

		EmitSound(filter, entindex(), ep);
	}
}

void CHL1Barney::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case BARNEY_SHOOT:
		BarneyFirePistol();
		break;

	case BARNEY_DRAW:
		SetBodygroup(1, BARNEY_BODY_GUNDRAWN);
		m_fGunDrawn = true;
		break;

	case BARNEY_HOLSTER:
		SetBodygroup(1, BARNEY_BODY_GUNHOLSTERED);
		m_fGunDrawn = false;
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
	}
}

int CHL1Barney::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_ARM_WEAPON:
		if (GetEnemy() != NULL)
		{
			return SCHED_BARNEY_ENEMY_DRAW;
		}
		break;

	case SCHED_TARGET_FACE:
	{
		int	baseType;

		baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_BARNEY_FACE_TARGET;
		else
			return baseType;
	}
	break;

	case SCHED_TARGET_CHASE:
	{
		return SCHED_BARNEY_FOLLOW;
		break;
	}

	case SCHED_IDLE_STAND:
	{
		int	baseType;

		baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_BARNEY_IDLE_STAND;
		else
			return baseType;
	}
	break;

	case SCHED_TAKE_COVER_FROM_ENEMY:
	case SCHED_CHASE_ENEMY:
	{
		if (HasCondition(COND_HEAVY_DAMAGE))
			return SCHED_TAKE_COVER_FROM_ENEMY;

		if (HasCondition(COND_CAN_RANGE_ATTACK1) && m_fGunDrawn)
			return SCHED_RANGE_ATTACK1;
	}
	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

int CHL1Barney::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_COMBAT || GetEnemy() != NULL)
	{
		if (!m_fGunDrawn)
			return SCHED_ARM_WEAPON;
	}

	if (GetFollowTarget() == NULL)
	{
		if (HasCondition(COND_PLAYER_PUSHING) && !(GetSpawnFlags() & SF_PRE_DISASTER))
			return SCHED_HL1TALKER_FOLLOW_MOVE_AWAY;
	}

	if (BehaviorSelectSchedule())
		return BaseClass::SelectSchedule();

	if (HasCondition(COND_HEAR_DANGER))
	{
		CSound *pSound;
		pSound = GetBestSound();

		ASSERT(pSound != NULL);

		if (pSound && pSound->IsSoundType(SOUND_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}
	if (HasCondition(COND_ENEMY_DEAD) && IsOkToSpeak())
	{
		Speak(BA_KILL);
	}

	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
			return BaseClass::SelectSchedule();

		if (HasCondition(COND_NEW_ENEMY) && HasCondition(COND_LIGHT_DAMAGE))
			return SCHED_SMALL_FLINCH;

		if (HasCondition(COND_HEAVY_DAMAGE))
			return SCHED_TAKE_COVER_FROM_ENEMY;

		if (!HasCondition(COND_SEE_ENEMY))
		{
			if (!HasCondition(COND_ENEMY_OCCLUDED))
			{
				return SCHED_COMBAT_FACE;
			}
			else
			{
				return SCHED_CHASE_ENEMY;
			}
		}
	}
	break;

	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			return SCHED_SMALL_FLINCH;
		}

		if (GetEnemy() == NULL && GetFollowTarget())
		{
			if (!GetFollowTarget()->IsAlive())
			{
				StopFollowing();
				break;
			}
			else
			{

				return SCHED_TARGET_FACE;
			}
		}

		TrySmellTalk();
		break;
	}

	return BaseClass::SelectSchedule();
}

void CHL1Barney::DeclineFollowing(void)
{
	if (CanSpeakAfterMyself())
	{
		Speak(BA_POK);
	}
}


bool CHL1Barney::CanBecomeRagdoll(void)
{
	if (UTIL_IsLowViolence())
	{
		return false;
	}

	return BaseClass::CanBecomeRagdoll();
}

bool CHL1Barney::ShouldGib(const CTakeDamageInfo &info)
{
	if (UTIL_IsLowViolence())
	{
		return false;
	}

	return BaseClass::ShouldGib(info);
}

int CHL1Barney::RangeAttack1Conditions(float flDot, float flDist)
{
	if (GetEnemy() == NULL)
	{
		return(COND_NONE);
	}

	else if (flDist > 1024)
	{
		return(COND_TOO_FAR_TO_ATTACK);
	}
	else if (flDot < 0.5)
	{
		return(COND_NOT_FACING_ATTACK);
	}

	if (CheckRangeAttack1(flDot, flDist))
		return(COND_CAN_RANGE_ATTACK1);

	return COND_NONE;
}

void CHL1Barney::SUB_StartLVFadeOut(float delay, bool notSolid)
{
	SetThink(&CHL1Barney::SUB_LVFadeOut);
	SetNextThink(gpGlobals->curtime + delay);
	SetRenderColorA(255);
	m_nRenderMode = kRenderNormal;

	if (notSolid)
	{
		AddSolidFlags(FSOLID_NOT_SOLID);
		SetLocalAngularVelocity(vec3_angle);
	}
}

void CHL1Barney::SUB_LVFadeOut(void)
{
	if (VPhysicsGetObject())
	{
		if (VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD || GetEFlags() & EFL_IS_BEING_LIFTED_BY_BARNACLE)
		{
			SetNextThink(gpGlobals->curtime + 5);
			SetRenderColorA(255);
			return;
		}
	}

	float dt = gpGlobals->frametime;
	if (dt > 0.1f)
	{
		dt = 0.1f;
	}
	m_nRenderMode = kRenderTransTexture;
	int speed = max(3, 256 * dt);
	SetRenderColorA(UTIL_Approach(0, m_clrRender->a, speed));
	NetworkStateChanged();

	if (m_clrRender->a == 0)
	{
		UTIL_Remove(this);
	}
	else
	{
		SetNextThink(gpGlobals->curtime);
	}
}

NPC_STATE CHL1Barney::SelectIdealState(void)
{
	return BaseClass::SelectIdealState();
}

AI_BEGIN_CUSTOM_NPC(monster_barney, CHL1Barney)
DEFINE_SCHEDULE
(
SCHED_BARNEY_FOLLOW,

"	Tasks"
"		TASK_GET_PATH_TO_TARGET			0"
"		TASK_MOVE_TO_TARGET_RANGE		180"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_TARGET_FACE"
"	"
"	Interrupts"
"			COND_NEW_ENEMY"
"			COND_LIGHT_DAMAGE"
"			COND_HEAVY_DAMAGE"
"			COND_HEAR_DANGER"
"			COND_PROVOKED"
)

DEFINE_SCHEDULE
(
SCHED_BARNEY_ENEMY_DRAW,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_ENEMY				0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_ARM"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_BARNEY_FACE_TARGET,

"	Tasks"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_FACE_TARGET			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_BARNEY_FOLLOW"
"	"
"	Interrupts"
"		COND_GIVE_WAY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_PROVOKED"
"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
SCHED_BARNEY_IDLE_STAND,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_WAIT					2"
"		TASK_TALKER_HEADRESET		0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_PROVOKED"
"		COND_HEAR_COMBAT"
"		COND_SMELL"
)

AI_END_CUSTOM_NPC()

class CHL1DeadBarney : public CAI_BaseNPC
{
	DECLARE_CLASS(CHL1DeadBarney, CAI_BaseNPC);
public:

	void Spawn(void);
	Class_T	Classify(void) { return	CLASS_NONE; }

	bool KeyValue(const char *szKeyName, const char *szValue);
	float MaxYawSpeed(void) { return 8.0f; }

	int	m_iPose;
	int m_iDesiredSequence;
	static char *m_szPoses[3];

	DECLARE_DATADESC();
};

char *CHL1DeadBarney::m_szPoses[] = { "lying_on_back", "lying_on_side", "lying_on_stomach" };

bool CHL1DeadBarney::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "pose"))
		m_iPose = atoi(szValue);
	else
		BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

LINK_ENTITY_TO_CLASS(monster_barney_dead, CHL1DeadBarney);

BEGIN_DATADESC(CHL1DeadBarney)
END_DATADESC()

void CHL1DeadBarney::Spawn(void)
{
	PrecacheModel("models/barney.mdl");
	SetModel("models/barney.mdl");

	ClearEffects();
	SetSequence(0);
	m_bloodColor = BLOOD_COLOR_RED;

	SetRenderColor(255, 255, 255, 255);

	SetSequence(m_iDesiredSequence = LookupSequence(m_szPoses[m_iPose]));
	if (GetSequence() == -1)
	{
		Msg("Dead barney with bad pose\n");
	}
	m_iHealth = 0.0;

	NPCInitDead();
}