//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for Portal.
//
//=============================================================================//

#include "cbase.h"
#include "portal_player.h"
#include "globalstate.h"
#include "trains.h"
#include "game.h"
#include "portal_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "portal_gamerules.h"
#include "weapon_portalgun.h"
#include "portal/weapon_physcannon.h"
#include "KeyValues.h"
#include "team.h"
#include "eventqueue.h"
#include "weapon_portalbase.h"
#include "engine/IEngineSound.h"
#include "ai_basenpc.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "prop_portal_shared.h"
#include "player_pickup.h"	// for player pickup code
#include "vphysics/player_controller.h"
#include "datacache/imdlcache.h"
#include "bone_setup.h"
#include "portal_gamestats.h"
#include "physicsshadowclone.h"
#include "physics_prop_ragdoll.h"
#include "soundenvelope.h"
#include "ai_speech.h"		// For expressors, vcd playing
#include "sceneentity.h"	// has the VCD precache function

LINK_ENTITY_TO_CLASS( player, CPortal_Player );

IMPLEMENT_SERVERCLASS_ST(CPortal_Player, DT_Portal_Player)
SendPropBool( SENDINFO( m_bHeldObjectOnOppositeSideOfPortal) ),
SendPropEHandle( SENDINFO( m_pHeldObjectPortal ) ),
SendPropBool( SENDINFO( m_bPitchReorientation ) ),
SendPropEHandle( SENDINFO( m_hPortalEnvironment ) ),
SendPropEHandle( SENDINFO( m_hSurroundingLiquidPortal ) ),
SendPropBool( SENDINFO( m_bSuppressingCrosshair ) ),

END_SEND_TABLE()

BEGIN_DATADESC( CPortal_Player )

	DEFINE_SOUNDPATCH( m_pWooshSound ),

	DEFINE_FIELD( m_bHeldObjectOnOppositeSideOfPortal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_pHeldObjectPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bPitchReorientation, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hPortalEnvironment, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSurroundingLiquidPortal, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bSuppressingCrosshair, FIELD_BOOLEAN ),

END_DATADESC()

ConVar sv_regeneration_wait_time ("sv_regeneration_wait_time", "1.0", FCVAR_REPLICATED );
ConVar sv_regeneration_enable("sv_regeneration_enable", "0", FCVAR_REPLICATED | FCVAR_ARCHIVE);

#define PORTALPLAYER_PHYSDAMAGE_SCALE 4.0f

CPortal_Player::CPortal_Player()
{
	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;
	m_bIntersectingPortalPlane = false;
	m_bPitchReorientation = false;
	m_bSilentDropAndPickup = false;
	m_bSuppressingCrosshair = false;
}

CPortal_Player::~CPortal_Player( void )
{
}

void CPortal_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "PortalPlayer.EnterPortal" );
	PrecacheScriptSound( "PortalPlayer.ExitPortal" );

	PrecacheScriptSound( "PortalPlayer.Woosh" );
	PrecacheScriptSound( "PortalPlayer.FallRecover" );

	PrecacheModel ( "sprites/glow01.vmt" );
}

void CPortal_Player::CreateSounds()
{
	if ( !m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		CPASAttenuationFilter filter( this );

		m_pWooshSound = controller.SoundCreate( filter, entindex(), "PortalPlayer.Woosh" );
		controller.Play( m_pWooshSound, 0, 100 );
	}
}

