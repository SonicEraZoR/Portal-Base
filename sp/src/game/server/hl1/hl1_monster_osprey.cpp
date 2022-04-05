// Patch 2.1

#include	"cbase.h"
#include	"beam_shared.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"ai_senses.h"
#include	"hl1_monster_hgrunt.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"basecombatweapon.h"
#include	"hl1_basegrenade.h"
#include	"soundenvelope.h"
#include	"hl1_CBaseHelicopter.h"
#include	"IEffects.h"
#include	"smoke_trail.h"

extern short	g_sModelIndexFireball;

typedef struct
{
	int isValid;
	EHANDLE hGrunt;
	Vector	vecOrigin;
	Vector  vecAngles;
} t_ospreygrunt;

#define LOADED_WITH_GRUNTS 0
#define UNLOADING_GRUNTS 1
#define GRUNTS_DEPLOYED 2
#define SF_WAITFORTRIGGER	0x40
#define MAX_CARRY	24
#define DEFAULT_SPEED 250
#define HELICOPTER_THINK_INTERVAL		0.1

class CHL1Osprey : public CBaseHelicopter
{
	DECLARE_CLASS(CHL1Osprey, CBaseHelicopter);
public:

	int		ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void Spawn(void);
	void Precache(void);
	Class_T  Classify(void) { return CLASS_NONE; };
	int  BloodColor(void) { return DONT_BLEED; }

	void FindAllThink(void);
	void DeployThink(void);
	bool HasDead(void);
	void Flight(void);
	void HoverThink(void);
	CAI_BaseNPC* MakeGrunt(Vector vecSrc);

	void InitializeRotorSound(void);
	void PrescheduleThink(void);

	void DyingThink(void);

	void CrashTouch(CBaseEntity* pOther);

	void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);

	float m_startTime;

	float m_flIdealtilt;
	float m_flRotortilt;

	float m_flRightHealth;
	float m_flLeftHealth;

	int	m_iUnits;
	EHANDLE m_hGrunt[MAX_CARRY];
	Vector m_vecOrigin[MAX_CARRY];
	EHANDLE m_hRepel[4];

	int m_iSoundState;
	int m_iSpriteTexture;

	int m_iPitch;

	int m_iExplode;
	int	m_iTailGibs;
	int	m_iBodyGibs;
	int	m_iEngineGibs;

	int m_iDoLeftSmokePuff;
	int m_iDoRightSmokePuff;

	int m_iRepelState;
	float m_flPrevGoalVel;

	int m_iRotorAngle;
	int	m_nDebrisModel;

	Vector m_cullBoxMins;
	Vector m_cullBoxMaxs;

	CHandle<SmokeTrail>	m_hLeftSmoke;
	CHandle<SmokeTrail>	m_hRightSmoke;

	DECLARE_DATADESC();

	int m_iNextCrashModel;
};

LINK_ENTITY_TO_CLASS(monster_osprey, CHL1Osprey);

BEGIN_DATADESC(CHL1Osprey)
DEFINE_FIELD(m_startTime, FIELD_TIME),

DEFINE_FIELD(m_flIdealtilt, FIELD_FLOAT),
DEFINE_FIELD(m_flRotortilt, FIELD_FLOAT),

DEFINE_FIELD(m_flRightHealth, FIELD_FLOAT),
DEFINE_FIELD(m_flLeftHealth, FIELD_FLOAT),

DEFINE_FIELD(m_iRepelState, FIELD_INTEGER),

DEFINE_FIELD(m_iUnits, FIELD_INTEGER),
DEFINE_ARRAY(m_hGrunt, FIELD_EHANDLE, MAX_CARRY),
DEFINE_ARRAY(m_vecOrigin, FIELD_POSITION_VECTOR, MAX_CARRY),
DEFINE_ARRAY(m_hRepel, FIELD_EHANDLE, 4),

DEFINE_FIELD(m_flPrevGoalVel, FIELD_FLOAT),
DEFINE_FIELD(m_iRotorAngle, FIELD_INTEGER),

DEFINE_FIELD(m_hLeftSmoke, FIELD_EHANDLE),
DEFINE_FIELD(m_hRightSmoke, FIELD_EHANDLE),

DEFINE_FIELD(m_iNextCrashModel, FIELD_INTEGER),

DEFINE_FIELD(m_iDoLeftSmokePuff, FIELD_INTEGER),
DEFINE_FIELD(m_iDoRightSmokePuff, FIELD_INTEGER),

DEFINE_THINKFUNC(FindAllThink),
DEFINE_THINKFUNC(DeployThink),

DEFINE_ENTITYFUNC(CrashTouch),

