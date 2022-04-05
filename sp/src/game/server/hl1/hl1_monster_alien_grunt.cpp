// Patch 2.1

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_navigator.h"
#include "ai_route.h"
#include "ai_squad.h"
#include "ai_squadslot.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "animation.h"
#include "basecombatweapon.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "te.h"
#include "hl1_monster_squad.h"

ConVar sk_agrunt_health("sk_agrunt_health", "0");
ConVar sk_agrunt_dmg_punch("sk_agrunt_dmg_punch", "0");

#define		AGRUNT_AE_HORNET1	( 1 )
#define		AGRUNT_AE_HORNET2	( 2 )
#define		AGRUNT_AE_HORNET3	( 3 )
#define		AGRUNT_AE_HORNET4	( 4 )
#define		AGRUNT_AE_HORNET5	( 5 )
#define		AGRUNT_AE_PUNCH		( 6 )
#define		AGRUNT_AE_BITE		( 7 )
#define		AGRUNT_AE_LEFT_FOOT	 ( 10 )
#define		AGRUNT_AE_RIGHT_FOOT ( 11 )
#define		AGRUNT_AE_LEFT_PUNCH ( 12 )
#define		AGRUNT_AE_RIGHT_PUNCH ( 13 )
#define		AGRUNT_MELEE_DIST	100

int iAgruntMuzzleFlash;
int ACT_THREAT_DISPLAY;

enum AGruntSquadSlot_T
{
	AGRUNT_SQUAD_SLOT_HORNET1 = LAST_SHARED_SQUADSLOT,
	AGRUNT_SQUAD_SLOT_HORNET2,
	AGRUNT_SQUAD_SLOT_CHASE,
};

enum
{
	SCHED_AGRUNT_FAIL = LAST_SHARED_SCHEDULE,
	SCHED_AGRUNT_COMBAT_FAIL,
	SCHED_AGRUNT_STANDOFF,
	SCHED_AGRUNT_SUPPRESS_HORNET,
	SCHED_AGRUNT_RANGE_ATTACK,
	SCHED_AGRUNT_HIDDEN_RANGE_ATTACK,
	SCHED_AGRUNT_TAKE_COVER_FROM_ENEMY,
	SCHED_AGRUNT_VICTORY_DANCE,
	SCHED_AGRUNT_THREAT_DISPLAY,
};

enum
{
	TASK_AGRUNT_SETUP_HIDE_ATTACK = LAST_SHARED_TASK,
	TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE,
	TASK_AGRUNT_RANGE_ATTACK1_NOTURN,
};

class CHL1AlienGrunt : public CHL1SquadMonster
{
	DECLARE_CLASS(CHL1AlienGrunt, CHL1SquadMonster);
public:

	void Spawn(void);
	void Precache(void);

	float	MaxYawSpeed(void);
	Class_T Classify(void) { return CLASS_ALIEN_MILITARY; }
	int  GetSoundInterests(void);
	void HandleAnimEvent(animevent_t* pEvent);

	void AlertSound(void);
	void DeathSound(const CTakeDamageInfo& info);
	void PainSound(const CTakeDamageInfo& info);
	void AttackSound(void);

	bool ShouldSpeak(void);
	void PrescheduleThink(void);

	bool FCanCheckAttacks(void);

	int MeleeAttack1Conditions(float flDot, float flDist);
	int RangeAttack1Conditions(float flDot, float flDist);

	void StopTalking(void);

	void StartTask(const Task_t* pTask);
	void RunTask(const Task_t* pTask);

	int TranslateSchedule(int scheduleType);
	int SelectSchedule(void);

	void	TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);
	int		IRelationPriority(CBaseEntity* pTarget);

public:
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	bool	m_fCanHornetAttack;
	float	m_flNextHornetAttackCheck;

	float	m_flNextPainTime;

	float	m_flNextSpeakTime;
	float	m_flNextWordTime;
	float   m_flDamageTime;
};

LINK_ENTITY_TO_CLASS(monster_alien_grunt, CHL1AlienGrunt);

BEGIN_DATADESC(CHL1AlienGrunt)
DEFINE_FIELD(m_fCanHornetAttack, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextHornetAttackCheck, FIELD_TIME),
DEFINE_FIELD(m_flNextPainTime, FIELD_TIME),
DEFINE_FIELD(m_flNextSpeakTime, FIELD_TIME),
DEFINE_FIELD(m_flNextWordTime, FIELD_TIME),
DEFINE_FIELD(m_flDamageTime, FIELD_TIME),
DEFINE_KEYFIELD(m_SquadName, FIELD_STRING, "netname"),
END_DATADESC()

