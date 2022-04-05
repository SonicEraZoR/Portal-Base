// Patch 2.1

#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"ai_senses.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"hl1_monster.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"Sprite.h"

#define TURRET_SHOTS	2
#define TURRET_RANGE	(100 * 12)
#define TURRET_SPREAD	Vector( 0, 0, 0 )
#define TURRET_TURNRATE	30
#define TURRET_MAXWAIT	15
#define TURRET_MAXSPIN	5
#define TURRET_MACHINE_VOLUME	0.5

typedef enum
{
	TURRET_ANIM_FIRE = 0,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;

#define SF_MONSTER_TURRET_AUTOACTIVATE	32
#define SF_MONSTER_TURRET_STARTINACTIVE	64

#define TURRET_GLOW_SPRITE "sprites/flare3.spr"

extern short g_sModelIndexSmoke;

class CHL1BaseTurret : public CHL1BaseMonster
{
public:
	DECLARE_CLASS(CHL1BaseTurret, CHL1BaseMonster);
	DECLARE_DATADESC()

	void Spawn(void);
	virtual void Precache(void);
	bool KeyValue(const char *szKeyName, const char * szValue);
	void TurretUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	virtual void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);

	virtual int OnTakeDamage(const CTakeDamageInfo &info);
	virtual int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

	Class_T Classify(void);

	int BloodColor(void) { return DONT_BLEED; }
	void GibMonster(void) {}

	void ActiveThink(void);
	void SearchThink(void);
	void AutoSearchThink(void);

	virtual void TurretDeath(void);
	virtual void SpinDownCall(void) { m_iSpin = 0; }
	virtual void SpinUpCall(void) { m_iSpin = 1; }

	void Deploy(void);
	void Retire(void);

	void Initialize(void);

	virtual void Ping(void);
	virtual void EyeOn(void);
	virtual void EyeOff(void);

	void SetTurretAnim(TURRET_ANIM anim);
	int MoveTurret(void);
	virtual void Shoot(Vector &vecSrc, Vector &vecDirToEnemy) { };

	float m_flMaxSpin;
	int m_iSpin;

	CSprite *m_pEyeGlow;
	int		m_eyeBrightness;

	int	m_iDeployHeight;
	int	m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate;
	float m_fTurnRate;
	int m_iOrientation;
	int	m_iOn;
	int m_fBeserk;
	int m_iAutoStart;

	Vector m_vecLastSight;
	float m_flLastSight;
	float m_flMaxWait;
	int m_iSearchSpeed;

	float	m_flStartYaw;
	Vector	m_vecCurAngles;
	Vector	m_vecGoalAngles;


	float	m_flPingTime;
	float	m_flSpinUpTime;

	float   m_flDamageTime;

	int		m_iAmmoType;

	void InputActivate(inputdata_t &inputdata);
	void InputDeactivate(inputdata_t &inputdata);

	COutputEvent	m_OnActivate;
	COutputEvent	m_OnDeactivate;
};

BEGIN_DATADESC(CHL1BaseTurret)
DEFINE_FIELD(m_flMaxSpin, FIELD_FLOAT),
DEFINE_FIELD(m_iSpin, FIELD_INTEGER),
DEFINE_FIELD(m_pEyeGlow, FIELD_CLASSPTR),
DEFINE_FIELD(m_eyeBrightness, FIELD_INTEGER),
DEFINE_FIELD(m_iDeployHeight, FIELD_INTEGER),
DEFINE_FIELD(m_iRetractHeight, FIELD_INTEGER),
DEFINE_FIELD(m_iMinPitch, FIELD_INTEGER),
DEFINE_FIELD(m_fTurnRate, FIELD_FLOAT),
DEFINE_FIELD(m_iOn, FIELD_INTEGER),
DEFINE_FIELD(m_fBeserk, FIELD_INTEGER),
DEFINE_FIELD(m_iAutoStart, FIELD_INTEGER),
DEFINE_FIELD(m_vecLastSight, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_flLastSight, FIELD_TIME),
DEFINE_FIELD(m_flStartYaw, FIELD_FLOAT),
DEFINE_FIELD(m_vecCurAngles, FIELD_VECTOR),
DEFINE_FIELD(m_vecGoalAngles, FIELD_VECTOR),
DEFINE_FIELD(m_flPingTime, FIELD_TIME),
DEFINE_FIELD(m_flSpinUpTime, FIELD_TIME),
DEFINE_FIELD(m_flDamageTime, FIELD_TIME),
DEFINE_KEYFIELD(m_flMaxWait, FIELD_FLOAT, "maxsleep"),
DEFINE_KEYFIELD(m_iOrientation, FIELD_INTEGER, "orientation"),
DEFINE_KEYFIELD(m_iSearchSpeed, FIELD_INTEGER, "searchspeed"),
DEFINE_KEYFIELD(m_iBaseTurnRate, FIELD_INTEGER, "turnrate"),
DEFINE_USEFUNC(TurretUse),
DEFINE_THINKFUNC(ActiveThink),
DEFINE_THINKFUNC(SearchThink),
DEFINE_THINKFUNC(AutoSearchThink),
DEFINE_THINKFUNC(TurretDeath),
DEFINE_THINKFUNC(SpinDownCall),
DEFINE_THINKFUNC(SpinUpCall),
DEFINE_THINKFUNC(Deploy),
DEFINE_THINKFUNC(Retire),
DEFINE_THINKFUNC(Initialize),
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Deactivate", InputDeactivate),
DEFINE_OUTPUT(m_OnActivate, "OnActivate"),
DEFINE_OUTPUT(m_OnDeactivate, "OnDeactivate"),
END_DATADESC()

