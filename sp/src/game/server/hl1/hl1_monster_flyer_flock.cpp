#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
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
#include	"hl1_monster.h"

#define		AFLOCK_MAX_RECRUIT_RADIUS	1024
#define		AFLOCK_FLY_SPEED			125
#define		AFLOCK_TURN_RATE			75
#define		AFLOCK_ACCELERATE			10
#define		AFLOCK_CHECK_DIST			192
#define		AFLOCK_TOO_CLOSE			100
#define		AFLOCK_TOO_FAR				256

class CHL1FlockingFlyerFlock : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1FlockingFlyerFlock, CHL1BaseMonster);
public:

	void Spawn(void);
	void Precache(void);
	bool KeyValue(const char* szKeyName, const char* szValue);
	void SpawnFlock(void);

	static  void PrecacheFlockSounds(void);

	DECLARE_DATADESC();

	int		m_cFlockSize;
	float	m_flFlockRadius;
};

class CHL1FlockingFlyer : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1FlockingFlyer, CHL1BaseMonster);
public:
	void Spawn(void);
	void Precache(void);
	void SpawnCommonCode(void);
	void IdleThink(void);
	void BoidAdvanceFrame(void);
	void Start(void);
	bool FPathBlocked(void);
	void FlockLeaderThink(void);
	void SpreadFlock(void);
	void SpreadFlock2(void);
	void MakeSound(void);
	void FlockFollowerThink(void);
	void Event_Killed(const CTakeDamageInfo& info);
	void FallHack(void);

	int IsLeader(void) { return m_pSquadLeader == this; }
	int	InSquad(void) { return m_pSquadLeader != NULL; }
	int	SquadCount(void);
	void SquadRemove(CHL1FlockingFlyer* pRemove);
	void SquadUnlink(void);
	void SquadAdd(CHL1FlockingFlyer* pAdd);
	void SquadDisband(void);

	CHL1FlockingFlyer* m_pSquadLeader;
	CHL1FlockingFlyer* m_pSquadNext;
	bool	m_fTurning;
	bool	m_fCourseAdjust;
	bool	m_fPathBlocked;
	Vector	m_vecReferencePoint;
	Vector	m_vecAdjustedVelocity;
	float	m_flGoalSpeed;
	float	m_flLastBlockedTime;
	float	m_flFakeBlockedTime;
	float	m_flAlertTime;
	float	m_flFlockNextSoundTime;
	float   m_flTempVar;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(monster_flyer_flock, CHL1FlockingFlyerFlock);

BEGIN_DATADESC(CHL1FlockingFlyerFlock)
DEFINE_FIELD(m_cFlockSize, FIELD_INTEGER),
DEFINE_FIELD(m_flFlockRadius, FIELD_FLOAT),
END_DATADESC()

bool CHL1FlockingFlyerFlock::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "iFlockSize"))
	{
		m_cFlockSize = atoi(szValue);
		return true;
	}
	else if (FStrEq(szKeyName, "flFlockRadius"))
	{
		m_flFlockRadius = atof(szValue);
		return true;
	}
	else
		BaseClass::KeyValue(szKeyName, szValue);

	return false;
}

