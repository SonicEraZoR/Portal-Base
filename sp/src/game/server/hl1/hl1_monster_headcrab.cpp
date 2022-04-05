#include "cbase.h"
#include "game.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_route.h"
#include "ai_motor.h"
#include "npcevent.h"
#include "gib.h"
#include "hl1_monster.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(void);

ConVar	headcrab_health("sk_headcrab_health", "0");
ConVar	headcrab_dmg_bite("sk_headcrab_dmg_bite", "0");

#define CRAB_ATTN_IDLE				(float)1.5
#define HEADCRAB_GUTS_GIB_COUNT		1
#define HEADCRAB_LEGS_GIB_COUNT		3
#define HEADCRAB_ALL_GIB_COUNT		5

#define HEADCRAB_MAX_JUMP_DIST		256

#define HEADCRAB_RUNMODE_ACCELERATE		1
#define HEADCRAB_RUNMODE_IDLE			2
#define HEADCRAB_RUNMODE_DECELERATE		3
#define HEADCRAB_RUNMODE_FULLSPEED		4
#define HEADCRAB_RUNMODE_PAUSE			5

#define HEADCRAB_RUN_MINSPEED	0.5
#define HEADCRAB_RUN_MAXSPEED	1.0

#define	HC_AE_JUMPATTACK		( 2 )

class CHL1Headcrab : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Headcrab, CHL1BaseMonster);
public:

	void Spawn(void);
	void Precache(void);

	void RunTask(const Task_t *pTask);
	void StartTask(const Task_t *pTask);
	void SetYawSpeed(void);
	Vector	Center(void);
	Vector	BodyTarget(const Vector &posSrc, bool bNoisy = true);

	float	MaxYawSpeed(void);
	Class_T Classify(void);

	void LeapTouch(CBaseEntity *pOther);
	void BiteSound(void);
	void AttackSound(void);
	void TouchDamage(CBaseEntity *pOther);
	void IdleSound(void);
	void DeathSound(const CTakeDamageInfo &info);
	void PainSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void HandleAnimEvent(animevent_t *pEvent);
	int	 SelectSchedule(void);
	void Touch(CBaseEntity *pOther);
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	int TranslateSchedule(int scheduleType);
	void PrescheduleThink(void);
	int RangeAttack1Conditions(float flDot, float flDist);
	float GetDamageAmount(void);

	virtual int		GetVoicePitch(void) { return 100; }
	virtual float	GetSoundVolume(void) { return 1.0; }

	int	m_nGibCount;

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

protected:
	Vector	m_vecJumpVel;
};

BEGIN_DATADESC(CHL1Headcrab)
DEFINE_ENTITYFUNC(LeapTouch),
DEFINE_FIELD(m_vecJumpVel, FIELD_VECTOR),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_headcrab, CHL1Headcrab);
PRECACHE_REGISTER(monster_headcrab);

enum
{
	SCHED_HEADCRAB_RANGE_ATTACK1 = LAST_SHARED_SCHEDULE,
	SCHED_FAST_HEADCRAB_RANGE_ATTACK1,
	SCHED_HEADCRAB_SEE,
};

void CHL1Headcrab::Spawn()
{
	Precache();

	SetRenderColor(255, 255, 255, 255);

	SetModel("models/headcrab.mdl");
	m_iHealth = headcrab_health.GetFloat();

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetViewOffset(Vector(6, 0, 11));

	m_bloodColor = BLOOD_COLOR_GREEN;
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	m_nGibCount = HEADCRAB_ALL_GIB_COUNT;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1);

	NPCInit();
}

void CHL1Headcrab::Precache(void)
{
	PrecacheModel("models/headcrab.mdl");

	PrecacheScriptSound("Headcrab.Idle");
	PrecacheScriptSound("Headcrab.Pain");
	PrecacheScriptSound("Headcrab.Alert");
	PrecacheScriptSound("Headcrab.Die");
	PrecacheScriptSound("Headcrab.Bite");
	PrecacheScriptSound("Headcrab.Attack");

	BaseClass::Precache();
}

void CHL1Headcrab::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		SetIdealActivity(ACT_RANGE_ATTACK1);
		SetTouch(&CHL1Headcrab::LeapTouch);
		break;
	}

	default:
		BaseClass::StartTask(pTask);
	}
}

void CHL1Headcrab::RunTask(const Task_t *pTask)
{


	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
	{
		if (IsSequenceFinished())
		{
			TaskComplete();
			SetTouch(NULL);
			SetIdealActivity(ACT_IDLE);
		}
		break;
	}

	default:
		CAI_BaseNPC::RunTask(pTask);
	}

}