bool CHL1BaseTurret::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "maxsleep"))
	{
		m_flMaxWait = atof(szValue);
	}
	else if (FStrEq(szKeyName, "orientation"))
	{
		m_iOrientation = atoi(szValue);

	}
	else if (FStrEq(szKeyName, "searchspeed"))
	{
		m_iSearchSpeed = atoi(szValue);

	}
	else if (FStrEq(szKeyName, "turnrate"))
	{
		m_iBaseTurnRate = atoi(szValue);
	}
	else if (FStrEq(szKeyName, "style") ||
		FStrEq(szKeyName, "height") ||
		FStrEq(szKeyName, "value1") ||
		FStrEq(szKeyName, "value2") ||
		FStrEq(szKeyName, "value3"))
	{
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

void CHL1BaseTurret::Spawn()
{
	Precache();
	SetNextThink(gpGlobals->curtime + 1);
	SetMoveType(MOVETYPE_FLY);
	SetSequence(0);
	SetCycle(0);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	m_takedamage = DAMAGE_YES;
	AddFlag(FL_AIMTARGET);

	AddFlag(FL_NPC);
	SetUse(&CHL1BaseTurret::TurretUse);

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	ResetSequenceInfo();

	if ((m_spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		&& !(m_spawnflags & SF_MONSTER_TURRET_STARTINACTIVE))
	{
		m_iAutoStart = true;
	}

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	m_flFieldOfView = VIEW_FIELD_FULL;

	m_flDamageTime = 0;

	SetTurretAnim(TURRET_ANIM_RETIRE);
	SetCycle(0.0f);
	m_flPlaybackRate = 0.0f;

}

void CHL1BaseTurret::Precache()
{
	m_iAmmoType = GetAmmoDef()->Index("12mmRound");

	PrecacheScriptSound("Turret.Alert");
	PrecacheScriptSound("Turret.Die");
	PrecacheScriptSound("Turret.Deploy");
	PrecacheScriptSound("Turret.Undeploy");
	PrecacheScriptSound("Turret.Ping");
	PrecacheScriptSound("Turret.Shoot");
}

void CHL1BaseTurret::Initialize(void)
{
	m_iOn = 0;
	m_fBeserk = 0;
	m_iSpin = 0;

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	if (m_iBaseTurnRate == 0) m_iBaseTurnRate = TURRET_TURNRATE;
	if (m_flMaxWait == 0) m_flMaxWait = TURRET_MAXWAIT;
	QAngle angles = GetAbsAngles();
	m_flStartYaw = angles.y;
	if (m_iOrientation == 1)
	{
		angles.x = 180;
		angles.y += 180;
		if (angles.y > 360)
			angles.y -= 360;
		SetAbsAngles(angles);

		Vector view_ofs = GetViewOffset();
		view_ofs.z = -view_ofs.z;
		SetViewOffset(view_ofs);
	}

	m_vecGoalAngles.x = 0;

	if (m_iAutoStart)
	{
		m_flLastSight = gpGlobals->curtime + m_flMaxWait;
		SetThink(&CHL1BaseTurret::AutoSearchThink);
		SetNextThink(gpGlobals->curtime + .1);
	}
	else
		SetThink(&CHL1BaseTurret::SUB_DoNothing);
}

void CHL1BaseTurret::TurretUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_iOn))
		return;

	if (m_iOn)
	{
		SetEnemy(NULL);
		SetNextThink(gpGlobals->curtime + 0.1);
		m_iAutoStart = false;
		SetThink(&CHL1BaseTurret::Retire);
	}
	else
	{
		SetNextThink(gpGlobals->curtime + 0.1);

		if (m_spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		{
			m_iAutoStart = true;
		}

		SetThink(&CHL1BaseTurret::Deploy);
	}
}

