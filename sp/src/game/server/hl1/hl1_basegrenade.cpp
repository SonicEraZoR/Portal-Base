#include "cbase.h"
#include "decals.h"
#include "basecombatcharacter.h"
#include "shake.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "entitylist.h"
#include "hl1_basegrenade.h"

int		g_sModelIndexFireball;
float	g_sModelIndexWExplosion;

unsigned int CHL1BaseGrenade::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

void CHL1BaseGrenade::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("BaseGrenade.Explode");
}


void CHL1BaseGrenade::Explode(trace_t *pTrace, int bitsDamageType)
{
	float		flRndSound;

	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	if (pTrace->fraction != 1.0)
	{
		SetLocalOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents(vecAbsOrigin);

	if (pTrace->fraction != 1.0)
	{
		Vector vecNormal = pTrace->plane.normal;
		const surfacedata_t *pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, 0.0,
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char)pdata->game.material);
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, 0.0,
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage);
	}

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);

	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	CTakeDamageInfo info(this, GetThrower(), GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);

	RadiusDamage(info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);

	UTIL_DecalTrace(pTrace, "Scorch");

	flRndSound = random->RandomFloat(0, 1);

	EmitSound("BaseGrenade.Explode");

	SetTouch(NULL);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);

	SetThink(&CBaseGrenade::Smoke);
	SetNextThink(gpGlobals->curtime + 0.3);

	if (GetWaterLevel() == 0)
	{
		int sparkCount = random->RandomInt(0, 3);
		QAngle angles;
		VectorAngles(pTrace->plane.normal, angles);

		for (int i = 0; i < sparkCount; i++)
			Create("spark_shower", GetAbsOrigin(), angles, NULL);
	}
}

int CHL1BaseGrenade::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	return 1;
}