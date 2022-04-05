#include "cbase.h"
#include "ai_basenpc.h"
#include "trains.h"
#include "ndebugoverlay.h"
#include "entitylist.h"
#include "engine/IEngineSound.h"
#include "doors.h"
#include "soundent.h"
#include "gamerules.h"
#include "hl1_basegrenade.h"
#include "shake.h"
#include "globalstate.h"
#include "player.h"
#include "soundscape.h"
#include "buttons.h"
#include "Sprite.h"
#include "items.h"
#include "hl1_items.h"
#include "actanimating.h"
#include "npcevent.h"
#include "func_break.h"
#include "eventqueue.h"
#include "triggers.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "grenade_beam.h"
#include "EnvLaser.h"
#include "explode.h"
#include "physics_cannister.h"

//---------------------------------
//			Pendulum
//---------------------------------

#define SF_PENDULUM_SWING 2

class CPendulum : public CBaseToggle
{
	DECLARE_CLASS(CPendulum, CBaseToggle);
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData *pkvd);
	void	Swing(void);
	void	PendulumUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void	Stop(void);
	void	Touch(CBaseEntity *pOther);
	void	RopeTouch(CBaseEntity *pOther);
	void	Blocked(CBaseEntity *pOther);

	void	InputActivate(inputdata_t &inputdata);

	DECLARE_DATADESC();

	float	m_flAccel;
	float	m_flTime;
	float	m_flDamp;
	float	m_flMaxSpeed;
	float	m_flDampSpeed;
	QAngle	m_vCenter;
	QAngle	m_vStart;
	float   m_flBlockDamage;

	EHANDLE		m_hEnemy;
};

LINK_ENTITY_TO_CLASS(func_pendulum, CPendulum);

BEGIN_DATADESC(CPendulum)
DEFINE_FIELD(m_flAccel, FIELD_FLOAT),
DEFINE_FIELD(m_flTime, FIELD_TIME),
DEFINE_FIELD(m_flMaxSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flDampSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_vCenter, FIELD_VECTOR),
DEFINE_FIELD(m_vStart, FIELD_VECTOR),

DEFINE_KEYFIELD(m_flMoveDistance, FIELD_FLOAT, "pendistance"),
DEFINE_KEYFIELD(m_flDamp, FIELD_FLOAT, "damp"),
DEFINE_KEYFIELD(m_flBlockDamage, FIELD_FLOAT, "dmg"),

DEFINE_FUNCTION(PendulumUse),
DEFINE_FUNCTION(Swing),
DEFINE_FUNCTION(Stop),
DEFINE_FUNCTION(RopeTouch),

DEFINE_FIELD(m_hEnemy, FIELD_EHANDLE),

DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
END_DATADESC()

void CPendulum::Spawn(void)
{
	CBaseToggle::AxisDir();

	m_flDamp *= 0.001;

	if (FBitSet(m_spawnflags, SF_DOOR_PASSABLE))
		SetSolid(SOLID_NONE);
	else
		SetSolid(SOLID_BBOX);

	SetMoveType(MOVETYPE_PUSH);
	SetModel(STRING(GetModelName()));

	if (m_flMoveDistance != 0)
	{
		if (m_flSpeed == 0)
			m_flSpeed = 100;

		m_flAccel = (m_flSpeed * m_flSpeed) / (2 * fabs(m_flMoveDistance));
		m_flMaxSpeed = m_flSpeed;
		m_vStart = GetAbsAngles();
		m_vCenter = GetAbsAngles() + (m_flMoveDistance * 0.05) * m_vecMoveAng;

		if (FBitSet(m_spawnflags, SF_BRUSH_ROTATE_START_ON))
		{
			SetThink(&CBaseEntity::SUB_CallUseToggle);
			SetNextThink(gpGlobals->curtime + 0.1f);
		}

		m_flSpeed = 0;
		SetUse(&CPendulum::PendulumUse);

		VPhysicsInitShadow(false, false);
	}

	if (FBitSet(m_spawnflags, SF_PENDULUM_SWING))
	{
		SetTouch(&CPendulum::RopeTouch);
	}
}

void CPendulum::PendulumUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_flSpeed)
	{
		if (FBitSet(m_spawnflags, SF_BRUSH_ROTATE_START_ON))
		{
			float	delta;

			delta = CBaseToggle::AxisDelta(m_spawnflags, GetAbsAngles(), m_vStart);

			SetLocalAngularVelocity(m_flMaxSpeed * m_vecMoveAng);
			SetNextThink(gpGlobals->curtime + delta / m_flMaxSpeed);
			SetThink(&CPendulum::Stop);
		}
		else
		{
			m_flSpeed = 0;
			SetThink(NULL);
			SetLocalAngularVelocity(QAngle(0, 0, 0));
		}
	}
	else
	{
		SetNextThink(gpGlobals->curtime + 0.1f);
		m_flTime = gpGlobals->curtime;
		SetThink(&CPendulum::Swing);
		m_flDampSpeed = m_flMaxSpeed;
	}
}

void CPendulum::InputActivate(inputdata_t &inputdata)
{
	SetNextThink(gpGlobals->curtime + 0.1f);
	m_flTime = gpGlobals->curtime;
	SetThink(&CPendulum::Swing);
	m_flDampSpeed = m_flMaxSpeed;
}

void CPendulum::Stop(void)
{
	SetAbsAngles(m_vStart);
	m_flSpeed = 0;
	SetThink(NULL);
	SetLocalAngularVelocity(QAngle(0, 0, 0));
}


void CPendulum::Blocked(CBaseEntity *pOther)
{
	m_flTime = gpGlobals->curtime;
}

void CPendulum::Swing(void)
{
	float delta, dt;

	delta = CBaseToggle::AxisDelta(m_spawnflags, GetAbsAngles(), m_vCenter);
	dt = gpGlobals->curtime - m_flTime;
	m_flTime = gpGlobals->curtime;

	if (delta > 0 && m_flAccel > 0)
		m_flSpeed -= m_flAccel * dt;
	else
		m_flSpeed += m_flAccel * dt;

	if (m_flSpeed > m_flMaxSpeed)
		m_flSpeed = m_flMaxSpeed;
	else if (m_flSpeed < -m_flMaxSpeed)
		m_flSpeed = -m_flMaxSpeed;

	SetLocalAngularVelocity(m_flSpeed * m_vecMoveAng);

	SetNextThink(gpGlobals->curtime + 0.1f);
	SetMoveDoneTime(0.1);

	if (m_flDamp)
	{
		m_flDampSpeed -= m_flDamp * m_flDampSpeed * dt;
		if (m_flDampSpeed < 30.0)
		{
			SetAbsAngles(m_vCenter);
			m_flSpeed = 0;
			SetThink(NULL);
			SetLocalAngularVelocity(QAngle(0, 0, 0));
		}
		else if (m_flSpeed > m_flDampSpeed)
			m_flSpeed = m_flDampSpeed;
		else if (m_flSpeed < -m_flDampSpeed)
			m_flSpeed = -m_flDampSpeed;
	}
}

void CPendulum::Touch(CBaseEntity *pOther)
{
	if (m_flBlockDamage <= 0)
		return;

	if (!pOther->m_takedamage)
		return;

	float damage = m_flBlockDamage * m_flSpeed * 0.01;

	if (damage < 0)
		damage = -damage;

	pOther->TakeDamage(CTakeDamageInfo(this, this, damage, DMG_CRUSH));

	Vector vNewVel = (pOther->GetAbsOrigin() - GetAbsOrigin());

	VectorNormalize(vNewVel);

	pOther->SetAbsVelocity(vNewVel * damage);
}

void CPendulum::RopeTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
	{
		DevMsg(2, "Not a client\n");
		return;
	}

	if (pOther == GetEnemy())
		return;

	m_hEnemy = pOther;
	pOther->SetAbsVelocity(Vector(0, 0, 0));
	pOther->SetMoveType(MOVETYPE_NONE);
}

//---------------------------------
//			Mortar
//---------------------------------

class CFuncMortarField : public CBaseToggle
{
	DECLARE_CLASS(CFuncMortarField, CBaseToggle)
public:
	void Spawn(void);
	void Precache(void);

	virtual int	ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void InputTrigger(inputdata_t &inputdata);

	DECLARE_DATADESC();

private:
	string_t m_iszXController;
	string_t m_iszYController;
	float m_flSpread;
	float m_flDelay;
	int m_iCount;
	int m_fControl;
};

LINK_ENTITY_TO_CLASS(func_mortar_field, CFuncMortarField);

