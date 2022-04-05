#include "cbase.h"
#include "game.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_route.h"
#include "ai_hint.h"
#include "ai_navigator.h"
#include "ai_senses.h"
#include "npcevent.h"
#include "animation.h"
#include "hl1_monster.h"
#include "gib.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "util.h"
#include "shake.h"
#include "movevars_shared.h"
#include "decals.h"
#include "hl2_shareddefs.h"
#include "ammodef.h"

#define	SQUID_SPRINT_DIST 256

ConVar sk_bullsquid_health("sk_bullsquid_health", "0");
ConVar sk_bullsquid_dmg_bite("sk_bullsquid_dmg_bite", "0");
ConVar sk_bullsquid_dmg_whip("sk_bullsquid_dmg_whip", "0");
ConVar sk_bullsquid_dmg_spit("sk_bullsquid_dmg_spit", "0");

extern ConVar hl1_bullsquid_spit;

enum
{
	SCHED_SQUID_HURTHOP = LAST_SHARED_SCHEDULE,
	SCHED_SQUID_SEECRAB,
	SCHED_SQUID_EAT,
	SCHED_SQUID_SNIFF_AND_EAT,
	SCHED_SQUID_WALLOW,
	SCHED_SQUID_CHASE_ENEMY,
};

enum
{
	TASK_SQUID_HOPTURN = LAST_SHARED_TASK,
	TASK_SQUID_EAT,
};

enum
{
	COND_SQUID_SMELL_FOOD = LAST_SHARED_CONDITION + 1,
};

class CSquidSpit : public CBaseEntity
{
	DECLARE_CLASS(CSquidSpit, CBaseEntity);
public:
	void Spawn(void);
	void Precache(void);

	static void Shoot(CBaseEntity *pOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity *pOther);
	void Animate(void);

	int m_nSquidSpitSprite;

	DECLARE_DATADESC();

	void SetSprite(CBaseEntity *pSprite)
	{
		m_hSprite = pSprite;
	}

	CBaseEntity *GetSprite(void)
	{
		return m_hSprite.Get();
	}

private:
	EHANDLE m_hSprite;


};

LINK_ENTITY_TO_CLASS(squidspit, CSquidSpit);
PRECACHE_REGISTER(squidspit);

BEGIN_DATADESC(CSquidSpit)
DEFINE_FIELD(m_nSquidSpitSprite, FIELD_INTEGER),
DEFINE_FIELD(m_hSprite, FIELD_EHANDLE),
END_DATADESC()

void CSquidSpit::Precache(void)
{
	m_nSquidSpitSprite = PrecacheModel("sprites/bigspit.vmt");

	PrecacheScriptSound("NPC_BigMomma.SpitTouch1");
	PrecacheScriptSound("NPC_BigMomma.SpitHit1");
	PrecacheScriptSound("NPC_BigMomma.SpitHit2");
}

void CSquidSpit::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_FLY);
	SetClassname("squidspit");

	SetSolid(SOLID_BBOX);

	m_nRenderMode = kRenderTransAlpha;
	SetRenderColorA(255);
	SetModel("");

	SetSprite(CSprite::SpriteCreate("sprites/bigspit.vmt", GetAbsOrigin(), true));

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));

	SetCollisionGroup(HL2COLLISION_GROUP_SPIT);
}

void CSquidSpit::Shoot(CBaseEntity *pOwner, Vector vecStart, Vector vecVelocity)
{
	CSquidSpit *pSpit = CREATE_ENTITY(CSquidSpit, "squidspit");
	pSpit->Spawn();

	UTIL_SetOrigin(pSpit, vecStart);
	pSpit->SetAbsVelocity(vecVelocity);
	pSpit->SetOwnerEntity(pOwner);

	CSprite *pSprite = (CSprite*)pSpit->GetSprite();

	if (pSprite)
	{
		pSprite->SetAttachment(pSpit, 0);
		pSprite->SetOwnerEntity(pSpit);

		pSprite->SetScale(0.5);
		pSprite->SetTransparency(pSpit->m_nRenderMode, pSpit->m_clrRender->r, pSpit->m_clrRender->g, pSpit->m_clrRender->b, pSpit->m_clrRender->a, pSpit->m_nRenderFX);
	}


	CPVSFilter filter(vecStart);

	VectorNormalize(vecVelocity);
	te->SpriteSpray(filter, 0.0, &vecStart, &vecVelocity, pSpit->m_nSquidSpitSprite, 210, 25, 15);
}

