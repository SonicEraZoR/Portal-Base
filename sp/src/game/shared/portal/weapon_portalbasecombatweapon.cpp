//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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

#ifndef HL1_CLIENT_DLL
#ifndef HL1_DLL
extern ConVar sk_auto_reload_time;
#endif
#endif

CBasePortalCombatWeapon::CBasePortalCombatWeapon( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortalCombatWeapon::ItemHolsterFrame( void )
{
	BaseClass::ItemHolsterFrame();

	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

#ifndef HL1_CLIENT_DLL
#ifndef HL1_DLL
	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Just load the clip with no animations
		FinishReload();
		m_flHolsterTime = gpGlobals->curtime;
	}
#endif
#endif
}

bool CBasePortalCombatWeapon::CanLower()
{
	if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) == ACTIVITY_NOT_AVAILABLE )
		return false;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Drops the weapon into a lowered pose
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePortalCombatWeapon::Lower( void )
{
	//Don't bother if we don't have the animation
	if ( SelectWeightedSequence( ACT_VM_IDLE_LOWERED ) == ACTIVITY_NOT_AVAILABLE )
		return false;

	m_bLowered = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Brings the weapon up to the ready position
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePortalCombatWeapon::Ready( void )
{
	//Don't bother if we don't have the animation
	if ( SelectWeightedSequence( ACT_VM_LOWERED_TO_IDLE ) == ACTIVITY_NOT_AVAILABLE )
		return false;

	m_bLowered = false;	
	m_flRaiseTime = gpGlobals->curtime + 0.5f;
	return true;
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
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePortalCombatWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( BaseClass::Holster( pSwitchingTo ) )
	{
		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePortalCombatWeapon::WeaponShouldBeLowered( void )
{
	// Can't be in the middle of another animation
  	if ( GetIdealActivity() != ACT_VM_IDLE_LOWERED && GetIdealActivity() != ACT_VM_IDLE &&
		 GetIdealActivity() != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity() != ACT_VM_LOWERED_TO_IDLE )
  		return false;

	if ( m_bLowered )
		return true;
	
#if !defined( CLIENT_DLL )

	if ( GlobalEntity_GetState( "friendly_encounter" ) == GLOBAL_ON )
		return true;

#endif

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Allows the weapon to choose proper weapon idle animation
//-----------------------------------------------------------------------------
void CBasePortalCombatWeapon::WeaponIdle( void )
{
	//See if we should idle high or low
	if ( WeaponShouldBeLowered() )
	{
#if !defined( CLIENT_DLL )
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player*>(GetOwner());

		if (pPlayer)
		{
			pPlayer->Weapon_Lower();
		}
#endif

		// Move to lowered position if we're not there yet
		if ( GetActivity() != ACT_VM_IDLE_LOWERED && GetActivity() != ACT_VM_IDLE_TO_LOWERED 
			 && GetActivity() != ACT_TRANSITION )
		{
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
		else if ( HasWeaponIdleTimeElapsed() )
		{
			// Keep idling low
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
	}
	else
	{
		// See if we need to raise immediately
		if ( m_flRaiseTime < gpGlobals->curtime && GetActivity() == ACT_VM_IDLE_LOWERED ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
		else if ( HasWeaponIdleTimeElapsed() ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

#if defined( CLIENT_DLL )

#define	HL2_BOB_CYCLE_MIN	1.0f
#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB			0.002f
#define	HL2_BOB_UP		0.5f

#ifndef HL1_CLIENT_DLL
#ifndef HL1_DLL
extern float	g_lateralBob;
extern float	g_verticalBob;
#endif
#endif

static ConVar	cl_bobcycle( "cl_bobcycle","0.8" );
static ConVar	cl_bob( "cl_bob","0.002" );
static ConVar	cl_bobup( "cl_bobup","0.5" );

// Register these cvars if needed for easy tweaking
static ConVar	v_iyaw_cycle( "v_iyaw_cycle", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iroll_cycle( "v_iroll_cycle", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_ipitch_cycle( "v_ipitch_cycle", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iyaw_level( "v_iyaw_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_iroll_level( "v_iroll_level", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );
static ConVar	v_ipitch_level( "v_ipitch_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBasePortalCombatWeapon::CalcViewmodelBob( void )
{
#ifndef HL1_CLIENT_DLL
#ifndef HL1_DLL
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;
	
	CBasePlayer *player = ToBasePlayer( GetOwner() );
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ( ( !gpGlobals->frametime ) || ( player == NULL ) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	speed = clamp( speed, -320, 320 );

	float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );
	
	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX)*HL2_BOB_CYCLE_MAX;
	cycle /= HL2_BOB_CYCLE_MAX;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}
	
	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX*2)*HL2_BOB_CYCLE_MAX*2;
	cycle /= HL2_BOB_CYCLE_MAX*2;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );
#endif
#endif
	
	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBasePortalCombatWeapon::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	CalcViewmodelBob();

#ifndef HL1_CLIENT_DLL
#ifndef HL1_DLL
	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * 0.1f, forward, origin );
	
	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;
	
	// bob the angles
	angles[ ROLL ]	+= g_verticalBob * 0.5f;
	angles[ PITCH ]	-= g_verticalBob * 0.4f;

	angles[ YAW ]	-= g_lateralBob  * 0.3f;

	VectorMA( origin, g_lateralBob * 0.8f, right, origin );
#endif
#endif
}

//-----------------------------------------------------------------------------
Vector CBasePortalCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	return BaseClass::GetBulletSpread( proficiency );
}

//-----------------------------------------------------------------------------
float CBasePortalCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	return BaseClass::GetSpreadBias( proficiency );
}
//-----------------------------------------------------------------------------

const WeaponProficiencyInfo_t *CBasePortalCombatWeapon::GetProficiencyValues()
{
	return NULL;
}

#else

// Server stubs
float CBasePortalCombatWeapon::CalcViewmodelBob( void )
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBasePortalCombatWeapon::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
}


//-----------------------------------------------------------------------------
Vector CBasePortalCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	Vector baseSpread = BaseClass::GetBulletSpread( proficiency );

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[ proficiency ].spreadscale;
	return ( baseSpread * flModifier );
}

//-----------------------------------------------------------------------------
float CBasePortalCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	return (pProficiencyValues)[ proficiency ].bias;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBasePortalCombatWeapon::GetProficiencyValues()
{
	return GetDefaultProficiencyValues();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CBasePortalCombatWeapon::GetDefaultProficiencyValues()
{
	// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
	static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
	{
		{ 2.50, 1.0	},
		{ 2.00, 1.0	},
		{ 1.50, 1.0	},
		{ 1.25, 1.0 },
		{ 1.00, 1.0	},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return g_BaseWeaponProficiencyTable;
}

#endif