BEGIN_DATADESC(CFuncMortarField)
DEFINE_KEYFIELD(m_iszXController, FIELD_STRING, "m_iszXController"),
DEFINE_KEYFIELD(m_iszYController, FIELD_STRING, "m_iszYController"),
DEFINE_KEYFIELD(m_flSpread, FIELD_FLOAT, "m_flSpread"),
DEFINE_KEYFIELD(m_iCount, FIELD_INTEGER, "m_iCount"),
DEFINE_KEYFIELD(m_fControl, FIELD_INTEGER, "m_fControl"),

DEFINE_INPUTFUNC(FIELD_VOID, "Trigger", InputTrigger),
END_DATADESC()

void CFuncMortarField::Spawn(void)
{
	SetSolid(SOLID_NONE);
	SetModel(STRING(GetModelName()));
	SetMoveType(MOVETYPE_NONE);
	AddEffects(EF_NODRAW);
	Precache();
}


void CFuncMortarField::Precache(void)
{
	PrecacheModel("sprites/lgtning.vmt");

	PrecacheScriptSound("MortarField.Trigger");
}

void CFuncMortarField::InputTrigger(inputdata_t &inputdata)
{
	Vector vecStart;
	CollisionProp()->RandomPointInBounds(Vector(0, 0, 1), Vector(1, 1, 1), &vecStart);

	switch (m_fControl)
	{
	case 0:
		break;
	case 1:
		if (inputdata.pActivator != NULL)
		{
			vecStart.x = inputdata.pActivator->GetAbsOrigin().x;
			vecStart.y = inputdata.pActivator->GetAbsOrigin().y;
		}
		break;
	case 2:
	{
		CBaseEntity* pController;

		if (m_iszXController != NULL_STRING)
		{
			pController = gEntList.FindEntityByName(NULL, STRING(m_iszXController));
			if (pController != NULL)
			{
				if (FClassnameIs(pController, "momentary_rot_button"))
				{
					CMomentaryRotButton* pXController = static_cast<CMomentaryRotButton*>(pController);
					Vector vecNormalizedPos(pXController->GetPos(pXController->GetLocalAngles()), 0.0f, 0.0f);
					Vector vecWorldSpace;
					CollisionProp()->NormalizedToWorldSpace(vecNormalizedPos, &vecWorldSpace);
					vecStart.x = vecWorldSpace.x;
				}
				else
				{
					DevMsg("func_mortarfield has X controller that isn't a momentary_rot_button.\n");
				}
			}
		}
		if (m_iszYController != NULL_STRING)
		{
			pController = gEntList.FindEntityByName(NULL, STRING(m_iszYController));
			if (pController != NULL)
			{
				if (FClassnameIs(pController, "momentary_rot_button"))
				{
					CMomentaryRotButton* pYController = static_cast<CMomentaryRotButton*>(pController);
					Vector vecNormalizedPos(0.0f, pYController->GetPos(pYController->GetLocalAngles()), 0.0f);
					Vector vecWorldSpace;
					CollisionProp()->NormalizedToWorldSpace(vecNormalizedPos, &vecWorldSpace);
					vecStart.y = vecWorldSpace.y;
				}
				else
				{
					DevMsg("func_mortarfield has Y controller that isn't a momentary_rot_button.\n");
				}
			}
		}
	}
	break;
	}

	CPASAttenuationFilter filter(this, ATTN_NONE);
	EmitSound(filter, entindex(), "MortarField.Trigger");

	float t = 2.5;
	for (int i = 0; i < m_iCount; i++)
	{
		Vector vecSpot = vecStart;
		vecSpot.x += random->RandomFloat(-m_flSpread, m_flSpread);
		vecSpot.y += random->RandomFloat(-m_flSpread, m_flSpread);

		trace_t tr;
		UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -1) * MAX_TRACE_LENGTH, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

		CBaseEntity* pMortar = Create("monster_mortar", tr.endpos, QAngle(0, 0, 0), inputdata.pActivator);
		pMortar->SetNextThink(gpGlobals->curtime + t);
		t += random->RandomFloat(0.2, 0.5);

		if (i == 0)
			CSoundEnt::InsertSound(SOUND_DANGER, tr.endpos, 400, 0.3);
	}
}

#define DMG_MISSILEDEFENSE	(DMG_LASTGENERICFLAG<<2)

class CMortar : public CHL1BaseGrenade
{
	DECLARE_CLASS(CMortar, CHL1BaseGrenade);
public:
	void Spawn(void);
	void Precache(void);

	void MortarExplode(void);

	int m_spriteTexture;

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CMortar)
DEFINE_THINKFUNC(MortarExplode),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_mortar, CMortar);

void CMortar::Spawn()
{
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

	SetDamage(200);
	SetDamageRadius(GetDamage() * 2.5);

	SetThink(&CMortar::MortarExplode);
	SetNextThink(TICK_NEVER_THINK);

	Precache();
}


void CMortar::Precache()
{
	m_spriteTexture = PrecacheModel("sprites/lgtning.vmt");
}

void CMortar::MortarExplode(void)
{
	Vector vecStart = GetAbsOrigin();
	Vector vecEnd = vecStart;
	vecEnd.z += 1024;

	UTIL_Beam(vecStart, vecEnd, m_spriteTexture, 0, 0, 0, 0.5, 4.0, 4.0, 100, 0, 255, 160, 100, 128, 0);

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin() + Vector(0, 0, 1024), GetAbsOrigin() - Vector(0, 0, 1024), MASK_ALL, this, COLLISION_GROUP_NONE, &tr);


	Explode(&tr, DMG_BLAST | DMG_MISSILEDEFENSE);
	UTIL_ScreenShake(tr.endpos, 25.0, 150.0, 1.0, 750, SHAKE_START);
}

//---------------------------------------
//            Func Tank
//---------------------------------------

#define SF_TANK_ACTIVE			0x0001
#define SF_TANK_PLAYER			0x0002
#define SF_TANK_HUMANS			0x0004
#define SF_TANK_ALIENS			0x0008
#define SF_TANK_LINEOFSIGHT		0x0010
#define SF_TANK_CANCONTROL		0x0020
#define	SF_TANK_DAMAGE_KICK		0x0040
#define	SF_TANK_AIM_AT_POS		0x0080
#define SF_TANK_SOUNDON			0x8000

#define FUNCTANK_FIREVOLUME	1000

#define MAX_FIRING_SPREADS ARRAYSIZE(gTankSpread)

static Vector gTankSpread[] =
{
	Vector(0, 0, 0),
	Vector(0.025, 0.025, 0.025),
	Vector(0.05, 0.05, 0.05),
	Vector(0.1, 0.1, 0.1),
	Vector(0.25, 0.25, 0.25),
};

enum TANKBULLET
{
	TANK_BULLET_NONE = 0,
	TANK_BULLET_SMALL = 1,
	TANK_BULLET_MEDIUM = 2,
	TANK_BULLET_LARGE = 3,
};

class CHL1FuncTank : public CBaseEntity
{
	DECLARE_CLASS(CHL1FuncTank, CBaseEntity);
public:
	~CHL1FuncTank(void);
	void	Spawn(void);
	void	Activate(void);
	void	Precache(void);
	bool	CreateVPhysics(void);
	bool	KeyValue(const char* szKeyName, const char* szValue);

	int		ObjectCaps(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	Think(void);
	void	TrackTarget(void);
	int		DrawDebugTextOverlays(void);

	virtual void FiringSequence(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
	virtual void Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);

	void	StartRotSound(void);
	void	StopRotSound(void);

	inline bool IsActive(void) { return (m_spawnflags & SF_TANK_ACTIVE) ? TRUE : FALSE; }

	void InputActivate(inputdata_t& inputdata);
	void InputDeactivate(inputdata_t& inputdata);
	void InputSetFireRate(inputdata_t& inputdata);
	void InputSetTargetPosition(inputdata_t& inputdata);
	void InputSetTargetEntityName(inputdata_t& inputdata);
	void InputSetTargetEntity(inputdata_t& inputdata);

	void TankActivate(void);
	void TankDeactivate(void);

	inline bool CanFire(void);
	bool		InRange(float range);

	void		TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, trace_t& tr);
	void		TraceAttack(CBaseEntity* pAttacker, float flDamage, const Vector& vecDir, trace_t* ptr, int bitsDamageType);

	Vector		WorldBarrelPosition(void)
	{
		EntityMatrix tmp;
		tmp.InitFromEntity(this);
		return tmp.LocalToWorld(m_barrelPos);
	}

	void		UpdateMatrix(void)
	{
		m_parentMatrix.InitFromEntity(GetParent() ? GetParent() : NULL);
	}
	QAngle		AimBarrelAt(const Vector& parentTarget);

