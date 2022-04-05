#include "cbase.h"
#include "beam_shared.h"
#include "game.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_route.h"
#include "ai_squad.h"
#include "npcevent.h"
#include "gib.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "hl1_monster_squad.h"
#include "soundent.h"
#include "player.h"
#include "IEffects.h"
#include "basecombatweapon.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

ConVar sk_islave_health("sk_islave_health", "30");
ConVar sk_islave_dmg_claw("sk_islave_dmg_claw", "10");
ConVar sk_islave_dmg_clawrake("sk_islave_dmg_clawrake", "25");
ConVar sk_islave_dmg_zap("sk_islave_dmg_zap", "10");

#define		ISLAVE_AE_CLAW		( 1 )
#define		ISLAVE_AE_CLAWRAKE	( 2 )
#define		ISLAVE_AE_ZAP_POWERUP	( 3 )
#define		ISLAVE_AE_ZAP_SHOOT		( 4 )
#define		ISLAVE_AE_ZAP_DONE		( 5 )

#define		ISLAVE_MAX_BEAMS	8

#define VORTIGAUNT_IGNORE_PLAYER 64

enum
{
	SCHED_VORTIGAUNT_ATTACK = LAST_SHARED_SCHEDULE,
};

class CHL1ISlave : public CHL1SquadMonster
{
public:
	DECLARE_CLASS(CHL1ISlave, CHL1SquadMonster);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void);
	Class_T  Classify(void);
	virtual Disposition_t  IRelationType(CBaseEntity *pTarget);
	void HandleAnimEvent(animevent_t *pEvent);
	int RangeAttack1Conditions(float flDot, float flDist);
	void CallForHelp(char *szClassname, float flDist, CBaseEntity * pEnemy, Vector &vecLocation);
	void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	int OnTakeDamage_Alive(const CTakeDamageInfo &info);

	void DeathSound(const CTakeDamageInfo &info);
	void PainSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void IdleSound(void);

	void Event_Killed(const CTakeDamageInfo &info);

	void StartTask(const Task_t *pTask);
	int SelectSchedule(void);
	int TranslateSchedule(int scheduleType);

	void ClearBeams();
	void ArmBeam(int side);
	void WackBeam(int side, CBaseEntity *pEntity);
	void ZapBeam(int side);
	void BeamGlow(void);

private:
	int m_iBravery;

	CBeam *m_pBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int	m_voicePitch;

	EHANDLE m_hDead;
};

LINK_ENTITY_TO_CLASS(monster_alien_slave, CHL1ISlave);
PRECACHE_REGISTER(monster_alien_slave);

BEGIN_DATADESC(CHL1ISlave)
DEFINE_FIELD(m_iBravery, FIELD_INTEGER),
DEFINE_ARRAY(m_pBeam, FIELD_CLASSPTR, ISLAVE_MAX_BEAMS),
DEFINE_FIELD(m_iBeams, FIELD_INTEGER),
DEFINE_FIELD(m_flNextAttack, FIELD_TIME),
DEFINE_FIELD(m_voicePitch, FIELD_INTEGER),
DEFINE_FIELD(m_hDead, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_SquadName, FIELD_STRING, "netname"),
END_DATADESC();

Class_T	CHL1ISlave::Classify(void)
{
	return	CLASS_ALIEN_MILITARY;
}

Disposition_t CHL1ISlave::IRelationType(CBaseEntity *pTarget)
{
	if ((pTarget->IsPlayer()))
		if ((GetSpawnFlags() & VORTIGAUNT_IGNORE_PLAYER) && !(m_afMemory & bits_MEMORY_PROVOKED))
			return D_NU;
	return BaseClass::IRelationType(pTarget);
}

void CHL1ISlave::CallForHelp(char *szClassname, float flDist, CBaseEntity * pEnemy, Vector &vecLocation)
{
	if (m_SquadName == NULL_STRING)
		return;

	CBaseEntity* pEntity = NULL;
	pEntity = gEntList.FindEntityByClassname(pEntity, "monster_alien_slave");

	while (pEntity)
	{
		CHL1ISlave* pSlave = (CHL1ISlave*)pEntity;
		if (pSlave->m_SquadName == m_SquadName)
		{
			float d = (GetAbsOrigin() - pEntity->GetAbsOrigin()).Length();
			if (d < flDist)
			{
				CAI_BaseNPC* pMonster = pEntity->MyNPCPointer();
				if (pMonster)
				{
					pMonster->Remember(bits_MEMORY_PROVOKED);
					pMonster->UpdateEnemyMemory(pEnemy, vecLocation);
				}
			}
		}
		pEntity = gEntList.FindEntityByClassname(pEntity, "monster_alien_slave");
	}
}