void CHL1BaseTurret::Ping(void)
{
	if (m_flPingTime == 0)
		m_flPingTime = gpGlobals->curtime + 1;
	else if (m_flPingTime <= gpGlobals->curtime)
	{
		m_flPingTime = gpGlobals->curtime + 1;
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Turret.Ping");

		EyeOn();
	}
	else if (m_eyeBrightness > 0)
	{
		EyeOff();
	}
}

void CHL1BaseTurret::EyeOn()
{
	if (m_pEyeGlow)
	{
		if (m_eyeBrightness != 255)
		{
			m_eyeBrightness = 255;
		}
		m_pEyeGlow->SetBrightness(m_eyeBrightness);
	}
}


void CHL1BaseTurret::EyeOff()
{
	if (m_pEyeGlow)
	{
		if (m_eyeBrightness > 0)
		{
			m_eyeBrightness = max(0, m_eyeBrightness - 30);
			m_pEyeGlow->SetBrightness(m_eyeBrightness);
		}
	}
}

void CHL1BaseTurret::ActiveThink(void)
{
	int fAttack = 0;

	SetNextThink(gpGlobals->curtime + 0.1);
	StudioFrameAdvance();

	if ((!m_iOn) || (GetEnemy() == NULL))
	{
		SetEnemy(NULL);
		m_flLastSight = gpGlobals->curtime + m_flMaxWait;
		SetThink(&CHL1BaseTurret::SearchThink);
		return;
	}

	if (!GetEnemy()->IsAlive())
	{
		if (!m_flLastSight)
		{
			m_flLastSight = gpGlobals->curtime + 0.5;
		}
		else
		{
			if (gpGlobals->curtime > m_flLastSight)
			{
				SetEnemy(NULL);
				m_flLastSight = gpGlobals->curtime + m_flMaxWait;
				SetThink(&CHL1BaseTurret::SearchThink);
				return;
			}
		}
	}

	Vector vecMid;
	QAngle vecAng;
	GetAttachment(1, vecMid, vecAng);

	Vector vecMidEnemy = GetEnemy()->BodyTarget(vecMid, false);

	Vector vecDirToEnemyEyes = vecMidEnemy - vecMid;

	int fEnemyVisible = FBoxVisible(this, GetEnemy(), vecMidEnemy);

	float flDistToEnemy = vecDirToEnemyEyes.Length();

	VectorNormalize(vecDirToEnemyEyes);

	QAngle vecAnglesToEnemy;

	VectorAngles(vecDirToEnemyEyes, vecAnglesToEnemy);

	if (!fEnemyVisible || (flDistToEnemy > TURRET_RANGE))
	{
		if (!m_flLastSight)
			m_flLastSight = gpGlobals->curtime + 0.5;
		else
		{
			if (gpGlobals->curtime > m_flLastSight)
			{
				SetEnemy(NULL);
				m_flLastSight = gpGlobals->curtime + m_flMaxWait;
				SetThink(&CHL1BaseTurret::SearchThink);
				return;
			}
		}
		fEnemyVisible = 0;
	}
	else
	{
		m_vecLastSight = vecMidEnemy;
	}

	if (m_fBeserk)
	{
		if (random->RandomInt(0, 9) == 0)
		{
			m_vecGoalAngles.y = random->RandomFloat(0, 360);
			m_vecGoalAngles.x = random->RandomFloat(0, -85) + 85 * m_iOrientation;

			CTakeDamageInfo info;
			info.SetAttacker(this);
			info.SetInflictor(this);
			info.SetDamage(1);
			info.SetDamageType(DMG_GENERIC);

			TakeDamage(info);
			return;
		}
	}
	else if (fEnemyVisible)
	{

		if (vecAnglesToEnemy.y > 360)
			vecAnglesToEnemy.y -= 360;

		if (vecAnglesToEnemy.y < 0)
			vecAnglesToEnemy.y += 360;

		if (vecAnglesToEnemy.x < -180)
			vecAnglesToEnemy.x += 360;

		if (vecAnglesToEnemy.x > 180)
			vecAnglesToEnemy.x -= 360;

		if (m_iOrientation == 1)
		{
			if (vecAnglesToEnemy.x > 90)
				vecAnglesToEnemy.x = 90;
			else if (vecAnglesToEnemy.x < m_iMinPitch)
				vecAnglesToEnemy.x = m_iMinPitch;
		}
		else
		{
			if (vecAnglesToEnemy.x < -90)
				vecAnglesToEnemy.x = -90;
			else if (vecAnglesToEnemy.x > -m_iMinPitch)
				vecAnglesToEnemy.x = -m_iMinPitch;
		}
		vecAnglesToEnemy.x += 1;

		m_vecGoalAngles.y = vecAnglesToEnemy.y;
		m_vecGoalAngles.x = vecAnglesToEnemy.x;
	}

	float int_x = m_vecGoalAngles.x - m_vecCurAngles.x;
	float int_y = m_vecGoalAngles.y - m_vecCurAngles.y;
	float int_z = m_vecGoalAngles.z - m_vecCurAngles.z;

	if (int_x < 0){
		int_x = -int_x;
	}
	if (int_y < 0){
		int_y = -int_y;
	}
	if (int_z < 0){
		int_z = -int_z;
	}

	float flt_average = ((int_x + int_y + int_z)) / ((float)3);

	if (flt_average > 1.8)
		fAttack = FALSE;
	else
		fAttack = TRUE;

	if (m_iSpin && (fAttack || m_fBeserk))
	{
		Vector forward;
		AngleVectors(vecAng, &forward);
		Shoot(vecMid, forward);
		SetTurretAnim(TURRET_ANIM_FIRE);
	}
	else
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}

	SpinUpCall();
	MoveTurret();
}