void CSquidSpit::Touch(CBaseEntity *pOther)
{
	trace_t tr;
	int		iPitch;

	if (pOther->GetSolidFlags() & FSOLID_TRIGGER)
		return;

	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
	{
		return;
	}

	iPitch = random->RandomFloat(90, 110);

	EmitSound("NPC_BigMomma.SpitTouch1");

	switch (random->RandomInt(0, 1))
	{
	case 0:
		EmitSound("NPC_BigMomma.SpitHit1");
		break;
	case 1:
		EmitSound("NPC_BigMomma.SpitHit2");
		break;
	}

	if (!pOther->m_takedamage)
	{
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		if (hl1_bullsquid_spit.GetBool())
		{
			if (random->RandomInt(0, 1))
				UTIL_DecalTrace(&tr, "BullsquidSpit1");
			else
				UTIL_DecalTrace(&tr, "BullsquidSpit2");
		}
		else
			UTIL_DecalTrace(&tr, "BeerSplash");

		CPVSFilter filter(tr.endpos);

		te->SpriteSpray(filter, 0.0, &tr.endpos, &tr.plane.normal, m_nSquidSpitSprite, 30, 8, 5);

	}
	else
	{
		CTakeDamageInfo info(this, this, sk_bullsquid_dmg_spit.GetFloat(), DMG_BULLET);
		CalculateBulletDamageForce(&info, GetAmmoDef()->Index("9mmRound"), GetAbsVelocity(), GetAbsOrigin());
		pOther->TakeDamage(info);
	}

	UTIL_Remove(m_hSprite);
	UTIL_Remove(this);
}

#define		BSQUID_AE_SPIT		( 1 )
#define		BSQUID_AE_BITE		( 2 )
#define		BSQUID_AE_BLINK		( 3 )
#define		BSQUID_AE_TAILWHIP	( 4 )
#define		BSQUID_AE_HOP		( 5 )
#define		BSQUID_AE_THROW		( 6 )

int ACT_SQUID_EXCITED;
int ACT_SQUID_EAT;
int ACT_SQUID_DETECT_SCENT;
int ACT_SQUID_INSPECT_FLOOR;

class CHL1Bullsquid : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Bullsquid, CHL1BaseMonster);

public:
	void Spawn(void);
	void Precache(void);
	Class_T	Classify(void);

	void IdleSound(void);
	void PainSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void DeathSound(const CTakeDamageInfo &info);
	void AttackSound(void);

	float MaxYawSpeed(void);

	void HandleAnimEvent(animevent_t *pEvent);

	int RangeAttack1Conditions(float flDot, float flDist);
	int MeleeAttack1Conditions(float flDot, float flDist);
	int MeleeAttack2Conditions(float flDot, float flDist);

	bool FValidateHintType(CAI_Hint *pHint);
	void RemoveIgnoredConditions(void);
	Disposition_t IRelationType(CBaseEntity *pTarget);
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

	int GetSoundInterests(void);
	void RunAI(void);
	virtual void OnListened(void);

	int SelectSchedule(void);
	int TranslateSchedule(int scheduleType);

	bool FInViewCone(Vector pOrigin);
	bool FVisible(Vector vecOrigin);

	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);

	NPC_STATE SelectIdealState(void);

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC()

private:

	bool  m_fCanThreatDisplay; 
	float m_flLastHurtTime;
	float m_flNextSpitTime;
	float m_flHungryTime; 
};

LINK_ENTITY_TO_CLASS(monster_bullchicken, CHL1Bullsquid);
PRECACHE_REGISTER(monster_bullchicken);