	bool	ShouldSavePhysics() { return false; }

	DECLARE_DATADESC();

	bool OnControls(CBaseEntity* pTest);
	bool StartControl(CBasePlayer* pController);
	void StopControl(void);
	void ControllerPostFrame(void);

	CBaseEntity* FindTarget(string_t targetName, CBaseEntity* pActivator);

protected:
	CBasePlayer* m_pController;
	float			m_flNextAttack;
	Vector			m_vecControllerUsePos;

	float			m_yawCenter;
	float			m_yawRate;
	float			m_yawRange;
	float			m_yawTolerance;

	float			m_pitchCenter;
	float			m_pitchRate;
	float			m_pitchRange;
	float			m_pitchTolerance;
	float			m_fireLast;
	float			m_fireRate;
	float			m_lastSightTime;
	float			m_persist;
	float			m_persist2;
	float			m_persist2burst;
	float			m_minRange;
	float			m_maxRange;
	Vector			m_barrelPos;
	float			m_spriteScale;
	string_t		m_iszSpriteSmoke;
	string_t		m_iszSpriteFlash;
	TANKBULLET		m_bulletType;
	int				m_iBulletDamage;
	Vector			m_sightOrigin;
	int				m_spread;
	string_t		m_iszMaster;
	int				m_iSmallAmmoType;
	int				m_iMediumAmmoType;
	int				m_iLargeAmmoType;

	string_t		m_soundStartRotate;
	string_t		m_soundStopRotate;
	string_t		m_soundLoopRotate;

	string_t		m_targetEntityName;
	EHANDLE			m_hTarget;
	Vector			m_vTargetPosition;
	EntityMatrix	m_parentMatrix;

	COutputEvent	m_OnFire;
	COutputEvent	m_OnLoseTarget;
	COutputEvent	m_OnAquireTarget;

	CHandle<CBaseTrigger>	m_hControlVolume;
	string_t				m_iszControlVolume;
};

BEGIN_DATADESC(CHL1FuncTank)
DEFINE_KEYFIELD(m_yawRate, FIELD_FLOAT, "yawrate"),
DEFINE_KEYFIELD(m_yawRange, FIELD_FLOAT, "yawrange"),
DEFINE_KEYFIELD(m_yawTolerance, FIELD_FLOAT, "yawtolerance"),
DEFINE_KEYFIELD(m_pitchRate, FIELD_FLOAT, "pitchrate"),
DEFINE_KEYFIELD(m_pitchRange, FIELD_FLOAT, "pitchrange"),
DEFINE_KEYFIELD(m_pitchTolerance, FIELD_FLOAT, "pitchtolerance"),
DEFINE_KEYFIELD(m_fireRate, FIELD_FLOAT, "firerate"),
DEFINE_KEYFIELD(m_persist, FIELD_FLOAT, "persistence"),
DEFINE_KEYFIELD(m_persist2, FIELD_FLOAT, "persistence2"),
DEFINE_KEYFIELD(m_minRange, FIELD_FLOAT, "minRange"),
DEFINE_KEYFIELD(m_maxRange, FIELD_FLOAT, "maxRange"),
DEFINE_KEYFIELD(m_spriteScale, FIELD_FLOAT, "spritescale"),
DEFINE_KEYFIELD(m_iszSpriteSmoke, FIELD_STRING, "spritesmoke"),
DEFINE_KEYFIELD(m_iszSpriteFlash, FIELD_STRING, "spriteflash"),
DEFINE_KEYFIELD(m_bulletType, FIELD_INTEGER, "bullet"),
DEFINE_KEYFIELD(m_spread, FIELD_INTEGER, "firespread"),
DEFINE_KEYFIELD(m_iBulletDamage, FIELD_INTEGER, "bullet_damage"),
DEFINE_KEYFIELD(m_iszMaster, FIELD_STRING, "master"),
DEFINE_KEYFIELD(m_soundStartRotate, FIELD_SOUNDNAME, "rotatestartsound"),
DEFINE_KEYFIELD(m_soundStopRotate, FIELD_SOUNDNAME, "rotatestopsound"),
DEFINE_KEYFIELD(m_soundLoopRotate, FIELD_SOUNDNAME, "rotatesound"),
DEFINE_KEYFIELD(m_iszControlVolume, FIELD_STRING, "control_volume"),
DEFINE_FIELD(m_yawCenter, FIELD_FLOAT),
DEFINE_FIELD(m_pitchCenter, FIELD_FLOAT),
DEFINE_FIELD(m_fireLast, FIELD_TIME),
DEFINE_FIELD(m_lastSightTime, FIELD_TIME),
DEFINE_FIELD(m_barrelPos, FIELD_VECTOR),
DEFINE_FIELD(m_sightOrigin, FIELD_VECTOR),
DEFINE_FIELD(m_pController, FIELD_CLASSPTR),
DEFINE_FIELD(m_vecControllerUsePos, FIELD_VECTOR),
DEFINE_FIELD(m_flNextAttack, FIELD_TIME),
DEFINE_FIELD(m_targetEntityName, FIELD_STRING),
DEFINE_FIELD(m_vTargetPosition, FIELD_VECTOR),
DEFINE_FIELD(m_persist2burst, FIELD_FLOAT),
DEFINE_FIELD(m_hTarget, FIELD_EHANDLE),
DEFINE_FIELD(m_hControlVolume, FIELD_EHANDLE),

DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Deactivate", InputDeactivate),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetFireRate", InputSetFireRate),
DEFINE_INPUTFUNC(FIELD_VECTOR, "SetTargetPosition", InputSetTargetPosition),
DEFINE_INPUTFUNC(FIELD_STRING, "SetTargetEntityName", InputSetTargetEntityName),
DEFINE_INPUTFUNC(FIELD_EHANDLE, "SetTargetEntity", InputSetTargetEntity),

DEFINE_OUTPUT(m_OnFire, "OnFire"),
DEFINE_OUTPUT(m_OnLoseTarget, "OnLoseTarget"),
DEFINE_OUTPUT(m_OnAquireTarget, "OnAquireTarget"),
END_DATADESC()

CHL1FuncTank::~CHL1FuncTank(void)
{
	if (m_soundLoopRotate != NULL_STRING && (m_spawnflags & SF_TANK_SOUNDON))
	{
		StopSound(entindex(), CHAN_STATIC, STRING(m_soundLoopRotate));
	}
}


int	CHL1FuncTank::ObjectCaps(void)
{
	int iCaps = BaseClass::ObjectCaps();

	if (m_spawnflags & SF_TANK_CANCONTROL)
	{
		iCaps |= FCAP_IMPULSE_USE;
	}

	return iCaps;
}

inline bool CHL1FuncTank::CanFire(void)
{
	float flTimeDelay = gpGlobals->curtime - m_lastSightTime;
	if (flTimeDelay <= m_persist)
	{
		return true;
	}
	else if (flTimeDelay <= m_persist2burst)
	{
		return true;
	}
	else if (flTimeDelay <= m_persist2)
	{
		if (random->RandomInt(0, 30) == 0)
		{
			m_persist2burst = flTimeDelay + 0.5;
			return true;
		}
	}
	return false;
}

void CHL1FuncTank::InputActivate(inputdata_t& inputdata)
{
	TankActivate();
}

void CHL1FuncTank::TankActivate(void)
{
	m_spawnflags |= SF_TANK_ACTIVE;
	SetNextThink(gpGlobals->curtime + 0.1f);
	m_fireLast = 0;
}

void CHL1FuncTank::InputDeactivate(inputdata_t& inputdata)
{
	TankDeactivate();
}

void CHL1FuncTank::TankDeactivate(void)
{
	m_spawnflags &= ~SF_TANK_ACTIVE;
	m_fireLast = 0;
	StopRotSound();
}

void CHL1FuncTank::InputSetTargetEntityName(inputdata_t& inputdata)
{
	m_targetEntityName = inputdata.value.StringID();
	m_hTarget = FindTarget(m_targetEntityName, inputdata.pActivator);

	m_spawnflags &= ~SF_TANK_AIM_AT_POS;
}

void CHL1FuncTank::InputSetTargetEntity(inputdata_t& inputdata)
{
	if (inputdata.value.Entity() != NULL)
	{
		m_targetEntityName = inputdata.value.Entity()->GetEntityName();
	}
	else
	{
		m_targetEntityName = NULL_STRING;
	}
	m_hTarget = inputdata.value.Entity();

	m_spawnflags &= ~SF_TANK_AIM_AT_POS;
}