int CHL1AlienGrunt::IRelationPriority(CBaseEntity* pTarget)
{
	if (pTarget->Classify() == CLASS_HUMAN_MILITARY)
	{
		if (FClassnameIs(pTarget, "monster_human_grunt"))
		{
			return (BaseClass::IRelationPriority(pTarget) + 1);
		}
	}

	return BaseClass::IRelationPriority(pTarget);
}

void CHL1AlienGrunt::Spawn()
{
	Precache();

	SetModel("models/agrunt.mdl");
	UTIL_SetSize(this, Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);

	Vector vecSurroundingMins(-32, -32, 0);
	Vector vecSurroundingMaxs(32, 32, 85);
	CollisionProp()->SetSurroundingBoundsType(USE_SPECIFIED_BOUNDS, &vecSurroundingMins, &vecSurroundingMaxs);

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_GREEN;
	ClearEffects();
	m_iHealth = sk_agrunt_health.GetFloat();
	m_flFieldOfView = 0.2;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_SQUAD | bits_CAP_MOVE_GROUND);

	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);

	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	m_HackedGunPos = Vector(24, 64, 48);

	m_flNextSpeakTime = m_flNextWordTime = gpGlobals->curtime + 10 + random->RandomInt(0, 10);

	SetHullType(HULL_WIDE_HUMAN);
	SetHullSizeNormal();

	SetRenderColor(255, 255, 255, 255);

	NPCInit();

	BaseClass::Spawn();
}

void CHL1AlienGrunt::Precache()
{
	PrecacheModel("models/agrunt.mdl");

	iAgruntMuzzleFlash = PrecacheModel("sprites/muz4.vmt");

	UTIL_PrecacheOther("hornet");

	PrecacheScriptSound("Weapon_Hornetgun.Single");
	PrecacheScriptSound("AlienGrunt.LeftFoot");
	PrecacheScriptSound("AlienGrunt.RightFoot");
	PrecacheScriptSound("AlienGrunt.AttackHit");
	PrecacheScriptSound("AlienGrunt.AttackMiss");
	PrecacheScriptSound("AlienGrunt.Die");
	PrecacheScriptSound("AlienGrunt.Alert");
	PrecacheScriptSound("AlienGrunt.Attack");
	PrecacheScriptSound("AlienGrunt.Pain");
	PrecacheScriptSound("AlienGrunt.Idle");
}

float CHL1AlienGrunt::MaxYawSpeed(void)
{
	float ys;

	switch (GetActivity())
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 110;
		break;
	default:
		ys = 100;
	}

	return ys;
}

int CHL1AlienGrunt::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_PLAYER |
		SOUND_DANGER;
}