END_DATADESC()


	void CHL1Osprey::Spawn(void)
{
	Precache();

	SetModel("models/osprey.mdl");

	BaseClass::Spawn();

	Vector mins, maxs;
	ExtractBbox(0, mins, maxs);
	UTIL_SetSize(this, mins, maxs);
	UTIL_SetOrigin(this, GetAbsOrigin());

	AddFlag(FL_NPC);
	m_takedamage = DAMAGE_YES;
	m_flRightHealth = 200;
	m_flLeftHealth = 200;
	m_iHealth = 400;

	m_flFieldOfView = 0;

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	SetSequence(0);
	ResetSequenceInfo();
	SetCycle(random->RandomInt(0, 0xFF));

	m_startTime = gpGlobals->curtime + 1;

	m_flMaxSpeed = (float)BASECHOPPER_MAX_SPEED / 2;

	m_iRepelState = LOADED_WITH_GRUNTS;
	m_flPrevGoalVel = 9999;

	m_iRotorAngle = -1;
	SetBoneController(0, m_iRotorAngle);

	m_hLeftSmoke = NULL;
	m_hRightSmoke = NULL;
}


void CHL1Osprey::Precache(void)
{
	UTIL_PrecacheOther("monster_human_grunt");

	PrecacheModel("models/osprey.mdl");
	PrecacheModel("models/HVR.mdl");

	m_iSpriteTexture = PrecacheModel("sprites/rope.vmt");

	m_iExplode = PrecacheModel("sprites/fexplo.vmt");
	m_iTailGibs = PrecacheModel("models/osprey_tailgibs.mdl");
	m_iBodyGibs = PrecacheModel("models/osprey_bodygibs.mdl");
	m_iEngineGibs = PrecacheModel("models/osprey_enginegibs.mdl");

	m_nDebrisModel = PrecacheModel("models/mechgibs.mdl");

	PrecacheScriptSound("Apache.RotorSpinup");

	BaseClass::Precache();
}

void CHL1Osprey::TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr)
{
	float flDamage = info.GetDamage();

	if (ptr->hitgroup == 3)
	{
		if (m_flRightHealth <= 0)
			return;
		else
			m_flRightHealth -= flDamage;

		if (m_flRightHealth <= 0)
		{
			Assert(m_hRightSmoke == NULL);

			if ((m_hRightSmoke = SmokeTrail::CreateSmokeTrail()) != NULL)
			{
				m_hRightSmoke->m_Opacity = 1.0f;
				m_hRightSmoke->m_SpawnRate = 60;
				m_hRightSmoke->m_ParticleLifetime = 1.3f;
				m_hRightSmoke->m_StartColor.Init(0.65f, 0.65f, 0.65f);
				m_hRightSmoke->m_EndColor.Init(0.65f, 0.65f, 0.65f);
				m_hRightSmoke->m_StartSize = 12;
				m_hRightSmoke->m_EndSize = 80;
				m_hRightSmoke->m_SpawnRadius = 8;
				m_hRightSmoke->m_MinSpeed = 2;
				m_hRightSmoke->m_MaxSpeed = 24;

				m_hRightSmoke->SetLifetime(1e6);
				m_hRightSmoke->FollowEntity(this, "right");
			}
		}
	}

	if (ptr->hitgroup == 2)
	{
		if (m_flLeftHealth <= 0)
			return;
		else
			m_flLeftHealth -= flDamage;

		if (m_flLeftHealth <= 0)
		{
			Assert(m_hLeftSmoke == NULL);

			if ((m_hLeftSmoke = SmokeTrail::CreateSmokeTrail()) != NULL)
			{
				m_hLeftSmoke->m_Opacity = 1.0f;
				m_hLeftSmoke->m_SpawnRate = 60;
				m_hLeftSmoke->m_ParticleLifetime = 1.3f;
				m_hLeftSmoke->m_StartColor.Init(0.65f, 0.65f, 0.65f);
				m_hLeftSmoke->m_EndColor.Init(0.65f, 0.65f, 0.65f);
				m_hLeftSmoke->m_StartSize = 12;
				m_hLeftSmoke->m_EndSize = 64;
				m_hLeftSmoke->m_SpawnRadius = 8;
				m_hLeftSmoke->m_MinSpeed = 2;
				m_hLeftSmoke->m_MaxSpeed = 24;

				m_hLeftSmoke->SetLifetime(1e6);
				m_hLeftSmoke->FollowEntity(this, "left");
			}
		}
	}

	if (flDamage > 50 || ptr->hitgroup == 1 || ptr->hitgroup == 2 || ptr->hitgroup == 3)
	{
		AddMultiDamage(info, this);
	}
	else
	{
		g_pEffects->Sparks(ptr->endpos);
	}
}