void CHL1FuncTank::InputSetFireRate(inputdata_t& inputdata)
{
	m_fireRate = inputdata.value.Float();
}

void CHL1FuncTank::InputSetTargetPosition(inputdata_t& inputdata)
{
	m_spawnflags |= SF_TANK_AIM_AT_POS;
	m_hTarget = NULL;

	inputdata.value.Vector3D(m_vTargetPosition);
}

int CHL1FuncTank::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[255];
		if (IsActive())
		{
			Q_strncpy(tempstr, "State: Active", sizeof(tempstr));
		}
		else
		{
			Q_strncpy(tempstr, "State: Inactive", sizeof(tempstr));
		}
		EntityText(text_offset, tempstr, 0);
		text_offset++;

		Q_snprintf(tempstr, sizeof(tempstr), "Fire Rate: %f", m_fireRate);

		EntityText(text_offset, tempstr, 0);
		text_offset++;

		if (m_hTarget != NULL)
		{
			Q_snprintf(tempstr, sizeof(tempstr), "Target: %s", m_hTarget->GetDebugName());
		}
		else
		{
			Q_snprintf(tempstr, sizeof(tempstr), "Target:   -  ");
		}
		EntityText(text_offset, tempstr, 0);
		text_offset++;

		if (m_spawnflags & SF_TANK_AIM_AT_POS)
		{
			Q_snprintf(tempstr, sizeof(tempstr), "Aim Pos: %3.0f %3.0f %3.0f", m_vTargetPosition.x, m_vTargetPosition.y, m_vTargetPosition.z);
		}
		else
		{
			Q_snprintf(tempstr, sizeof(tempstr), "Aim Pos:    -  ");
		}
		EntityText(text_offset, tempstr, 0);
		text_offset++;

	}
	return text_offset;
}

void CHL1FuncTank::TraceAttack(CBaseEntity* pAttacker, float flDamage, const Vector& vecDir, trace_t* ptr, int bitsDamageType)
{
	if (m_spawnflags & SF_TANK_DAMAGE_KICK)
	{
		if (pAttacker)
		{
			Vector vFromAttacker = (pAttacker->EyePosition() - GetAbsOrigin());
			vFromAttacker.z = 0;
			VectorNormalize(vFromAttacker);

			Vector vFromAttacker2 = (ptr->endpos - GetAbsOrigin());
			vFromAttacker2.z = 0;
			VectorNormalize(vFromAttacker2);


			Vector vCrossProduct;
			CrossProduct(vFromAttacker, vFromAttacker2, vCrossProduct);
			Msg("%f\n", vCrossProduct.z);
			QAngle angles;
			angles = GetLocalAngles();
			if (vCrossProduct.z > 0)
			{
				angles.y += 10;
			}
			else
			{
				angles.y -= 10;
			}

			if (angles.y > m_yawCenter + m_yawRange)
			{
				angles.y = m_yawCenter + m_yawRange;
			}
			else if (angles.y < (m_yawCenter - m_yawRange))
			{
				angles.y = (m_yawCenter - m_yawRange);
			}

			SetLocalAngles(angles);
		}
	}
}

CBaseEntity* CHL1FuncTank::FindTarget(string_t targetName, CBaseEntity* pActivator)
{
	return gEntList.FindEntityGenericNearest(STRING(targetName), GetAbsOrigin(), 0, this, pActivator);
}

bool CHL1FuncTank::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "barrel"))
	{
		m_barrelPos.x = atof(szValue);
	}
	else if (FStrEq(szKeyName, "barrely"))
	{
		m_barrelPos.y = atof(szValue);
	}
	else if (FStrEq(szKeyName, "barrelz"))
	{
		m_barrelPos.z = atof(szValue);
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

void CHL1FuncTank::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_PUSH);
	SetSolid(SOLID_VPHYSICS);
	SetModel(STRING(GetModelName()));

	m_yawCenter = GetLocalAngles().y;
	m_pitchCenter = GetLocalAngles().x;
	m_vTargetPosition = vec3_origin;

	if (IsActive())
		SetNextThink(gpGlobals->curtime + 1.0f);

	UpdateMatrix();

	m_sightOrigin = WorldBarrelPosition();

	if (m_spread > MAX_FIRING_SPREADS)
	{
		m_spread = 0;
	}

	m_spawnflags &= ~SF_TANK_AIM_AT_POS;

	if (m_spawnflags & SF_TANK_DAMAGE_KICK)
	{
		m_takedamage = DAMAGE_YES;
	}

	CreateVPhysics();
}

void CHL1FuncTank::Activate(void)
{
	BaseClass::Activate();

	m_hControlVolume = NULL;

	if (m_iszControlVolume != NULL_STRING)
	{
		m_hControlVolume = dynamic_cast<CBaseTrigger*>(gEntList.FindEntityByName(NULL, m_iszControlVolume));
	}

	if ((!m_hControlVolume) && (m_spawnflags & SF_TANK_CANCONTROL))
	{
		Msg("ERROR: Couldn't find control volume for player-controllable func_tank %s.\n", STRING(GetEntityName()));
		UTIL_Remove(this);
	}
}

bool CHL1FuncTank::CreateVPhysics()
{
	VPhysicsInitShadow(false, false);
	return true;
}


void CHL1FuncTank::Precache(void)
{
	m_iSmallAmmoType = GetAmmoDef()->Index("9mmRound");
	m_iMediumAmmoType = GetAmmoDef()->Index("9mmRound");
	m_iLargeAmmoType = GetAmmoDef()->Index("12mmRound");

	if (m_iszSpriteSmoke != NULL_STRING)
		PrecacheModel(STRING(m_iszSpriteSmoke));
	if (m_iszSpriteFlash != NULL_STRING)
		PrecacheModel(STRING(m_iszSpriteFlash));

	if (m_soundStartRotate != NULL_STRING)
		PrecacheScriptSound(STRING(m_soundStartRotate));
	if (m_soundStopRotate != NULL_STRING)
		PrecacheScriptSound(STRING(m_soundStopRotate));
	if (m_soundLoopRotate != NULL_STRING)
		PrecacheScriptSound(STRING(m_soundLoopRotate));
}

bool CHL1FuncTank::OnControls(CBaseEntity* pTest)
{
	if (!(m_spawnflags & SF_TANK_CANCONTROL))
		return FALSE;

	Vector offset = pTest->GetLocalOrigin() - GetLocalOrigin();

	if ((m_vecControllerUsePos - pTest->GetLocalOrigin()).Length() < 30)
		return TRUE;

	return FALSE;
}

bool CHL1FuncTank::StartControl(CBasePlayer* pController)
{
	if (m_pController != NULL)
		return FALSE;

	if (m_iszMaster != NULL_STRING)
	{
		if (!UTIL_IsMasterTriggered(m_iszMaster, pController))
			return FALSE;
	}

	m_pController = pController;
	if (m_pController->GetActiveWeapon())
	{
		m_pController->GetActiveWeapon()->Holster();
	}

	m_pController->m_Local.m_iHideHUD |= HIDEHUD_WEAPONSELECTION;
	m_vecControllerUsePos = m_pController->GetLocalOrigin();

	SetNextThink(gpGlobals->curtime + 0.1f);

	return TRUE;
}

void CHL1FuncTank::StopControl()
{
	if (!m_pController)
		return;

	if (m_pController->GetActiveWeapon())
		m_pController->GetActiveWeapon()->Deploy();

	m_pController->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPONSELECTION;
	SetNextThink(TICK_NEVER_THINK);
	m_pController = NULL;

	if (IsActive())
		SetNextThink(gpGlobals->curtime + 1.0f);
}

void CHL1FuncTank::ControllerPostFrame(void)
{
	Assert(m_pController != NULL);

	if (gpGlobals->curtime < m_flNextAttack)
		return;

	Vector forward;
	AngleVectors(GetAbsAngles(), &forward);
	if (m_pController->m_nButtons & IN_ATTACK)
	{
		m_fireLast = gpGlobals->curtime - (1 / m_fireRate) - 0.01;

		int bulletCount = (gpGlobals->curtime - m_fireLast) * m_fireRate;

		Fire(bulletCount, WorldBarrelPosition(), forward, m_pController);

		CSoundEnt::InsertSound(SOUND_COMBAT, WorldSpaceCenter(), FUNCTANK_FIREVOLUME, 0.2);

		m_flNextAttack = gpGlobals->curtime + (1 / m_fireRate);
	}
}


