// Patch 2.1

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
#include	"hl1_basegrenade.h"
#include	"animation.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"soundenvelope.h"
#include	"hl1_CBaseHelicopter.h"
#include	"ndebugoverlay.h"
#include	"smoke_trail.h"
#include	"beam_shared.h"
#include	"grenade_homer.h"

#define	 HOMER_TRAIL0_LIFE		0.1
#define	 HOMER_TRAIL1_LIFE		0.2
#define	 HOMER_TRAIL2_LIFE		3.0

#define  SF_NOTRANSITION 128

ConVar	sk_apache_health("sk_apache_health", "0");

extern short g_sModelIndexFireball;

class CHL1Apache : public CBaseHelicopter
{
	DECLARE_CLASS(CHL1Apache, CBaseHelicopter);
public:
	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);

	int  BloodColor(void) { return DONT_BLEED; }
	Class_T  Classify(void) { return CLASS_HUMAN_MILITARY; };
	void InitializeRotorSound(void);
	void LaunchRocket(Vector& viewDir, int damage, int radius, Vector vecLaunchPoint);

	void Flight(void);

	bool FireGun(void);
	void AimRocketGun(void);
	void FireRocket(void);
	void DyingThink(void);

	int  ObjectCaps(void);

	int m_iRockets;
	float m_flForce;
	float m_flNextRocket;

	int m_iAmmoType;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	Vector m_vecGoal;

	Vector m_angGun;

	Vector m_cullBoxMins;
	Vector m_cullBoxMaxs;

	int m_iSoundState;

	int m_iExplode;
	int m_iBodyGibs;
	int	m_nDebrisModel;

	float m_flGoalSpeed;

	int m_iDoSmokePuff;
	CBeam* m_pBeam;
};

BEGIN_DATADESC(CHL1Apache)
DEFINE_FIELD(m_iRockets, FIELD_INTEGER),
DEFINE_FIELD(m_flForce, FIELD_FLOAT),
DEFINE_FIELD(m_flNextRocket, FIELD_TIME),
DEFINE_FIELD(m_vecTarget, FIELD_VECTOR),
DEFINE_FIELD(m_posTarget, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecDesired, FIELD_VECTOR),
DEFINE_FIELD(m_posDesired, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecGoal, FIELD_VECTOR),
DEFINE_FIELD(m_angGun, FIELD_VECTOR),
DEFINE_FIELD(m_flLastSeen, FIELD_TIME),
DEFINE_FIELD(m_flPrevSeen, FIELD_TIME),
DEFINE_FIELD(m_pBeam, FIELD_CLASSPTR),
DEFINE_FIELD(m_flGoalSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_iDoSmokePuff, FIELD_INTEGER),
END_DATADESC()

void CHL1Apache::Spawn(void)
{
	Precache();
	SetModel("models/apache.mdl");

	BaseClass::Spawn();

	Vector mins, maxs;
	ExtractBbox(0, mins, maxs);
	UTIL_SetSize(this, mins, maxs);

	m_iHealth = sk_apache_health.GetFloat();

	m_flFieldOfView = -0.707;

	m_fHelicopterFlags = BITS_HELICOPTER_MISSILE_ON | BITS_HELICOPTER_GUN_ON;

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	InitBoneControllers();

	SetRenderColor(255, 255, 255, 255);

	m_iRockets = 10;
}

LINK_ENTITY_TO_CLASS(monster_apache, CHL1Apache);

int CHL1Apache::ObjectCaps(void)
{
	if (GetSpawnFlags() & SF_NOTRANSITION)
		return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	else
		return BaseClass::ObjectCaps();
}

void CHL1Apache::Precache(void)
{
	PrecacheModel("models/apache.mdl");
	PrecacheScriptSound("Apache.Rotor");
	m_nDebrisModel = PrecacheModel("models/metalplategibs_green.mdl");

	PrecacheScriptSound("Apache.FireGun");
	m_iAmmoType = GetAmmoDef()->Index("9mmRound");

	UTIL_PrecacheOther("grenade_homer");
	PrecacheScriptSound("Apache.RPG");
	PrecacheModel("models/weapons/w_missile.mdl");

	BaseClass::Precache();
}