void CHL1FlockingFlyerFlock::Spawn(void)
{
	Precache();

	SetRenderColor(255, 255, 255, 255);
	SpawnFlock();


	SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CHL1FlockingFlyerFlock::Precache(void)
{	
	PrecacheModel("models/boid.mdl");

	PrecacheFlockSounds();
}

void CHL1FlockingFlyerFlock::SpawnFlock(void)
{
	float R = m_flFlockRadius;
	int iCount;
	Vector vecSpot;
	CHL1FlockingFlyer* pBoid, * pLeader;

	pLeader = pBoid = NULL;

	for (iCount = 0; iCount < m_cFlockSize; iCount++)
	{
		pBoid = CREATE_ENTITY(CHL1FlockingFlyer, "monster_flyer");

		if (!pLeader)
		{
			pLeader = pBoid;

			pLeader->m_pSquadLeader = pLeader;
			pLeader->m_pSquadNext = NULL;
		}

		vecSpot.x = random->RandomFloat(-R, R);
		vecSpot.y = random->RandomFloat(-R, R);
		vecSpot.z = random->RandomFloat(0, 16);
		vecSpot = GetAbsOrigin() + vecSpot;

		UTIL_SetOrigin(pBoid, vecSpot);
		pBoid->SetMoveType(MOVETYPE_FLY);
		pBoid->SpawnCommonCode();
		pBoid->SetGroundEntity(NULL);
		pBoid->SetAbsVelocity(Vector(0, 0, 0));
		pBoid->SetAbsAngles(GetAbsAngles());

		pBoid->SetCycle(0);
		pBoid->SetThink(&CHL1FlockingFlyer::IdleThink);
		pBoid->SetNextThink(gpGlobals->curtime + 0.2);

		if (pBoid != pLeader)
		{
			pLeader->SquadAdd(pBoid);
		}
	}
}


void CHL1FlockingFlyerFlock::PrecacheFlockSounds(void)
{
}

LINK_ENTITY_TO_CLASS(monster_flyer, CHL1FlockingFlyer);

BEGIN_DATADESC(CHL1FlockingFlyer)
DEFINE_FIELD(m_pSquadLeader, FIELD_CLASSPTR),
DEFINE_FIELD(m_pSquadNext, FIELD_CLASSPTR),
DEFINE_FIELD(m_fTurning, FIELD_BOOLEAN),
DEFINE_FIELD(m_fCourseAdjust, FIELD_BOOLEAN),
DEFINE_FIELD(m_fPathBlocked, FIELD_BOOLEAN),
DEFINE_FIELD(m_vecReferencePoint, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecAdjustedVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flGoalSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flLastBlockedTime, FIELD_TIME),
DEFINE_FIELD(m_flFakeBlockedTime, FIELD_TIME),
DEFINE_FIELD(m_flAlertTime, FIELD_TIME),
DEFINE_THINKFUNC(IdleThink),
DEFINE_THINKFUNC(Start),
DEFINE_THINKFUNC(FlockLeaderThink),
DEFINE_THINKFUNC(FlockFollowerThink),
DEFINE_THINKFUNC(FallHack),

DEFINE_FIELD(m_flFlockNextSoundTime, FIELD_TIME),
DEFINE_FIELD(m_flTempVar, FIELD_FLOAT),
END_DATADESC()

void CHL1FlockingFlyer::Spawn()
{
	Precache();
	SpawnCommonCode();

	SetCycle(0);
	SetNextThink(gpGlobals->curtime + 0.1f);
	SetThink(&CHL1FlockingFlyer::IdleThink);
}

void CHL1FlockingFlyer::SpawnCommonCode()
{
	m_lifeState = LIFE_ALIVE;
	SetClassname("monster_flyer");
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_FLY);
	m_takedamage = DAMAGE_NO;
	m_iHealth = 1;

	m_fPathBlocked = FALSE;
	m_flFieldOfView = 0.2;
	m_flTempVar = 0;

	SetModel("models/boid.mdl");

	UTIL_SetSize(this, Vector(-5, -5, 0), Vector(5, 5, 2));
}

void CHL1FlockingFlyer::Precache()
{
	PrecacheModel("models/boid.mdl");
	CHL1FlockingFlyerFlock::PrecacheFlockSounds();

	PrecacheScriptSound("FlockingFlyer.Alert");
	PrecacheScriptSound("FlockingFlyer.Idle");
}

void CHL1FlockingFlyer::IdleThink(void)
{
	SetNextThink(gpGlobals->curtime + 0.2);

	if (!FNullEnt(UTIL_FindClientInPVS(edict())))
	{
		SetThink(&CHL1FlockingFlyer::Start);
		SetNextThink(gpGlobals->curtime + 0.1f);
	}
}

void CHL1FlockingFlyer::SquadUnlink(void)
{
	m_pSquadLeader = NULL;
	m_pSquadNext = NULL;
}

void CHL1FlockingFlyer::SquadAdd(CHL1FlockingFlyer* pAdd)
{
	ASSERT(pAdd != NULL);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	pAdd->m_pSquadNext = m_pSquadNext;
	m_pSquadNext = pAdd;
	pAdd->m_pSquadLeader = this;
}