void CHL1FuncTank::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_spawnflags & SF_TANK_CANCONTROL)
	{
		CBasePlayer* pPlayer = ToBasePlayer(pActivator);
		if (!pPlayer)
			return;

		if (value == 2 && useType == USE_SET)
		{
			ControllerPostFrame();
		}
		else if (!m_pController && useType != USE_OFF)
		{
			Assert(m_hControlVolume);
			if (!m_hControlVolume->IsTouching(pPlayer))
				return;

			pPlayer->SetUseEntity(this);
			StartControl(pPlayer);
		}
		else
		{
			StopControl();
		}
	}
	else
	{
		if (!ShouldToggle(useType, IsActive()))
			return;

		if (IsActive())
		{
			TankDeactivate();
		}
		else
		{
			TankActivate();
		}
	}
}


bool CHL1FuncTank::InRange(float range)
{
	if (range < m_minRange)
		return FALSE;
	if (m_maxRange > 0 && range > m_maxRange)
		return FALSE;

	return TRUE;
}

void CHL1FuncTank::Think(void)
{
	UpdateMatrix();

	SetLocalAngularVelocity(vec3_angle);
	TrackTarget();

	if (fabs(GetLocalAngularVelocity().x) > 1 || fabs(GetLocalAngularVelocity().y) > 1)
		StartRotSound();
	else
		StopRotSound();
}

QAngle CHL1FuncTank::AimBarrelAt(const Vector& parentTarget)
{
	Vector target = parentTarget - GetLocalOrigin();
	float quadTarget = target.LengthSqr();
	float quadTargetXY = target.x * target.x + target.y * target.y;

	if (quadTarget <= m_barrelPos.LengthSqr())
	{
		return GetLocalAngles();
	}
	else
	{
		float targetToCenterYaw = atan2(target.y, target.x);
		float centerToGunYaw = atan2(m_barrelPos.y, sqrt(quadTarget - (m_barrelPos.y * m_barrelPos.y)));

		float targetToCenterPitch = atan2(target.z, sqrt(quadTargetXY));
		float centerToGunPitch = atan2(-m_barrelPos.z, sqrt(quadTarget - (m_barrelPos.z * m_barrelPos.z)));
		return QAngle(-RAD2DEG(targetToCenterPitch + centerToGunPitch), RAD2DEG(targetToCenterYaw - centerToGunYaw), 0);
	}
}


void CHL1FuncTank::TrackTarget(void)
{
	trace_t tr;
	bool updateTime = FALSE, lineOfSight;
	QAngle angles;
	Vector barrelEnd;
	CBaseEntity* pTarget = NULL;

	barrelEnd.Init();

	if (m_pController)
	{
		angles = m_pController->EyeAngles();
		SetNextThink(gpGlobals->curtime + 0.05);
	}
	else
	{
		if (IsActive())
		{
			SetNextThink(gpGlobals->curtime + 0.1f);
		}
		else
		{
			return;
		}

		barrelEnd = WorldBarrelPosition();
		Vector worldTargetPosition;
		if (m_spawnflags & SF_TANK_AIM_AT_POS)
		{
			worldTargetPosition = m_vTargetPosition;
		}
		else
		{
			CBaseEntity* pEntity = (CBaseEntity*)m_hTarget;
			if (!pEntity || (pEntity->GetFlags() & FL_NOTARGET))
			{
				if (m_targetEntityName != NULL_STRING)
				{
					m_hTarget = FindTarget(m_targetEntityName, NULL);
				}
				else
				{
					m_hTarget = ToBasePlayer(GetContainingEntity(UTIL_FindClientInPVS(edict())));
				}

				if (m_hTarget != NULL)
				{
					SetNextThink(gpGlobals->curtime);
				}
				else
				{
					if (IsActive())
					{
						SetNextThink(gpGlobals->curtime + 2);
					}

					if (m_fireLast != 0)
					{
						m_OnLoseTarget.FireOutput(this, this);
						m_fireLast = 0;
					}
				}

				return;
			}
			pTarget = pEntity;

			worldTargetPosition = pEntity->EyePosition();
		}

		float range = (worldTargetPosition - barrelEnd).Length();

		if (!InRange(range))
		{
			m_fireLast = 0;
			return;
		}

		UTIL_TraceLine(barrelEnd, worldTargetPosition, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if (m_spawnflags & SF_TANK_AIM_AT_POS)
		{
			updateTime = TRUE;
			m_sightOrigin = m_vTargetPosition;
		}
		else
		{
			lineOfSight = FALSE;
			if (tr.fraction == 1.0 || tr.m_pEnt == pTarget)
			{
				lineOfSight = TRUE;

				CBaseEntity* pInstance = pTarget;
				if (InRange(range) && pInstance && pInstance->IsAlive())
				{
					updateTime = TRUE;

					m_sightOrigin = pInstance->BodyTarget(GetLocalOrigin(), false);
				}
			}
		}
		angles = AimBarrelAt(m_parentMatrix.WorldToLocal(m_sightOrigin));
	}
	float offsetY = UTIL_AngleDistance(angles.y, m_yawCenter);
	float offsetX = UTIL_AngleDistance(angles.x, m_pitchCenter);
	angles.y = m_yawCenter + offsetY;
	angles.x = m_pitchCenter + offsetX;

	bool bOutsideYawRange = (fabs(offsetY) > m_yawRange + m_yawTolerance);
	bool bOutsidePitchRange = (fabs(offsetX) > m_pitchRange + m_pitchTolerance);

	Vector vecToTarget = m_sightOrigin - GetLocalOrigin();

	if (bOutsideYawRange)
	{
		if (angles.y > m_yawCenter + m_yawRange)
		{
			angles.y = m_yawCenter + m_yawRange;
		}
		else if (angles.y < (m_yawCenter - m_yawRange))
		{
			angles.y = (m_yawCenter - m_yawRange);
		}
	}

	if (bOutsidePitchRange || bOutsideYawRange || (vecToTarget.Length() < (barrelEnd - GetAbsOrigin()).Length()))
	{
		updateTime = false;
	}

	if (updateTime)
	{
		m_lastSightTime = gpGlobals->curtime;
		m_persist2burst = 0;
	}

	float distY = UTIL_AngleDistance(angles.y, GetLocalAngles().y);

	QAngle vecAngVel = GetLocalAngularVelocity();
	vecAngVel.y = distY * 10;
	vecAngVel.y = clamp(vecAngVel.y, -m_yawRate, m_yawRate);

	angles.x = clamp(angles.x, m_pitchCenter - m_pitchRange, m_pitchCenter + m_pitchRange);

	float distX = UTIL_AngleDistance(angles.x, GetLocalAngles().x);
	vecAngVel.x = distX * 10;
	vecAngVel.x = clamp(vecAngVel.x, -m_pitchRate, m_pitchRate);
	SetLocalAngularVelocity(vecAngVel);

	SetMoveDoneTime(0.1);
	if (m_pController)
		return;

	if (CanFire() && ((fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (m_spawnflags & SF_TANK_LINEOFSIGHT)))
	{
		bool fire = FALSE;
		Vector forward;
		AngleVectors(GetLocalAngles(), &forward);
		forward = m_parentMatrix.ApplyRotation(forward);


		if (m_spawnflags & SF_TANK_LINEOFSIGHT)
		{
			float length = (m_maxRange > 0) ? m_maxRange : MAX_TRACE_LENGTH;
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			if (tr.m_pEnt == pTarget)
				fire = TRUE;
		}
		else
			fire = TRUE;

		if (fire)
		{
			if (m_fireLast == 0)
			{
				m_OnAquireTarget.FireOutput(this, this);
			}
			FiringSequence(barrelEnd, forward, this);
		}
		else
		{
			if (m_fireLast != 0)
			{
				m_OnLoseTarget.FireOutput(this, this);
			}
			m_fireLast = 0;
		}
	}
	else
	{
		if (m_fireLast != 0)
		{
			m_OnLoseTarget.FireOutput(this, this);
		}
		m_fireLast = 0;
	}
}

void CHL1FuncTank::FiringSequence(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		int bulletCount = (gpGlobals->curtime - m_fireLast) * m_fireRate;

		if (bulletCount > 0)
		{
			Fire(bulletCount, barrelEnd, forward, pAttacker);
			m_fireLast = gpGlobals->curtime;
		}
	}
	else
	{
		m_fireLast = gpGlobals->curtime;
	}
}

void CHL1FuncTank::Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_iszSpriteSmoke != NULL_STRING)
	{
		CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteSmoke), barrelEnd, TRUE);
		pSprite->AnimateAndDie(random->RandomFloat(15.0, 20.0));
		pSprite->SetTransparency(kRenderTransAlpha, m_clrRender->r, m_clrRender->g, m_clrRender->b, 255, kRenderFxNone);

		Vector vecVelocity(0, 0, random->RandomFloat(40, 80));
		pSprite->SetAbsVelocity(vecVelocity);
		pSprite->SetScale(m_spriteScale);
	}
	if (m_iszSpriteFlash != NULL_STRING)
	{
		CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteFlash), barrelEnd, TRUE);
		pSprite->AnimateAndDie(60);
		pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
		pSprite->SetScale(m_spriteScale);
	}

	m_OnFire.FireOutput(this, this);
}

