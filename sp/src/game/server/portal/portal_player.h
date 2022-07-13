//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef PORTAL_PLAYER_H
#define PORTAL_PLAYER_H
#pragma once

#include "hl2_player.h"
#include "prop_portal.h"
#include "in_buttons.h"
#include "func_liquidportal.h"

//=============================================================================
// >> Portal_Player
//=============================================================================
class CPortal_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CPortal_Player, CHL2_Player );

	CPortal_Player();
	~CPortal_Player( void );
	
	static CPortal_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CPortal_Player::s_PlayerEdict = ed;
		return (CPortal_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void CreateSounds( void );
	virtual void StopLoopingSounds( void );
	virtual void Spawn( void );

	virtual void PostThink( void );
	virtual void PreThink( void );

	void UpdatePortalPlaneSounds( void );
	void UpdateWooshSounds( void );

	virtual void Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity );

	virtual void PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper);

	virtual int	OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void ShutdownUseEntity( void );

	virtual const Vector&	WorldSpaceCenter( ) const;

	virtual void VPhysicsShadowUpdate( IPhysicsObject *pPhysics );

	bool UseFoundEntity( CBaseEntity *pUseEntity );
	CBaseEntity* FindUseEntityThroughPortal( void );

	virtual void PlayerUse( void );

	virtual void SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );
	virtual void UpdatePortalViewAreaBits( unsigned char *pvs, int pvssize );

	void CheatImpulseCommands( int iImpulse );

	void ForceDuckThisFrame( void );
	virtual void ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldingThis );

	void ToggleHeldObjectOnOppositeSideOfPortal( void ) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal( bool p_bHeldObjectOnOppositeSideOfPortal ) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal( void ) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal( void ) { return m_pHeldObjectPortal; }
	void SetHeldObjectPortal( CProp_Portal *pPortal ) { m_pHeldObjectPortal = pPortal; }

	void SetStuckOnPortalCollisionObject( void ) { m_bStuckOnPortalCollisionObject = true; }

	void SetNeuroToxinDamageTime( float fCountdownSeconds ) { m_fNeuroToxinDamageTime = gpGlobals->curtime + fCountdownSeconds; }

	void IncNumCamerasDetatched( void ) { ++m_iNumCamerasDetatched; }
	int GetNumCamerasDetatched( void ) const { return m_iNumCamerasDetatched; }

	bool m_bSilentDropAndPickup;

	void SuppressCrosshair( bool bState ) { m_bSuppressingCrosshair = bState; }
	
private:
	CSoundPatch		*m_pWooshSound;

	CNetworkVar( bool, m_bSuppressingCrosshair );

	CNetworkVar( bool, m_bHeldObjectOnOppositeSideOfPortal );
	CNetworkHandle( CProp_Portal, m_pHeldObjectPortal );	// networked entity handle

	bool m_bIntersectingPortalPlane;
	bool m_bStuckOnPortalCollisionObject;

	float m_fTimeLastHurt;
	bool  m_bIsRegenerating;		// Is the player currently regaining health

	float m_fNeuroToxinDamageTime;

	int		m_iNumCamerasDetatched;

	QAngle						m_qPrePortalledViewAngles;
	bool						m_bFixEyeAnglesFromPortalling;
	VMatrix						m_matLastPortalled;

	mutable Vector m_vWorldSpaceCenterHolder; //WorldSpaceCenter() returns a reference, need an actual value somewhere



public:

	CNetworkVar( bool, m_bPitchReorientation );
	CNetworkHandle( CProp_Portal, m_hPortalEnvironment ); //if the player is in a portal environment, this is the associated portal
	CNetworkHandle( CFunc_LiquidPortal, m_hSurroundingLiquidPortal ); //if the player is standing in a liquid portal, this will point to it

	friend class CProp_Portal;


#ifdef PORTAL_MP
public:
	virtual CBaseEntity* EntSelectSpawnPoint( void );
	void PickTeam( void );
#endif
};

inline CPortal_Player *ToPortalPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CPortal_Player*>( pEntity );
}

inline CPortal_Player *GetPortalPlayer( int iPlayerIndex )
{
	return static_cast<CPortal_Player*>( UTIL_PlayerByIndex( iPlayerIndex ) );
}

#endif //PORTAL_PLAYER_H