void CHL1AlienGrunt::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case AGRUNT_AE_HORNET1:
	case AGRUNT_AE_HORNET2:
	case AGRUNT_AE_HORNET3:
	case AGRUNT_AE_HORNET4:
	case AGRUNT_AE_HORNET5:
	{
		Vector vecArmPos;
		QAngle angArmDir;
		Vector vecDirToEnemy;
		QAngle angDir;

		if (HasCondition(COND_SEE_ENEMY) && GetEnemy())
		{
			Vector vecEnemyLKP = GetEnemy()->GetAbsOrigin();

			vecDirToEnemy = ((vecEnemyLKP)-GetAbsOrigin());
			VectorAngles(vecDirToEnemy, angDir);
			VectorNormalize(vecDirToEnemy);
		}
		else
		{
			angDir = GetAbsAngles();
			angDir.x = -angDir.x;

			Vector vForward;
			AngleVectors(angDir, &vForward);
			vecDirToEnemy = vForward;
		}

		DoMuzzleFlash();

		if (angDir.x > 180)
		{
			angDir.x = angDir.x - 360;
		}

		GetAttachment("0", vecArmPos, angArmDir);

		vecArmPos = vecArmPos + vecDirToEnemy * 32;

		CPVSFilter filter(GetAbsOrigin());
		te->Sprite(filter, 0.0,
			&vecArmPos, iAgruntMuzzleFlash, random->RandomFloat(0.4, 0.8), 128);

		CBaseEntity* pHornet = CBaseEntity::Create("hornet", vecArmPos, QAngle(0, 0, 0), this);

		Vector vForward;
		AngleVectors(angDir, &vForward);

		pHornet->SetAbsVelocity(vForward * 300);
		pHornet->SetOwnerEntity(this);

		EmitSound("Weapon_Hornetgun.Single");

		CHL1BaseMonster* pHornetMonster = (CHL1BaseMonster*)pHornet->MyNPCPointer();

		if (pHornetMonster)
		{
			pHornetMonster->SetEnemy(GetEnemy());
		}
	}
	break;

	case AGRUNT_AE_LEFT_FOOT:
	{
		CPASAttenuationFilter filter2(this);
		EmitSound(filter2, entindex(), "AlienGrunt.LeftFoot");
	}
	break;
	case AGRUNT_AE_RIGHT_FOOT:
	{
		CPASAttenuationFilter filter3(this);
		EmitSound(filter3, entindex(), "AlienGrunt.RightFoot");
	}
	break;

	case AGRUNT_AE_LEFT_PUNCH:
	{
		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity* pHurt = CheckTraceHullAttack(AGRUNT_MELEE_DIST, vecMins, vecMaxs, sk_agrunt_dmg_punch.GetFloat(), DMG_CLUB);
		CPASAttenuationFilter filter4(this);

		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
				pHurt->ViewPunch(QAngle(-25, 8, 0));

			Vector vRight;
			AngleVectors(GetAbsAngles(), NULL, &vRight, NULL);

			if (pHurt->IsPlayer())
			{
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + vRight * 250);
			}

			EmitSound(filter4, entindex(), "AlienGrunt.AttackHit");

			Vector vecArmPos;
			QAngle angArmAng;
			GetAttachment(0, vecArmPos, angArmAng);
			SpawnBlood(vecArmPos, g_vecAttackDir, pHurt->BloodColor(), 25);
		}
		else
		{
			EmitSound(filter4, entindex(), "AlienGrunt.AttackMiss");
		}
	}
	break;

	case AGRUNT_AE_RIGHT_PUNCH:
	{
		Vector vecMins = GetHullMins();
		Vector vecMaxs = GetHullMaxs();
		vecMins.z = vecMins.x;
		vecMaxs.z = vecMaxs.x;

		CBaseEntity* pHurt = CheckTraceHullAttack(AGRUNT_MELEE_DIST, vecMins, vecMaxs, sk_agrunt_dmg_punch.GetFloat(), DMG_CLUB);
		CPASAttenuationFilter filter5(this);

		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
				pHurt->ViewPunch(QAngle(25, 8, 0));

			if (pHurt->IsPlayer())
			{
				Vector vRight;
				AngleVectors(GetAbsAngles(), NULL, &vRight, NULL);
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + vRight * -250);
			}

			EmitSound(filter5, entindex(), "AlienGrunt.AttackHit");

			Vector vecArmPos;
			QAngle angArmAng;
			GetAttachment(0, vecArmPos, angArmAng);
			SpawnBlood(vecArmPos, g_vecAttackDir, pHurt->BloodColor(), 25);
		}
		else
		{
			EmitSound(filter5, entindex(), "AlienGrunt.AttackMiss");
		}
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CHL1AlienGrunt::DeathSound(const CTakeDamageInfo& info)
{
	StopTalking();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlienGrunt.Die");
}

void CHL1AlienGrunt::AlertSound(void)
{
	StopTalking();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlienGrunt.Alert");
}

void CHL1AlienGrunt::AttackSound(void)
{
	StopTalking();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlienGrunt.Attack");
}

void CHL1AlienGrunt::PainSound(const CTakeDamageInfo& info)
{
	if (m_flNextPainTime > gpGlobals->curtime)
	{
		return;
	}

	m_flNextPainTime = gpGlobals->curtime + 0.6;

	StopTalking();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlienGrunt.Pain");
}

bool CHL1AlienGrunt::ShouldSpeak(void)
{
	if (m_flNextSpeakTime > gpGlobals->curtime)
	{
		return FALSE;
	}

	if (m_spawnflags & SF_NPC_GAG)
	{
		if (m_NPCState != NPC_STATE_COMBAT)
		{
			m_flNextSpeakTime = gpGlobals->curtime + 3;
			return FALSE;
		}
	}

	return TRUE;
}

void CHL1AlienGrunt::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (ShouldSpeak())
	{
		if (m_flNextWordTime < gpGlobals->curtime)
		{
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "AlienGrunt.Idle");

			if (random->RandomInt(1, 10) <= 1)
			{
				StopTalking();
			}
			else
			{
				m_flNextWordTime = gpGlobals->curtime + random->RandomFloat(0.5, 1);
			}
		}
	}
}