void CHL1ISlave::AlertSound(void)
{
	if (GetEnemy() != NULL)
	{
		SENTENCEG_PlayRndSz(edict(), "SLV_ALERT", 0.85, SNDLVL_NORM, 0, m_voicePitch);

		Vector vecOrig = GetEnemy()->GetAbsOrigin();
		CallForHelp("monster_alien_slave", 512, GetEnemy(), vecOrig);
	}
}

void CHL1ISlave::IdleSound(void)
{
	if (random->RandomInt(0, 2) == 0)
	{
		SENTENCEG_PlayRndSz(edict(), "SLV_IDLE", 0.85, SNDLVL_NORM, 0, m_voicePitch);
	}
}

void CHL1ISlave::PainSound(const CTakeDamageInfo &info)
{
	if (random->RandomInt(0, 2) == 0)
	{
		CPASAttenuationFilter filter(this);

		CSoundParameters params;
		if (GetParametersForSound("Vortigaunt.Pain", params, NULL))
		{
			EmitSound_t ep(params);
			params.pitch = m_voicePitch;

			EmitSound(filter, entindex(), ep);
		}
	}
}

void CHL1ISlave::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	CSoundParameters params;
	if (GetParametersForSound("Vortigaunt.Die", params, NULL))
	{
		EmitSound_t ep(params);
		params.pitch = m_voicePitch;

		EmitSound(filter, entindex(), ep);
	}
}

void CHL1ISlave::Event_Killed(const CTakeDamageInfo &info)
{
	ClearBeams();
	BaseClass::Event_Killed(info);
}

float CHL1ISlave::MaxYawSpeed(void)
{
	int ys;

	switch (GetActivity())
	{
	case ACT_WALK:
		ys = 50;
		break;
	case ACT_RUN:
		ys = 70;
		break;
	case ACT_IDLE:
		ys = 50;
		break;
	default:
		ys = 90;
		break;
	}

	return ys;
}

