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
#include "c_func_liquidportal.h"

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

	virtual int DrawModel( int flags );
	virtual void AddEntity( void );

	virtual bool			ShouldDraw( void );
	virtual void			OnPreDataChanged( DataUpdateType_t type );
	virtual void			OnDataChanged( DataUpdateType_t type );
	bool					DetectAndHandlePortalTeleportation( void ); //detects if the player has portalled and fixes views
	virtual void			PreThink( void );
	virtual void			DoImpactEffect( trace_t &tr, int nDamageType );

	void					PlayerPortalled( C_Prop_Portal *pEnteredPortal );

	virtual void	CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	void			CalcPortalView( Vector &eyeOrigin, QAngle &eyeAngles );
	virtual void	CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles);

	inline bool		IsCloseToPortal( void ) //it's usually a good idea to turn on draw hacks when this is true
	{
		return (m_hPortalEnvironment.Get() != NULL);	
	} 

	void ToggleHeldObjectOnOppositeSideOfPortal( void ) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal( bool p_bHeldObjectOnOppositeSideOfPortal ) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal( void ) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal( void ) { return m_pHeldObjectPortal; }

	bool IsSuppressingCrosshair( void ) { return m_bSuppressingCrosshair; }

private:

	bool m_bSuppressingCrosshair;
	bool  m_bHeldObjectOnOppositeSideOfPortal;
	CProp_Portal *m_pHeldObjectPortal;

	int	m_iForceNoDrawInPortalSurface; //only valid for one frame, used to temp disable drawing of the player model in a surface because of freaky artifacts

	QAngle	m_qEyeAngles_LastCalcView; //we've got some VERY persistent single frame errors while teleporting, this will be updated every frame in CalcView() and will serve as a central source for fixed angles

	bool	m_bPortalledMessagePending; //Player portalled. It's easier to wait until we get a OnDataChanged() event or a CalcView() before we do anything about it. Otherwise bits and pieces can get undone
	VMatrix m_PendingPortalMatrix;

	Vector	m_vEyePosition;
public:
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	struct PreDataChanged_Backup_t
	{
		CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal;
		QAngle					m_qEyeAngles;
	} PreDataChanged_Backup;

	bool	m_bPitchReorientation;
	float	m_fReorientationRate;
	bool	m_bEyePositionIsTransformedByPortal; //when the eye and body positions are not on the same side of a portal

	CHandle<C_Prop_Portal>	m_hPortalEnvironment; //a portal whose environment the player is currently in, should be invalid most of the time
	CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal; //a liquid portal whose volume the player is standing in
};

inline C_Portal_Player *ToPortalPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_Portal_Player*>( pEntity );
}

inline C_Portal_Player *GetPortalPlayer( void )
{
	return static_cast<C_Portal_Player*>( C_BasePlayer::GetLocalPlayer() );
}

#endif //PORTAL_PLAYER_H