bool CHL1AlienGrunt::FCanCheckAttacks(void)
{
	if (!HasCondition(COND_ENEMY_TOO_FAR))
		return true;
	else
		return false;
}

int CHL1AlienGrunt::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > AGRUNT_MELEE_DIST)
		return COND_NONE;

	if (flDot < 0.6)
		return COND_NONE;

	if (HasCondition(COND_SEE_ENEMY) && GetEnemy() != NULL)
		return COND_CAN_MELEE_ATTACK1;

	return COND_NONE;
}

int CHL1AlienGrunt::RangeAttack1Conditions(float flDot, float flDist)
{
	if (gpGlobals->curtime < m_flNextHornetAttackCheck)
	{
		if (HasCondition(COND_SEE_ENEMY))
		{
			if (m_fCanHornetAttack == true)
			{
				return COND_CAN_RANGE_ATTACK1;
			}
			else
			{
				return COND_NONE;
			}
		}
		else
			return COND_NONE;
	}

	if (flDist < AGRUNT_MELEE_DIST)
		return COND_NONE;

	if (flDist > 1024)
		return COND_NONE;

	if (flDot < 0.5)
		return COND_NONE;

	if (HasCondition(COND_SEE_ENEMY))
	{
		trace_t tr;
		Vector	vecArmPos;
		QAngle	angArmDir;

		GetAttachment("0", vecArmPos, angArmDir);
		UTIL_TraceLine(vecArmPos, GetEnemy()->BodyTarget(vecArmPos), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction == 1.0 || tr.m_pEnt == GetEnemy())
		{
			m_flNextHornetAttackCheck = gpGlobals->curtime + random->RandomFloat(2, 5);
			m_fCanHornetAttack = true;

			return COND_CAN_RANGE_ATTACK1;
		}
	}

	m_flNextHornetAttackCheck = gpGlobals->curtime + 0.2;
	m_fCanHornetAttack = false;
	return COND_NONE;
}

void CHL1AlienGrunt::StopTalking(void)
{
	m_flNextWordTime = m_flNextSpeakTime = gpGlobals->curtime + 10 + random->RandomInt(0, 10);
}


void CHL1AlienGrunt::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_AGRUNT_RANGE_ATTACK1_NOTURN:
	{
		SetLastAttackTime(gpGlobals->curtime);
		ResetIdealActivity(ACT_RANGE_ATTACK1);
	}
	break;

	case TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE:
	{
		Vector forward;
		AngleVectors(GetAbsAngles(), &forward);
		Vector flEnemyLKP = GetEnemyLKP();

		GetNavigator()->SetGoal(flEnemyLKP - forward * 64, AIN_CLEAR_TARGET);

		if (GetNavigator()->SetGoal(flEnemyLKP - forward * 64, AIN_CLEAR_TARGET))
		{
			TaskComplete();
		}
		else
		{
			Msg("AGruntGetPathToEnemyCorpse failed!!\n");
			TaskFail(FAIL_NO_ROUTE);
		}
	}
	break;

	case TASK_AGRUNT_SETUP_HIDE_ATTACK:
		CHL1BaseMonster* pEnemyMonsterPtr;

		pEnemyMonsterPtr = (CHL1BaseMonster*)GetEnemy()->MyNPCPointer();

		if (pEnemyMonsterPtr)
		{
			Vector		vecCenter, vForward, vRight, vecEnemyLKP;
			QAngle		angTmp;
			trace_t		tr;
			BOOL		fSkip;

			fSkip = FALSE;
			vecCenter = WorldSpaceCenter();
			vecEnemyLKP = GetEnemyLKP();

			VectorAngles(vecEnemyLKP - GetAbsOrigin(), angTmp);
			SetAbsAngles(angTmp);
			AngleVectors(GetAbsAngles(), &vForward, &vRight, NULL);

			UTIL_TraceLine(WorldSpaceCenter() + vForward * 128, vecEnemyLKP, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
			if (tr.fraction == 1.0)
			{
				GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + vRight * 128);
				fSkip = TRUE;
				TaskComplete();
			}

			if (!fSkip)
			{
				UTIL_TraceLine(WorldSpaceCenter() - vForward * 128, vecEnemyLKP, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
				if (tr.fraction == 1.0)
				{
					GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() - vRight * 128);
					fSkip = TRUE;
					TaskComplete();
				}
			}

			if (!fSkip)
			{
				UTIL_TraceLine(WorldSpaceCenter() + vForward * 256, vecEnemyLKP, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
				if (tr.fraction == 1.0)
				{
					GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + vRight * 256);
					fSkip = TRUE;
					TaskComplete();
				}
			}

			if (!fSkip)
			{
				UTIL_TraceLine(WorldSpaceCenter() - vForward * 256, vecEnemyLKP, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
				if (tr.fraction == 1.0)
				{
					GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() - vRight * 256);
					fSkip = TRUE;
					TaskComplete();
				}
			}

			if (!fSkip)
			{
				TaskFail(FAIL_NO_COVER);
			}
		}
		else
		{
			Msg("AGRunt - no enemy monster ptr!!!\n");
			TaskFail(FAIL_NO_ENEMY);
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CHL1AlienGrunt::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_AGRUNT_RANGE_ATTACK1_NOTURN:
	{
		AutoMovement();

		if (IsActivityFinished())
		{
			TaskComplete();
		}
		break;
	}

	default:
		BaseClass::RunTask(pTask);
	}
}