void CHL1BaseTurret::Deploy(void)
{
	SetNextThink(gpGlobals->curtime + 0.1);
	StudioFrameAdvance();

	if (GetSequence() != TURRET_ANIM_DEPLOY)
	{
		m_iOn = 1;
		SetTurretAnim(TURRET_ANIM_DEPLOY);
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Turret.Deploy");
		m_OnActivate.FireOutput(this, this);
	}

	if (IsSequenceFinished())
	{
		Vector curmins, curmaxs;
		curmins = WorldAlignMins();
		curmaxs = WorldAlignMaxs();

		curmaxs.z = m_iDeployHeight;
		curmins.z = -m_iDeployHeight;
		SetCollisionBounds(curmins, curmaxs);

		m_vecCurAngles.x = 0;

		if (m_iOrientation == 1)
		{
			m_vecCurAngles.y = UTIL_AngleMod(GetAbsAngles().y + 180);
		}
		else
		{
			m_vecCurAngles.y = UTIL_AngleMod(GetAbsAngles().y);
		}

		SetTurretAnim(TURRET_ANIM_SPIN);
		m_flPlaybackRate = 0;
		SetThink(&CHL1BaseTurret::SearchThink);
	}

	m_flLastSight = gpGlobals->curtime + m_flMaxWait;
}

void CHL1BaseTurret::Retire(void)
{
	m_vecGoalAngles.x = 0;
	m_vecGoalAngles.y = m_flStartYaw;

	SetNextThink(gpGlobals->curtime + 0.1);

	StudioFrameAdvance();

	EyeOff();

	if (!MoveTurret())
	{
		if (m_iSpin)
		{
			SpinDownCall();
		}
		else if (GetSequence() != TURRET_ANIM_RETIRE)
		{
			SetTurretAnim(TURRET_ANIM_RETIRE);
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Turret.Undeploy");
			m_OnDeactivate.FireOutput(this, this);
		}
		else if( GetSequence() == TURRET_ANIM_RETIRE && GetCycle() <= 0.0 )
		{
			m_iOn = 0;
			m_flLastSight = 0;

			Vector curmins, curmaxs;
			curmins = WorldAlignMins();
			curmaxs = WorldAlignMaxs();

			curmaxs.z = m_iRetractHeight;
			curmins.z = -m_iRetractHeight;
			
			SetCollisionBounds(curmins, curmaxs);
			if (m_iAutoStart)
			{
				SetThink(&CHL1BaseTurret::AutoSearchThink);
				SetNextThink(gpGlobals->curtime + .1);
			}
			else
				SetThink(&CBaseEntity::SUB_DoNothing);
		}
	}
	else
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}
}