void CHL1Osprey::InitializeRotorSound(void)
{
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);
	m_pRotorSound = controller.SoundCreate(filter, entindex(), CHAN_STATIC, "Apache.RotorSpinup", 0.2);

	BaseClass::InitializeRotorSound();
}

void CHL1Osprey::FindAllThink(void)
{
	CBaseEntity* pEntity = NULL;

	m_iUnits = 0;
	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "monster_human_grunt")) != NULL)
	{
		if (m_iUnits > MAX_CARRY)
			break;

		if (pEntity->IsAlive())
		{
			m_hGrunt[m_iUnits] = pEntity;
			m_vecOrigin[m_iUnits] = pEntity->GetAbsOrigin();
			m_iUnits++;
		}
	}

	if (m_iUnits == 0)
	{
		Msg("osprey error: no grunts to resupply\n");
		UTIL_Remove(this);
		return;
	}

	m_startTime = 0.0f;
}

void CHL1Osprey::DeployThink(void)
{
	Vector vecForward;
	Vector vecRight;
	Vector vecUp;
	Vector vecSrc;

	SetLocalAngularVelocity(QAngle(0, 0, 0));
	SetAbsVelocity(Vector(0, 0, 0));

	AngleVectors(GetAbsAngles(), &vecForward, &vecRight, &vecUp);

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -4096.0), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	CSoundEnt::InsertSound(SOUND_DANGER, tr.endpos, 400, 0.3);

	vecSrc = GetAbsOrigin() + vecForward * 32 + vecRight * 100 + vecUp * -96;
	m_hRepel[0] = MakeGrunt(vecSrc);

	vecSrc = GetAbsOrigin() + vecForward * -64 + vecRight * 100 + vecUp * -96;
	m_hRepel[1] = MakeGrunt(vecSrc);

	vecSrc = GetAbsOrigin() + vecForward * 32 + vecRight * -100 + vecUp * -96;
	m_hRepel[2] = MakeGrunt(vecSrc);

	vecSrc = GetAbsOrigin() + vecForward * -64 + vecRight * -100 + vecUp * -96;
	m_hRepel[3] = MakeGrunt(vecSrc);

	m_iRepelState = GRUNTS_DEPLOYED;

	HoverThink();
}

bool CHL1Osprey::HasDead()
{
	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive())
		{
			return TRUE;
		}
		else
		{
			m_vecOrigin[i] = m_hGrunt[i]->GetAbsOrigin();
		}
	}
	return FALSE;
}

void CHL1Osprey::HoverThink(void)
{
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		CBaseEntity* pRepel = (CBaseEntity*)m_hRepel[i];

		if (pRepel != NULL && pRepel->m_iHealth > 0 && pRepel->GetMoveType() == MOVETYPE_FLYGRAVITY)
		{
			break;
		}
	}

	if (i == 4)
		m_iRepelState = LOADED_WITH_GRUNTS;

	if (m_iRepelState != LOADED_WITH_GRUNTS)
	{
		m_iRotorAngle = UTIL_Approach(0, m_iRotorAngle, 5);
	}
}

CAI_BaseNPC* CHL1Osprey::MakeGrunt(Vector vecSrc)
{
	CBaseEntity* pEntity;
	CAI_BaseNPC* pGrunt;

	trace_t tr;
	UTIL_TraceLine(vecSrc, vecSrc + Vector(0, 0, -4096.0), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	if (tr.m_pEnt && tr.m_pEnt->GetSolid() != SOLID_BSP)
		return NULL;

	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive())
		{
			if (m_hGrunt[i] != NULL && m_hGrunt[i]->m_nRenderMode == kRenderNormal)
			{
				m_hGrunt[i]->SUB_StartFadeOut();
			}
			pEntity = Create("monster_human_grunt", vecSrc, GetAbsAngles());
			pGrunt = pEntity->MyNPCPointer();
			pGrunt->SetMoveType(MOVETYPE_FLYGRAVITY);
			pGrunt->SetGravity(0.0001);

			Vector spd = Vector(0, 0, random->RandomFloat(-196, -128));
			pGrunt->SetLocalVelocity(spd);
			pGrunt->SetActivity(ACT_GLIDE);
			pGrunt->SetGroundEntity(NULL);

			pGrunt->SetOwnerEntity(this);


			CBeam* pBeam = CBeam::BeamCreate("sprites/rope.vmt", 1.0);
			pBeam->PointEntInit(vecSrc + Vector(0, 0, 112), pGrunt);
			pBeam->SetBeamFlags(FBEAM_SOLID);
			pBeam->SetColor(255, 255, 255);
			pBeam->SetThink(&CBaseEntity::SUB_Remove);
			pBeam->SetNextThink(gpGlobals->curtime + -4096.0 * tr.fraction / pGrunt->GetAbsVelocity().z + 0.5);

			pGrunt->m_vecLastPosition = m_vecOrigin[i];
			m_hGrunt[i] = pGrunt;
			return pGrunt;
		}
	}

	return NULL;
}