void CHL1ISlave::HandleAnimEvent(animevent_t *pEvent)
{
	CPASAttenuationFilter filter(this);
	CSoundParameters params;
	switch (pEvent->event)
	{
	case ISLAVE_AE_CLAW:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(40, Vector(-10, -10, -10), Vector(10, 10, 10), sk_islave_dmg_claw.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, -18));
			}

			if (GetParametersForSound("Vortigaunt.AttackHit", params, NULL))
			{
				EmitSound_t ep(params);
				params.pitch = m_voicePitch;

				EmitSound(filter, entindex(), ep);
			}
		}
		else
		{
			if (GetParametersForSound("Vortigaunt.AttackMiss", params, NULL))
			{
				EmitSound_t ep(params);
				params.pitch = m_voicePitch;

				EmitSound(filter, entindex(), ep);
			}
		}
	}
	break;

	case ISLAVE_AE_CLAWRAKE:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(40, Vector(-10, -10, -10), Vector(10, 10, 10), sk_islave_dmg_clawrake.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, 18));
			}
			if (GetParametersForSound("Vortigaunt.AttackHit", params, NULL))
			{
				EmitSound_t ep(params);
				params.pitch = m_voicePitch;

				EmitSound(filter, entindex(), ep);
			}
		}
		else
		{
			if (GetParametersForSound("Vortigaunt.AttackMiss", params, NULL))
			{
				EmitSound_t ep(params);
				params.pitch = m_voicePitch;

				EmitSound(filter, entindex(), ep);
			}
		}
	}
	break;

	case ISLAVE_AE_ZAP_POWERUP:
	{
		if (g_iSkillLevel == SKILL_HARD)
			m_flPlaybackRate = 1.5;

		Vector v_forward;
		GetVectors(&v_forward, NULL, NULL);

		CBroadcastRecipientFilter filter;
		te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 125, 200, 100, 2, 120, 0.2 / m_flPlaybackRate, 0);

		if (m_hDead != NULL)
		{
			WackBeam(-1, m_hDead);
			WackBeam(1, m_hDead);
		}
		else
		{
			ArmBeam(-1);
			ArmBeam(1);
			BeamGlow();
		}

		if (GetParametersForSound("Vortigaunt.ZapPowerup", params, NULL))
		{
			EmitSound_t ep(params);
			ep.m_nPitch = 100 + m_iBeams * 10;
			EmitSound(filter, entindex(), ep);
		}
	}
	break;

	case ISLAVE_AE_ZAP_SHOOT:
	{
		ClearBeams();

		if (m_hDead != NULL)
		{
			Vector vecDest = m_hDead->GetAbsOrigin() + Vector(0, 0, 38);
			trace_t trace;
			UTIL_TraceHull(vecDest, vecDest, GetHullMins(), GetHullMaxs(), MASK_SOLID, m_hDead, COLLISION_GROUP_NONE, &trace);

			if (!trace.startsolid)
			{
				CBaseEntity *pNew = Create("monster_alien_slave", m_hDead->GetAbsOrigin(), m_hDead->GetAbsAngles());

				pNew->AddSpawnFlags(1);
				WackBeam(-1, pNew);
				WackBeam(1, pNew);
				UTIL_Remove(m_hDead);
			break;
			}
		}
		ClearMultiDamage();
		
		ZapBeam(-1);
		ZapBeam(1);

		EmitSound(filter, entindex(), "Vortigaunt.ZapShoot");

		ApplyMultiDamage();

		m_flNextAttack = gpGlobals->curtime + random->RandomFloat(0.5, 4.0);
	}
	break;

	case ISLAVE_AE_ZAP_DONE:
	{
		ClearBeams();
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

int CHL1ISlave::RangeAttack1Conditions(float flDot, float flDist)
{
	if (GetEnemy() == NULL)
		return(COND_LOST_ENEMY);

	if (gpGlobals->curtime < m_flNextAttack)
		return COND_NONE;

	if (HasCondition(COND_CAN_MELEE_ATTACK1))
		return COND_NONE;

	return COND_CAN_RANGE_ATTACK1;
}

void CHL1ISlave::StartTask(const Task_t *pTask)
{
	ClearBeams();

	BaseClass::StartTask(pTask);
}

void CHL1ISlave::Spawn()
{
	Precache();

	SetModel("models/islave.mdl");
	SetRenderColor(255, 255, 255, 255);
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_GREEN;
	ClearEffects();
	m_iHealth = sk_islave_health.GetFloat();

	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_NPCState = NPC_STATE_NONE;
	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_SQUAD);

	CapabilitiesAdd(bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP);

	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	m_voicePitch = random->RandomInt(85, 110);

	m_iBravery = 0;


	NPCInit();

	BaseClass::Spawn();
}

void CHL1ISlave::Precache()
{
	PrecacheModel("models/islave.mdl");
	PrecacheModel("sprites/lgtning.vmt");

	PrecacheScriptSound("Vortigaunt.Pain");
	PrecacheScriptSound("Vortigaunt.Die");
	PrecacheScriptSound("Vortigaunt.AttackHit");
	PrecacheScriptSound("Vortigaunt.AttackMiss");
	PrecacheScriptSound("Vortigaunt.ZapPowerup");
	PrecacheScriptSound("Vortigaunt.ZapShoot");

	BaseClass::Precache();
}

int CHL1ISlave::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	if ((inputInfo.GetDamageType() & DMG_SLASH) && inputInfo.GetAttacker() && IRelationType(inputInfo.GetAttacker()) == D_NU)
		return 0;

	Remember(bits_MEMORY_PROVOKED);

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}


void CHL1ISlave::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
	if (info.GetDamageType() & DMG_SHOCK)
		return;

	BaseClass::TraceAttack(info, vecDir, ptr);
}

int CHL1ISlave::SelectSchedule(void)
{
	ClearBeams();

	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
		if (HasCondition(COND_ENEMY_DEAD))
		{
			return BaseClass::SelectSchedule();
		}

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_RANGE_ATTACK1;

		if (m_iHealth < 20 || m_iBravery < 0)
		{
			if (!HasCondition(COND_CAN_MELEE_ATTACK1))
			{
				SetDefaultFailSchedule(SCHED_CHASE_ENEMY);
				if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
					return SCHED_TAKE_COVER_FROM_ENEMY;
				if (HasCondition(COND_SEE_ENEMY) && HasCondition(COND_ENEMY_FACING_ME))
					return SCHED_TAKE_COVER_FROM_ENEMY;
			}
		}
		break;
	}
	return BaseClass::SelectSchedule();
}