void CHL1BaseTurret::SetTurretAnim(TURRET_ANIM anim)
{
	if (GetSequence() != anim)
	{
		switch (anim)
		{
		case TURRET_ANIM_FIRE:
		case TURRET_ANIM_SPIN:
			if (GetSequence() != TURRET_ANIM_FIRE && GetSequence() != TURRET_ANIM_SPIN)
			{
				SetCycle(0);
			}
			break;
		default:
			SetCycle(0);
			break;
		}

		SetSequence(anim);
		ResetSequenceInfo();

		switch (anim)
		{
		case TURRET_ANIM_RETIRE:
			SetCycle(255);
			m_flPlaybackRate = -1.0;
			break;
		case TURRET_ANIM_DIE:
			m_flPlaybackRate = 1.0;
			break;
		}
	}
}

void CHL1BaseTurret::SearchThink(void)
{
	SetTurretAnim(TURRET_ANIM_SPIN);
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	if (m_flSpinUpTime == 0 && m_flMaxSpin)
		m_flSpinUpTime = gpGlobals->curtime + m_flMaxSpin;

	Ping();

	CBaseEntity *pEnemy = GetEnemy();

	if (pEnemy != NULL)
	{
		if (!pEnemy->IsAlive())
			pEnemy = NULL;
	}

	if (pEnemy == NULL)
	{
		GetSenses()->Look(TURRET_RANGE);
		pEnemy = BestEnemy();

		if (pEnemy && !FVisible(pEnemy))
			pEnemy = NULL;
	}

	if (pEnemy != NULL)
	{
		m_flLastSight = 0;
		m_flSpinUpTime = 0;
		SetThink(&CHL1BaseTurret::ActiveThink);
	}
	else
	{
		if (gpGlobals->curtime > m_flLastSight)
		{
			m_flLastSight = 0;
			m_flSpinUpTime = 0;
			SetThink(&CHL1BaseTurret::Retire);
		}
		else if ((m_flSpinUpTime) && (gpGlobals->curtime > m_flSpinUpTime))
		{
			SpinDownCall();
		}

		m_vecGoalAngles.y = (m_vecGoalAngles.y + 0.1 * m_fTurnRate);
		if (m_vecGoalAngles.y >= 360)
			m_vecGoalAngles.y -= 360;
		MoveTurret();
	}

	SetEnemy(pEnemy);
}

void CHL1BaseTurret::AutoSearchThink(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.3);

	CBaseEntity *pEnemy = GetEnemy();

	if (pEnemy != NULL)
	{
		if (!pEnemy->IsAlive())
			pEnemy = NULL;
	}

	if (pEnemy == NULL)
	{
		GetSenses()->Look(TURRET_RANGE);
		pEnemy = BestEnemy();

		if (pEnemy && !FVisible(pEnemy))
			pEnemy = NULL;
	}

	if (pEnemy != NULL)
	{
		SetThink(&CHL1BaseTurret::Deploy);
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Turret.Alert");
	}

	SetEnemy(pEnemy);
}

void CHL1BaseTurret::TurretDeath(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	if (m_lifeState != LIFE_DEAD)
	{
		m_lifeState = LIFE_DEAD;

		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Turret.Die");

		StopSound(entindex(), "Turret.Spinup");

		if (m_iOrientation == 0)
			m_vecGoalAngles.x = -14;
		else
			m_vecGoalAngles.x = 85;

		SetTurretAnim(TURRET_ANIM_DIE);

		EyeOn();
	}

	EyeOff();

	if (m_flDamageTime + random->RandomFloat(0, 2) > gpGlobals->curtime)
	{
		Vector pos;
		CollisionProp()->RandomPointInBounds(vec3_origin, Vector(1, 1, 1), &pos);
		pos.z = CollisionProp()->GetCollisionOrigin().z;

		CBroadcastRecipientFilter filter;
		te->Smoke(filter, 0.0, &pos,
			g_sModelIndexSmoke,
			2.5,
			10);
	}
	
	if (m_flDamageTime + random->RandomFloat(0, 5) > gpGlobals->curtime)
	{
		Vector vecSrc;
		CollisionProp()->RandomPointInBounds(vec3_origin, Vector(1, 1, 1), &vecSrc);
		g_pEffects->Sparks(vecSrc);
	}

	if (IsSequenceFinished() && !MoveTurret() && m_flDamageTime + 5 < gpGlobals->curtime)
	{
		m_flPlaybackRate = 0;
		SetThink(NULL);
	}
}