void CHL1Osprey::Flight(void)
{
	if (m_iRepelState == LOADED_WITH_GRUNTS)
	{
		BaseClass::Flight();

		if (m_angleVelocity > 0)
		{
			m_iRotorAngle = UTIL_Approach(-45, m_iRotorAngle, 5 * (m_angleVelocity / 10));
		}
		else
		{
			m_iRotorAngle = UTIL_Approach(-1, m_iRotorAngle, 5);
		}
		SetBoneController(0, m_iRotorAngle);

	}
}

void CHL1Osprey::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	StudioFrameAdvance();

	if (m_startTime != 0.0 && m_startTime <= gpGlobals->curtime)
		FindAllThink();

	if (GetGoalEnt())
	{
		if (m_flPrevGoalVel != GetGoalEnt()->m_flSpeed)
		{
			if (m_flPrevGoalVel == 0 && GetGoalEnt()->m_flSpeed != 0)
			{
				if (HasDead() && m_iRepelState == LOADED_WITH_GRUNTS)
					m_iRepelState = UNLOADING_GRUNTS;
			}

			m_flPrevGoalVel = GetGoalEnt()->m_flSpeed;
		}
	}

	if (m_iRepelState == UNLOADING_GRUNTS)
		DeployThink();
	else if (m_iRepelState == GRUNTS_DEPLOYED)
		HoverThink();
}

void CHL1Osprey::DyingThink(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1f);

	if (gpGlobals->curtime > m_flNextCrashExplosion)
	{
		CPASFilter filter(GetAbsOrigin());
		Vector pos;
		QAngle dummy;

		int rand = RandomInt(0, 10);

		if (rand < 4)
		{
			int iAttach = LookupAttachment(rand % 2 ? "left" : "right");
			GetAttachment(iAttach, pos, dummy);
		}
		else
		{
			pos = GetAbsOrigin();
			pos.x += random->RandomFloat(-150, 150);
			pos.y += random->RandomFloat(-150, 150);
			pos.z += random->RandomFloat(-150, -50);
		}

		te->Explosion(filter, 0.0, &pos, g_sModelIndexFireball, 10, 15, TE_EXPLFLAG_NONE, 100, 0);
		m_flNextCrashExplosion = gpGlobals->curtime + random->RandomFloat(0.4, 0.7);

		Vector vecSize = Vector(500, 500, 60);

		switch (m_iNextCrashModel)
		{
		case 0:
		{
			te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle,
				vecSize, vec3_origin, m_iTailGibs, 100, 0, 2.5, BREAK_METAL);
			break;
		}
		case 1:
		{
			te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle,
				vecSize, vec3_origin, m_iBodyGibs, 100, 0, 2.5, BREAK_METAL);
			break;
		}
		case 2:
		{
			te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle,
				vecSize, vec3_origin, m_iEngineGibs, 100, 0, 2.5, BREAK_METAL);
			break;
		}
		case 3:
		{
			te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle,
				vecSize, vec3_origin, m_nDebrisModel, 100, 0, 2.5, BREAK_METAL);
			break;
		}
		}

		m_iNextCrashModel++;
		if (m_iNextCrashModel > 3) m_iNextCrashModel = 0;
	}

	QAngle angVel = GetLocalAngularVelocity();
	if (angVel.y < 400)
	{
		angVel.y *= 1.1;
		SetLocalAngularVelocity(angVel);
	}
	Vector vecImpulse(0, 0, 0);
	vecImpulse.z -= 38.4;
	ApplyAbsVelocityImpulse(vecImpulse);

}

void CHL1Osprey::CrashTouch(CBaseEntity* pOther)
{
	Vector vecSize = Vector(120, 120, 30);
	CPVSFilter filter(GetAbsOrigin());

	te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle,
		vecSize, vec3_origin, m_iTailGibs, 100, 0, 2.5, BREAK_METAL);

	if (m_hLeftSmoke)
	{
		m_hLeftSmoke->SetLifetime(0.1f);
		m_hLeftSmoke = NULL;
	}

	if (m_hRightSmoke)
	{
		m_hRightSmoke->SetLifetime(0.1f);
		m_hRightSmoke = NULL;
	}

	BaseClass::CrashTouch(pOther);
}

BEGIN_DATADESC(CBaseHelicopter)