void CHL1FlockingFlyer::SquadRemove(CHL1FlockingFlyer* pRemove)
{
	ASSERT(pRemove != NULL);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_pSquadLeader == this);

	if (SquadCount() > 2)
	{
		if (pRemove == this)
		{
			CHL1FlockingFlyer* pLeader = m_pSquadNext;

			if (pLeader)
			{
				CHL1FlockingFlyer* pList = pLeader;

				while (pList)
				{
					pList->m_pSquadLeader = pLeader;
					pList = pList->m_pSquadNext;
				}

			}
			SquadUnlink();
		}
		else
		{
			CHL1FlockingFlyer* pList = this;

			while (pList->m_pSquadNext != pRemove)
			{
				ASSERT(pList->m_pSquadNext != NULL);
				pList = pList->m_pSquadNext;
			}

			ASSERT(pList->m_pSquadNext == pRemove);

			pList->m_pSquadNext = pRemove->m_pSquadNext;

			pRemove->SquadUnlink();
		}
	}
	else
		SquadDisband();
}

int CHL1FlockingFlyer::SquadCount(void)
{
	CHL1FlockingFlyer* pList = m_pSquadLeader;
	int squadCount = 0;
	while (pList)
	{
		squadCount++;
		pList = pList->m_pSquadNext;
	}

	return squadCount;
}

void CHL1FlockingFlyer::SquadDisband(void)
{
	CHL1FlockingFlyer* pList = m_pSquadLeader;
	CHL1FlockingFlyer* pNext;

	while (pList)
	{
		pNext = pList->m_pSquadNext;
		pList->SquadUnlink();
		pList = pNext;
	}
}

void CHL1FlockingFlyer::Start(void)
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	if (IsLeader())
	{
		SetThink(&CHL1FlockingFlyer::FlockLeaderThink);
	}
	else
	{
		SetThink(&CHL1FlockingFlyer::FlockFollowerThink);
	}

	SetActivity(ACT_FLY);
	ResetSequenceInfo();
	BoidAdvanceFrame();

	m_flSpeed = AFLOCK_FLY_SPEED;
}

void CHL1FlockingFlyer::BoidAdvanceFrame(void)
{
	float flapspeed = (m_flSpeed - m_flTempVar) / AFLOCK_ACCELERATE;
	m_flTempVar = m_flTempVar * .8 + m_flSpeed * .2;

	if (flapspeed < 0) flapspeed = -flapspeed;
	if (flapspeed < 0.25) flapspeed = 0.25;
	if (flapspeed > 1.9) flapspeed = 1.9;

	m_flPlaybackRate = flapspeed;

	QAngle angVel = GetLocalAngularVelocity();

	angVel.x = -GetAbsAngles().x + flapspeed * 5;

	angVel.z = -GetAbsAngles().z + angVel.y;

	SetLocalAngularVelocity(angVel);

	StudioFrameAdvance();
}

