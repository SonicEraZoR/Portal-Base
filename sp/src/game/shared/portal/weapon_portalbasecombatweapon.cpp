//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_portalbasecombatweapon.h"

#include "portal_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( baseportalcombatweapon, CBasePortalCombatWeapon );

IMPLEMENT_NETWORKCLASS_ALIASED(BasePortalCombatWeapon, DT_BasePortalCombatWeapon)

BEGIN_NETWORK_TABLE(CBasePortalCombatWeapon, DT_BasePortalCombatWeapon)
#if !defined( CLIENT_DLL )
//	SendPropInt( SENDINFO( m_bReflectViewModelAnimations ), 1, SPROP_UNSIGNED ),
#else
//	RecvPropInt( RECVINFO( m_bReflectViewModelAnimations ) ),
#endif
END_NETWORK_TABLE()


#if !defined( CLIENT_DLL )

#include "globalstate.h"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CBasePortalCombatWeapon )

DEFINE_FIELD( m_bLowered,			FIELD_BOOLEAN ),
DEFINE_FIELD( m_flRaiseTime,		FIELD_TIME ),
DEFINE_FIELD( m_flHolsterTime,		FIELD_TIME ),
DEFINE_FIELD( m_iPrimaryAttacks,	FIELD_INTEGER ),
DEFINE_FIELD( m_iSecondaryAttacks,	FIELD_INTEGER ),

END_DATADESC()

#endif

BEGIN_PREDICTION_DATA(CBasePortalCombatWeapon)
END_PREDICTION_DATA()

extern ConVar sk_auto_reload_time;

//mygamepedia: any reasons to keep this ?
CBasePortalCombatWeapon::CBasePortalCombatWeapon( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePortalCombatWeapon::Deploy( void )
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	if ( GetOwner() && GetOwner()->IsPlayer() )
	{
		//mygamepedia: the next line of code is the reason why i didn't removed this method
		//the only dif from CBaseHLCombatWeapon version is using portal player for some reason
		//not sure for what, so i keep it
		CPortal_Player *pPlayer = assert_cast<CPortal_Player*>( GetOwner() );
		if ( pPlayer->IsWeaponLowered() )
		{
			if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) != ACTIVITY_NOT_AVAILABLE )
			{
				if ( DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix() ) )
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack( gpGlobals->curtime + 1.0 );
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack	= gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}

	m_bLowered = false;

	//mygamepedia: replaced BaseClass with this so it doesn't run almost identical code again
	return CBaseCombatWeapon::Deploy();
}