DEFINE_THINKFUNC(HelicopterThink),
DEFINE_THINKFUNC(CallDyingThink),
DEFINE_ENTITYFUNC(CrashTouch),
DEFINE_ENTITYFUNC(FlyTouch),

DEFINE_SOUNDPATCH(m_pRotorSound),

DEFINE_FIELD(m_flForce, FIELD_FLOAT),
DEFINE_FIELD(m_fHelicopterFlags, FIELD_INTEGER),
DEFINE_FIELD(m_vecDesiredFaceDir, FIELD_VECTOR),
DEFINE_FIELD(m_vecDesiredPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecGoalOrientation, FIELD_VECTOR),
DEFINE_FIELD(m_flLastSeen, FIELD_TIME),
DEFINE_FIELD(m_flPrevSeen, FIELD_TIME),

DEFINE_FIELD(m_vecTarget, FIELD_VECTOR),
DEFINE_FIELD(m_vecTargetPosition, FIELD_POSITION_VECTOR),

DEFINE_FIELD(m_angleVelocity, FIELD_FLOAT),
DEFINE_FIELD(m_flNextCrashExplosion, FIELD_TIME),

DEFINE_FIELD(m_flMaxSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flMaxSpeedFiring, FIELD_FLOAT),
DEFINE_FIELD(m_flGoalSpeed, FIELD_FLOAT),
DEFINE_KEYFIELD(m_flInitialSpeed, FIELD_FLOAT, "InitialSpeed"),

DEFINE_INPUTFUNC(FIELD_STRING, "ChangePathCorner", InputChangePathCorner),
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),

DEFINE_OUTPUT(m_AtTarget, "AtPathCorner"),
DEFINE_OUTPUT(m_LeaveTarget, "LeavePathCorner"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CBaseHelicopter, DT_BaseHelicopter)
END_SEND_TABLE()

void CBaseHelicopter::Precache(void)
{
}

void CBaseHelicopter::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_STEP);
	AddFlag(FL_FLY);

	m_lifeState = LIFE_ALIVE;

	m_fHelicopterFlags &= ~BITS_HELICOPTER_MISSILE_ON;
	m_fHelicopterFlags &= ~BITS_HELICOPTER_GUN_ON;

	m_pRotorSound = NULL;

	SetCycle(0);
	ResetSequenceInfo();

	AddFlag(FL_NPC);

	m_flMaxSpeed = BASECHOPPER_MAX_SPEED;
	m_flMaxSpeedFiring = BASECHOPPER_MAX_FIRING_SPEED;
	m_takedamage = DAMAGE_AIM;

	if (!(m_spawnflags & SF_AWAITINPUT))
	{
		Startup();
		SetNextThink(gpGlobals->curtime + 1.0f);
	}

}

bool CBaseHelicopter::FireGun(void)
{
	return true;
}

void CBaseHelicopter::HelicopterThink(void)
{
	SetNextThink(gpGlobals->curtime + HELICOPTER_THINK_INTERVAL);

	ClearCondition(COND_ENEMY_DEAD);

	DispatchAnimEvents(this);

	PrescheduleThink();

	ShowDamage();

	if (CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI)
	{
		SetAbsVelocity(vec3_origin);
		SetLocalAngularVelocity(vec3_angle);
		SetNextThink(gpGlobals->curtime + HELICOPTER_THINK_INTERVAL);
		return;
	}

	Hunt();

	HelicopterPostThink();
}

void CBaseHelicopter::Hunt(void)
{
	FlyPathCorners();

	if (HasCondition(COND_ENEMY_DEAD))
	{
		SetEnemy(NULL);
	}

	if (m_lifeState == LIFE_ALIVE)
	{
		GetSenses()->Look(4092);

		ChooseEnemy();

		if (HasEnemy())
		{
			CheckEnemy(GetEnemy());

			if (FVisible(GetEnemy()))
			{
				if (m_flLastSeen < gpGlobals->curtime - 2)
				{
					m_flPrevSeen = gpGlobals->curtime;
				}

				m_flLastSeen = gpGlobals->curtime;
				m_vecTargetPosition = GetEnemy()->WorldSpaceCenter();
			}
		}
		else
		{
			m_vecTargetPosition = m_vecDesiredPosition;
		}
	}
	else
	{
		SetEnemy(NULL);
	}

	if (1)
	{
		Vector targetDir = m_vecTargetPosition - GetAbsOrigin();
		Vector desiredDir = m_vecDesiredPosition - GetAbsOrigin();

		VectorNormalize(targetDir);
		VectorNormalize(desiredDir);

		if (!IsCrashing() && m_flLastSeen + 5 > gpGlobals->curtime)
		{
			m_vecDesiredFaceDir = targetDir;
		}
		else
		{
			m_vecDesiredFaceDir = desiredDir;
		}
	}
	else
	{
		m_vecDesiredFaceDir = m_vecGoalOrientation;
	}

	Flight();

	UpdatePlayerDopplerShift();

	if (m_fHelicopterFlags & BITS_HELICOPTER_GUN_ON)
	{
		if ((m_flLastSeen + 1 > gpGlobals->curtime) && (m_flPrevSeen + 2 < gpGlobals->curtime))
		{
			if (FireGun())
			{
				if (m_flGoalSpeed > m_flMaxSpeedFiring)
				{
					m_flGoalSpeed = m_flMaxSpeedFiring;
				}
			}
		}
	}

	if (m_fHelicopterFlags & BITS_HELICOPTER_MISSILE_ON)
	{
		AimRocketGun();
	}

	if (GetEnemy() != NULL && !GetEnemy()->IsAlive())
	{
		SetEnemy(NULL);
	}
}