BEGIN_DATADESC(CHL1Bullsquid)
DEFINE_FIELD(m_fCanThreatDisplay, FIELD_BOOLEAN),
DEFINE_FIELD(m_flLastHurtTime, FIELD_TIME),
DEFINE_FIELD(m_flNextSpitTime, FIELD_TIME),
DEFINE_FIELD(m_flHungryTime, FIELD_TIME),
END_DATADESC()

void CHL1Bullsquid::Spawn()
{
	Precache();

	SetModel("models/bullsquid.mdl");
	SetHullType(HULL_WIDE_SHORT);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_GREEN;

	SetRenderColor(255, 255, 255, 255);

	m_iHealth = sk_bullsquid_health.GetFloat();
	m_flFieldOfView = 0.2;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2);

	m_fCanThreatDisplay = TRUE;
	m_flNextSpitTime = gpGlobals->curtime;

	NPCInit();

	m_flDistTooFar = 784;
}

void CHL1Bullsquid::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/bullsquid.mdl");

	PrecacheModel("sprites/bigspit.vmt");

	PrecacheScriptSound("Bullsquid.Idle");
	PrecacheScriptSound("Bullsquid.Pain");
	PrecacheScriptSound("Bullsquid.Alert");
	PrecacheScriptSound("Bullsquid.Die");
	PrecacheScriptSound("Bullsquid.Attack");
	PrecacheScriptSound("Bullsquid.Bite");
	PrecacheScriptSound("Bullsquid.Growl");
}

int CHL1Bullsquid::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_CHASE_ENEMY:
		return SCHED_SQUID_CHASE_ENEMY;
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

Class_T	CHL1Bullsquid::Classify(void)
{
	return CLASS_ALIEN_PREDATOR;
}

void CHL1Bullsquid::IdleSound(void)
{
	CPASAttenuationFilter filter(this, 1.5f);
	EmitSound(filter, entindex(), "Bullsquid.Idle");
}

void CHL1Bullsquid::PainSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Bullsquid.Pain");
}


void CHL1Bullsquid::AlertSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Bullsquid.Alert");
}

void CHL1Bullsquid::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Bullsquid.Die");
}

void CHL1Bullsquid::AttackSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Bullsquid.Attack");
}

float CHL1Bullsquid::MaxYawSpeed(void)
{
	float flYS = 0;

	switch (GetActivity())
	{
	case	ACT_WALK:			flYS = 90;	break;
	case	ACT_RUN:			flYS = 90;	break;
	case	ACT_IDLE:			flYS = 90;	break;
	case	ACT_RANGE_ATTACK1:	flYS = 90;	break;
	default:
		flYS = 90;
		break;
	}

	return flYS;
}

void CHL1Bullsquid::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case BSQUID_AE_SPIT:
	{
		if (GetEnemy())
		{
			Vector	vecSpitOffset;
			Vector	vecSpitDir;
			Vector  vRight, vUp, vForward;

			AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);

			vecSpitOffset = (vRight * 8 + vForward * 60 + vUp * 50);
			vecSpitOffset = (GetAbsOrigin() + vecSpitOffset);
			vecSpitDir = ((GetEnemy()->BodyTarget(GetAbsOrigin())) - vecSpitOffset);

			VectorNormalize(vecSpitDir);

			vecSpitDir.x += random->RandomFloat(-0.05, 0.05);
			vecSpitDir.y += random->RandomFloat(-0.05, 0.05);
			vecSpitDir.z += random->RandomFloat(-0.05, 0);

			AttackSound();

			CSquidSpit::Shoot(this, vecSpitOffset, vecSpitDir * 900);
		}
	}
	break;

	case BSQUID_AE_BITE:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), sk_bullsquid_dmg_bite.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			Vector forward, up;
			AngleVectors(GetAbsAngles(), &forward, NULL, &up);
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - (forward * 100));
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (up * 100));
			pHurt->SetGroundEntity(NULL);
		}
	}
	break;

	case BSQUID_AE_TAILWHIP:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), sk_bullsquid_dmg_whip.GetFloat(), DMG_SLASH | DMG_ALWAYSGIB);
		if (pHurt)
		{
			Vector right, up;
			AngleVectors(GetAbsAngles(), NULL, &right, &up);

			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
				pHurt->ViewPunch(QAngle(20, 0, -20));

			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (right * 200));
			pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (up * 100));
		}
	}
	break;

	case BSQUID_AE_BLINK:
	{
		m_nSkin = 1;
	}
	break;

	case BSQUID_AE_HOP:
	{
		float flGravity = sv_gravity.GetFloat();

		if (GetFlags() & FL_ONGROUND)
		{
			SetGroundEntity(NULL);
		}

		Vector vecVel = GetAbsVelocity();
		vecVel.z += (0.625 * flGravity) * 0.5;
		SetAbsVelocity(vecVel);
	}
	break;

	case BSQUID_AE_THROW:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), 0, 0);


		if (pHurt)
		{
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Bullsquid.Bite");

			UTIL_ScreenShake(pHurt->GetAbsOrigin(), 25.0, 1.5, 0.7, 2, SHAKE_START);

			if (pHurt->IsPlayer())
			{
				Vector forward, up;
				AngleVectors(GetAbsAngles(), &forward, NULL, &up);

				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + forward * 300 + up * 300);
			}
		}
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
	}
}