int CHL1Headcrab::SelectSchedule(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_ALERT:
	{
		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			if (fabs(GetMotor()->DeltaIdealYaw()) < (1.0 - m_flFieldOfView) * 60)
			{
				return SCHED_TAKE_COVER_FROM_ORIGIN;
			}
			else if (SelectWeightedSequence(ACT_SMALL_FLINCH) != -1)
			{
				return SCHED_SMALL_FLINCH;
			}
		}
		else if (HasCondition(COND_HEAR_DANGER) ||
			HasCondition(COND_HEAR_PLAYER) ||
			HasCondition(COND_HEAR_WORLD) ||
			HasCondition(COND_HEAR_COMBAT))
		{
			return SCHED_ALERT_FACE_BESTSOUND;
		}
		else
		{
			if (HasCondition(COND_SEE_ENEMY))
			{
				
			}

			return SCHED_PATROL_WALK;
		}
		break;
	}
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
			return BaseClass::SelectSchedule();

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_RANGE_ATTACK1;

		return SCHED_CHASE_ENEMY;

		break;
	}
	}

	return BaseClass::SelectSchedule();
}

void CHL1Headcrab::Touch(CBaseEntity *pOther)
{
	BaseClass::Touch(pOther);
}

int CHL1Headcrab::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;

	if (info.GetDamageType() & DMG_ACID)
	{
		return 0;
	}

	return BaseClass::OnTakeDamage_Alive(info);
}

float CHL1Headcrab::GetDamageAmount(void)
{
	return headcrab_dmg_bite.GetFloat();
}

int CHL1Headcrab::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_HEADCRAB_RANGE_ATTACK1;

	//case SCHED_CHASE_ENEMY:
	//	return SCHED_HEADCRAB_SEE;

	case SCHED_FAIL_TAKE_COVER:
		return SCHED_ALERT_FACE;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

void CHL1Headcrab::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if ((m_NPCState == NPC_STATE_COMBAT) && (random->RandomFloat(0, 5) < 0.1))
	{
		IdleSound();
	}
}

int CHL1Headcrab::RangeAttack1Conditions(float flDot, float flDist)
{
	if (gpGlobals->curtime < m_flNextAttack)
	{
		return(0);
	}

	if (!(GetFlags() & FL_ONGROUND))
	{
		return(0);
	}

	if (flDist > 256)
	{
		return(COND_TOO_FAR_TO_ATTACK);
	}
	else if (flDot < 0.65)
	{
		return(COND_NOT_FACING_ATTACK);
	}

	return(COND_CAN_RANGE_ATTACK1);
}

Class_T	CHL1Headcrab::Classify(void)
{
	return CLASS_ALIEN_PREY;
}

Vector CHL1Headcrab::Center(void)
{
	return Vector(GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 6);
}

Vector CHL1Headcrab::BodyTarget(const Vector &posSrc, bool bNoisy)
{
	return(Center());
}

float CHL1Headcrab::MaxYawSpeed(void)
{
	switch (GetActivity())
	{
	case ACT_IDLE:
		return 30;
		break;

	case ACT_RUN:
	case ACT_WALK:
		return 20;
		break;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 15;
		break;

	case ACT_RANGE_ATTACK1:
		return 30;
		break;

	default:
		return 30;
		break;
	}

	return BaseClass::MaxYawSpeed();
}

void CHL1Headcrab::LeapTouch(CBaseEntity *pOther)
{
	if (pOther->Classify() == Classify())
	{
		return;
	}

	if (!(GetFlags() & FL_ONGROUND) && (pOther->IsNPC() || pOther->IsPlayer()))
	{
		BiteSound();
		TouchDamage(pOther);
	}

	SetTouch(NULL);
}

void CHL1Headcrab::BiteSound(void)
{
	CPASAttenuationFilter filter(this, ATTN_IDLE);

	CSoundParameters params;
	if (GetParametersForSound("Headcrab.Bite", params, NULL))
	{
		EmitSound_t ep(params);

		ep.m_flVolume = GetSoundVolume();
		ep.m_nPitch = GetVoicePitch();

		EmitSound(filter, entindex(), ep);
	}
}

void CHL1Headcrab::IdleSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Headcrab.Idle");
}

void CHL1Headcrab::PainSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Headcrab.Pain");
}

void CHL1Headcrab::AlertSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Headcrab.Alert");
}

void CHL1Headcrab::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Headcrab.Die");
}

void CHL1Headcrab::TouchDamage(CBaseEntity *pOther)
{
	CTakeDamageInfo info(this, this, GetDamageAmount(), DMG_SLASH);
	CalculateMeleeDamageForce(&info, GetAbsVelocity(), GetAbsOrigin());
	pOther->TakeDamage(info);
}

