#include "cbase.h"
#include "npcevent.h"
#include "soundent.h"
#include "hl1_baseweapon_shared.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "vstdlib/random.h"

extern ConVar sk_plr_dmg_tripmine;

class CWeaponTripMine : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CWeaponTripMine, CBaseHL1CombatWeapon);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
public:

	CWeaponTripMine(void);

	void	Precache(void);
	void	PrimaryAttack(void);
	void	WeaponIdle(void);
};

LINK_ENTITY_TO_CLASS(weapon_tripmine, CWeaponTripMine);

PRECACHE_WEAPON_REGISTER(weapon_tripmine);

IMPLEMENT_SERVERCLASS_ST(CWeaponTripMine, DT_WeaponTripMine)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponTripMine)
END_DATADESC()

CWeaponTripMine::CWeaponTripMine(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
}

void CWeaponTripMine::Precache(void)
{
	BaseClass::Precache();
	UTIL_PrecacheOther("monster_tripmine");
}

void CWeaponTripMine::PrimaryAttack(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return;

	Vector vecAiming = pPlayer->GetAutoaimVector(0);
	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	trace_t tr;

	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 128, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		CBaseEntity* pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			QAngle angles;
			VectorAngles(tr.plane.normal, angles);

			CBaseEntity::Create("monster_tripmine", tr.endpos + tr.plane.normal * 8, angles, pPlayer);

			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

			pPlayer->SetAnimation(PLAYER_ATTACK1);

			if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				if (!pPlayer->SwitchToNextBestWeapon(pPlayer->GetActiveWeapon()))
					Holster();
			}
			else
			{
				SendWeaponAnim(ACT_VM_DRAW);
				SetWeaponIdleTime(gpGlobals->curtime + random->RandomFloat(10, 15));
			}

			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;

			SetWeaponIdleTime(gpGlobals->curtime);
		}
	}
	else
	{
		SetWeaponIdleTime(m_flTimeWeaponIdle = gpGlobals->curtime + random->RandomFloat(10, 15));
	}
}

void CWeaponTripMine::WeaponIdle(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (!HasWeaponIdleTimeElapsed())
		return;

	int iAnim;

	if (random->RandomFloat(0, 1) <= 0.75)
	{
		iAnim = ACT_VM_IDLE;
	}
	else
	{
		iAnim = ACT_VM_FIDGET;
	}

	SendWeaponAnim(iAnim);
}