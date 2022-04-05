#include "cbase.h"
#include "game.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_navigator.h"
#include "ai_route.h"
#include "ai_squad.h"
#include "ai_squadslot.h"
#include "ai_hint.h"
#include "npcevent.h"
#include "animation.h"
#include "hl1_monster_squad.h"
#include "gib.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"

#define HOUNDEYE_MAX_SQUAD_SIZE			4
#define	HOUNDEYE_MAX_ATTACK_RADIUS		384
#define	HOUNDEYE_SQUAD_BONUS			(float)1.1

#define HOUNDEYE_EYE_FRAMES 4

#define HOUNDEYE_SOUND_STARTLE_VOLUME	128

#define HOUNDEYE_TOP_MASS	 300.0f

#define SF_HOUNDEYE_LEADER 32

enum
{
	TASK_HOUND_CLOSE_EYE = LAST_SHARED_TASK,
	TASK_HOUND_OPEN_EYE,
	TASK_HOUND_THREAT_DISPLAY,
	TASK_HOUND_FALL_ASLEEP,
	TASK_HOUND_WAKE_UP,
	TASK_HOUND_HOP_BACK,
};

enum
{
	SCHED_HOUND_AGITATED = LAST_SHARED_SCHEDULE,
	SCHED_HOUND_HOP_RETREAT,
	SCHED_HOUND_YELL1,
	SCHED_HOUND_YELL2,
	SCHED_HOUND_RANGEATTACK,
	SCHED_HOUND_SLEEP,
	SCHED_HOUND_WAKE_LAZY,
	SCHED_HOUND_WAKE_URGENT,
	SCHED_HOUND_SPECIALATTACK,
	SCHED_HOUND_COMBAT_FAIL_PVS,
	SCHED_HOUND_COMBAT_FAIL_NOPVS,
	SCHED_HOUND_SEE,
};

enum HoundEyeSquadSlots
{
	SQUAD_SLOTS_HOUND_ATTACK = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOTS_HOUND_ATTACK2,
	SQUAD_SLOTS_HOUND_ATTACK3,
	SQUAD_SLOTS_HOUND_ATTACK4
};

#define		HOUND_AE_WARN			1
#define		HOUND_AE_STARTATTACK	2
#define		HOUND_AE_THUMP			3
#define		HOUND_AE_ANGERSOUND1	4
#define		HOUND_AE_ANGERSOUND2	5
#define		HOUND_AE_HOPBACK		6
#define		HOUND_AE_CLOSE_EYE		7

ConVar sk_houndeye_health("sk_houndeye_health", "20");
ConVar sk_houndeye_dmg_blast("sk_houndeye_dmg_blast", "15");

int g_iSquadIndexHound = 0;

class CHL1Houndeye : public CHL1SquadMonster
{
public:
	DECLARE_CLASS(CHL1Houndeye, CHL1SquadMonster)
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn(void);
	void Precache(void);
	Class_T  Classify(void);
	void HandleAnimEvent(animevent_t *pEvent);
	float MaxYawSpeed(void);
	void WarmUpSound(void);
	void AlertSound(void);
	void DeathSound(const CTakeDamageInfo &info);
	void WarnSound(void);
	void PainSound(const CTakeDamageInfo &info);
	void IdleSound(void);
	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);
	void SonicAttack(void);
	void PrescheduleThink(void);
	void SetActivity(Activity NewActivity);
	Vector WriteBeamColor(void);
	int RangeAttack1Conditions(float flDot, float flDist);
	bool FValidateHintType(CAI_Hint *sHint);
	bool ShouldGoToIdleState(void);
	int TranslateSchedule(int scheduleType);
	int SelectSchedule(void);

	float FLSoundVolume(CSound *pSound);

protected:
	int m_iSpriteTexture;
	bool m_fAsleep;
	bool m_fDontBlink;
	Vector	m_vecPackCenter;
};

LINK_ENTITY_TO_CLASS(monster_houndeye, CHL1Houndeye);
PRECACHE_REGISTER(monster_houndeye);