int CHL1AlienGrunt::SelectSchedule(void)
{
	if (HasCondition(COND_HEAR_DANGER))
	{
		return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
		{
			return BaseClass::SelectSchedule();
		}

		if (HasCondition(COND_NEW_ENEMY))
		{
			return SCHED_WAKE_ANGRY;
		}

		if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			AttackSound();
			return SCHED_MELEE_ATTACK1;
		}

		if (HasCondition(COND_HEAVY_DAMAGE))
		{
			return SCHED_SMALL_FLINCH;
		}

		if (HasCondition(COND_CAN_RANGE_ATTACK1) && OccupyStrategySlotRange(AGRUNT_SQUAD_SLOT_HORNET1, AGRUNT_SQUAD_SLOT_HORNET2))
		{
			return SCHED_RANGE_ATTACK1;
		}

		if (OccupyStrategySlot(AGRUNT_SQUAD_SLOT_CHASE))
		{
			return SCHED_CHASE_ENEMY;
		}

		return  SCHED_STANDOFF;
	}
	}

	return BaseClass::SelectSchedule();
}

int CHL1AlienGrunt::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		return SCHED_AGRUNT_TAKE_COVER_FROM_ENEMY;
		break;

	case SCHED_STANDOFF:
		return SCHED_AGRUNT_STANDOFF;
		break;

	case SCHED_VICTORY_DANCE:
		return SCHED_AGRUNT_VICTORY_DANCE;
		break;
	case SCHED_FAIL:
	{
		if (GetEnemy() != NULL)
		{
			return SCHED_AGRUNT_COMBAT_FAIL;
		}

		return SCHED_AGRUNT_FAIL;
	}
	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

void CHL1AlienGrunt::TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr)
{
	CTakeDamageInfo ainfo = info;

	float flDamage = ainfo.GetDamage();

	if (ptr->hitgroup == 10 && (ainfo.GetDamageType() & (DMG_BULLET | DMG_SLASH | DMG_CLUB)))
	{
		if (m_flDamageTime != gpGlobals->curtime || (random->RandomInt(0, 10) < 1))
		{
			CPVSFilter filter(ptr->endpos);
			te->ArmorRicochet(filter, 0.0, &ptr->endpos, &ptr->plane.normal);
			m_flDamageTime = gpGlobals->curtime;
		}

		if (random->RandomInt(0, 1) == 0)
		{
			Vector vecTracerDir = vecDir;

			vecTracerDir.x += random->RandomFloat(-0.3, 0.3);
			vecTracerDir.y += random->RandomFloat(-0.3, 0.3);
			vecTracerDir.z += random->RandomFloat(-0.3, 0.3);

			vecTracerDir = vecTracerDir * -512;

			Vector vEndPos = ptr->endpos + vecTracerDir;

			UTIL_Tracer(ptr->endpos, vEndPos, ENTINDEX(edict()));
		}

		flDamage -= 20;
		if (flDamage <= 0)
			flDamage = 0.1;

		ainfo.SetDamage(flDamage);
	}
	else
	{
		SpawnBlood(ptr->endpos, vecDir, BloodColor(), flDamage);
		TraceBleed(flDamage, vecDir, ptr, ainfo.GetDamageType());
	}

	AddMultiDamage(ainfo, this);
}

AI_BEGIN_CUSTOM_NPC(monster_alien_grunt, CHL1AlienGrunt)

DECLARE_ACTIVITY(ACT_THREAT_DISPLAY)

