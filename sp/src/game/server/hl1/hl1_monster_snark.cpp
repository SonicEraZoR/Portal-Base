#include "cbase.h"
#include "soundent.h"
#include "engine/IEngineSound.h"
#include "ai_senses.h"
#include "hl1_monster_snark.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#define SQUEEK_DETONATE_DELAY	15.0
#define SNARK_EXPLOSION_VOLUME	512

ConVar sk_snark_health("sk_snark_health", "0");
ConVar sk_snark_dmg_bite("sk_snark_dmg_bite", "0");
ConVar sk_snark_dmg_pop("sk_snark_dmg_pop", "0");

enum w_squeak_e {
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

float CHL1Snark::m_flNextBounceSoundTime = 0;

LINK_ENTITY_TO_CLASS(monster_snark, CHL1Snark);

BEGIN_DATADESC(CHL1Snark)
DEFINE_FIELD(m_flDie, FIELD_TIME),
DEFINE_FIELD(m_vecTarget, FIELD_VECTOR),
DEFINE_FIELD(m_flNextHunt, FIELD_TIME),
DEFINE_FIELD(m_flNextHit, FIELD_TIME),
DEFINE_FIELD(m_posPrev, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),
DEFINE_FIELD(m_iMyClass, FIELD_INTEGER),

DEFINE_ENTITYFUNC(SuperBounceTouch),
DEFINE_THINKFUNC(HuntThink),
END_DATADESC()

void CHL1Snark::Precache(void)
{
	BaseClass::Precache();

	PrecacheModel("models/w_squeak2.mdl");

	PrecacheScriptSound("Snark.Die");
	PrecacheScriptSound("Snark.Gibbed");
	PrecacheScriptSound("Snark.Squeak");
	PrecacheScriptSound("Snark.Deploy");
	PrecacheScriptSound("Snark.Bounce");

}

void CHL1Snark::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetFriction(1.0);

	SetModel("models/w_squeak2.mdl");
	UTIL_SetSize(this, Vector(-4, -4, 0), Vector(4, 4, 8));

	SetBloodColor(BLOOD_COLOR_YELLOW);

	SetTouch(&CHL1Snark::SuperBounceTouch);
	SetThink(&CHL1Snark::HuntThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_flNextHit = gpGlobals->curtime;
	m_flNextHunt = gpGlobals->curtime + 1E6;
	m_flNextBounceSoundTime = gpGlobals->curtime;

	AddFlag(FL_AIMTARGET | FL_NPC);
	m_takedamage = DAMAGE_YES;

	m_iHealth = sk_snark_health.GetFloat();
	m_iMaxHealth = m_iHealth;

	SetGravity(UTIL_ScaleForGravity(400));
	SetFriction(0.5);

	SetDamage(sk_snark_dmg_pop.GetFloat());

	m_flDie = gpGlobals->curtime + SQUEEK_DETONATE_DELAY;

	m_flFieldOfView = 0;

	if (GetOwnerEntity())
		m_hOwner = GetOwnerEntity();

	m_flNextBounceSoundTime = gpGlobals->curtime;

	SetSequence(WSQUEAK_RUN);
	ResetSequenceInfo();

	m_iMyClass = CLASS_NONE;

	m_posPrev = Vector(0, 0, 0);
}


Class_T	CHL1Snark::Classify(void)
{
	if (m_iMyClass != CLASS_NONE)
		return m_iMyClass;

	if (GetEnemy() != NULL)
	{
		m_iMyClass = CLASS_INSECT;
		switch (GetEnemy()->Classify())
		{
		case CLASS_PLAYER:
		case CLASS_HUMAN_PASSIVE:
		case CLASS_HUMAN_MILITARY:
			m_iMyClass = CLASS_NONE;
			return CLASS_ALIEN_MILITARY;
		}
		m_iMyClass = CLASS_NONE;
	}

	return CLASS_ALIEN_BIOWEAPON;
}

void CHL1Snark::Event_Killed(const CTakeDamageInfo& inputInfo)
{
	SetThink(&CHL1Snark::SUB_Remove);
	SetNextThink(gpGlobals->curtime + 0.1f);
	SetTouch(NULL);

	m_takedamage = DAMAGE_NO;

	CPASAttenuationFilter filter(this, 0.5);
	EmitSound(filter, entindex(), "Snark.Die");

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SNARK_EXPLOSION_VOLUME, 3.0);

	UTIL_BloodDrips(WorldSpaceCenter(), Vector(0, 0, 0), BLOOD_COLOR_YELLOW, 80);

