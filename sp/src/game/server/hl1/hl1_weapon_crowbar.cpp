#include "cbase.h"
#include "hl1_baseweapon_shared.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "decals.h"
#include "vstdlib/random.h"

extern ConVar sk_plr_dmg_crowbar;

//Crowbar Sounds
extern ConVar hl1_crowbar_sound;
extern ConVar hl1_crowbar_concrete;
extern ConVar hl1_crowbar_metal;
extern ConVar hl1_crowbar_dirt;
extern ConVar hl1_crowbar_vent;
extern ConVar hl1_crowbar_grate;
extern ConVar hl1_crowbar_tile;
extern ConVar hl1_crowbar_wood;
extern ConVar hl1_crowbar_glass;
extern ConVar hl1_crowbar_computer;

extern ConVar hl1_crowbar_concrete_vol;
extern ConVar hl1_crowbar_metal_vol;
extern ConVar hl1_crowbar_dirt_vol;
extern ConVar hl1_crowbar_vent_vol;
extern ConVar hl1_crowbar_grate_vol;
extern ConVar hl1_crowbar_tile_vol;
extern ConVar hl1_crowbar_wood_vol;
extern ConVar hl1_crowbar_glass_vol;
extern ConVar hl1_crowbar_computer_vol;

#define	CROWBAR_RANGE		32.0f
#define	CROWBAR_REFIRE_MISS	0.5f
#define	CROWBAR_REFIRE_HIT	0.25f

float GetCrowbarVolume(trace_t &ptr)
{
	if (!hl1_crowbar_sound.GetBool())
		return 0.0f;

	float fvol = 0.0;
	char mType = TEXTURETYPE_Find(&ptr);

	CBaseEntity* pEntity = ptr.m_pEnt;
	
	switch (mType)
	{
	case CHAR_TEX_CONCRETE:
		if (hl1_crowbar_concrete.GetBool())
			fvol = hl1_crowbar_concrete_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_METAL:
		if (hl1_crowbar_metal.GetBool())
			fvol = hl1_crowbar_metal_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_DIRT:
		if (hl1_crowbar_dirt.GetBool())
			fvol = hl1_crowbar_dirt_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_VENT:
		if (hl1_crowbar_vent.GetBool())
			fvol = hl1_crowbar_vent_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_GRATE:
		if (hl1_crowbar_grate.GetBool())
			fvol = hl1_crowbar_grate_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_TILE:
		if (hl1_crowbar_tile.GetBool())
			fvol = hl1_crowbar_tile_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_WOOD:
		if (hl1_crowbar_wood.GetBool())
			fvol = hl1_crowbar_wood_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_GLASS:
		if (hl1_crowbar_glass.GetBool())
			fvol = hl1_crowbar_glass_vol.GetFloat() / 10;
		break;
	case CHAR_TEX_COMPUTER:
		if (hl1_crowbar_computer.GetBool())
			fvol = hl1_crowbar_computer_vol.GetFloat() / 10;
		break;
	default: fvol = 0.0;
		break;
	}

	if (pEntity && FClassnameIs(pEntity, "func_breakable"))
	{
		fvol /= 2.0;
	}

	return fvol;
}

class CWeaponCrowbar : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CWeaponCrowbar, CBaseHL1CombatWeapon);
public:

	CWeaponCrowbar();

	void			Precache(void);
	virtual void	ItemPostFrame(void);
	void			PrimaryAttack(void);

public:
	trace_t		m_traceHit;
	Activity	m_nHitActivity;

private:
	virtual void		Swing(void);
	virtual	void		Hit(void);
	virtual	void		ImpactEffect(void);
	virtual	void		ImpactSound(bool isWorld, trace_t &hitTrace);
	virtual Activity	ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);

public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponCrowbar, DT_WeaponCrowbar)
END_SEND_TABLE()
PRECACHE_WEAPON_REGISTER(weapon_crowbar);

LINK_ENTITY_TO_CLASS(weapon_crowbar, CWeaponCrowbar);

