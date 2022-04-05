#include	"cbase.h"
#include	"hl1_monster.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_memory.h"
#include	"ai_route.h"
#include	"ai_motor.h"
#include	"ai_senses.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"ai_behavior_follow.h"
#include	"ai_navigator.h"
#include	"decals.h"


#define		ROACH_IDLE				0
#define		ROACH_BORED				1
#define		ROACH_SCARED_BY_ENT		2
#define		ROACH_SCARED_BY_LIGHT	3
#define		ROACH_SMELL_FOOD		4
#define		ROACH_EAT				5

class CHL1Roach : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Roach, CHL1BaseMonster);

public:
	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void);

	void NPCThink(void);
	void PickNewDest(int iCondition);
	void Look(int iDistance);
	void Move(float flInterval);

	Class_T  Classify(void) { return CLASS_INSECT; }

	void Touch(CBaseEntity *pOther);

	void Event_Killed(const CTakeDamageInfo &info);
	int		GetSoundInterests(void);

	void Eat(float flFullDuration);
	bool ShouldEat(void);

	bool ShouldGib(const CTakeDamageInfo &info) { return false; }

	float	m_flLastLightLevel;
	float	m_flNextSmellTime;

	bool	m_fLightHacked;
	int		m_iMode;

	float m_flHungryTime;
};

LINK_ENTITY_TO_CLASS(monster_cockroach, CHL1Roach);

void CHL1Roach::Spawn()
{
	Precache();

	SetModel("models/roach.mdl");
	UTIL_SetSize(this, Vector(-1, -1, 0), Vector(1, 1, 2));

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_YELLOW;
	ClearEffects();
	m_iHealth = 1;
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;

	SetRenderColor(255, 255, 255, 255);

	NPCInit();
	SetActivity(ACT_IDLE);

	SetViewOffset(Vector(0, 0, 1));
	m_takedamage = DAMAGE_YES;
	m_fLightHacked = FALSE;
	m_flLastLightLevel = -1;
	m_iMode = ROACH_IDLE;
	m_flNextSmellTime = gpGlobals->curtime;

	AddEffects(EF_NOSHADOW);
}

void CHL1Roach::Precache()
{
	PrecacheModel("models/roach.mdl");

	PrecacheScriptSound("Roach.Walk");
	PrecacheScriptSound("Roach.Die");
	PrecacheScriptSound("Roach.Smash");
}

float CHL1Roach::MaxYawSpeed(void)
{
	return 120.0f;
}

void CHL1Roach::NPCThink(void)
{
	if (FNullEnt(UTIL_FindClientInPVS(edict())))
		SetNextThink(gpGlobals->curtime + random->RandomFloat(1.0f, 1.5f));
	else
		SetNextThink(gpGlobals->curtime + 0.1f);

	float flInterval = gpGlobals->curtime - GetLastThink();

	StudioFrameAdvance();

	if (!m_fLightHacked)
	{
		SetNextThink(gpGlobals->curtime + 1);
		m_fLightHacked = TRUE;
		return;
	}
	else if (m_flLastLightLevel < 0)
	{
		m_flLastLightLevel = 0;
	}

	switch (m_iMode)
	{
	case	ROACH_IDLE:
	case	ROACH_EAT:
	{
		if (random->RandomInt(0, 3) == 1)
		{
			Look(150);

			if (HasCondition(COND_SEE_FEAR))
			{
				Eat(30 + (random->RandomInt(0, 14)));
				PickNewDest(ROACH_SCARED_BY_ENT);
				SetActivity(ACT_WALK);
			}
			else if (random->RandomInt(0, 149) == 1)
			{
				PickNewDest(ROACH_BORED);
				SetActivity(ACT_WALK);

				if (m_iMode == ROACH_EAT)
				{
					Eat(30 + (random->RandomInt(0, 14)));
				}
			}
		}
		
		if (m_iMode == ROACH_IDLE)
		{
			if (ShouldEat())
			{
				GetSenses()->Listen();
			}

			if (0 > m_flLastLightLevel)
			{
				PickNewDest(ROACH_SCARED_BY_LIGHT);
				SetActivity(ACT_WALK);
			}
			else if (HasCondition(COND_SMELL))
			{
				CSound *pSound = GetLoudestSoundOfType(ALL_SOUNDS);

				if (pSound && abs(pSound->GetSoundOrigin().z - GetAbsOrigin().z) <= 3)
				{
					PickNewDest(ROACH_SMELL_FOOD);
					SetActivity(ACT_WALK);
				}
			}
		}

		break;
	}
	case	ROACH_SCARED_BY_LIGHT:
	{
		if (0 <= m_flLastLightLevel)
		{
			SetActivity(ACT_IDLE);
			m_flLastLightLevel = 0;
		}
		break;
	}
	}

	if (GetActivity() != ACT_IDLE)
	{
		Move(flInterval);
	}
}