	if (m_hOwner != NULL)
	{
		RadiusDamage(CTakeDamageInfo(this, m_hOwner, GetDamage(), DMG_BLAST), GetAbsOrigin(), GetDamage() * 2.5, CLASS_NONE, NULL);
	}
	else
	{
		RadiusDamage(CTakeDamageInfo(this, this, GetDamage(), DMG_BLAST), GetAbsOrigin(), GetDamage() * 2.5, CLASS_NONE, NULL);
	}

	if (m_hOwner != NULL)
		SetOwnerEntity(m_hOwner);

	CTakeDamageInfo info = inputInfo;
	int iGibDamage = g_pGameRules->Damage_GetShouldGibCorpse();
	info.SetDamageType(iGibDamage);

	BaseClass::Event_Killed(info);
}


bool CHL1Snark::Event_Gibbed(const CTakeDamageInfo& info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Snark.Gibbed");

	return BaseClass::Event_Gibbed(info);
}

void CHL1Snark::HuntThink(void)
{
	if (!IsInWorld())
	{
		SetTouch(NULL);
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1f);

	SetGroundEntity(NULL);
	PhysicsStepRecheckGround();

	if (gpGlobals->curtime >= m_flDie)
	{
		g_vecAttackDir = GetAbsVelocity();
		VectorNormalize(g_vecAttackDir);
		m_iHealth = -1;
		CTakeDamageInfo	info(this, this, 1, DMG_GENERIC);
		Event_Killed(info);
		return;
	}

	if (GetWaterLevel() != 0)
	{
		if (GetMoveType() == MOVETYPE_FLYGRAVITY)
		{
			SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM);
		}

		Vector vecVel = GetAbsVelocity();
		vecVel *= 0.9;
		vecVel.z += 8.0;
		SetAbsVelocity(vecVel);
	}
	else if (GetMoveType() == MOVETYPE_FLY)
	{
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	}

	if (m_flNextHunt > gpGlobals->curtime)
		return;

	m_flNextHunt = gpGlobals->curtime + 2.0;

	Vector vecFlat = GetAbsVelocity();
	vecFlat.z = 0;
	VectorNormalize(vecFlat);

	if (GetEnemy() == NULL || !GetEnemy()->IsAlive())
	{
		GetSenses()->Look(1024);
		SetEnemy(BestEnemy());
	}

	if ((m_flDie - gpGlobals->curtime <= 0.5) && (m_flDie - gpGlobals->curtime >= 0.3))
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Snark.Squeak");
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 256, 0.25);
	}

	float flpitch = 155.0 - 60.0 * ((m_flDie - gpGlobals->curtime) / SQUEEK_DETONATE_DELAY);
	if (flpitch < 80)
		flpitch = 80;

	if (GetEnemy() != NULL)
	{
		if (FVisible(GetEnemy()))
		{
			m_vecTarget = GetEnemy()->EyePosition() - GetAbsOrigin();
			VectorNormalize(m_vecTarget);
		}

		float flVel = GetAbsVelocity().Length();
		float flAdj = 50.0 / (flVel + 10.0);

		if (flAdj > 1.2)
			flAdj = 1.2;

		SetAbsVelocity(GetAbsVelocity() * flAdj + (m_vecTarget * 300));
	}

	if (GetFlags() & FL_ONGROUND)
	{
		SetLocalAngularVelocity(QAngle(0, 0, 0));
	}
	else
	{
		QAngle angVel = GetLocalAngularVelocity();
		if (angVel == QAngle(0, 0, 0))
		{
			angVel.x = random->RandomFloat(-100, 100);
			angVel.z = random->RandomFloat(-100, 100);
			SetLocalAngularVelocity(angVel);
		}
	}

	if ((GetAbsOrigin() - m_posPrev).Length() < 1.0)
	{
		Vector vecVel = GetAbsVelocity();
		vecVel.x = random->RandomFloat(-100, 100);
		vecVel.y = random->RandomFloat(-100, 100);
		SetAbsVelocity(vecVel);
	}

	m_posPrev = GetAbsOrigin();

	QAngle angles;
	VectorAngles(GetAbsVelocity(), angles);
	angles.z = 0;
	angles.x = 0;
	SetAbsAngles(angles);
}

unsigned int CHL1Snark::PhysicsSolidMaskForEntity(void) const
{
	unsigned int iMask = BaseClass::PhysicsSolidMaskForEntity();

	iMask &= ~CONTENTS_MONSTERCLIP;

	return iMask;
}