BEGIN_DATADESC(CWeaponCrowbar)
DEFINE_FUNCTION(Hit),
END_DATADESC()

static const Vector g_bludgeonMins(-16, -16, -16);
static const Vector g_bludgeonMaxs(16, 16, 16);

CWeaponCrowbar::CWeaponCrowbar()
{
	m_bFiresUnderwater = true;
}

void CWeaponCrowbar::Precache(void)
{
	PrecacheSound("weapons/cbar_hit1.wav");
	PrecacheSound("weapons/cbar_hit2.wav");

	BaseClass::Precache();
}

void CWeaponCrowbar::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}

void CWeaponCrowbar::PrimaryAttack()
{
	Swing();
}

void CWeaponCrowbar::Hit(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 400, 0.2);
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, m_traceHit.endpos, 400, 0.2f, pPlayer);

	CBaseEntity	*pHitEntity = m_traceHit.m_pEnt;

	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		ClearMultiDamage();
		CTakeDamageInfo info(GetOwner(), GetOwner(), sk_plr_dmg_crowbar.GetFloat(), DMG_CLUB);
		CalculateMeleeDamageForce(&info, hitDirection, m_traceHit.endpos);
		pHitEntity->DispatchTraceAttack(info, hitDirection, &m_traceHit);
		ApplyMultiDamage();
 
		TraceAttackToTriggers(CTakeDamageInfo(GetOwner(), GetOwner(), sk_plr_dmg_crowbar.GetFloat(), DMG_CLUB), m_traceHit.startpos, m_traceHit.endpos, hitDirection);

		ImpactSound(pHitEntity->Classify() == CLASS_NONE || pHitEntity->Classify() == CLASS_MACHINE, m_traceHit);
	}

	ImpactEffect();
}

void CWeaponCrowbar::ImpactSound(bool isWorld, trace_t &hitTrace)
{
	if (isWorld)
	{
		float flvol = GetCrowbarVolume(hitTrace);
		switch (random->RandomInt(0, 1))
		{
		case 0:
			UTIL_EmitAmbientSound(GetOwner()->entindex(), GetOwner()->GetAbsOrigin(), "weapons/cbar_hit1.wav", flvol, SNDLVL_GUNFIRE, 0, 98 + random->RandomInt(0, 3));
			break;
		case 1:
			UTIL_EmitAmbientSound(GetOwner()->entindex(), GetOwner()->GetAbsOrigin(), "weapons/cbar_hit2.wav", flvol, SNDLVL_GUNFIRE, 0, 98 + random->RandomInt(0, 3));
			break;
		}
	}
	else
	{
		WeaponSound(MELEE_HIT);
	}
}

Activity CWeaponCrowbar::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner)
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}

void CWeaponCrowbar::ImpactEffect(void)
{
	UTIL_ImpactTrace(&m_traceHit, DMG_CLUB);
}

void CWeaponCrowbar::Swing(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	pOwner->EyeVectors(&forward, NULL, NULL);

	UTIL_TraceLine(swingStart, swingStart + forward * CROWBAR_RANGE, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);
	m_nHitActivity = ACT_VM_HITCENTER;

	if (m_traceHit.fraction == 1.0)
	{
		UTIL_TraceHull(swingStart, swingStart + forward * CROWBAR_RANGE, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);
		if (m_traceHit.fraction < 1.0)
		{
			m_nHitActivity = ChooseIntersectionPointAndActivity(m_traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
		}
	}

	if (m_traceHit.fraction == 1.0f)
	{
		m_nHitActivity = ACT_VM_MISSCENTER;

		WeaponSound(SINGLE);

		m_flNextPrimaryAttack = gpGlobals->curtime + CROWBAR_REFIRE_MISS;
	}
	else
	{
		Hit();

		m_flNextPrimaryAttack = gpGlobals->curtime + CROWBAR_REFIRE_HIT;
	}

	SendWeaponAnim(m_nHitActivity);
}