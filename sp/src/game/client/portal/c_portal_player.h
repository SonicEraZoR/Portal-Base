//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#ifndef PORTAL_PLAYER_H
#define PORTAL_PLAYER_H
#pragma once

#include "c_basehlplayer.h"
#include "portal_player_shared.h"
#include "c_prop_portal.h"
#include "weapon_portalbase.h"
#include "c_func_liquidportal.h"
#include "colorcorrectionmgr.h"

//=============================================================================
// >> Portal_Player
//=============================================================================
class C_Portal_Player : public C_BaseHLPlayer
{
public:
	DECLARE_CLASS( C_Portal_Player, C_BaseHLPlayer );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();


	C_Portal_Player();
	~C_Portal_Player( void );

	void ClientThink( void );
	void FixTeleportationRoll( void );

	static inline C_Portal_Player* GetLocalPortalPlayer()
	{
		return (C_Portal_Player*)C_BasePlayer::GetLocalPlayer();
	}

	static inline C_Portal_Player* GetLocalPlayer()
	{
		return (C_Portal_Player*)C_BasePlayer::GetLocalPlayer();
	}

	void ToggleHeldObjectOnOppositeSideOfPortal(void) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal(bool p_bHeldObjectOnOppositeSideOfPortal) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal(void) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal(void) { return m_pHeldObjectPortal; }

	void PlayerPortalled(C_Prop_Portal *pEnteredPortal);

	bool IsSuppressingCrosshair(void) { return m_bSuppressingCrosshair; }

	virtual bool ShouldDraw(void);

	inline bool IsCloseToPortal(void) //it's usually a good idea to turn on draw hacks when this is true
	{
		return ((PortalEyeInterpolation.m_bEyePositionIsInterpolating) || (m_hPortalEnvironment.Get() != NULL));
	}

	CHandle<C_Prop_Portal>	m_hPortalEnvironment; //a portal whose environment the player is currently in, should be invalid most of the time
	CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal; //a liquid portal whose volume the player is standing in
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	bool	m_bEyePositionIsTransformedByPortal; //when the eye and body positions are not on the same side of a portal

	bool	m_bPitchReorientation;
	float	m_fReorientationRate;

	struct PreDataChanged_Backup_t
	{
		QAngle					m_qEyeAngles;
	} PreDataChanged_Backup;
	
private:
	bool m_bSuppressingCrosshair;
	bool  m_bHeldObjectOnOppositeSideOfPortal;
	CProp_Portal *m_pHeldObjectPortal;

	bool	m_bPortalledMessagePending; //Player portalled. It's easier to wait until we get a OnDataChanged() event or a CalcView() before we do anything about it. Otherwise bits and pieces can get undone
	VMatrix m_PendingPortalMatrix;

	struct PortalEyeInterpolation_t
	{
		bool	m_bEyePositionIsInterpolating; //flagged when the eye position would have popped between two distinct positions and we're smoothing it over
	} PortalEyeInterpolation;
};

inline C_Portal_Player *ToPortalPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<C_Portal_Player*>(pEntity);
}

inline C_Portal_Player *GetPortalPlayer(void)
{
	return static_cast<C_Portal_Player*>(C_BasePlayer::GetLocalPlayer());
}

#endif //PORTAL_PLAYER_H