void CHL1Apache::InitializeRotorSound(void)
{
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter(this);
	m_pRotorSound = controller.SoundCreate(filter, entindex(), "Apache.Rotor");

	BaseClass::InitializeRotorSound();
}

void CHL1Apache::Flight(void)
{
	StudioFrameAdvance();

	float flDistToDesiredPosition = (GetAbsOrigin() - m_vecDesiredPosition).Length();

	if (m_flGoalSpeed < 800)
		m_flGoalSpeed += GetAcceleration();

	if (flDistToDesiredPosition > 250)
	{
		Vector v1 = (m_vecTargetPosition - GetAbsOrigin());
		Vector v2 = (m_vecDesiredPosition - GetAbsOrigin());

		VectorNormalize(v1);
		VectorNormalize(v2);

		if (m_flLastSeen + 90 > gpGlobals->curtime && DotProduct(v1, v2) > 0.25)
		{
			m_vecGoalOrientation = (m_vecTargetPosition - GetAbsOrigin());
		}
		else
		{
			m_vecGoalOrientation = (m_vecDesiredPosition - GetAbsOrigin());
		}
		VectorNormalize(m_vecGoalOrientation);
	}
	else
	{
		AngleVectors(GetGoalEnt()->GetAbsAngles(), &m_vecGoalOrientation);
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

				AngleVectors(GetGoalEnt()->GetAbsAngles(), &m_vecGoalOrientation);

				flDistToDesiredPosition = (GetAbsOrigin() - m_vecDesiredPosition).Length();
			}
		}
	}
	else
	{
		m_vecDesiredPosition = GetAbsOrigin();
	}

	QAngle angAdj = QAngle(5.0, 0, 0);

	Vector forward, right, up;
	AngleVectors(GetAbsAngles() + GetLocalAngularVelocity() * 2 + angAdj, &forward, &right, &up);

	QAngle angVel = GetLocalAngularVelocity();
	float flSide = DotProduct(m_vecGoalOrientation, right);

	if (flSide < 0)
	{
		if (angVel.y < 60)
		{
			angVel.y += 8;
		}
	}
	else
	{
		if (angVel.y > -60)
		{
			angVel.y -= 8;
		}
	}
	angVel.y *= 0.98;
	SetLocalAngularVelocity(angVel);

	Vector vecVel = GetAbsVelocity();

	AngleVectors(GetAbsAngles() + GetLocalAngularVelocity() * 1 + angAdj, &forward, &right, &up);
	Vector vecEst = GetAbsOrigin() + vecVel * 2.0 + up * m_flForce * 20 - Vector(0, 0, 384 * 2);

	AngleVectors(GetAbsAngles() + angAdj, &forward, &right, &up);

	vecVel.x += up.x * m_flForce;
	vecVel.y += up.y * m_flForce;
	vecVel.z += up.z * m_flForce;
	vecVel.z -= 38.4;

	float flSpeed = vecVel.Length();
	float flDir = DotProduct(Vector(forward.x, forward.y, 0), Vector(vecVel.x, vecVel.y, 0));
	if (flDir < 0)
		flSpeed = -flSpeed;

	float flDist = DotProduct(m_vecDesiredPosition - vecEst, forward);

	float flSlip = -DotProduct(m_vecDesiredPosition - vecEst, right);

	angVel = GetLocalAngularVelocity();

	if (flSlip > 0)
	{
		if (GetAbsAngles().z > -30 && angVel.z > -15)
			angVel.z -= 4;
		else
			angVel.z += 2;
	}
	else
	{

		if (GetAbsAngles().z < 30 && angVel.z < 15)
			angVel.z += 4;
		else
			angVel.z -= 2;
	}
	SetLocalAngularVelocity(angVel);

	vecVel.x = vecVel.x * (1.0 - fabs(right.x) * 0.05);
	vecVel.y = vecVel.y * (1.0 - fabs(right.y) * 0.05);
	vecVel.z = vecVel.z * (1.0 - fabs(right.z) * 0.05);

	vecVel = vecVel * 0.995;

	SetAbsVelocity(vecVel);

	if (m_flForce < 80 && vecEst.z < m_vecDesiredPosition.z)
	{
		m_flForce += 12;
	}
	else if (m_flForce > 30)
	{
		if (vecEst.z > m_vecDesiredPosition.z)
			m_flForce -= 8;
	}


	angVel = GetLocalAngularVelocity();

	if (flDist > 0 && flSpeed < m_flGoalSpeed && GetAbsAngles().x + angVel.x < 40)
	{
		angVel.x += 12.0;
	}
	else if (flDist < 0 && flSpeed > -50 && GetAbsAngles().x + angVel.x > -20)
	{
		angVel.x -= 12.0;
	}
	else if (GetAbsAngles().x + angVel.x < 0)
	{
		angVel.x += 4.0;
	}
	else if (GetAbsAngles().x + angVel.x > 0)
	{
		angVel.x -= 4.0;
	}

	SetLocalAngularVelocity(angVel);
}

