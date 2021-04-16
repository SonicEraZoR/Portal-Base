//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_WEAPON__CROWBAR_H
#define C_WEAPON__CROWBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"
#include "client_class.h"

class C_WeaponCrowbar : public C_BaseHLBludgeonWeapon
{
	DECLARE_CLASS(C_WeaponCrowbar, C_BaseHLBludgeonWeapon);
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLIENTCLASS();
	C_WeaponCrowbar() {};
private:
	C_WeaponCrowbar(const C_WeaponCrowbar &);
};
BEGIN_PREDICTION_DATA(C_WeaponCrowbar)
END_PREDICTION_DATA()
LINK_ENTITY_TO_CLASS(weapon_crowbar, C_WeaponCrowbar);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponCrowbar, DT_WeaponCrowbar, CWeaponCrowbar)
END_RECV_TABLE()

#endif