void CHL1Roach::PickNewDest(int iCondition)
{
	Vector	vecNewDir;
	Vector	vecDest;
	float	flDist;

	m_iMode = iCondition;

	GetNavigator()->ClearGoal();

	if (m_iMode == ROACH_SMELL_FOOD)
	{
		CSound *pSound = GetLoudestSoundOfType(ALL_SOUNDS);

		if (pSound)
		{
			GetNavigator()->SetRandomGoal(3 - random->RandomInt(0, 5));
			return;
		}
	}

	do
	{
		vecNewDir.x = random->RandomInt(-1, 1);
		vecNewDir.y = random->RandomInt(-1, 1);
		flDist = 256 + (random->RandomInt(0, 255));
		vecDest = GetAbsOrigin() + vecNewDir * flDist;

	} while ((vecDest - GetAbsOrigin()).Length2D() < 128);

	Vector vecLocation;

	vecLocation.x = vecDest.x;
	vecLocation.y = vecDest.y;
	vecLocation.z = GetAbsOrigin().z;

	AI_NavGoal_t goal(GOALTYPE_LOCATION, vecLocation, ACT_WALK);

	GetNavigator()->SetGoal(goal);

	if (random->RandomInt(0, 9) == 1)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Roach.Walk");
	}
}

void CHL1Roach::Look(int iDistance)
{
	CBaseEntity	*pSightEnt = NULL;

	ClearCondition(COND_SEE_HATE | COND_SEE_DISLIKE | COND_SEE_ENEMY | COND_SEE_FEAR);

	if (FNullEnt(UTIL_FindClientInPVS(edict())))
	{
		return;
	}

	for (CEntitySphereQuery sphere(GetAbsOrigin(), iDistance); (pSightEnt = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pSightEnt->IsPlayer() || FBitSet(pSightEnt->GetFlags(), FL_NPC))
		{
			if (!FBitSet(pSightEnt->GetFlags(), FL_NOTARGET) && pSightEnt->m_iHealth > 0)
			{
				switch (IRelationType(pSightEnt))
				{
				case	D_FR:
					SetCondition(COND_SEE_FEAR);
					break;
				case	D_NU:
					break;
				default:
					Msg("%s can't asses %s\n", GetClassname(), pSightEnt->GetClassname());
					break;
				}
			}
		}
	}
}

void CHL1Roach::Move(float flInterval)
{
	float		flWaypointDist;
	Vector		vecApex;

	flWaypointDist = (GetNavigator()->GetGoalPos() - GetAbsOrigin()).Length2D();

	GetMotor()->SetIdealYawToTargetAndUpdate(GetNavigator()->GetGoalPos());

	float speed = 150 * flInterval;

	Vector vToTarget = GetNavigator()->GetGoalPos() - GetAbsOrigin();
	vToTarget.NormalizeInPlace();
	Vector vMovePos = vToTarget * speed;

	if (random->RandomInt(0, 7) == 1)
	{
		PickNewDest(m_iMode);
	}

	if (!WalkMove(vMovePos, MASK_NPCSOLID))
	{
		PickNewDest(m_iMode);
	}

	if (flWaypointDist <= m_flGroundSpeed * flInterval)
	{

		SetActivity(ACT_IDLE);
		m_flLastLightLevel = 0;

		if (m_iMode == ROACH_SMELL_FOOD)
		{
			m_iMode = ROACH_EAT;
		}
		else
		{
			m_iMode = ROACH_IDLE;
		}
	}

	if (random->RandomInt(0, 149) == 1 && m_iMode != ROACH_SCARED_BY_LIGHT && m_iMode != ROACH_SMELL_FOOD)
	{
		PickNewDest(FALSE);
	}
}

void CHL1Roach::Touch(CBaseEntity *pOther)
{
	Vector		vecSpot;
	trace_t		tr;

	if (pOther->GetAbsVelocity() == vec3_origin || !pOther->IsPlayer())
	{
		return;
	}

	vecSpot = GetAbsOrigin() + Vector(0, 0, 8);

	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

	UTIL_DecalTrace(&tr, "YellowBlood");

	TakeDamage(CTakeDamageInfo(pOther, pOther, m_iHealth, DMG_GENERIC));
}

void CHL1Roach::Event_Killed(const CTakeDamageInfo &info)
{
	RemoveSolidFlags(FSOLID_NOT_SOLID);

	CPASAttenuationFilter filter(this);

	if (random->RandomInt(0, 4) == 1)
	{
		EmitSound(filter, entindex(), "Roach.Die");
	}
	else
	{
		EmitSound(filter, entindex(), "Roach.Smash");
	}

	CSoundEnt::InsertSound(SOUND_WORLD, GetAbsOrigin(), 128, 1);

	UTIL_Remove(this);
}

int CHL1Roach::GetSoundInterests(void)
{
	return	SOUND_CARCASS |
		SOUND_MEAT;
}

void CHL1Roach::Eat(float flFullDuration)
{
	m_flHungryTime = gpGlobals->curtime + flFullDuration;
}

bool CHL1Roach::ShouldEat(void)
{
	if (m_flHungryTime > gpGlobals->curtime)
	{
		return false;
	}

	return true;
}