int CHL1ISlave::TranslateSchedule(int scheduleType)
{
	if (scheduleType == SCHED_CHASE_ENEMY_FAILED)
	{
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	switch (scheduleType)
	{
	case SCHED_FAIL:

		if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			return (SCHED_MELEE_ATTACK1);
		}

		break;

	case SCHED_RANGE_ATTACK1:
	{
		if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			return (SCHED_MELEE_ATTACK1);
		}

		return SCHED_VORTIGAUNT_ATTACK;
	}

	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

void CHL1ISlave::ArmBeam(int side)
{
	trace_t tr;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	Vector forward, right, up;
	Vector vecAim;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);
	Vector vecSrc = GetAbsOrigin() + up * 36 + right * side * 16 + forward * 32;

	for (int i = 0; i < 3; i++)
	{
		vecAim = right * side * random->RandomFloat(0, 1) + up * random->RandomFloat(-1, 1);
		trace_t tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr1);
		if (flDist > tr1.fraction)
		{
			tr = tr1;
			flDist = tr.fraction;
		}
	}

	if (flDist == 1.0)
		return;

	UTIL_DecalTrace(&tr, "FadingScorch");

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.vmt", 3.0f);

	if (m_pBeam[m_iBeams] == NULL)
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.endpos, this);
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(96, 128, 16);
	m_pBeam[m_iBeams]->SetBrightness(64);
	m_pBeam[m_iBeams]->SetNoise(12.8);
	m_pBeam[m_iBeams]->AddSpawnFlags(SF_BEAM_TEMPORARY);
	m_iBeams++;
}

void CHL1ISlave::BeamGlow()
{
	int b = m_iBeams * 32;
	if (b > 255)
		b = 255;

	for (int i = 0; i < m_iBeams; i++)
	{
		if (m_pBeam[i]->GetBrightness() != 255)
		{
			m_pBeam[i]->SetBrightness(b);
		}
	}
}

void CHL1ISlave::WackBeam(int side, CBaseEntity *pEntity)
{
	Vector vecDest;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	if (pEntity == NULL)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.vmt", 3.0f);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(pEntity->WorldSpaceCenter(), this);
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(180, 255, 96);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(12.8);
	m_pBeam[m_iBeams]->AddSpawnFlags(SF_BEAM_TEMPORARY);
	m_iBeams++;
}

void CHL1ISlave::ZapBeam(int side)
{
	Vector vecSrc, vecAim;
	trace_t tr;
	CBaseEntity *pEntity;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	vecSrc = GetAbsOrigin() + up * 36;
	vecAim = GetShootEnemyDir(vecSrc);

	float deflection = 0.01;
	vecAim = vecAim + side * right * random->RandomFloat(0, deflection) + up * random->RandomFloat(-deflection, deflection);
	UTIL_TraceLine ( vecSrc, vecSrc + vecAim * 1024, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.vmt", 5.0f);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.endpos, this);
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(180, 255, 96);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(3.2f);
	m_pBeam[m_iBeams]->AddSpawnFlags(SF_BEAM_TEMPORARY);
	m_iBeams++;
	
	pEntity = tr.m_pEnt;

	if (pEntity != NULL && m_takedamage)
	{
		CTakeDamageInfo info(this, this, sk_islave_dmg_zap.GetFloat(), DMG_SHOCK);
		CalculateMeleeDamageForce(&info, vecAim, tr.endpos);
		pEntity->DispatchTraceAttack(info, vecAim, &tr);
	}
}

void CHL1ISlave::ClearBeams()
{
	for (int i = 0; i < ISLAVE_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove(m_pBeam[i]);
			m_pBeam[i] = NULL;
		}
	}
	m_iBeams = 0;
	m_nSkin = 0;
}

AI_BEGIN_CUSTOM_NPC(monster_alien_slave, CHL1ISlave)

DEFINE_SCHEDULE
(
SCHED_VORTIGAUNT_ATTACK,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_IDEAL				0"
"		TASK_RANGE_ATTACK1			0"
"	"
"	Interrupts"
"		COND_CAN_MELEE_ATTACK1"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
)

AI_END_CUSTOM_NPC()