BEGIN_DATADESC(CHL1Houndeye)
DEFINE_FIELD(m_iSpriteTexture, FIELD_INTEGER),
DEFINE_FIELD(m_fAsleep, FIELD_BOOLEAN),
DEFINE_FIELD(m_fDontBlink, FIELD_BOOLEAN),
DEFINE_FIELD(m_vecPackCenter, FIELD_POSITION_VECTOR),
DEFINE_KEYFIELD(m_SquadName, FIELD_STRING, "netname"),
END_DATADESC();

Class_T	CHL1Houndeye::Classify(void)
{
	return	CLASS_ALIEN_MONSTER;
}

bool CHL1Houndeye::FValidateHintType(CAI_Hint *sHint)
{
	switch (sHint->HintType())
	{
	case HINT_HL1_WORLD_MACHINERY:
		return true;
		break;
	case HINT_HL1_WORLD_BLINKING_LIGHT:
		return true;
		break;
	case HINT_HL1_WORLD_HUMAN_BLOOD:
		return true;
		break;
	case HINT_HL1_WORLD_ALIEN_BLOOD:
		return true;
		break;
	}

	return false;
}

bool CHL1Houndeye::ShouldGoToIdleState(void)
{
	if (InSquad())
	{
		CHL1SquadMonster* pSquadLeader = MySquadLeader();

		for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
		{
			CHL1SquadMonster* pMember = pSquadLeader->MySquadMember(i);

			if (pMember != NULL && pMember != this && pMember->GetHintNode()->HintType() != NO_NODE)
			{
				return false;
			}
		}

		return true;
	}

	return true;
}

int CHL1Houndeye::RangeAttack1Conditions(float flDot, float flDist)
{
	trace_t tr;
	UTIL_TraceHull(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, 0.1),
		GetHullMins(), GetHullMaxs(),
		MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
	if (tr.startsolid)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity->Classify() == CLASS_ALIEN_MONSTER)
		{
			return(COND_NONE);
		}
	}

	if (flDist < 100 && flDot >= 0.3)
	{
		return COND_CAN_RANGE_ATTACK1;
	}
	if (gpGlobals->curtime < m_flNextAttack)
	{
		return(COND_NONE);
	}
	if (flDist >(HOUNDEYE_MAX_ATTACK_RADIUS * 0.5))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	if (flDot < 0.3)
	{
		return COND_NOT_FACING_ATTACK;
	}
	return COND_CAN_RANGE_ATTACK1;
}

float CHL1Houndeye::MaxYawSpeed(void)
{
	int ys;

	ys = 90;

	switch (GetActivity())
	{
	case ACT_CROUCHIDLE:
		ys = 0;
		break;
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_WALK:
		ys = 90;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 90;
		break;
	}

	return ys;
}

void CHL1Houndeye::SetActivity(Activity NewActivity)
{
	int	iSequence;

	if (NewActivity == GetActivity())
		return;

	if (m_NPCState == NPC_STATE_COMBAT && NewActivity == ACT_IDLE && random->RandomInt(0, 1))
	{
		iSequence = LookupSequence("madidle");

		SetActivity(NewActivity);

		SetIdealActivity(GetActivity());

		if (iSequence > ACTIVITY_NOT_AVAILABLE)
		{
			SetSequence(iSequence);
			SetCycle(0);
			ResetSequenceInfo();
			ResetSequenceInfo();
		}
	}
	else
	{
		BaseClass::SetActivity(NewActivity);
	}
}