void CBaseHelicopter::FlyPathCorners(void)
{

	if (GetGoalEnt() == NULL && m_target != NULL_STRING)
	{
		SetGoalEnt(gEntList.FindEntityByName(NULL, m_target));
		if (GetGoalEnt())
		{
			m_vecDesiredPosition = GetGoalEnt()->GetLocalOrigin();

			AngleVectors(GetGoalEnt()->GetLocalAngles(), &m_vecGoalOrientation);
		}
	}

	if (GetGoalEnt())
	{
		if (HasReachedTarget())
		{
			m_AtTarget.FireOutput(GetGoalEnt(), this);

			OnReachedTarget(GetGoalEnt());

			SetGoalEnt(gEntList.FindEntityByName(NULL, GetGoalEnt()->m_target));

			if (GetGoalEnt())
			{
				m_vecDesiredPosition = GetGoalEnt()->GetAbsOrigin();

				AngleVectors(GetGoalEnt()->GetLocalAngles(), &m_vecGoalOrientation);
			}
		}
	}
	else
	{
		m_vecDesiredPosition = GetAbsOrigin();
	}

}

void CBaseHelicopter::UpdatePlayerDopplerShift()
{
	if (m_iSoundState == 0)
	{
		InitializeRotorSound();
	}
	else
	{
		CBaseEntity* pPlayer = NULL;

		pPlayer = gEntList.FindEntityByName(NULL, "!player");
		if (pPlayer)
		{
			Vector dir = pPlayer->GetLocalOrigin() - GetLocalOrigin();
			VectorNormalize(dir);

			float velReceiver = -DotProduct(pPlayer->GetAbsVelocity(), dir);
			float velTransmitter = -DotProduct(GetAbsVelocity(), dir);
			int iPitch = 100 * ((1 - velReceiver / 13049) / (1 + velTransmitter / 13049));

			if (iPitch > 250)
				iPitch = 250;
			if (iPitch < 50)
				iPitch = 50;

			UpdateRotorSoundPitch(iPitch);
		}
		else
		{
			Msg("Chopper didn't find a player!\n");
		}
	}
}