void CHL1FlockingFlyer::FlockLeaderThink(void)
{
	trace_t			tr;
	Vector			vecDist;
	Vector			vecDir;
	float			flLeftSide;
	float			flRightSide;
	Vector			vForward, vRight, vUp;


	SetNextThink(gpGlobals->curtime + 0.1f);

	AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);

	if (!FPathBlocked())
	{
		if (m_fTurning)
		{
			m_fTurning = FALSE;

			QAngle angVel = GetLocalAngularVelocity();
			angVel.y = 0;
			SetLocalAngularVelocity(angVel);
		}

		m_fPathBlocked = FALSE;

		if (m_flSpeed <= AFLOCK_FLY_SPEED)
			m_flSpeed += 5;

		SetAbsVelocity(vForward * m_flSpeed);

		BoidAdvanceFrame();

		return;
	}

	m_fPathBlocked = TRUE;

	if (!m_fTurning)
	{
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vRight * AFLOCK_CHECK_DIST, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		vecDist = (tr.endpos - GetAbsOrigin());
		flRightSide = vecDist.Length();

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - vRight * AFLOCK_CHECK_DIST, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		vecDist = (tr.endpos - GetAbsOrigin());
		flLeftSide = vecDist.Length();

		if (flRightSide > flLeftSide)
		{
			QAngle angVel = GetLocalAngularVelocity();
			angVel.y = -AFLOCK_TURN_RATE;
			SetLocalAngularVelocity(angVel);

			m_fTurning = TRUE;
		}
		else if (flLeftSide > flRightSide)
		{
			QAngle angVel = GetLocalAngularVelocity();
			angVel.y = AFLOCK_TURN_RATE;
			SetLocalAngularVelocity(angVel);

			m_fTurning = TRUE;
		}
		else
		{
			m_fTurning = TRUE;

			QAngle angVel = GetLocalAngularVelocity();

			if (random->RandomInt(0, 1) == 0)
			{
				angVel.y = AFLOCK_TURN_RATE;
			}
			else
			{
				angVel.y = -AFLOCK_TURN_RATE;
			}

			SetLocalAngularVelocity(angVel);
		}
	}

	SpreadFlock();

	SetAbsVelocity(vForward * m_flSpeed);

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - vUp * 16, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0 && GetAbsVelocity().z < 0)
	{
		Vector vecVel = GetAbsVelocity();
		vecVel.z = 0;
		SetAbsVelocity(vecVel);
	}

	if (GetFlags() & FL_ONGROUND)
	{
		UTIL_SetOrigin(this, GetAbsOrigin() + Vector(0, 0, 1));
		Vector vecVel = GetAbsVelocity();
		vecVel.z = 0;
		SetAbsVelocity(vecVel);
	}

	if (m_flFlockNextSoundTime < gpGlobals->curtime)
	{
		m_flFlockNextSoundTime = gpGlobals->curtime + random->RandomFloat(1, 3);
	}

	BoidAdvanceFrame();

	return;
}

bool CHL1FlockingFlyer::FPathBlocked(void)
{
	trace_t			tr;
	Vector			vecDist;
	Vector			vecDir;
	bool			fBlocked;
	Vector			vForward, vRight, vUp;

	if (m_flFakeBlockedTime > gpGlobals->curtime)
	{
		m_flLastBlockedTime = gpGlobals->curtime;
		return TRUE;
	}

	AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);

	fBlocked = FALSE;

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * AFLOCK_CHECK_DIST, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->curtime;
		fBlocked = TRUE;
	}

	UTIL_TraceLine(GetAbsOrigin() + vRight * 12, GetAbsOrigin() + vRight * 12 + vForward * AFLOCK_CHECK_DIST, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->curtime;
		fBlocked = TRUE;
	}

	UTIL_TraceLine(GetAbsOrigin() - vRight * 12, GetAbsOrigin() - vRight * 12 + vForward * AFLOCK_CHECK_DIST, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0)
	{
		m_flLastBlockedTime = gpGlobals->curtime;
		fBlocked = TRUE;
	}

	if (!fBlocked && gpGlobals->curtime - m_flLastBlockedTime > 6)
	{
		m_flFakeBlockedTime = gpGlobals->curtime + random->RandomInt(1, 3);
	}

	return	fBlocked;
}

void CHL1FlockingFlyer::SpreadFlock()
{
	Vector		vecDir;
	float		flSpeed;

	CHL1FlockingFlyer* pList = m_pSquadLeader;
	while (pList)
	{
		if (pList != this && (GetAbsOrigin() - pList->GetAbsOrigin()).Length() <= AFLOCK_TOO_CLOSE)
		{
			vecDir = (pList->GetAbsOrigin() - GetAbsOrigin());
			VectorNormalize(vecDir);

			flSpeed = pList->GetAbsVelocity().Length();

			Vector vecVel = pList->GetAbsVelocity();
			VectorNormalize(vecVel);
			pList->SetAbsVelocity((vecVel + vecDir) * 0.5 * flSpeed);
		}

		pList = pList->m_pSquadNext;
	}
}

void CHL1FlockingFlyer::SpreadFlock2()
{
	Vector		vecDir;

	CHL1FlockingFlyer* pList = m_pSquadLeader;

	while (pList)
	{
		if (pList != this && (GetAbsOrigin() - pList->GetAbsOrigin()).Length() <= AFLOCK_TOO_CLOSE)
		{
			vecDir = (GetAbsOrigin() - pList->GetAbsOrigin());
			VectorNormalize(vecDir);

			SetAbsVelocity((GetAbsVelocity() + vecDir));
		}

		pList = pList->m_pSquadNext;
	}
}