#define CHOPPER_AP_GUN_TIP			0
#define CHOPPER_AP_GUN_BASE			1

#define CHOPPER_BC_GUN_YAW			0
#define CHOPPER_BC_GUN_PITCH		1
#define CHOPPER_BC_POD_PITCH		2

bool CHL1Apache::FireGun()
{
	Vector vForward, vRight, vUp;

	AngleVectors(GetAbsAngles(), &vForward, &vUp, &vRight);

	Vector posGun;
	QAngle angGun;
	GetAttachment(1, posGun, angGun);

	Vector vecTarget = (m_vecTargetPosition - posGun);

	VectorNormalize(vecTarget);

	Vector vecOut;

	vecOut.x = DotProduct(vForward, vecTarget);
	vecOut.y = -DotProduct(vUp, vecTarget);
	vecOut.z = DotProduct(vRight, vecTarget);

	QAngle angles;

	VectorAngles(vecOut, angles);

	angles.y = AngleNormalize(angles.y);
	angles.x = AngleNormalize(angles.x);

	if (angles.x > m_angGun.x)
		m_angGun.x = min(angles.x, m_angGun.x + 12);
	if (angles.x < m_angGun.x)
		m_angGun.x = max(angles.x, m_angGun.x - 12);
	if (angles.y > m_angGun.y)
		m_angGun.y = min(angles.y, m_angGun.y + 12);
	if (angles.y < m_angGun.y)
		m_angGun.y = max(angles.y, m_angGun.y - 12);

	m_angGun.y = clamp(m_angGun.y, -89.9, 89.9);
	m_angGun.x = clamp(m_angGun.x, -9.9, 44.9);

	m_angGun.y = SetBoneController(0, m_angGun.y);
	m_angGun.x = SetBoneController(1, m_angGun.x);

	Vector posBarrel;
	QAngle angBarrel;
	GetAttachment(2, posBarrel, angBarrel);
	Vector vecGun = (posGun - posBarrel);

	VectorNormalize(vecGun);

	if (DotProduct(vecGun, vecTarget) > 0.98)
	{
		CPASAttenuationFilter filter(this, 0.2f);

		EmitSound(filter, entindex(), "Apache.FireGun");

		FireBullets(1, posBarrel, vecTarget, VECTOR_CONE_4DEGREES, 8192, m_iAmmoType, 2);

		return true;
	}

	return false;
}