void CHL1BaseTurret::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
	CTakeDamageInfo ainfo = info;
	
	if (ptr->hitgroup == 10)
	{
		if (m_flDamageTime != gpGlobals->curtime || (random->RandomInt(0, 10) < 1))
		{
			g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
			m_flDamageTime = gpGlobals->curtime;
		}

		ainfo.SetDamage(0.1);
	}

	if (m_takedamage == DAMAGE_NO)
		return;

	AddMultiDamage(ainfo, this);
}

int CHL1BaseTurret::OnTakeDamage(const CTakeDamageInfo &info)
{
	int retVal = 0;

	if (!m_takedamage)
		return 0;

	switch (m_lifeState)
	{
	case LIFE_ALIVE:
		retVal = OnTakeDamage_Alive(info);
		if (m_iHealth <= 0)
		{
			IPhysicsObject *pPhysics = VPhysicsGetObject();
			if (pPhysics)
			{
				pPhysics->EnableCollisions(false);
			}

			Event_Killed(info);
			Event_Dying();
		}
		return retVal;
		break;

	case LIFE_DYING:
		return OnTakeDamage_Dying(info);

	default:
	case LIFE_DEAD:
		return OnTakeDamage_Dead(info);
	}
}

int CHL1BaseTurret::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	if (m_takedamage == DAMAGE_NO)
		return 0;

	float flDamage = inputInfo.GetDamage();

	if (!m_iOn)
		flDamage /= 10.0;

	m_iHealth -= flDamage;
	if (m_iHealth <= 0)
	{
		m_iHealth = 0;
		m_takedamage = DAMAGE_NO;
		m_flDamageTime = gpGlobals->curtime;

		SetUse(NULL);
		SetThink(&CHL1BaseTurret::TurretDeath);
		SetNextThink(gpGlobals->curtime + 0.1);

		m_OnDeactivate.FireOutput(this, this);

		return 0;
	}

	if (m_iHealth <= 10)
	{
		if (m_iOn)
		{
			m_fBeserk = 1;
			SetThink(&CHL1BaseTurret::SearchThink);
		}
	}
	return 1;
}

int CHL1BaseTurret::MoveTurret(void)
{
	int state = 0;

	if (m_vecCurAngles.x != m_vecGoalAngles.x)
	{
		float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;

		m_vecCurAngles.x += 0.1 * m_fTurnRate * flDir;

		if (flDir == 1)
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		if (m_iOrientation == 0)
			SetBoneController(1, m_vecCurAngles.x);
		else
			SetBoneController(1, -m_vecCurAngles.x);
		state = 1;
	}

	if (m_vecCurAngles.y != m_vecGoalAngles.y)
	{
		float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
		float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);

		if (flDist > 180)
		{
			flDist = 360 - flDist;
			flDir = -flDir;
		}
		if (flDist > 30)
		{
			if (m_fTurnRate < m_iBaseTurnRate * 10)
			{
				m_fTurnRate += m_iBaseTurnRate;
			}
		}
		else if (m_fTurnRate > 45)
		{
			m_fTurnRate -= m_iBaseTurnRate;
		}
		else
		{
			m_fTurnRate += m_iBaseTurnRate;
		}

		m_vecCurAngles.y += 0.1 * m_fTurnRate * flDir;

		if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;

		if (flDist < (0.05 * m_iBaseTurnRate))
			m_vecCurAngles.y = m_vecGoalAngles.y;

		if (m_iOrientation == 0)
			SetBoneController(0, m_vecCurAngles.y - GetAbsAngles().y);
		else
			SetBoneController(0, GetAbsAngles().y - 180 - m_vecCurAngles.y);
		state = 1;
	}

	if (!state)
		m_fTurnRate = m_iBaseTurnRate;

	return state;
}

Class_T	CHL1BaseTurret::Classify(void)
{
	if (m_iOn || m_iAutoStart)
		return	CLASS_MACHINE;
	return CLASS_NONE;
}

void CHL1BaseTurret::InputDeactivate(inputdata_t &inputdata)
{
	if (m_iOn && m_lifeState == LIFE_ALIVE)
	{
		SetEnemy(NULL);
		SetNextThink(gpGlobals->curtime + 0.1);
		m_iAutoStart = FALSE;
		SetThink(&CHL1BaseTurret::Retire);
	}
}