void CPortal_Player::StopLoopingSounds()
{
	if ( m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		controller.SoundDestroy( m_pWooshSound );
		m_pWooshSound = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: Sets  specific defaults.
//-----------------------------------------------------------------------------
void CPortal_Player::Spawn(void)
{
	SetPlayerModel();

	BaseClass::Spawn();

	CreateSounds();

	pl.deadflag = false;
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	RemoveEffects( EF_NODRAW );
	StopObserverMode();

//	GiveDefaultItems(); no need to do this

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;

	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = PORTALPLAYER_PHYSDAMAGE_SCALE;

	RemoveFlag( FL_FROZEN );

	//m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

#ifdef PORTAL_MP
	PickTeam();
#endif
}

/*
I think it makes the player make rebel hurt sounds when hit
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortal_Player::UpdateExpression( void )
{
	if ( !m_pExpresser )
		return;

	int iConcept = CONCEPT_CHELL_IDLE;
	if ( GetHealth() <= 0 )
	{
		iConcept = CONCEPT_CHELL_DEAD;
	}

	GetExpresser()->SetOuter( this );

	ClearExpression();
	AI_Response response;
	bool result = SpeakFindResponse( response, g_pszChellConcepts[iConcept] );
	if ( !result )
	{
		m_flExpressionLoopTime = gpGlobals->curtime + RandomFloat(30,40);
		return;
	}

	char const *szScene = response.GetResponsePtr();

	// Ignore updates that choose the same scene
	if ( m_iszExpressionScene != NULL_STRING && stricmp( STRING(m_iszExpressionScene), szScene ) == 0 )
		return;

	if ( m_hExpressionSceneEnt )
	{
		ClearExpression();
	}

	m_iszExpressionScene = AllocPooledString( szScene );
	float flDuration = InstancedScriptedScene( this, szScene, &m_hExpressionSceneEnt, 0.0, true, NULL );
	m_flExpressionLoopTime = gpGlobals->curtime + flDuration;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortal_Player::ClearExpression( void )
{
	if ( m_hExpressionSceneEnt != NULL )
	{ 
		StopScriptedScene( this, m_hExpressionSceneEnt );
	}
	m_flExpressionLoopTime = gpGlobals->curtime;
}
*/

void CPortal_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();

	SetLocalAngles( vOldAngles );
}

void CPortal_Player::PostThink( void )
{
	BaseClass::PostThink();

	// Regenerate heath after 3 seconds
	if (IsAlive() && GetHealth() < GetMaxHealth() && sv_regeneration_enable.GetBool())
	{
		// Color to overlay on the screen while the player is taking damage
		color32 hurtScreenOverlay = {64,0,0,64};

		if ( gpGlobals->curtime > m_fTimeLastHurt + sv_regeneration_wait_time.GetFloat() )
		{
			TakeHealth( 1, DMG_GENERIC );
			m_bIsRegenerating = true;

			if ( GetHealth() >= GetMaxHealth() )
			{
				m_bIsRegenerating = false;
			}
		}
		else
		{
			m_bIsRegenerating = false;
			if (gpGlobals->curtime < m_fTimeLastHurt + 1.0)
			{
				UTIL_ScreenFade(this, hurtScreenOverlay, 1.0f, 0.1f, FFADE_IN | FFADE_PURGE);
			}
		}
	}

	UpdatePortalPlaneSounds();
	UpdateWooshSounds();

	//if ( IsAlive() && m_flExpressionLoopTime >= 0 && gpGlobals->curtime > m_flExpressionLoopTime )
	//{
		// Random expressions need to be cleared, because they don't loop. So if we
		// pick the same one again, we want to restart it.
		//ClearExpression();
		//m_iszExpressionScene = NULL_STRING;
		//UpdateExpression();
	//}

	//UpdateSecondsTaken();

	// Try to fix the player if they're stuck
	if ( m_bStuckOnPortalCollisionObject )
	{
		Vector vForward = ((CProp_Portal*)m_hPortalEnvironment.Get())->m_vPrevForward;
		Vector vNewPos = GetAbsOrigin() + vForward * gpGlobals->frametime * -1000.0f;
		Teleport( &vNewPos, NULL, &vForward );
		m_bStuckOnPortalCollisionObject = false;
	}
}

void CPortal_Player::UpdatePortalPlaneSounds( void )
{
	CProp_Portal *pPortal = m_hPortalEnvironment;
	if ( pPortal && pPortal->m_bActivated )
	{
		Vector vVelocity;
		GetVelocity( &vVelocity, NULL );

		if ( !vVelocity.IsZero() )
		{
			Vector vMin, vMax;
			CollisionProp()->WorldSpaceAABB( &vMin, &vMax );

			Vector vEarCenter = ( vMax + vMin ) / 2.0f;
			Vector vDiagonal = vMax - vMin;

			if ( !m_bIntersectingPortalPlane )
			{
				vDiagonal *= 0.25f;

				if ( UTIL_IsBoxIntersectingPortal( vEarCenter, vDiagonal, pPortal ) )
				{
					m_bIntersectingPortalPlane = true;

					CPASAttenuationFilter filter( this );
					CSoundParameters params;
					if ( GetParametersForSound( "PortalPlayer.EnterPortal", params, NULL ) )
					{
						EmitSound_t ep( params );
						ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
						ep.m_flVolume = MIN( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

						EmitSound( filter, entindex(), ep );
					}
				}
			}
			else
			{
				vDiagonal *= 0.30f;

				if ( !UTIL_IsBoxIntersectingPortal( vEarCenter, vDiagonal, pPortal ) )
				{
					m_bIntersectingPortalPlane = false;

					CPASAttenuationFilter filter( this );
					CSoundParameters params;
					if ( GetParametersForSound( "PortalPlayer.ExitPortal", params, NULL ) )
					{
						EmitSound_t ep( params );
						ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
						ep.m_flVolume = MIN( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

						EmitSound( filter, entindex(), ep );
					}
				}
			}
		}
	}
	else if ( m_bIntersectingPortalPlane )
	{
		m_bIntersectingPortalPlane = false;

		CPASAttenuationFilter filter( this );
		CSoundParameters params;
		if ( GetParametersForSound( "PortalPlayer.ExitPortal", params, NULL ) )
		{
			EmitSound_t ep( params );
			Vector vVelocity;
			GetVelocity( &vVelocity, NULL );
			ep.m_nPitch = 80.0f + vVelocity.Length() * 0.03f;
			ep.m_flVolume = MIN( 0.3f + vVelocity.Length() * 0.00075f, 1.0f );

			EmitSound( filter, entindex(), ep );
		}
	}
}

void CPortal_Player::UpdateWooshSounds( void )
{
	if ( m_pWooshSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

		float fWooshVolume = GetAbsVelocity().Length() - MIN_FLING_SPEED;

		if ( fWooshVolume < 0.0f )
		{
			controller.SoundChangeVolume( m_pWooshSound, 0.0f, 0.1f );
			return;
		}

		fWooshVolume /= 2000.0f;
		if ( fWooshVolume > 1.0f )
			fWooshVolume = 1.0f;

		controller.SoundChangeVolume( m_pWooshSound, fWooshVolume, 0.1f );
		//		controller.SoundChangePitch( m_pWooshSound, fWooshVolume + 0.5f, 0.1f );
	}
}

extern int	gEvilImpulse101;

void CPortal_Player::Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity )
{
	Vector oldOrigin = GetLocalOrigin();
	QAngle oldAngles = GetLocalAngles();
	BaseClass::Teleport( newPosition, newAngles, newVelocity );
	m_angEyeAngles = pl.v_angle;

	// teleportation player animstate code now lives here
	QAngle absangles = GetAbsAngles();
	m_pPlayerAnimState->m_angRender = absangles;
	m_pPlayerAnimState->m_angRender.x = m_pPlayerAnimState->m_angRender.z = 0.0f;
	// Snap the yaw pose parameter lerping variables to face new angles.
	m_pPlayerAnimState->m_flCurrentFeetYaw = m_pPlayerAnimState->m_flGoalFeetYaw = m_pPlayerAnimState->m_flEyeYaw = EyeAngles()[YAW];
}

void CPortal_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
	case 101:
	{
		if (sv_cheats->GetBool())
		{
			gEvilImpulse101 = true;

			CBasePlayer::EquipSuit();

			// Give the player everything!
			CBaseCombatCharacter::GiveAmmo(255, "Pistol");
			CBaseCombatCharacter::GiveAmmo(255, "AR2");
			CBaseCombatCharacter::GiveAmmo(5, "AR2AltFire");
			CBaseCombatCharacter::GiveAmmo(255, "SMG1");
			CBaseCombatCharacter::GiveAmmo(255, "Buckshot");
			CBaseCombatCharacter::GiveAmmo(3, "smg1_grenade");
			CBaseCombatCharacter::GiveAmmo(3, "rpg_round");
			CBaseCombatCharacter::GiveAmmo(5, "grenade");
			CBaseCombatCharacter::GiveAmmo(32, "357");
			CBaseCombatCharacter::GiveAmmo(16, "XBowBolt");
#ifdef HL2_EPISODIC
			CBaseCombatCharacter::GiveAmmo(5, "Hopwire");
#endif		
			GiveNamedItem("weapon_smg1");
			GiveNamedItem("weapon_frag");
			GiveNamedItem("weapon_crowbar");
			GiveNamedItem("weapon_pistol");
			GiveNamedItem("weapon_ar2");
			GiveNamedItem("weapon_shotgun");
			GiveNamedItem("weapon_physcannon");
			GiveNamedItem("weapon_bugbait");
			GiveNamedItem("weapon_rpg");
			GiveNamedItem("weapon_357");
			GiveNamedItem("weapon_crossbow");
#ifdef HL2_EPISODIC
			// GiveNamedItem( "weapon_magnade" );
#endif
			if (GetHealth() < 100)
			{
				TakeHealth(25, DMG_GENERIC);
			}
			
			CWeaponPortalgun *pPortalGun = static_cast<CWeaponPortalgun*>(GiveNamedItem("weapon_portalgun"));

			if (!pPortalGun)
			{
				pPortalGun = static_cast<CWeaponPortalgun*>(Weapon_OwnsThisType("weapon_portalgun"));
			}

			if (pPortalGun)
			{
				pPortalGun->SetCanFirePortal1();
				pPortalGun->SetCanFirePortal2();
			}

			gEvilImpulse101 = false;
		}
	}
	break;
	case 102:
	{
		CWeaponPortalgun *pPortalGun = static_cast<CWeaponPortalgun*>(GiveNamedItem("weapon_portalgun"));

		if (!pPortalGun)
		{
			pPortalGun = static_cast<CWeaponPortalgun*>(Weapon_OwnsThisType("weapon_portalgun"));
		}

		if (pPortalGun)
		{
			pPortalGun->SetCanFirePortal1();
			pPortalGun->SetCanFirePortal2();
		}
	}
	break;

	default:
		BaseClass::CheatImpulseCommands( iImpulse );
	}
}