int CHL1Bullsquid::RangeAttack1Conditions(float flDot, float flDist)
{
	if (IsMoving() && flDist >= 512)
	{
		return (COND_NONE);
	}

	if (flDist > 64 && flDist <= 784 && flDot >= 0.5 && gpGlobals->curtime >= m_flNextSpitTime)
	{
		if (GetEnemy() != NULL)
		{
			if (fabs(GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z) > 256)
			{
				return(COND_NONE);
			}
		}

		if (IsMoving())
		{
			m_flNextSpitTime = gpGlobals->curtime + 5;
		}
		else
		{
			m_flNextSpitTime = gpGlobals->curtime + 0.5;
		}
		
		return(COND_CAN_RANGE_ATTACK1);
	}

	return(COND_NONE);
}

int CHL1Bullsquid::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (GetEnemy()->m_iHealth <= sk_bullsquid_dmg_whip.GetFloat() && flDist <= 85 && flDot >= 0.7)
	{
		return (COND_CAN_MELEE_ATTACK1);
	}

	return(COND_NONE);
}

int CHL1Bullsquid::MeleeAttack2Conditions(float flDot, float flDist)
{
	if (flDist <= 85 && flDot >= 0.7 && !HasCondition(COND_CAN_MELEE_ATTACK1))
		return (COND_CAN_MELEE_ATTACK2);

	return(COND_NONE);
}

bool CHL1Bullsquid::FValidateHintType(CAI_Hint *pHint)
{
	if (pHint->HintType() == HINT_HL1_WORLD_HUMAN_BLOOD)
		return true;

	Msg("Couldn't validate hint type");

	return false;
}

void CHL1Bullsquid::RemoveIgnoredConditions(void)
{
	if (m_flHungryTime > gpGlobals->curtime)
		ClearCondition(COND_SQUID_SMELL_FOOD);

	if (gpGlobals->curtime - m_flLastHurtTime <= 20)
	{
		ClearCondition(COND_SMELL);
	}

	if (GetEnemy() != NULL)
	{
		if (FClassnameIs(GetEnemy(), "monster_headcrab"))
			ClearCondition(COND_SMELL);
	}
}

Disposition_t CHL1Bullsquid::IRelationType(CBaseEntity *pTarget)
{
	if (gpGlobals->curtime - m_flLastHurtTime < 5 && FClassnameIs(pTarget, "monster_headcrab"))
	{
		return D_NU;
	}

	return BaseClass::IRelationType(pTarget);
}

int CHL1Bullsquid::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	if (!FClassnameIs(inputInfo.GetAttacker(), "monster_headcrab"))
	{
		m_flLastHurtTime = gpGlobals->curtime;
	}

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

int CHL1Bullsquid::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_CARCASS |
		SOUND_MEAT |
		SOUND_GARBAGE |
		SOUND_PLAYER;
}