void CHL1FlockingFlyer::MakeSound(void)
{
	if (m_flAlertTime > gpGlobals->curtime)
	{
		CPASAttenuationFilter filter1(this);

		EmitSound(filter1, entindex(), "FlockingFlyer.Alert");
		return;
	}

	CPASAttenuationFilter filter2(this);

	EmitSound(filter2, entindex(), "FlockingFlyer.Idle");
}

void CHL1FlockingFlyer::FlockFollowerThink(void)
{
	Vector			vecDist;
	Vector			vecDir;
	Vector			vecDirToLeader;
	float			flDistToLeader;

	SetNextThink(gpGlobals->curtime + 0.1f);

	if (IsLeader() || !InSquad())
	{
		SetThink(&CHL1FlockingFlyer::FlockLeaderThink);
		return;
	}

	vecDirToLeader = (m_pSquadLeader->GetAbsOrigin() - GetAbsOrigin());
	flDistToLeader = vecDirToLeader.Length();

	SetAbsAngles(m_pSquadLeader->GetAbsAngles());

	if (FInViewCone(m_pSquadLeader))
	{
		if (flDistToLeader > AFLOCK_TOO_FAR)
		{
			m_flGoalSpeed = m_pSquadLeader->GetAbsVelocity().Length() * 1.5;
		}

		else if (flDistToLeader < AFLOCK_TOO_CLOSE)
		{
			m_flGoalSpeed = m_pSquadLeader->GetAbsVelocity().Length() * 0.5;
		}
	}
	else
	{
		m_flGoalSpeed = m_pSquadLeader->GetAbsVelocity().Length() * 0.5;
	}

	SpreadFlock2();

	Vector vecVel = GetAbsVelocity();
	m_flSpeed = vecVel.Length();
	VectorNormalize(vecVel);

	if (flDistToLeader > AFLOCK_TOO_FAR)
	{
		VectorNormalize(vecDirToLeader);
		vecVel = (vecVel + vecDirToLeader) * 0.5;
	}

	if (m_flGoalSpeed > AFLOCK_FLY_SPEED * 2)
	{
		m_flGoalSpeed = AFLOCK_FLY_SPEED * 2;
	}

	if (m_flSpeed < m_flGoalSpeed)
	{
		m_flSpeed += AFLOCK_ACCELERATE;
	}
	else if (m_flSpeed > m_flGoalSpeed)
	{
		m_flSpeed -= AFLOCK_ACCELERATE;
	}

	SetAbsVelocity(vecVel * m_flSpeed);

	BoidAdvanceFrame();
}

void CHL1FlockingFlyer::Event_Killed(const CTakeDamageInfo& info)
{
	CHL1FlockingFlyer* pSquad;

	pSquad = (CHL1FlockingFlyer*)m_pSquadLeader;

	while (pSquad)
	{
		pSquad->m_flAlertTime = gpGlobals->curtime + 15;
		pSquad = (CHL1FlockingFlyer*)pSquad->m_pSquadNext;
	}

	if (m_pSquadLeader)
	{
		m_pSquadLeader->SquadRemove(this);
	}

	m_lifeState = LIFE_DEAD;

	m_flPlaybackRate = 0;
	AddEffects(EF_NOINTERP);

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	SetMoveType(MOVETYPE_FLYGRAVITY);

	SetThink(&CHL1FlockingFlyer::FallHack);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CHL1FlockingFlyer::FallHack(void)
{
	if (GetFlags() & FL_ONGROUND)
	{
		CBaseEntity* groundentity = GetContainingEntity(GetGroundEntity()->edict());

		if (!FClassnameIs(groundentity, "worldspawn"))
		{
			SetGroundEntity(NULL);
			SetNextThink(gpGlobals->curtime + 0.1f);
		}
		else
		{
			SetAbsVelocity(Vector(0, 0, 0));
			SetThink(NULL);
		}
	}
}