void CHL1BaseTurret::InputActivate(inputdata_t &inputdata)
{
	if (!m_iOn && m_lifeState == LIFE_ALIVE)
	{
		SetNextThink(gpGlobals->curtime + 0.1);

		if (m_spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		{
			m_iAutoStart = TRUE;
		}

		SetThink(&CHL1BaseTurret::Deploy);
	}
}

ConVar	sk_turret_health("sk_turret_health", "50");
ConVar	sk_miniturret_health("sk_miniturret_health", "40");
ConVar	sk_sentry_health("sk_sentry_health", "40");

class CHL1Turret : public CHL1BaseTurret
{
public:
	DECLARE_CLASS(CHL1Turret, CHL1BaseTurret);
	DECLARE_DATADESC();
	void Spawn(void);
	void Precache(void);
	void SpinUpCall(void);
	void SpinDownCall(void);
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);

private:
	int m_iStartSpin;

};

BEGIN_DATADESC(CHL1Turret)
DEFINE_FIELD(m_iStartSpin, FIELD_INTEGER),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_turret, CHL1Turret);

void CHL1Turret::Spawn()
{
	Precache();
	SetModel("models/turret.mdl");
	m_iHealth = sk_turret_health.GetFloat();
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;

	Vector view_ofs(0, 0, 12.75);
	SetViewOffset(view_ofs);

	CHL1BaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -90;
	UTIL_SetSize(this, Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CHL1BaseTurret::Initialize);

	m_pEyeGlow = CSprite::SpriteCreate(TURRET_GLOW_SPRITE, GetAbsOrigin(), FALSE);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 0, 0, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(this, 2);

	m_eyeBrightness = 0;

	SetNextThink(gpGlobals->curtime + 0.3);
}

void CHL1Turret::Precache()
{
	CHL1BaseTurret::Precache();
	PrecacheModel("models/turret.mdl");
	PrecacheModel(TURRET_GLOW_SPRITE);
	PrecacheModel("sprites/xspark4.vmt");

	PrecacheScriptSound("Turret.Shoot");
	PrecacheScriptSound("Turret.SpinUpCall");
	PrecacheScriptSound("Turret.Spinup");
	PrecacheScriptSound("Turret.SpinDownCall");
}

void CHL1Turret::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	CPASAttenuationFilter filter(this);

	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, m_iAmmoType, 1);

	EmitSound(filter, entindex(), "Turret.Shoot");

	DoMuzzleFlash();
}

void CHL1Turret::SpinUpCall(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	if (!m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);

		if (!m_iStartSpin)
		{
			SetNextThink(gpGlobals->curtime + 1.0);
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Turret.SpinUpCall");
			m_iStartSpin = 1;
			m_flPlaybackRate = 0.1;
		}
		else if (m_flPlaybackRate >= 1.0)
		{
			SetNextThink(gpGlobals->curtime + 0.1);
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Turret.Spinup");
			SetThink(&CHL1Turret::ActiveThink);
			m_iStartSpin = 0;
			m_iSpin = 1;
		}
		else
		{
			m_flPlaybackRate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CHL1Turret::ActiveThink);
	}
}

void CHL1Turret::SpinDownCall(void)
{
	if (m_iSpin)
	{
		CPASAttenuationFilter filter(this);

		SetTurretAnim(TURRET_ANIM_SPIN);
		if (m_flPlaybackRate == 1.0)
		{
			StopSound(entindex(), "Turret.Spinup");
			EmitSound(filter, entindex(), "Turret.SpinDownCall");
		}
		m_flPlaybackRate -= 0.02;
		if (m_flPlaybackRate <= 0)
		{
			m_flPlaybackRate = 0;
			m_iSpin = 0;
		}
	}
}

class CHL1MiniTurret : public CHL1BaseTurret
{
public:
	DECLARE_CLASS(CHL1MiniTurret, CHL1BaseTurret)
	void Spawn();
	void Precache(void);
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);
};

LINK_ENTITY_TO_CLASS(monster_miniturret, CHL1MiniTurret);