void CHL1FuncTank::TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, trace_t& tr)
{
	Vector forward, right, up;

	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	float x, y, z;
	do {
		x = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
		y = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
		z = x * x + y * y;
	} while (z > 1);
	Vector vecDir = vecForward +
		x * vecSpread.x * right +
		y * vecSpread.y * up;
	Vector vecEnd;

	vecEnd = vecStart + vecDir * MAX_TRACE_LENGTH;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
}


void CHL1FuncTank::StartRotSound(void)
{
	if (m_spawnflags & SF_TANK_SOUNDON)
		return;
	m_spawnflags |= SF_TANK_SOUNDON;

	if (m_soundLoopRotate != NULL_STRING)
	{
		CPASAttenuationFilter filter(this);
		filter.MakeReliable();

		EmitSound_t ep;
		ep.m_nChannel = CHAN_STATIC;
		ep.m_pSoundName = (char*)STRING(m_soundLoopRotate);
		ep.m_flVolume = 0.85f;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}

	if (m_soundStartRotate != NULL_STRING)
	{
		CPASAttenuationFilter filter(this);

		EmitSound_t ep;
		ep.m_nChannel = CHAN_BODY;
		ep.m_pSoundName = (char*)STRING(m_soundStartRotate);
		ep.m_flVolume = 1.0f;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}
}


void CHL1FuncTank::StopRotSound(void)
{
	if (m_spawnflags & SF_TANK_SOUNDON)
	{
		if (m_soundLoopRotate != NULL_STRING)
		{
			StopSound(entindex(), CHAN_STATIC, (char*)STRING(m_soundLoopRotate));
		}
		if (m_soundStopRotate != NULL_STRING)
		{
			CPASAttenuationFilter filter(this);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_BODY;
			ep.m_pSoundName = (char*)STRING(m_soundStopRotate);
			ep.m_flVolume = 1.0f;
			ep.m_SoundLevel = SNDLVL_NORM;

			EmitSound(filter, entindex(), ep);
		}
	}
	m_spawnflags &= ~SF_TANK_SOUNDON;
}

class CHL1FuncTankGun : public CHL1FuncTank
{
public:
	DECLARE_CLASS(CHL1FuncTankGun, CHL1FuncTank);

	void Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
};

LINK_ENTITY_TO_CLASS(func_tank, CHL1FuncTankGun);

void CHL1FuncTankGun::Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	int i;

	for (i = 0; i < bulletCount; i++)
	{
		switch (m_bulletType)
		{
		case TANK_BULLET_SMALL:
			FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], MAX_TRACE_LENGTH, m_iSmallAmmoType, 1, -1, -1, m_iBulletDamage, pAttacker);
			break;

		case TANK_BULLET_MEDIUM:
			FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], MAX_TRACE_LENGTH, m_iMediumAmmoType, 1, -1, -1, m_iBulletDamage, pAttacker);
			break;

		case TANK_BULLET_LARGE:
			FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], MAX_TRACE_LENGTH, m_iLargeAmmoType, 1, -1, -1, m_iBulletDamage, pAttacker);
			break;
		default:
		case TANK_BULLET_NONE:
			break;
		}
	}
	BaseClass::Fire(bulletCount, barrelEnd, forward, pAttacker);
}

class CHL1FuncTankPulseLaser : public CHL1FuncTankGun
{
public:
	DECLARE_CLASS(CHL1FuncTankPulseLaser, CHL1FuncTankGun);
	DECLARE_DATADESC();

	void Precache();
	void Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);

	float		m_flPulseSpeed;
	float		m_flPulseWidth;
	color32		m_flPulseColor;
	float		m_flPulseLife;
	float		m_flPulseLag;
	string_t	m_sPulseFireSound;
};

LINK_ENTITY_TO_CLASS(func_tankpulselaser, CHL1FuncTankPulseLaser);

BEGIN_DATADESC(CHL1FuncTankPulseLaser)
DEFINE_KEYFIELD(m_flPulseSpeed, FIELD_FLOAT, "PulseSpeed"),
DEFINE_KEYFIELD(m_flPulseWidth, FIELD_FLOAT, "PulseWidth"),
DEFINE_KEYFIELD(m_flPulseColor, FIELD_COLOR32, "PulseColor"),
DEFINE_KEYFIELD(m_flPulseLife, FIELD_FLOAT, "PulseLife"),
DEFINE_KEYFIELD(m_flPulseLag, FIELD_FLOAT, "PulseLag"),
DEFINE_KEYFIELD(m_sPulseFireSound, FIELD_SOUNDNAME, "PulseFireSound"),
END_DATADESC()

void CHL1FuncTankPulseLaser::Precache(void)
{
	UTIL_PrecacheOther("grenade_beam");

	if (m_sPulseFireSound != NULL_STRING)
	{
		PrecacheScriptSound(STRING(m_sPulseFireSound));
	}
	BaseClass::Precache();
}

void CHL1FuncTankPulseLaser::Fire(int bulletCount, const Vector& barrelEnd, const Vector& vecForward, CBaseEntity* pAttacker)
{
	Vector vecUp = Vector(0, 0, 1);
	Vector vecRight;
	CrossProduct(vecForward, vecUp, vecRight);
	CrossProduct(vecForward, -vecRight, vecUp);

	for (int i = 0; i < bulletCount; i++)
	{
		float x, y, z;
		do {
			x = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
			y = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
			z = x * x + y * y;
		} while (z > 1);

		Vector vecDir = vecForward + x * gTankSpread[m_spread].x * vecRight + y * gTankSpread[m_spread].y * vecUp;

		CGrenadeBeam* pPulse = CGrenadeBeam::Create(pAttacker, barrelEnd);
		pPulse->Format(m_flPulseColor, m_flPulseWidth);
		pPulse->Shoot(vecDir, m_flPulseSpeed, m_flPulseLife, m_flPulseLag, m_iBulletDamage);

		if (m_sPulseFireSound != NULL_STRING)
		{
			CPASAttenuationFilter filter(this, 0.6f);

			EmitSound_t ep;
			ep.m_nChannel = CHAN_WEAPON;
			ep.m_pSoundName = (char*)STRING(m_sPulseFireSound);
			ep.m_flVolume = 1.0f;
			ep.m_SoundLevel = ATTN_TO_SNDLVL(0.6);

			EmitSound(filter, entindex(), ep);
		}

	}
	CHL1FuncTank::Fire(bulletCount, barrelEnd, vecForward, pAttacker);
}

class CHL1FuncTankLaser : public CHL1FuncTank
{
	DECLARE_CLASS(CHL1FuncTankLaser, CHL1FuncTank);
public:
	void	Activate(void);
	void	Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
	void	Think(void);
	CEnvLaser* GetLaser(void);
	void	UpdateOnRemove(void);

	DECLARE_DATADESC();

private:
	CEnvLaser* m_pLaser;
	float	m_laserTime;
	string_t m_iszLaserName;
};

LINK_ENTITY_TO_CLASS(func_tanklaser, CHL1FuncTankLaser);

BEGIN_DATADESC(CHL1FuncTankLaser)
DEFINE_KEYFIELD(m_iszLaserName, FIELD_STRING, "laserentity"),
DEFINE_FIELD(m_pLaser, FIELD_CLASSPTR),
DEFINE_FIELD(m_laserTime, FIELD_TIME),
END_DATADESC()

void CHL1FuncTankLaser::Activate(void)
{
	BaseClass::Activate();

	if (!GetLaser())
	{
		UTIL_Remove(this);
		Warning("Laser tank with no env_laser!\n");
	}
	else
	{
		m_pLaser->TurnOff();
	}
}

void CHL1FuncTankLaser::UpdateOnRemove(void)
{
	if (GetLaser())
	{
		m_pLaser->TurnOff();
	}

	BaseClass::UpdateOnRemove();
}

