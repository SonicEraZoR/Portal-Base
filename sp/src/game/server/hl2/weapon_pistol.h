//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_PISTOL_H
#define WEAPON_PISTOL_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"

#define	PISTOL_FASTEST_REFIRE_TIME		0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//-----------------------------------------------------------------------------
// CWeaponPistol
//-----------------------------------------------------------------------------

class CWeaponPistol : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponPistol, CBaseHLCombatWeapon);

	CWeaponPistol(void);

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	ItemPostFrame(void);
	void	ItemPreFrame(void);
	void	ItemBusyFrame(void);
	void	PrimaryAttack(void);
	void	AddViewKick(void);
	void	DryFire(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	void	UpdatePenaltyTime(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity(void);

	virtual bool Reload(void);

	virtual const Vector& GetBulletSpread(void);

	virtual int	GetMinBurst()
	{
		return 1;
	}

	virtual int	GetMaxBurst()
	{
		return 3;
	}

	virtual float GetFireRate(void)
	{
		return 0.5f;
	}

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};

#endif // WEAPON_PISTOL_H