void CHL1Bullsquid::OnListened(void)
{
	AISoundIter_t iter;

	CSound *pCurrentSound;

	static int conditionsToClear[] =
	{
		COND_SQUID_SMELL_FOOD,
	};

	ClearConditions(conditionsToClear, ARRAYSIZE(conditionsToClear));

	pCurrentSound = GetSenses()->GetFirstHeardSound(&iter);

	while (pCurrentSound)
	{
		int condition = COND_NONE;

		if (!pCurrentSound->FIsSound())
		{
			if (pCurrentSound->IsSoundType(SOUND_MEAT | SOUND_CARCASS))
			{
				condition = COND_SQUID_SMELL_FOOD;
			}
		}

		if (condition != COND_NONE)
			SetCondition(condition);

		pCurrentSound = GetSenses()->GetNextHeardSound(&iter);
	}

	BaseClass::OnListened();
}

void CHL1Bullsquid::RunAI(void)
{
	BaseClass::RunAI();

	if (m_nSkin != 0)
	{
		m_nSkin = 0;
	}

	if (random->RandomInt(0, 39) == 0)
	{
		m_nSkin = 1;
	}

	if (GetEnemy() != NULL && GetActivity() == ACT_RUN)
	{
		if ((GetAbsOrigin() - GetEnemy()->GetAbsOrigin()).Length2D() < SQUID_SPRINT_DIST)
		{
			m_flPlaybackRate = 1.25;
		}
	}

}

int CHL1Bullsquid::SelectSchedule(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_ALERT:
	{
		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			return SCHED_SQUID_HURTHOP;
		}

		if (HasCondition(COND_SQUID_SMELL_FOOD))
		{
			CSound		*pSound;

			pSound = GetBestScent();

			if (pSound && (!FInViewCone(pSound->GetSoundOrigin()) || !FVisible(pSound->GetSoundOrigin())))
			{
				return SCHED_SQUID_SNIFF_AND_EAT;
			}

			return SCHED_SQUID_EAT;
		}

		if (HasCondition(COND_SMELL))
		{
			CSound		*pSound;

			pSound = GetBestScent();
			if (pSound)
				return SCHED_SQUID_WALLOW;
		}

		break;
	}
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
		{
			return BaseClass::SelectSchedule();
		}

		if (HasCondition(COND_NEW_ENEMY))
		{
			if (m_fCanThreatDisplay && IRelationType(GetEnemy()) == D_HT && FClassnameIs(GetEnemy(), "monster_headcrab"))
			{
				m_fCanThreatDisplay = FALSE;
				return SCHED_SQUID_SEECRAB;
			}
			else
			{
				return SCHED_WAKE_ANGRY;
			}
		}

		if (HasCondition(COND_SQUID_SMELL_FOOD))
		{
			CSound		*pSound;

			pSound = GetBestScent();

			if (pSound && (!FInViewCone(pSound->GetSoundOrigin()) || !FVisible(pSound->GetSoundOrigin())))
			{
				return SCHED_SQUID_SNIFF_AND_EAT;
			}

			return SCHED_SQUID_EAT;
		}

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			return SCHED_RANGE_ATTACK1;
		}

		if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			return SCHED_MELEE_ATTACK1;
		}

		if (HasCondition(COND_CAN_MELEE_ATTACK2))
		{
			return SCHED_MELEE_ATTACK2;
		}

		//For some reason, removing this part fixed the bullsquid's tendency to freak out when there is cover between him and the player
		//without stopping him from pursuing enemies.
		//return SCHED_CHASE_ENEMY;

		break;
	}
	}

	return BaseClass::SelectSchedule();
}

bool CHL1Bullsquid::FInViewCone(Vector pOrigin)
{
	Vector los = (pOrigin - GetAbsOrigin());

	los.z = 0;
	VectorNormalize(los);

	Vector facingDir = EyeDirection2D();

	float flDot = DotProduct(los, facingDir);

	if (flDot > m_flFieldOfView)
		return true;

	return false;
}

bool CHL1Bullsquid::FVisible(Vector vecOrigin)
{
	trace_t tr;
	Vector		vecLookerOrigin;

	vecLookerOrigin = EyePosition();
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, MASK_BLOCKLOS, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0)
		return false;
	else
		return true;
}