CEnvLaser* CHL1FuncTankLaser::GetLaser(void)
{
	if (m_pLaser)
		return m_pLaser;

	CBaseEntity* pLaser = gEntList.FindEntityByName(NULL, m_iszLaserName);
	while (pLaser)
	{
		if (FClassnameIs(pLaser, "env_laser"))
		{
			m_pLaser = (CEnvLaser*)pLaser;
			break;
		}
		else
		{
			pLaser = gEntList.FindEntityByName(pLaser, m_iszLaserName);
		}
	}

	return m_pLaser;
}

void CHL1FuncTankLaser::Think(void)
{
	if (m_pLaser && (gpGlobals->curtime > m_laserTime))
		m_pLaser->TurnOff();

	CHL1FuncTank::Think();
}


void CHL1FuncTankLaser::Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	int i;
	trace_t tr;

	if (GetLaser())
	{
		for (i = 0; i < bulletCount; i++)
		{
			m_pLaser->SetLocalOrigin(barrelEnd);
			TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

			m_laserTime = gpGlobals->curtime;
			m_pLaser->TurnOn();
			m_pLaser->SetFireTime(gpGlobals->curtime - 1.0);
			m_pLaser->FireAtPoint(tr);
			m_pLaser->SetNextThink(TICK_NEVER_THINK);
		}
		CHL1FuncTank::Fire(bulletCount, barrelEnd, forward, this);
	}
}

class CHL1FuncTankRocket : public CHL1FuncTank
{
public:
	DECLARE_CLASS(CHL1FuncTankRocket, CHL1FuncTank);

	void Precache(void);
	void Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
};

LINK_ENTITY_TO_CLASS(func_tankrocket, CHL1FuncTankRocket);

void CHL1FuncTankRocket::Precache(void)
{
	UTIL_PrecacheOther("rpg_rocket");
	CHL1FuncTank::Precache();
}

void CHL1FuncTankRocket::Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	int i;

	for (i = 0; i < bulletCount; i++)
	{
		CBaseEntity* pRocket = CBaseEntity::Create("rpg_rocket", barrelEnd, GetAbsAngles(), this);
		pRocket->SetAbsAngles(GetAbsAngles());
	}
	CHL1FuncTank::Fire(bulletCount, barrelEnd, forward, this);
}

class CHL1FuncTankMortar : public CHL1FuncTank
{
public:
	DECLARE_CLASS(CHL1FuncTankMortar, CHL1FuncTank);

	void Precache(void);
	void FiringSequence(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
	void Fire(int bulletCount, const Vector& barrelEnd, const Vector& vecForward, CBaseEntity* pAttacker);
	void ShootGun(void);

	void InputShootGun(inputdata_t& inputdata);

	DECLARE_DATADESC();

	int			m_Magnitude;
	float		m_fireDelay;
	string_t	m_fireStartSound;
	string_t	m_fireEndSound;

	CBaseEntity* m_pAttacker;
};

LINK_ENTITY_TO_CLASS(func_tankmortar, CHL1FuncTankMortar);

BEGIN_DATADESC(CHL1FuncTankMortar)
DEFINE_KEYFIELD(m_Magnitude, FIELD_INTEGER, "iMagnitude"),
DEFINE_KEYFIELD(m_fireDelay, FIELD_FLOAT, "firedelay"),
DEFINE_KEYFIELD(m_fireStartSound, FIELD_STRING, "firestartsound"),
DEFINE_KEYFIELD(m_fireEndSound, FIELD_STRING, "fireendsound"),
DEFINE_FIELD(m_pAttacker, FIELD_CLASSPTR),
DEFINE_INPUTFUNC(FIELD_VOID, "ShootGun", InputShootGun),
END_DATADESC()

void CHL1FuncTankMortar::Precache(void)
{
	if (m_fireStartSound != NULL_STRING)
		PrecacheScriptSound(STRING(m_fireStartSound));
	if (m_fireEndSound != NULL_STRING)
		PrecacheScriptSound(STRING(m_fireEndSound));
	BaseClass::Precache();
}

void CHL1FuncTankMortar::InputShootGun(inputdata_t& inputdata)
{
	ShootGun();
}

void CHL1FuncTankMortar::ShootGun(void)
{
	Vector forward;
	AngleVectors(GetLocalAngles(), &forward);
	UpdateMatrix();
	forward = m_parentMatrix.ApplyRotation(forward);

	Fire(1, WorldBarrelPosition(), forward, m_pAttacker);
}


void CHL1FuncTankMortar::FiringSequence(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		int bulletCount = (gpGlobals->curtime - m_fireLast) * m_fireRate;

		if (bulletCount > 0)
		{
			if (m_fireStartSound != NULL_STRING)
			{
				CPASAttenuationFilter filter(this);

				EmitSound_t ep;
				ep.m_nChannel = CHAN_WEAPON;
				ep.m_pSoundName = (char*)STRING(m_fireStartSound);
				ep.m_flVolume = 1.0f;
				ep.m_SoundLevel = SNDLVL_NORM;

				EmitSound(filter, entindex(), ep);
			}

			if (m_fireDelay != 0)
			{
				g_EventQueue.AddEvent(this, "ShootGun", m_fireDelay, pAttacker, this, 0);
			}
			else
			{
				ShootGun();
			}
			m_fireLast = gpGlobals->curtime;
		}
	}
	else
	{
		m_fireLast = gpGlobals->curtime;
	}
}

void CHL1FuncTankMortar::Fire(int bulletCount, const Vector& barrelEnd, const Vector& vecForward, CBaseEntity* pAttacker)
{
	if (m_fireEndSound != NULL_STRING)
	{
		CPASAttenuationFilter filter(this);

		EmitSound_t ep;
		ep.m_nChannel = CHAN_WEAPON;
		ep.m_pSoundName = STRING(m_fireEndSound);
		ep.m_flVolume = 1.0f;
		ep.m_SoundLevel = SNDLVL_NORM;

		EmitSound(filter, entindex(), ep);
	}

	trace_t tr;

	TankTrace(barrelEnd, vecForward, gTankSpread[m_spread], tr);

	const CBaseEntity* ent = NULL;
	if (g_pGameRules->IsMultiplayer())
	{
		ent = te->GetSuppressHost();
		te->SetSuppressHost(NULL);
	}

	ExplosionCreate(tr.endpos, GetAbsAngles(), this, m_Magnitude, 0, true);

	if (g_pGameRules->IsMultiplayer())
	{
		te->SetSuppressHost((CBaseEntity*)ent);
	}

	BaseClass::Fire(bulletCount, barrelEnd, vecForward, this);
}

class CHL1FuncTankPhysCannister : public CHL1FuncTank
{
public:
	DECLARE_CLASS(CHL1FuncTankPhysCannister, CHL1FuncTank);
	DECLARE_DATADESC();

	void Activate(void);
	void Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);

protected:
	string_t				m_iszBarrelVolume;
	CHandle<CBaseTrigger>	m_hBarrelVolume;
};

LINK_ENTITY_TO_CLASS(func_tankphyscannister, CHL1FuncTankPhysCannister);

BEGIN_DATADESC(CHL1FuncTankPhysCannister)
DEFINE_KEYFIELD(m_iszBarrelVolume, FIELD_STRING, "barrel_volume"),
DEFINE_FIELD(m_hBarrelVolume, FIELD_EHANDLE),
END_DATADESC()

void CHL1FuncTankPhysCannister::Activate(void)
{
	BaseClass::Activate();

	m_hBarrelVolume = NULL;

	if (m_iszBarrelVolume != NULL_STRING)
	{
		m_hBarrelVolume = dynamic_cast<CBaseTrigger*>(gEntList.FindEntityByName(NULL, m_iszBarrelVolume));
	}

	if (!m_hBarrelVolume)
	{
		Msg("ERROR: Couldn't find barrel volume for func_tankphyscannister %s.\n", STRING(GetEntityName()));
		UTIL_Remove(this);
	}
}

void CHL1FuncTankPhysCannister::Fire(int bulletCount, const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	Assert(m_hBarrelVolume);

	CPhysicsCannister* pCannister = (CPhysicsCannister*)m_hBarrelVolume->GetTouchedEntityOfType("physics_cannister");
	if (!pCannister)
	{
		return;
	}

	pCannister->CannisterFire(pAttacker);
}

//---------------------------------
//			Health Charger
//---------------------------------

ConVar	sk_healthcharger("sk_healthcharger", "0");

class CWallHealth : public CBaseToggle
{
public:
	DECLARE_CLASS(CWallHealth, CBaseToggle);
	DECLARE_DATADESC();