void CHL1MiniTurret::Spawn()
{
	Precache();
	SetModel("models/miniturret.mdl");
	m_iHealth = sk_miniturret_health.GetFloat();
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = 0;
	Vector view_ofs(0, 0, 12.75);
	SetViewOffset(view_ofs);

	CHL1BaseTurret::Spawn();
	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	UTIL_SetSize(this, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetThink(&CHL1MiniTurret::Initialize);
	SetNextThink(gpGlobals->curtime + 0.3);
}

void CHL1MiniTurret::Precache()
{
	CHL1BaseTurret::Precache();

	m_iAmmoType = GetAmmoDef()->Index("9mmRound");

	PrecacheScriptSound("Turret.Shoot");

	PrecacheModel("models/miniturret.mdl");
}

void CHL1MiniTurret::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, m_iAmmoType, 1);

	CPASAttenuationFilter filter(this);

	EmitSound(filter, entindex(), "Turret.Shoot");

	DoMuzzleFlash();
}

class CHL1Sentry : public CHL1BaseTurret
{
public:
	DECLARE_CLASS(CHL1Sentry, CHL1BaseTurret);
	DECLARE_DATADESC();
	void Spawn();
	void Precache(void);
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);
	int OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void Event_Killed(const CTakeDamageInfo &info);
	void SentryTouch(CBaseEntity *pOther);

private:
	bool m_bStartedDeploy;
};

BEGIN_DATADESC(CHL1Sentry)
DEFINE_ENTITYFUNC(SentryTouch),
DEFINE_FIELD(m_bStartedDeploy, FIELD_BOOLEAN),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_sentry, CHL1Sentry);

void CHL1Sentry::Precache()
{
	BaseClass::Precache();

	m_iAmmoType = GetAmmoDef()->Index("9mmRound");

	PrecacheScriptSound("Sentry.Shoot");
	PrecacheScriptSound("Sentry.Die");

	PrecacheModel("models/sentry.mdl");
}

void CHL1Sentry::Spawn()
{
	Precache();
	SetModel("models/sentry.mdl");
	m_iHealth = sk_sentry_health.GetFloat();
	m_HackedGunPos = Vector(0, 0, 48);
	SetViewOffset(Vector(0, 0, 48));
	m_flMaxWait = 1E6;
	m_flMaxSpin = 1E6;

	CHL1BaseTurret::Spawn();
	m_iRetractHeight = 64;
	m_iDeployHeight = 64;
	m_iMinPitch = -60;
	UTIL_SetSize(this, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetTouch(&CHL1Sentry::SentryTouch);
	SetThink(&CHL1Sentry::Initialize);
	SetNextThink(gpGlobals->curtime + 0.3);
}

void CHL1Sentry::Shoot(Vector &vecSrc, Vector &vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, m_iAmmoType, 1);

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Sentry.Shoot");

	DoMuzzleFlash();
}

int CHL1Sentry::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	if (m_takedamage == DAMAGE_NO)
		return 0;

	if (!m_iOn && !m_bStartedDeploy)
	{
		m_bStartedDeploy = true;
		SetThink(&CHL1Sentry::Deploy);
		SetUse(NULL);
		SetNextThink(gpGlobals->curtime + 0.1);
	}

	m_iHealth -= info.GetDamage();
	if (m_iHealth <= 0)
	{
		m_iHealth = 0;
		m_takedamage = DAMAGE_NO;
		m_flDamageTime = gpGlobals->curtime;

		SetUse(NULL);
		SetThink(&CHL1BaseTurret::TurretDeath);
		SetNextThink(gpGlobals->curtime + 0.1);

		m_OnDeactivate.FireOutput(this, this);

		return 0;
	}

	return 1;
}

void CHL1Sentry::SentryTouch(CBaseEntity *pOther)
{
	if (pOther && (pOther->IsPlayer() || FBitSet(pOther->GetFlags(), FL_NPC)))
	{
		CTakeDamageInfo info;
		info.SetAttacker(pOther);
		info.SetInflictor(pOther);
		info.SetDamage(0);

		TakeDamage(info);
	}
}

void CHL1Sentry::Event_Killed(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Sentry.Die");

	StopSound(entindex(), "Turret.Spinup");

	AddSolidFlags(FSOLID_NOT_STANDABLE);

	Vector vecSrc;
	QAngle vecAng;
	GetAttachment(2, vecSrc, vecAng);

	te->Smoke(filter, 0.0, &vecSrc,
		g_sModelIndexSmoke,
		2.5,
		10);

	g_pEffects->Sparks(vecSrc);

	BaseClass::Event_Killed(info);
}