DECLARE_TASK(TASK_AGRUNT_SETUP_HIDE_ATTACK)
DECLARE_TASK(TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE)
DECLARE_TASK(TASK_AGRUNT_RANGE_ATTACK1_NOTURN)

DECLARE_SQUADSLOT(AGRUNT_SQUAD_SLOT_HORNET1)
DECLARE_SQUADSLOT(AGRUNT_SQUAD_SLOT_HORNET2)
DECLARE_SQUADSLOT(AGRUNT_SQUAD_SLOT_CHASE)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_FAIL,
	"	Tasks"
	"	TASK_STOP_MOVING			0"
	"	TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	"	TASK_WAIT					2"
	"	TASK_WAIT_PVS				0"
	"	"
	"	Interrupts"
	"	COND_CAN_RANGE_ATTACK1"
	"	COND_CAN_MELEE_ATTACK1"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_COMBAT_FAIL,
	"	Tasks"
	"	TASK_STOP_MOVING			0"
	"	TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	"	TASK_WAIT_FACE_ENEMY		2"
	"	TASK_WAIT_PVS				0"
	"	"
	"	Interrupts"
	"	COND_CAN_RANGE_ATTACK1"
	"	COND_CAN_MELEE_ATTACK1"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_STANDOFF,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	"		TASK_WAIT_FACE_ENEMY		2"
	"	"
	"	Interrupts"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_SEE_ENEMY"
	"		COND_NEW_ENEMY"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_SUPPRESS_HORNET,
	"	Tasks"
	"	TASK_STOP_MOVING			0"
	"	TASK_RANGE_ATTACK1			0"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_RANGE_ATTACK,
	"	Tasks"
	"	TASK_STOP_MOVING			0"
	"	TASK_FACE_ENEMY				0"
	"	TASK_RANGE_ATTACK1			0"
	"	"
	"	Interrupts"
	"	COND_NEW_ENEMY"
	"	COND_ENEMY_DEAD"
	"	COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_HIDDEN_RANGE_ATTACK,
	"	Tasks"
	"	TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_AGRUNT_STANDOFF"
	"	TASK_AGRUNT_SETUP_HIDE_ATTACK		0"
	"	TASK_STOP_MOVING					0"
	"	TASK_FACE_IDEAL						0"
	"	TASK_AGRUNT_RANGE_ATTACK1_NOTURN	0"
	"	"
	"	Interrupts"
	"	COND_NEW_ENEMY"
	"	COND_HEAVY_DAMAGE"
	"	COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_TAKE_COVER_FROM_ENEMY,
	"	Tasks"
	"	TASK_STOP_MOVING				0"
	"	TASK_WAIT						0.2"
	"	TASK_FIND_COVER_FROM_ENEMY		0"
	"	TASK_RUN_PATH					0"
	"	TASK_WAIT_FOR_MOVEMENT			0"
	"	TASK_REMEMBER					MEMORY:INCOVER"
	"	TASK_FACE_ENEMY					0"
	"	"
	"	Interrupts"
	"	COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_VICTORY_DANCE,
	"	Tasks"
	"	TASK_STOP_MOVING							0"
	"	TASK_SET_FAIL_SCHEDULE						SCHEDULE:SCHED_AGRUNT_THREAT_DISPLAY"
	"	TASK_WAIT									0.2"
	"	TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE		0"
	"	TASK_WALK_PATH								0"
	"	TASK_WAIT_FOR_MOVEMENT						0"
	"	TASK_FACE_ENEMY								0"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_CROUCH"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_STAND"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_THREAT_DISPLAY"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_CROUCH"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_VICTORY_DANCE"
	"	TASK_PLAY_SEQUENCE							ACTIVITY:ACT_STAND"
	"	"
	"	Interrupts"
	"	COND_NEW_ENEMY"
	"	COND_LIGHT_DAMAGE"
	"	COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
	SCHED_AGRUNT_THREAT_DISPLAY,
	"	Tasks"
	"	TASK_STOP_MOVING		0"
	"	TASK_FACE_ENEMY			0"
	"	TASK_PLAY_SEQUENCE		ACTIVITY:ACT_THREAT_DISPLAY"
	"	"
	"	Interrupts"
	"	COND_NEW_ENEMY"
	"	COND_LIGHT_DAMAGE"
	"	COND_HEAVY_DAMAGE"
	"	COND_HEAR_PLAYER"
	"	COND_HEAR_COMBAT"
	"	COND_HEAR_WORLD"
)

AI_END_CUSTOM_NPC()