	void Spawn();
	void Precache(void);
	bool CreateVPhysics(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue(const char* szKeyName, const char* szValue);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

	float m_flNextCharge;
	int		m_iReactivate;
	int		m_iJuice;
	int		m_iOn;
	float   m_flSoundTime;
	int		m_iCaps;
};

BEGIN_DATADESC(CWallHealth)
DEFINE_FIELD(m_flNextCharge, FIELD_TIME),
DEFINE_FIELD(m_iReactivate, FIELD_INTEGER),
DEFINE_FIELD(m_iJuice, FIELD_INTEGER),
DEFINE_FIELD(m_iOn, FIELD_INTEGER),
DEFINE_FIELD(m_flSoundTime, FIELD_TIME),
DEFINE_FIELD(m_iCaps, FIELD_INTEGER),
DEFINE_FUNCTION(Off),
DEFINE_FUNCTION(Recharge),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);
PRECACHE_REGISTER(func_healthcharger);

bool CWallHealth::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "style") ||
		FStrEq(szKeyName, "height") ||
		FStrEq(szKeyName, "value1") ||
		FStrEq(szKeyName, "value2") ||
		FStrEq(szKeyName, "value3"))
	{
		return true;
	}
	else if (FStrEq(szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(szValue);
		return true;
	}
	else
		return CBaseToggle::KeyValue(szKeyName, szValue);
}

void CWallHealth::Spawn()
{
	Precache();

	SetSolid(SOLID_BSP);
	SetMoveType(MOVETYPE_PUSH);

	SetModel(STRING(GetModelName()));

	m_iJuice = sk_healthcharger.GetFloat();
	SetTextureFrameIndex(0);

	m_iCaps = FCAP_CONTINUOUS_USE;

	CreateVPhysics();
}

bool CWallHealth::CreateVPhysics(void)
{
	VPhysicsInitStatic();
	return true;

}

void CWallHealth::Precache(void)
{
	PrecacheScriptSound("WallHealth.Deny");
	PrecacheScriptSound("WallHealth.Start");
	PrecacheScriptSound("WallHealth.LoopingContinueCharge");
	PrecacheScriptSound("WallHealth.Recharge");
}

void CWallHealth::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	m_iCaps = FCAP_CONTINUOUS_USE;

	if (m_iJuice <= 0)
	{
		Off();
		SetTextureFrameIndex(1);
	}

	CBasePlayer* pPlayer = ToBasePlayer(pActivator);

	if ((m_iJuice <= 0) || (!(pPlayer->m_Local.m_bWearingSuit)))
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound("WallHealth.Deny");
		}
		return;
	}

	if (pActivator->GetHealth() >= pActivator->GetMaxHealth())
	{
		CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pActivator);

		if (pPlayer)
		{
			pPlayer->m_afButtonPressed &= ~IN_USE;
		}

		m_iCaps = FCAP_IMPULSE_USE;

		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = 0.56 + gpGlobals->curtime;
			EmitSound("WallHealth.Deny");
		}
		return;
	}

	SetNextThink(gpGlobals->curtime + 0.25);
	SetThink(&CWallHealth::Off);

	if (m_flNextCharge >= gpGlobals->curtime)
		return;

	if (!m_iOn)
	{
		m_iOn++;
		EmitSound("WallHealth.Start");
		m_flSoundTime = 0.56 + gpGlobals->curtime;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->curtime))
	{
		m_iOn++;
		CPASAttenuationFilter filter(this, "WallHealth.LoopingContinueCharge");
		filter.MakeReliable();
		EmitSound(filter, entindex(), "WallHealth.LoopingContinueCharge");
	}

	if (pActivator->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}

	m_flNextCharge = gpGlobals->curtime + 0.1;
}

void CWallHealth::Recharge(void)
{
	EmitSound("WallHealth.Recharge");
	m_iJuice = sk_healthcharger.GetFloat();
	SetTextureFrameIndex(0);
	SetThink(NULL);
}

void CWallHealth::Off(void)
{
	if (m_iOn > 1)
		StopSound("WallHealth.LoopingContinueCharge");

	m_iOn = 0;

	if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime()) > 0))
	{
		SetNextThink(gpGlobals->curtime + m_iReactivate);
		SetThink(&CWallHealth::Recharge);
	}
	else
		SetThink(NULL);
}

//---------------------------------
//			Suit Charger
//---------------------------------

ConVar	sk_suitcharger("sk_suitcharger", "0");
#define HL1_MAX_ARMOR 100

class CRecharge : public CBaseToggle
{
public:
	DECLARE_CLASS(CRecharge, CBaseToggle)
	DECLARE_DATADESC()
	void Spawn();
	void Precache(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue(const char* szKeyName, const char* szValue);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

	bool CreateVPhysics();

	float m_flNextCharge;
	int		m_iReactivate;
	int		m_iJuice;
	int		m_iOn;
	float   m_flSoundTime;

	int		m_iCaps;
};

BEGIN_DATADESC(CRecharge)

DEFINE_FIELD(m_flNextCharge, FIELD_TIME),
DEFINE_FIELD(m_iReactivate, FIELD_INTEGER),
DEFINE_FIELD(m_iJuice, FIELD_INTEGER),
DEFINE_FIELD(m_iOn, FIELD_INTEGER),
DEFINE_FIELD(m_flSoundTime, FIELD_TIME),
DEFINE_FIELD(m_iCaps, FIELD_INTEGER),
DEFINE_FUNCTION(Off),
DEFINE_FUNCTION(Recharge),
END_DATADESC()

LINK_ENTITY_TO_CLASS(func_recharge, CRecharge);

bool CRecharge::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "style") ||
		FStrEq(szKeyName, "height") ||
		FStrEq(szKeyName, "value1") ||
		FStrEq(szKeyName, "value2") ||
		FStrEq(szKeyName, "value3"))
	{
	}
	else if (FStrEq(szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(szValue);
	}
	else
		return CBaseToggle::KeyValue(szKeyName, szValue);

	return true;
}

void CRecharge::Spawn()
{
	Precache();

	SetSolid(SOLID_BSP);
	SetMoveType(MOVETYPE_PUSH);

	SetModel(STRING(GetModelName()));
	m_iJuice = sk_suitcharger.GetFloat();
	SetTextureFrameIndex(0);

	m_iCaps = FCAP_CONTINUOUS_USE;

	CreateVPhysics();
}

void CRecharge::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("SuitRecharge.Deny");
	PrecacheScriptSound("SuitRecharge.Start");
	PrecacheScriptSound("SuitRecharge.ChargingLoop");
}

void CRecharge::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator)
		return;

	if (!pActivator->IsPlayer())
		return;

	CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pActivator);

	if (pPlayer == NULL)
		return;

	m_iCaps = FCAP_CONTINUOUS_USE;

	if (m_iJuice <= 0)
	{
		SetTextureFrameIndex(1);
		Off();
	}

	if ((m_iJuice <= 0) || (!(pPlayer->m_Local.m_bWearingSuit)))
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound("SuitRecharge.Deny");
		}
		return;
	}

	if (pPlayer->ArmorValue() >= 100)
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound("SuitRecharge.Deny");
		}
		return;
	}

	SetNextThink(gpGlobals->curtime + 0.25);
	SetThink(&CRecharge::Off);

	if (m_flNextCharge >= gpGlobals->curtime)
		return;

	if (!pActivator)
		return;

	m_hActivator = pActivator;

	if (!m_hActivator->IsPlayer())
		return;

	if (!m_iOn)
	{
		m_iOn++;
		EmitSound("SuitRecharge.Start");
		m_flSoundTime = 0.56 + gpGlobals->curtime;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->curtime))
	{
		m_iOn++;
		CPASAttenuationFilter filter(this, "SuitRecharge.ChargingLoop");
		filter.MakeReliable();
		EmitSound(filter, entindex(), "SuitRecharge.ChargingLoop");
	}

	CBasePlayer* pl = (CBasePlayer*)m_hActivator.Get();

	if (pl->ArmorValue() < 100)
	{
		m_iJuice--;
		pl->IncrementArmorValue(1, HL1_MAX_ARMOR);
	}

	m_flNextCharge = gpGlobals->curtime + 0.1;
}

void CRecharge::Recharge(void)
{
	m_iJuice = sk_suitcharger.GetFloat();
	SetTextureFrameIndex(0);
	SetThink(&CRecharge::SUB_DoNothing);
}

void CRecharge::Off(void)
{
	if (m_iOn > 1)
		StopSound("SuitRecharge.ChargingLoop");

	m_iOn = 0;

	if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHEVChargerRechargeTime()) > 0))
	{
		SetNextThink(gpGlobals->curtime + m_iReactivate);
		SetThink(&CRecharge::Recharge);
	}
	else
		SetThink(&CRecharge::SUB_DoNothing);
}

bool CRecharge::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}