void CHL1Houndeye::HandleAnimEvent(animevent_t *pEvent)
{
	CPASAttenuationFilter filter(this);
	switch (pEvent->event)
	{
	case HOUND_AE_WARN:
		WarnSound();
		break;

	case HOUND_AE_STARTATTACK:
		WarmUpSound();
		break;

	case HOUND_AE_HOPBACK:
	{
		float flGravity = sv_gravity.GetFloat();
		Vector v_forward;
		GetVectors(&v_forward, NULL, NULL);

		SetGroundEntity(NULL);

		Vector vecVel = v_forward * -200;
		vecVel.z += (0.6 * flGravity) * 0.5;
		SetAbsVelocity(vecVel);

		break;
	}

	case HOUND_AE_THUMP:
		SonicAttack();
		break;

	case HOUND_AE_ANGERSOUND1:
		EmitSound(filter, entindex(), "HoundEye.Anger1");
		break;

	case HOUND_AE_ANGERSOUND2:
		EmitSound(filter, entindex(), "HoundEye.Anger2");
		break;

	case HOUND_AE_CLOSE_EYE:
		if (!m_fDontBlink)
		{
			m_nSkin = HOUNDEYE_EYE_FRAMES - 1;
		}
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CHL1Houndeye::Spawn()
{
	Precache();

	SetRenderColor(255, 255, 255, 255);

	SetModel("models/houndeye.mdl");

	UTIL_SetSize(this, Vector(-16, -16, 0), Vector(16, 16, 36));

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_YELLOW;
	ClearEffects();
	m_iHealth = sk_houndeye_health.GetFloat();
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	m_fAsleep = FALSE;
	m_fDontBlink = FALSE;

	CapabilitiesClear();

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_SQUAD);

	g_iSquadIndexHound = 0;

	NPCInit();
}

void CHL1Houndeye::Precache()
{
	PrecacheModel("models/houndeye.mdl");

	m_iSpriteTexture = PrecacheModel("sprites/shockwave.vmt");

	PrecacheScriptSound("HoundEye.Idle");
	PrecacheScriptSound("HoundEye.Warn");
	PrecacheScriptSound("HoundEye.Hunt");
	PrecacheScriptSound("HoundEye.Alert");
	PrecacheScriptSound("HoundEye.Die");
	PrecacheScriptSound("HoundEye.Pain");
	PrecacheScriptSound("HoundEye.Anger1");
	PrecacheScriptSound("HoundEye.Anger2");
	PrecacheScriptSound("HoundEye.Sonic");

	BaseClass::Precache();
}

void CHL1Houndeye::IdleSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Idle");
}

void CHL1Houndeye::WarmUpSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Warn");
}

void CHL1Houndeye::WarnSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Hunt");
}

void CHL1Houndeye::AlertSound(void)
{
	if (InSquad() && !IsLeader())
		return;

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Alert");
}

void CHL1Houndeye::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Die");
}

void CHL1Houndeye::PainSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Pain");
}

Vector CHL1Houndeye::WriteBeamColor(void)
{
	BYTE	bRed, bGreen, bBlue;


	if (InSquad())
	{
		switch (SquadCount())
		{
		case 2:
			bRed = 101;
			bGreen = 133;
			bBlue = 221;
			break;
		case 3:
			bRed = 67;
			bGreen = 85;
			bBlue = 255;
			break;
		case 4:
			bRed = 62;
			bGreen = 33;
			bBlue = 211;
			break;
		default:
			bRed = 188;
			bGreen = 220;
			bBlue = 255;
			break;
		}
	}
	else
	{
		bRed = 188;
		bGreen = 220;
		bBlue = 255;
	}


	return Vector(bRed, bGreen, bBlue);
}