void CBaseHelicopter::Flight(void)
{
	if (GetFlags() & FL_ONGROUND)
	{
		SetGroundEntity(NULL);
	}

	if (m_flGoalSpeed < m_flMaxSpeed)
	{
		m_flGoalSpeed += GetAcceleration();
	}

	Vector forward, right, up;
	AngleVectors(GetLocalAngles() + GetLocalAngularVelocity() * 2, &forward, &right, &up);

	QAngle angVel = GetLocalAngularVelocity();
	float flSide = DotProduct(m_vecDesiredFaceDir, right);
	if (flSide < 0)
	{
		if (angVel.y < 8)
		{
			angVel.y += 2;
		}
		else if (angVel.y < 60)
		{
			angVel.y += 8;
		}
	}
	else
	{
		if (angVel.y > -8)
		{
			angVel.y -= 2;
		}
		else if (angVel.y > -60)
		{
			angVel.y -= 8;
		}
	}

	angVel.y *= (0.98);

	AngleVectors(GetLocalAngles() + angVel, NULL, NULL, &up);
	Vector vecEst = GetAbsOrigin() + GetAbsVelocity() * 2.0 + up * m_flForce * 20 - Vector(0, 0, 384 * 2);

	AngleVectors(GetLocalAngles(), &forward, &right, &up);

	Vector vecImpulse(0, 0, 0);
	vecImpulse.x += up.x * m_flForce;
	vecImpulse.y += up.y * m_flForce;
	vecImpulse.z += up.z * m_flForce;

	vecImpulse.z -= 38.4;
	ApplyAbsVelocityImpulse(vecImpulse);

	float flSpeed = GetAbsVelocity().Length();
	float flDir = DotProduct(Vector(forward.x, forward.y, 0), Vector(GetAbsVelocity().x, GetAbsVelocity().y, 0));
	if (flDir < 0)
	{
		flSpeed = -flSpeed;
	}

	float flDist = DotProduct(m_vecDesiredPosition - vecEst, forward);

	float flSlip = -DotProduct(m_vecDesiredPosition - vecEst, right);

	if (flSlip > 0)
	{
		if (GetLocalAngles().z > -30 && angVel.z > -15)
			angVel.z -= 4;
		else
			angVel.z += 2;
	}
	else
	{
		if (GetLocalAngles().z < 30 && angVel.z < 15)
			angVel.z += 4;
		else
			angVel.z -= 2;
	}

	ApplySidewaysDrag(right);
	ApplyGeneralDrag();

#define MAX_FORCE		80
#define FORCE_POSDELTA	12	
#define FORCE_NEGDELTA	8

	if (m_flForce < MAX_FORCE && vecEst.z < m_vecDesiredPosition.z)
	{
		m_flForce += FORCE_POSDELTA;
	}
	else if (m_flForce > 30)
	{
		if (vecEst.z > m_vecDesiredPosition.z)
			m_flForce -= FORCE_NEGDELTA;
	}

	m_angleVelocity = GetLocalAngles().x + angVel.x;
	float angleX = angVel.x;

	if (flDist > 128 && flSpeed < m_flGoalSpeed && m_angleVelocity < 30)
	{
		angleX += 6;
	}
	else if (flDist < -128 && flSpeed > -50 && m_angleVelocity > -20)
	{
		angleX -= 12.0;
	}
	else if ((m_angleVelocity < -20) || (m_angleVelocity < 0 && flDist < 128))
	{
		if (abs(m_angleVelocity) < 5)
		{
			angleX += 1.0;
		}
		else
		{
			angleX += 4.0;
		}
	}
	else if ((m_angleVelocity > 20) || (m_angleVelocity > 0 && flDist < 128))
	{
		if (abs(m_angleVelocity) < 5)
		{
			angleX -= 1.0;
		}
		else
		{
			angleX -= 4.0;
		}
	}

	angVel.x = angleX;

	SetLocalAngularVelocity(angVel);
}

void CBaseHelicopter::InitializeRotorSound(void)
{
	if (m_pRotorSound)
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

		controller.Play(m_pRotorSound, 0.0, 100);
		controller.SoundChangeVolume(m_pRotorSound, GetRotorVolume(), 2.0);
	}

	m_iSoundState = SND_CHANGE_PITCH;
}

void CBaseHelicopter::UpdateRotorSoundPitch(int iPitch)
{
	if (m_pRotorSound)
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
		controller.SoundChangePitch(m_pRotorSound, iPitch, 0.1);
		controller.SoundChangeVolume(m_pRotorSound, GetRotorVolume(), 0.1);
	}
}

void CBaseHelicopter::FlyTouch(CBaseEntity* pOther)
{
	if (pOther->GetSolid() == SOLID_BSP)
	{
		const trace_t& tr = CBaseEntity::GetTouchTrace();

		ApplyAbsVelocityImpulse(tr.plane.normal * (GetAbsVelocity().Length() + 200));
	}
}

void CBaseHelicopter::CrashTouch(CBaseEntity* pOther)
{
	if (pOther->GetSolid() == SOLID_BSP)
	{
		SetTouch(NULL);
		SetNextThink(gpGlobals->curtime);

		CPASFilter filter(GetAbsOrigin());
		for (int i = 0; i < 5; i++)
		{
			Vector pos = GetAbsOrigin();

			pos.x += random->RandomFloat(-150, 150);
			pos.y += random->RandomFloat(-150, 150);
			pos.z += random->RandomFloat(-150, -50);
			te->Explosion(filter, min(0.99, i * 0.2), &pos, g_sModelIndexFireball, 10, 15, TE_EXPLFLAG_NONE, 100, 0);
		}

		UTIL_Remove(this);
	}
}

void CBaseHelicopter::DyingThink(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1f);

	SetLocalAngularVelocity(GetLocalAngularVelocity() * 1.02);
}

int CBaseHelicopter::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	return BaseClass::OnTakeDamage_Alive(info);
}

void CBaseHelicopter::DrawDebugGeometryOverlays(void)
{
	if (m_pfnThink != NULL)
	{
		if (m_debugOverlays & OVERLAY_NPC_ROUTE_BIT)
		{
			NDebugOverlay::Line(GetAbsOrigin(), m_vecDesiredPosition, 0, 0, 255, true, 0);
		}
	}
	BaseClass::DrawDebugGeometryOverlays();
}