void CHL1Headcrab::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case HC_AE_JUMPATTACK:
	{
		SetGroundEntity(NULL);

		UTIL_SetOrigin(this, GetAbsOrigin() + Vector(0, 0, 1));

		Vector vecJumpDir;
		CBaseEntity *pEnemy = GetEnemy();
		if (pEnemy)
		{
			Vector vecEnemyEyePos = pEnemy->EyePosition();

			float gravity = sv_gravity.GetFloat();
			if (gravity <= 1)
			{
				gravity = 1;
			}

			float height = (vecEnemyEyePos.z - GetAbsOrigin().z);
			if (height < 16)
			{
				height = 16;
			}
			else if (height > 120)
			{
				height = 120;
			}
			float speed = sqrt(2 * gravity * height);
			float time = speed / gravity;

			vecJumpDir = vecEnemyEyePos - GetAbsOrigin();
			vecJumpDir = vecJumpDir / time;

			vecJumpDir.z = speed;

			float distance = vecJumpDir.Length();
			if (distance > 650)
			{
				vecJumpDir = vecJumpDir * (650.0 / distance);
			}
		}
		else
		{
			Vector forward, up;
			AngleVectors(GetAbsAngles(), &forward, NULL, &up);
			vecJumpDir = Vector(forward.x, forward.y, up.z) * 350;
		}

		int iSound = random->RandomInt(0, 2);
		if (iSound != 0)
		{
			AttackSound();
		}

		SetAbsVelocity(vecJumpDir);
		m_flNextAttack = gpGlobals->curtime + 2;
		break;
	}

	default:
	{
		CAI_BaseNPC::HandleAnimEvent(pEvent);
		break;
	}
	}
}

void CHL1Headcrab::AttackSound(void)
{
	CPASAttenuationFilter filter(this, ATTN_IDLE);

	CSoundParameters params;
	if (GetParametersForSound("Headcrab.Attack", params, NULL))
	{
		EmitSound_t ep(params);

		ep.m_flVolume = GetSoundVolume();
		ep.m_nPitch = GetVoicePitch();

		EmitSound(filter, entindex(), ep);
	}
}

AI_BEGIN_CUSTOM_NPC(monster_headcrab, CHL1Headcrab)

DEFINE_SCHEDULE
(
SCHED_HEADCRAB_RANGE_ATTACK1,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_FACE_IDEAL				0"
"		TASK_WAIT_RANDOM			0.5"
"	"
"	Interrupts"
"		COND_ENEMY_OCCLUDED"
"		COND_NO_PRIMARY_AMMO"
)

DEFINE_SCHEDULE
(
SCHED_FAST_HEADCRAB_RANGE_ATTACK1,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_RANGE_ATTACK1			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"	"
"	Interrupts"
"		COND_ENEMY_OCCLUDED"
"		COND_NO_PRIMARY_AMMO"
)

DEFINE_SCHEDULE
(
SCHED_HEADCRAB_SEE,


"	Tasks"
"		TASK_FACE_ENEMY				0"

"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
"		TASK_WALK_PATH					0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_NO_PRIMARY_AMMO"

)


AI_END_CUSTOM_NPC()

class CHL1BabyCrab : public CHL1Headcrab
{
	DECLARE_CLASS(CHL1BabyCrab, CHL1Headcrab);
public:
	void Spawn(void);
	void Precache(void);

	unsigned int PhysicsSolidMaskForEntity(void) const;

	int RangeAttack1Conditions(float flDot, float flDist);
	float MaxYawSpeed(void) { return 120.0f; }
	float GetDamageAmount(void);

	virtual int GetVoicePitch(void) { return PITCH_NORM + random->RandomInt(40, 50); }
	virtual float GetSoundVolume(void) { return 0.8; }
};

LINK_ENTITY_TO_CLASS(monster_babycrab, CHL1BabyCrab);

unsigned int CHL1BabyCrab::PhysicsSolidMaskForEntity(void) const
{
	unsigned int iMask = BaseClass::PhysicsSolidMaskForEntity();

	iMask &= ~CONTENTS_MONSTERCLIP;

	return iMask;
}

void CHL1BabyCrab::Spawn(void)
{
	CHL1Headcrab::Spawn();
	SetModel("models/baby_headcrab.mdl");
	m_nRenderMode = kRenderTransTexture;

	SetRenderColor(255, 255, 255, 192);

	UTIL_SetSize(this, Vector(-12, -12, 0), Vector(12, 12, 24));

	m_iHealth = headcrab_health.GetFloat() * 0.25;
}

void CHL1BabyCrab::Precache(void)
{
	PrecacheModel("models/baby_headcrab.mdl");
	CHL1Headcrab::Precache();
}

int CHL1BabyCrab::RangeAttack1Conditions(float flDot, float flDist)
{
	if (GetFlags() & FL_ONGROUND)
	{
		if (GetGroundEntity() && (GetGroundEntity()->GetFlags() & (FL_CLIENT | FL_NPC)))
			return COND_CAN_RANGE_ATTACK1;

		if (flDist <= 180 && flDot >= 0.55)
			return COND_CAN_RANGE_ATTACK1;
	}

	return COND_NONE;
}

float CHL1BabyCrab::GetDamageAmount(void)
{
	return headcrab_dmg_bite.GetFloat() * 0.3;
}