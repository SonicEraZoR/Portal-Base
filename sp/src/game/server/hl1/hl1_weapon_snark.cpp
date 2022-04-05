#include "cbase.h"
#include "npcevent.h"
#include "hl1_baseweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "hl1_monster_snark.h"
#include "beam_shared.h"

class CWeaponSnark : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CWeaponSnark, CBaseHL1CombatWeapon);
public:
	CWeaponSnark(void);

	void	Precache(void);
	void	PrimaryAttack(void);
	void	WeaponIdle(void);
	void	Materialize(void);
	void	GroundThink(void);
	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	bool	m_bJustThrown;
};

LINK_ENTITY_TO_CLASS(weapon_snark, CWeaponSnark);

PRECACHE_WEAPON_REGISTER(weapon_snark);

IMPLEMENT_SERVERCLASS_ST(CWeaponSnark, DT_WeaponSnark)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponSnark)
DEFINE_FIELD(m_bJustThrown, FIELD_BOOLEAN),
DEFINE_THINKFUNC(GroundThink),
END_DATADESC()

CWeaponSnark::CWeaponSnark(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
	m_bJustThrown = false;
}

void CWeaponSnark::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("WpnSnark.PrimaryAttack");
	PrecacheScriptSound("WpnSnark.Deploy");

	UTIL_PrecacheOther("monster_snark");
}

void CWeaponSnark::PrimaryAttack(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return;

	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);

	Vector vecStart = pPlayer->WorldSpaceCenter() + (vecForward * 20);
	Vector vecEnd = vecStart + (vecForward * 44);
	trace_t tr;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
	if (tr.allsolid || tr.startsolid || tr.fraction <= 0.25)
		return;

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	CHL1Snark* pSnark = (CHL1Snark*)Create("monster_snark", tr.endpos, pPlayer->EyeAngles(), GetOwner());
	if (pSnark)
	{
		pSnark->SetAbsVelocity(vecForward * 200 + pPlayer->GetAbsVelocity());
	}

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "WpnSnark.PrimaryAttack");

	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 200, 0.2);

	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

	m_bJustThrown = true;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
	SetWeaponIdleTime(gpGlobals->curtime + 1.0);
}

void CWeaponSnark::WeaponIdle(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_bJustThrown)
	{
		m_bJustThrown = false;

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
	}
	else
	{
		if (random->RandomFloat(0, 1) <= 0.75)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
		else
		{
			SendWeaponAnim(ACT_VM_FIDGET);
		}
	}
}

void CWeaponSnark::Materialize(void)
{
	BaseClass::Materialize();

	SetSequence(1);
	ResetSequenceInfo();

	SetThink(&CWeaponSnark::GroundThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CWeaponSnark::GroundThink(void)
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	StudioFrameAdvance();
}

bool CWeaponSnark::Deploy(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "WpnSnark.Deploy");

	return BaseClass::Deploy();
}

bool CWeaponSnark::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return false;
	}

	if (!BaseClass::Holster(pSwitchingTo))
	{
		return false;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		SetThink(&CWeaponSnark::DestroyItem);
		SetNextThink(gpGlobals->curtime + 0.1);
	}

	pPlayer->SetNextAttack(gpGlobals->curtime + 0.5);

	return true;
}