void CHL1Apache::FireRocket(void)
{
	static float side = 1.0;
	static int count;
	Vector vForward, vRight, vUp;


	AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);
	Vector vecSrc = GetAbsOrigin() +1.5 * (vForward * 21 + vRight * 40 * side + vUp);

	switch (m_iRockets % 5)
	{
	case 0:	vecSrc = vecSrc + vRight * 10; break;
	case 1: vecSrc = vecSrc - vRight * 10; break;
	case 2: vecSrc = vecSrc + vUp * 10; break;
	case 3: vecSrc = vecSrc - vUp * 10; break;
	case 4: break;
	}

	Vector vTargetDir = m_vecTargetPosition - GetAbsOrigin();
	VectorNormalize(vTargetDir);
	LaunchRocket(vTargetDir, 100, 150, vecSrc);

	m_iRockets--;

	side = -side;
}
void CHL1Apache::AimRocketGun(void)
{
	Vector vForward, vRight, vUp;

	if (m_iRockets <= 0)
		return;

	Vector vTargetDir = m_vecTargetPosition - GetAbsOrigin();
	VectorNormalize(vTargetDir);

	AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);
	Vector vecEst = (vForward * 800 + GetAbsVelocity());
	VectorNormalize(vecEst);

	trace_t tr1;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecEst * 4096, MASK_ALL, this, COLLISION_GROUP_NONE, &tr1);

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vTargetDir * 4096, MASK_ALL, this, COLLISION_GROUP_NONE, &tr1);

	if ((m_iRockets % 2) == 1)
	{
		FireRocket();
		m_flNextRocket = gpGlobals->curtime + 0.5;
		if (m_iRockets <= 0)
		{
			m_flNextRocket = gpGlobals->curtime + 10;
			m_iRockets = 10;
		}
	}
	else if (DotProduct(GetAbsVelocity(), vForward) > -100 && m_flNextRocket < gpGlobals->curtime)
	{
		if (m_flLastSeen + 60 > gpGlobals->curtime)
		{
			if (GetEnemy() != NULL)
			{
				trace_t tr;

				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecEst * 4096, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

				if ((tr.endpos - m_vecTargetPosition).Length() < 1024)
					FireRocket();
			}
			else
			{
				trace_t tr;

				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecEst * 4096, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);
				if ((tr.endpos - m_vecTargetPosition).Length() < 512)
					FireRocket();
			}
		}
	}
}

#define MISSILE_HOMING_STRENGTH		0.3
#define MISSILE_HOMING_DELAY		5.0
#define MISSILE_HOMING_RAMP_UP		0.5
#define MISSILE_HOMING_DURATION		1.0
#define MISSILE_HOMING_RAMP_DOWN	0.5

void CHL1Apache::LaunchRocket(Vector& viewDir, int damage, int radius, Vector vecLaunchPoint)
{

	CGrenadeHomer* pGrenade = CGrenadeHomer::CreateGrenadeHomer(
		MAKE_STRING("models/weapons/w_missile.mdl"),
		MAKE_STRING("Apache.RPG"),
		vecLaunchPoint, vec3_angle, edict());
	pGrenade->Spawn();
	pGrenade->SetHoming(MISSILE_HOMING_STRENGTH, MISSILE_HOMING_DELAY,
		MISSILE_HOMING_RAMP_UP, MISSILE_HOMING_DURATION,
		MISSILE_HOMING_RAMP_DOWN);
	pGrenade->SetDamage(damage);
	pGrenade->m_DmgRadius = radius;

	pGrenade->Launch(this, GetEnemy(), viewDir * 1500, 500, 0, HOMER_SMOKE_TRAIL_ON);

}

void CHL1Apache::DyingThink(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1f);

	if (gpGlobals->curtime > m_flNextCrashExplosion)
	{
		CPASFilter pasFilter(GetAbsOrigin());
		Vector pos;

		pos = GetAbsOrigin();
		pos.x += random->RandomFloat(-150, 150);
		pos.y += random->RandomFloat(-150, 150);
		pos.z += random->RandomFloat(-150, -50);

		te->Explosion(pasFilter, 0.0, &pos, g_sModelIndexFireball, 10, 15, TE_EXPLFLAG_NONE, 100, 0);

		Vector vecSize = Vector(500, 500, 60);
		CPVSFilter pvsFilter(GetAbsOrigin());

		te->BreakModel(pvsFilter, 0.0, GetAbsOrigin(), vec3_angle, vecSize, vec3_origin,
			m_nDebrisModel, 100, 0, 2.5, BREAK_METAL);

		m_flNextCrashExplosion = gpGlobals->curtime + random->RandomFloat(0.3, 0.5);
	}

	QAngle angVel = GetLocalAngularVelocity();
	if (angVel.y < 400)
	{
		angVel.y *= 1.1;
		SetLocalAngularVelocity(angVel);
	}

	Vector vecImpulse(0, 0, -38.4);
	ApplyAbsVelocityImpulse(vecImpulse);
}