void CHL1Houndeye::SonicAttack(void)
{
	float		flAdjustedDamage;
	float		flDist;

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "HoundEye.Sonic");

	CBroadcastRecipientFilter filter2;
	te->BeamRingPoint(filter2, 0.0,
		GetAbsOrigin(),
		16,
		HOUNDEYE_MAX_ATTACK_RADIUS,
		m_iSpriteTexture,
		0,
		0,
		0,
		0.2,
		24,
		16,
		0,
		WriteBeamColor().x,
		WriteBeamColor().y,
		WriteBeamColor().z,
		192,
		0
		);

	CBroadcastRecipientFilter filter3;
	te->BeamRingPoint(filter3, 0.0,
		GetAbsOrigin(),
		16,
		HOUNDEYE_MAX_ATTACK_RADIUS / 2,
		m_iSpriteTexture,
		0,
		0,
		0,
		0.2,
		24,
		16,
		0,
		WriteBeamColor().x,
		WriteBeamColor().y,
		WriteBeamColor().z,
		192,
		0
		);

	CBaseEntity* pEntity = NULL;

	while ((pEntity = gEntList.FindEntityInSphere(pEntity, GetAbsOrigin(), HOUNDEYE_MAX_ATTACK_RADIUS)) != NULL)
	{
		if (pEntity->m_takedamage != DAMAGE_NO)
		{
			if (!FClassnameIs(pEntity, "monster_houndeye"))
			{
				if (InSquad() && SquadCount() > 1 && SquadCount() < HOUNDEYE_MAX_SQUAD_SIZE + 1)
				{
					flAdjustedDamage = sk_houndeye_dmg_blast.GetFloat() + sk_houndeye_dmg_blast.GetFloat() * (HOUNDEYE_SQUAD_BONUS * (SquadCount() - 1));
				}
				else
				{
					flAdjustedDamage = sk_houndeye_dmg_blast.GetFloat();
				}

				flDist = (pEntity->WorldSpaceCenter() - GetAbsOrigin()).Length();

				flAdjustedDamage -= (flDist / HOUNDEYE_MAX_ATTACK_RADIUS) * flAdjustedDamage;

				if (!FVisible(pEntity))
				{
					if (pEntity->IsPlayer())
					{
						flAdjustedDamage *= 0.5;
					}
					else if (!FClassnameIs(pEntity, "func_breakable") && !FClassnameIs(pEntity, "func_pushable"))
					{
						flAdjustedDamage = 0;
					}
				}

				if (flAdjustedDamage > 0)
				{
					CTakeDamageInfo info(this, this, flAdjustedDamage, DMG_SONIC | DMG_ALWAYSGIB);
					CalculateExplosiveDamageForce(&info, (pEntity->GetAbsOrigin() - GetAbsOrigin()), pEntity->GetAbsOrigin());

					pEntity->TakeDamage(info);

					if ((pEntity->GetAbsOrigin() - GetAbsOrigin()).Length2D() <= HOUNDEYE_MAX_ATTACK_RADIUS)
					{
						if (pEntity->GetMoveType() == MOVETYPE_VPHYSICS || (pEntity->VPhysicsGetObject() && !pEntity->IsPlayer()))
						{
							IPhysicsObject *pPhysObject = pEntity->VPhysicsGetObject();

							if (pPhysObject)
							{
								float flMass = pPhysObject->GetMass();

								if (flMass <= HOUNDEYE_TOP_MASS)
								{
									Vector vecForce = info.GetDamageForce();
									vecForce.z *= 2.0f;
									info.SetDamageForce(vecForce);

									pEntity->VPhysicsTakeDamage(info);
								}
							}
						}
					}
				}
			}
		}
	}
}

void CHL1Houndeye::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HOUND_FALL_ASLEEP:
	{
		m_fAsleep = TRUE;
		TaskComplete();
		break;
	}
	case TASK_HOUND_WAKE_UP:
	{
		m_fAsleep = FALSE;
		TaskComplete();
		break;
	}
	case TASK_HOUND_OPEN_EYE:
	{
		m_fDontBlink = FALSE;
		TaskComplete();
		break;
	}
	case TASK_HOUND_CLOSE_EYE:
	{
		m_nSkin = 0;
		m_fDontBlink = TRUE;
		break;
	}
	case TASK_HOUND_THREAT_DISPLAY:
	{
		SetIdealActivity(ACT_IDLE_ANGRY);
		break;
	}
	case TASK_HOUND_HOP_BACK:
	{
		SetIdealActivity(ACT_LEAP);
		break;
	}
	case TASK_RANGE_ATTACK1:
	{
		SetIdealActivity(ACT_RANGE_ATTACK1);
		break;
	}
	case TASK_SPECIAL_ATTACK1:
	{
		SetIdealActivity(ACT_SPECIAL_ATTACK1);
		break;
	}
	default:
	{
		BaseClass::StartTask(pTask);
		break;
	}
	}
}

