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

#include "player.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "portal_player_shared.h"
#include "prop_portal.h"
#include "weapon_portalbase.h"
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

	virtual void Precache(void);
	virtual void CreateSounds(void);
	virtual void StopLoopingSounds(void);
	virtual void Spawn(void);
	
	virtual void PostThink(void);
	virtual void PreThink(void);

	void SetPlayerModel(void);
	bool ValidatePlayerModel(const char *pModel);

	void UpdatePortalPlaneSounds(void);
	void UpdateWooshSounds(void);

	void SuppressCrosshair(bool bState) { m_bSuppressingCrosshair = bState; }

	void IncNumCamerasDetatched(void) { ++m_iNumCamerasDetatched; }
	int GetNumCamerasDetatched(void) const { return m_iNumCamerasDetatched; }

	virtual void UpdatePortalViewAreaBits(unsigned char *pvs, int pvssize);

	void ToggleHeldObjectOnOppositeSideOfPortal(void) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal(bool p_bHeldObjectOnOppositeSideOfPortal) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal(void) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal(void) { return m_pHeldObjectPortal; }
	void SetHeldObjectPortal(CProp_Portal *pPortal) { m_pHeldObjectPortal = pPortal; }

	bool m_bSilentDropAndPickup;

	void ForceDuckThisFrame(void);

	void SetStuckOnPortalCollisionObject(void) { m_bStuckOnPortalCollisionObject = true; }

	void CheatImpulseCommands(int iImpulse);

	virtual void Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity);

	CNetworkHandle(CProp_Portal, m_hPortalEnvironment); //if the player is in a portal environment, this is the associated portal
	CNetworkHandle(CFunc_LiquidPortal, m_hSurroundingLiquidPortal); //if the player is standing in a liquid portal, this will point to it

	CNetworkVar(bool, m_bPitchReorientation);

	friend class CProp_Portal;
private:
	CSoundPatch		*m_pWooshSound;

	float m_fTimeLastHurt;
	bool  m_bIsRegenerating;		// Is the player currently regaining health

	bool m_bIntersectingPortalPlane;
	bool m_bStuckOnPortalCollisionObject;

	int m_iNumCamerasDetatched;

	QAngle m_qPrePortalledViewAngles;
	bool m_bFixEyeAnglesFromPortalling;
	VMatrix m_matLastPortalled;
	
	CNetworkVar(bool, m_bHeldObjectOnOppositeSideOfPortal);
	CNetworkHandle(CProp_Portal, m_pHeldObjectPortal);	// networked entity handle

	CNetworkVar(bool, m_bSuppressingCrosshair);
};

inline CPortal_Player *ToPortalPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<CPortal_Player*>(pEntity);
}

inline CPortal_Player *GetPortalPlayer(int iPlayerIndex)
{
	return static_cast<CPortal_Player*>(UTIL_PlayerByIndex(iPlayerIndex));
}

#endif //PORTAL_PLAYER_H
