#ifndef BASEHLCOMBATWEAPON_SHARED_H
#define BASEHLCOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "basecombatweapon_shared.h"

#if defined( CLIENT_DLL )
#define CBaseHL1CombatWeapon C_BaseHL1CombatWeapon
#endif

class CBaseHL1CombatWeapon : public CBaseCombatWeapon
{
	DECLARE_CLASS(CBaseHL1CombatWeapon, CBaseCombatWeapon);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:
	virtual void Spawn(void);

#if !defined( CLIENT_DLL )
	virtual void Precache();

	void FallInit(void);
	void FallThink(void);

	void EjectShell(CBaseEntity *pPlayer, int iType);

	Vector GetSoundEmissionOrigin() const;

#else

	virtual void	AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles);
	virtual	float	CalcViewmodelBob(void);

#endif
};

#endif // BASEHLCOMBATWEAPON_SHARED_H