void CHL1Houndeye::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HOUND_THREAT_DISPLAY:
	{
		if (GetEnemy())
		{
			float idealYaw;
			idealYaw = UTIL_VecToYaw(GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
			GetMotor()->SetIdealYawAndUpdate(idealYaw);
		}

		if (IsSequenceFinished())
		{
			TaskComplete();
		}

		break;
	}
	case TASK_HOUND_CLOSE_EYE:
	{
		if (m_nSkin < HOUNDEYE_EYE_FRAMES - 1)
			m_nSkin++;
		break;
	}
	case TASK_HOUND_HOP_BACK:
	{
		if (IsSequenceFinished())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_SPECIAL_ATTACK1:
	{
		m_nSkin = random->RandomInt(0, HOUNDEYE_EYE_FRAMES - 1);

		float idealYaw;
		idealYaw = UTIL_VecToYaw(GetNavigator()->GetGoalPos());
		GetMotor()->SetIdealYawAndUpdate(idealYaw);

		float life;
		life = ((255 - GetCycle()) / (m_flPlaybackRate * m_flPlaybackRate));
		if (life < 0.1)
		{
			life = 0.1;
		}

		if (IsSequenceFinished())
		{
			SonicAttack();
			TaskComplete();
		}

		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

void CHL1Houndeye::PrescheduleThink(void)
{
	if (m_NPCState == NPC_STATE_COMBAT && GetActivity() == ACT_RUN && random->RandomFloat(0, 1) < 0.2)
	{
		WarnSound();
	}

	if (!m_fDontBlink)
	{
		if ((m_nSkin == 0) && random->RandomInt(0, 0x7F) == 0)
		{
			m_nSkin = HOUNDEYE_EYE_FRAMES - 1;
		}
		else if (m_nSkin != 0)
		{
			m_nSkin--;
		}
	}
	if (IsLeader())
	{
		CHL1SquadMonster* pSquadMember;
		int iSquadCount = 0;

		for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
		{
			pSquadMember = MySquadMember(i);

			if (pSquadMember)
			{
				iSquadCount++;
				m_vecPackCenter = m_vecPackCenter + pSquadMember->GetAbsOrigin();
			}
		}

		m_vecPackCenter = m_vecPackCenter / iSquadCount;
	}
}

int CHL1Houndeye::TranslateSchedule(int scheduleType)
{
	if (m_fAsleep)
	{
		if (HasCondition(COND_HEAR_DANGER) || HasCondition(COND_HEAR_THUMPER) || HasCondition(COND_HEAR_COMBAT) ||
			HasCondition(COND_HEAR_WORLD) || HasCondition(COND_HEAR_PLAYER) || HasCondition(COND_HEAR_BULLET_IMPACT))
		{
			CSound *pWakeSound;

			pWakeSound = GetBestSound();
			ASSERT(pWakeSound != NULL);
			if (pWakeSound)
			{
				GetMotor()->SetIdealYawToTarget(pWakeSound->GetSoundOrigin());

				if (FLSoundVolume(pWakeSound) >= HOUNDEYE_SOUND_STARTLE_VOLUME)
				{
					return SCHED_HOUND_WAKE_URGENT;
				}
			}
			return SCHED_HOUND_WAKE_LAZY;
		}
		else if (HasCondition(COND_NEW_ENEMY))
		{
			return SCHED_HOUND_WAKE_URGENT;
		}

		else
		{
			return SCHED_HOUND_WAKE_LAZY;
		}
	}
	switch (scheduleType)
	{
	case SCHED_IDLE_STAND:
	{
		if (InSquad() && !IsLeader() && !m_fAsleep && random->RandomInt(0, 29) < 1)
		{
			return SCHED_HOUND_SLEEP;
		}
		else
		{
			return BaseClass::TranslateSchedule(scheduleType);
		}
	}

	case SCHED_RANGE_ATTACK1:
		return SCHED_HOUND_RANGEATTACK;
	case SCHED_SPECIAL_ATTACK1:
		return SCHED_HOUND_SPECIALATTACK;

	case SCHED_FAIL:
	{
		if (m_NPCState == NPC_STATE_COMBAT)
		{
			if (!FNullEnt(UTIL_FindClientInPVS(edict())))
			{
				return SCHED_HOUND_COMBAT_FAIL_PVS;
			}
			else
			{
				return SCHED_HOUND_COMBAT_FAIL_NOPVS;
			}
		}
		else
		{
			return BaseClass::TranslateSchedule(scheduleType);
		}
	}
	default:
	{
		return BaseClass::TranslateSchedule(scheduleType);
	}
	}
}

int CHL1Houndeye::SelectSchedule(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
		{
			return BaseClass::SelectSchedule();
		}

		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			if (random->RandomFloat(0, 1) <= 0.4)
			{
				trace_t trace;
				Vector v_forward;
				GetVectors(&v_forward, NULL, NULL);
				UTIL_TraceEntity(this, GetAbsOrigin(), GetAbsOrigin() + v_forward * -128, MASK_SOLID, &trace);

				if (trace.fraction == 1.0)
				{
					return SCHED_HOUND_HOP_RETREAT;
				}
			}

			return SCHED_TAKE_COVER_FROM_ENEMY;
		}

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			if (OccupyStrategySlot(SQUAD_SLOTS_HOUND_ATTACK))
			{
				return SCHED_RANGE_ATTACK1;
			}
			if (OccupyStrategySlot(SQUAD_SLOTS_HOUND_ATTACK2))
			{
				return SCHED_RANGE_ATTACK1;
			}
			if (OccupyStrategySlot(SQUAD_SLOTS_HOUND_ATTACK3))
			{
				return SCHED_RANGE_ATTACK1;
			}
			if (OccupyStrategySlot(SQUAD_SLOTS_HOUND_ATTACK4))
			{
				return SCHED_RANGE_ATTACK1;
			}

			return SCHED_HOUND_AGITATED;
		}
		if (HasCondition(COND_SEE_ENEMY)){
			return SCHED_HOUND_SEE;

		}



		break;

	}
	}

	return BaseClass::SelectSchedule();
}