void CPortal_Player::ForceDuckThisFrame( void )
{
	if( m_Local.m_bDucked != true )
	{
		//m_Local.m_bDucking = false;
		m_Local.m_bDucked = true;
		ForceButtons( IN_DUCK );
		AddFlag( FL_DUCKING );
		SetVCollisionState( GetAbsOrigin(), GetAbsVelocity(), VPHYS_CROUCH );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update the area bits variable which is networked down to the client to determine
//			which area portals should be closed based on visibility.
// Input  : *pvs - pvs to be used to determine visibility of the portals
//-----------------------------------------------------------------------------
void CPortal_Player::UpdatePortalViewAreaBits( unsigned char *pvs, int pvssize )
{
	Assert ( pvs );

	int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
	if( iPortalCount == 0 )
		return;

	CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
	int *portalArea = (int *)stackalloc( sizeof( int ) * iPortalCount );
	bool *bUsePortalForVis = (bool *)stackalloc( sizeof( bool ) * iPortalCount );

	unsigned char *portalTempBits = (unsigned char *)stackalloc( sizeof( unsigned char ) * 32 * iPortalCount );
	COMPILE_TIME_ASSERT( (sizeof( unsigned char ) * 32) >= sizeof( ((CPlayerLocalData*)0)->m_chAreaBits ) );

	// setup area bits for these portals
	for ( int i = 0; i < iPortalCount; ++i )
	{
		CProp_Portal* pLocalPortal = pPortals[ i ];
		// Make sure this portal is active before adding it's location to the pvs
		if ( pLocalPortal && pLocalPortal->m_bActivated )
		{
			CProp_Portal* pRemotePortal = pLocalPortal->m_hLinkedPortal.Get();

			// Make sure this portal's linked portal is in the PVS before we add what it can see
			if ( pRemotePortal && pRemotePortal->m_bActivated && pRemotePortal->NetworkProp() && 
				pRemotePortal->NetworkProp()->IsInPVS( edict(), pvs, pvssize ) )
			{
				portalArea[ i ] = engine->GetArea( pPortals[ i ]->GetAbsOrigin() );

				if ( portalArea [ i ] >= 0 )
				{
					bUsePortalForVis[ i ] = true;
				}

				engine->GetAreaBits( portalArea[ i ], &portalTempBits[ i * 32 ], sizeof( unsigned char ) * 32 );
			}
		}
	}

	// Use the union of player-view area bits and the portal-view area bits of each portal
	for ( int i = 0; i < m_Local.m_chAreaBits.Count(); i++ )
	{
		for ( int j = 0; j < iPortalCount; ++j )
		{
			// If this portal is active, in PVS and it's location is valid
			if ( bUsePortalForVis[ j ]  )
			{
				m_Local.m_chAreaBits.Set( i, m_Local.m_chAreaBits[ i ] | portalTempBits[ (j * 32) + i ] );
			}
		}
	}
}