void CHL1Snark::ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity)
{
	float flSurfaceFriction;
	physprops->GetPhysicsProperties(trace.surface.surfaceProps, NULL, NULL, &flSurfaceFriction, NULL);

	Vector vecAbsVelocity = GetAbsVelocity();

	if (trace.plane.normal.z <= 0.7)
	{
		Vector vecDir = vecAbsVelocity;

		float speed = vecDir.Length();

		VectorNormalize(vecDir);

		float hitDot = DotProduct(trace.plane.normal, -vecDir);

		Vector vReflection = 2.0f * trace.plane.normal * hitDot + vecDir;

		SetAbsVelocity(vReflection * speed * 0.6f);

		return;
	}

	VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);
	float flSpeedSqr = DotProduct(vecVelocity, vecVelocity);

	CBaseEntity* pEntity = trace.m_pEnt;
	Assert(pEntity);

	if (vecVelocity.z < (800 * gpGlobals->frametime))
	{
		vecAbsVelocity.z = 0.0f;

		VectorAdd(vecAbsVelocity, GetBaseVelocity(), vecVelocity);
		flSpeedSqr = DotProduct(vecVelocity, vecVelocity);
	}
	SetAbsVelocity(vecAbsVelocity);

	if (flSpeedSqr < (30 * 30))
	{
		if (pEntity->IsStandable())
		{
			SetGroundEntity(pEntity);
		}

		SetAbsVelocity(vec3_origin);
		SetLocalAngularVelocity(vec3_angle);
	}
	else
	{
		vecAbsVelocity += GetBaseVelocity();
		vecAbsVelocity *= (1.0f - trace.fraction) * gpGlobals->frametime * flSurfaceFriction;
		PhysicsPushEntity(vecAbsVelocity, &trace);
	}
}

void CHL1Snark::SuperBounceTouch(CBaseEntity* pOther)
{
	float	flpitch;
	trace_t tr;
	tr = CBaseEntity::GetTouchTrace();

	if (GetOwnerEntity() && (pOther == GetOwnerEntity()))
		return;

	SetOwnerEntity(NULL);

	QAngle angles = GetAbsAngles();
	angles.x = 0;
	angles.z = 0;
	SetAbsAngles(angles);

	if (m_flNextHit > gpGlobals->curtime)
		return;

	flpitch = 155.0 - 60.0 * ((m_flDie - gpGlobals->curtime) / SQUEEK_DETONATE_DELAY);

	if (pOther->m_takedamage && m_flNextAttack < gpGlobals->curtime)
	{
		if (tr.m_pEnt == pOther)
		{
			if (tr.m_pEnt->GetModelIndex() != GetModelIndex())
			{
				ClearMultiDamage();

				Vector vecForward;
				AngleVectors(GetAbsAngles(), &vecForward);

				if (m_hOwner != NULL)
				{
					CTakeDamageInfo info(this, m_hOwner, sk_snark_dmg_bite.GetFloat(), DMG_SLASH);
					CalculateMeleeDamageForce(&info, vecForward, tr.endpos);
					pOther->DispatchTraceAttack(info, vecForward, &tr);
				}
				else
				{
					CTakeDamageInfo info(this, this, sk_snark_dmg_bite.GetFloat(), DMG_SLASH);
					CalculateMeleeDamageForce(&info, vecForward, tr.endpos);
					pOther->DispatchTraceAttack(info, vecForward, &tr);
				}

				ApplyMultiDamage();

				SetDamage(GetDamage() + sk_snark_dmg_pop.GetFloat());

				CPASAttenuationFilter filter(this);
				CSoundParameters params;
				if (GetParametersForSound("Snark.Deploy", params, NULL))
				{
					EmitSound_t ep(params);
					ep.m_nPitch = (int)flpitch;

					EmitSound(filter, entindex(), ep);
				}
				m_flNextAttack = gpGlobals->curtime + 0.5;
			}
		}
	}

	m_flNextHit = gpGlobals->curtime + 0.1;
	m_flNextHunt = gpGlobals->curtime;

	if (g_pGameRules->IsMultiplayer())
	{
		if (gpGlobals->curtime < m_flNextBounceSoundTime)
		{
			return;
		}
	}

	if (!(GetFlags() & FL_ONGROUND))
	{
		CPASAttenuationFilter filter2(this);

		CSoundParameters params;
		if (GetParametersForSound("Snark.Bounce", params, NULL))
		{
			EmitSound_t ep(params);
			ep.m_nPitch = (int)flpitch;

			EmitSound(filter2, entindex(), ep);
		}

		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 256, 0.25);
	}
	else
	{
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 100, 0.1);
	}

	m_flNextBounceSoundTime = gpGlobals->curtime + 0.5;
}


bool CHL1Snark::IsValidEnemy(CBaseEntity* pEnemy)
{
	return CHL1BaseMonster::IsValidEnemy(pEnemy);
}