float CHL1Houndeye::FLSoundVolume(CSound *pSound)
{
	return (pSound->Volume() - ((pSound->GetSoundOrigin() - GetAbsOrigin()).Length()));
}

AI_BEGIN_CUSTOM_NPC(monster_houndeye, CHL1Houndeye)

DECLARE_TASK(TASK_HOUND_CLOSE_EYE)
DECLARE_TASK(TASK_HOUND_OPEN_EYE)
DECLARE_TASK(TASK_HOUND_THREAT_DISPLAY)
DECLARE_TASK(TASK_HOUND_FALL_ASLEEP)
DECLARE_TASK(TASK_HOUND_WAKE_UP)
DECLARE_TASK(TASK_HOUND_HOP_BACK)

DEFINE_SCHEDULE
(
SCHED_HOUND_AGITATED,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_HOUND_THREAT_DISPLAY		0"

"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_FACE_ENEMY					0"


"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_HOP_RETREAT,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_HOUND_HOP_BACK			0"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_TAKE_COVER_FROM_ENEMY"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_YELL1,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_HOUND_AGITATED"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_YELL2,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_RANGE_ATTACK1			0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_RANGEATTACK,

"	Tasks"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_HOUND_YELL1"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_SLEEP,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_WAIT_RANDOM			5"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CROUCH"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_CROUCHIDLE"
"		TASK_HOUND_FALL_ASLEEP		0"
"		TASK_WAIT_RANDOM			25"
"	TASK_HOUND_CLOSE_EYE		0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_NEW_ENEMY"
"		COND_HEAR_COMBAT"
"		COND_HEAR_DANGER"
"		COND_HEAR_PLAYER"
"		COND_HEAR_WORLD"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_WAKE_LAZY,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_HOUND_OPEN_EYE			0"
"		TASK_WAIT_RANDOM			2.5"
"		TASK_PLAY_SEQUENCE			ACT_STAND"
"		TASK_HOUND_WAKE_UP			0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_WAKE_URGENT,

"	Tasks"
"		TASK_HOUND_OPEN_EYE			0"
"		TASK_PLAY_SEQUENCE			ACT_HOP"
"		TASK_FACE_IDEAL				0"
"		TASK_HOUND_WAKE_UP			0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_SPECIALATTACK,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_SPECIAL_ATTACK1		0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_IDLE_ANGRY"
"	"
"	Interrupts"
"		"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_ENEMY_OCCLUDED"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_COMBAT_FAIL_PVS,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_HOUND_THREAT_DISPLAY	0"
"		TASK_WAIT_FACE_ENEMY		1"
"	"
"	Interrupts"
"		"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_COMBAT_FAIL_NOPVS,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_HOUND_THREAT_DISPLAY	0"
"		TASK_WAIT_FACE_ENEMY		1"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_WAIT_PVS				0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
SCHED_HOUND_SEE,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_WAIT_FACE_ENEMY		1"

"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_FACE_ENEMY					0"


"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_WAIT_PVS				0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

AI_END_CUSTOM_NPC()