void CBaseHelicopter::TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator)
{
	CTakeDamageInfo dmgInfo = info;

	if (info.GetDamage() > 50 || ptr->hitgroup == 11)
	{
		AddMultiDamage(dmgInfo, this);
	}
	else
	{
		dmgInfo.ScaleDamage(0.5);
		AddMultiDamage(dmgInfo, this);
		g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
	}

}

void CBaseHelicopter::NullThink(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.5f);
}


void CBaseHelicopter::Startup(void)
{
	m_flGoalSpeed = m_flInitialSpeed;
	SetThink(&CBaseHelicopter::HelicopterThink);
	SetTouch(&CBaseHelicopter::FlyTouch);
	SetNextThink(gpGlobals->curtime + 0.1f);
}


void CBaseHelicopter::Event_Killed(const CTakeDamageInfo& info)
{
	m_lifeState = LIFE_DYING;

	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetGravity(UTIL_ScaleForGravity(240));

	UTIL_SetSize(this, Vector(-32, -32, -64), Vector(32, 32, 0));
	SetThink(&CBaseHelicopter::CallDyingThink);
	SetTouch(&CBaseHelicopter::CrashTouch);

	m_flNextCrashExplosion = gpGlobals->curtime + 0.0f;

	SetNextThink(gpGlobals->curtime + 0.1f);
	m_iHealth = 0;
	m_takedamage = DAMAGE_NO;

	m_OnDeath.FireOutput(info.GetAttacker(), this);
}


void CBaseHelicopter::StopLoopingSounds()
{
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
	controller.SoundDestroy(m_pRotorSound);
	m_pRotorSound = NULL;

	BaseClass::StopLoopingSounds();
}

void CBaseHelicopter::GibMonster(void)
{
}


void CBaseHelicopter::ChangePathCorner(const char* pszName)
{
	if (m_lifeState != LIFE_ALIVE)
	{
		return;
	}

	if (GetGoalEnt())
	{
		SetGoalEnt(gEntList.FindEntityByName(NULL, pszName));

		if (GetGoalEnt())
		{
			m_vecDesiredPosition = GetGoalEnt()->GetLocalOrigin();
			AngleVectors(GetGoalEnt()->GetLocalAngles(), &m_vecGoalOrientation);
		}
	}
}

void CBaseHelicopter::InputChangePathCorner(inputdata_t& inputdata)
{
	ChangePathCorner(inputdata.value.String());
}

void CBaseHelicopter::InputActivate(inputdata_t& inputdata)
{
	if (m_spawnflags & SF_AWAITINPUT)
	{
		Startup();

		m_spawnflags &= ~SF_AWAITINPUT;
	}
}

void CBaseHelicopter::ApplySidewaysDrag(const Vector& vecRight)
{
	Vector vecNewVelocity = GetAbsVelocity();
	vecNewVelocity.x *= 1.0 - fabs(vecRight.x) * 0.05;
	vecNewVelocity.y *= 1.0 - fabs(vecRight.y) * 0.05;
	vecNewVelocity.z *= 1.0 - fabs(vecRight.z) * 0.05;
	SetAbsVelocity(vecNewVelocity);
}

void CBaseHelicopter::ApplyGeneralDrag(void)
{
	Vector vecNewVelocity = GetAbsVelocity();
	vecNewVelocity *= 0.995;
	SetAbsVelocity(vecNewVelocity);
}

bool CBaseHelicopter::ChooseEnemy(void)
{
	CBaseEntity* pNewEnemy;

	pNewEnemy = BestEnemy();

	if ((pNewEnemy != GetEnemy()) && pNewEnemy != NULL)
	{
		SetEnemy(pNewEnemy);
		m_flLastSeen = m_flPrevSeen = gpGlobals->curtime;
		return true;
	}
	else
	{
		ClearCondition(COND_NEW_ENEMY);
		return false;
	}
}

void CBaseHelicopter::CheckEnemy(CBaseEntity* pEnemy)
{
	if (!pEnemy->IsAlive())
	{
		SetCondition(COND_ENEMY_DEAD);
		ClearCondition(COND_SEE_ENEMY);
		ClearCondition(COND_ENEMY_OCCLUDED);
		return;
	}
}

bool CBaseHelicopter::HasReachedTarget(void)
{
	float flDist = (WorldSpaceCenter() - m_vecDesiredPosition).Length();

	if (GetGoalEnt()->m_flSpeed <= 0)
		return (flDist < 145);
	else
		return(flDist < 512);
}