void CHL1Bullsquid::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_MELEE_ATTACK2:
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Bullsquid.Growl");
		BaseClass::StartTask(pTask);
		break;
	}
	case TASK_SQUID_HOPTURN:
	{
		SetActivity(ACT_HOP);

		if (GetEnemy())
		{
			Vector	vecFacing = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
			VectorNormalize(vecFacing);

			GetMotor()->SetIdealYaw(vecFacing);
		}

		break;
	}
	case TASK_SQUID_EAT:
	{
		m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
		break;
	}

	default:
	{
		BaseClass::StartTask(pTask);
		break;
	}
	}
}

void CHL1Bullsquid::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SQUID_HOPTURN:
	{
		if (GetEnemy())
		{
			Vector	vecFacing = (GetEnemy()->GetAbsOrigin() - GetAbsOrigin());
			VectorNormalize(vecFacing);
			GetMotor()->SetIdealYaw(vecFacing);
		}

		if (IsSequenceFinished())
		{
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

NPC_STATE CHL1Bullsquid::SelectIdealState(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		if (GetEnemy() != NULL && (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE)) && FClassnameIs(GetEnemy(), "monster_headcrab"))
		{
			SetEnemy(NULL);
			return NPC_STATE_ALERT;
		}
		break;
	}
	}

	return BaseClass::SelectIdealState();
}

AI_BEGIN_CUSTOM_NPC(monster_bullchicken, CHL1Bullsquid)

DECLARE_TASK(TASK_SQUID_HOPTURN)
DECLARE_TASK(TASK_SQUID_EAT)

DECLARE_CONDITION(COND_SQUID_SMELL_FOOD)

DECLARE_ACTIVITY(ACT_SQUID_EXCITED)
DECLARE_ACTIVITY(ACT_SQUID_EAT)
DECLARE_ACTIVITY(ACT_SQUID_DETECT_SCENT)
DECLARE_ACTIVITY(ACT_SQUID_INSPECT_FLOOR)

DEFINE_SCHEDULE
(
SCHED_SQUID_HURTHOP,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SOUND_WAKE				0"
"		TASK_SQUID_HOPTURN			0"
"		TASK_FACE_ENEMY				0"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_SQUID_SEECRAB,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SOUND_WAKE				0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_SQUID_EXCITED"
"		TASK_FACE_ENEMY				0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
SCHED_SQUID_EAT,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_SQUID_EAT						10"
"		TASK_STORE_LASTPOSITION				0"
"		TASK_GET_PATH_TO_BESTSCENT			0"
"		TASK_WALK_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_SQUID_EAT						50"
"		TASK_GET_PATH_TO_LASTPOSITION		0"
"		TASK_WALK_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_CLEAR_LASTPOSITION				0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_NEW_ENEMY"
"		COND_SMELL"
)

DEFINE_SCHEDULE
(
SCHED_SQUID_SNIFF_AND_EAT,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_SQUID_EAT						10"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_DETECT_SCENT"
"		TASK_STORE_LASTPOSITION				0"
"		TASK_GET_PATH_TO_BESTSCENT			0"
"		TASK_WALK_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_SQUID_EAT"
"		TASK_SQUID_EAT						50"
"		TASK_GET_PATH_TO_LASTPOSITION		0"
"		TASK_WALK_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_CLEAR_LASTPOSITION				0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_NEW_ENEMY"
"		COND_SMELL"
)

DEFINE_SCHEDULE
(
SCHED_SQUID_WALLOW,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_SQUID_EAT					10"
"		TASK_STORE_LASTPOSITION			0"
"		TASK_GET_PATH_TO_BESTSCENT		0"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_SQUID_INSPECT_FLOOR"
"		TASK_SQUID_EAT					50"
"		TASK_GET_PATH_TO_LASTPOSITION	0"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_CLEAR_LASTPOSITION			0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
SCHED_SQUID_CHASE_ENEMY,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_RANGE_ATTACK1"
"		TASK_GET_PATH_TO_ENEMY			0"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"	"
"	Interrupts"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_SMELL"
"		COND_CAN_RANGE_ATTACK1"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_MELEE_ATTACK2"
"		COND_TASK_FAILED"
)

AI_END_CUSTOM_NPC()