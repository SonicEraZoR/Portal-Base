//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "clientmode_hlnormal.h"
#include "clientmode_portal.h"
#include "vgui_int.h"
#include "hud.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "iinput.h"
#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool g_bRollingCredits;

ConVar fov_desired("fov_desired", "75", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.", true, 75.0, true, 90.0);

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
vgui::HScheme g_hVGuiCombineScheme = 0;


// Instance the singleton and expose the interface to it.
IClientMode *GetClientModeNormal()
{
	static ClientModePortalNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}

//extern EHANDLE g_eKillTarget1;
//extern EHANDLE g_eKillTarget2;
//void MsgFunc_KillCam(bf_read &msg) 
//{
//	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
//
//	if ( !pPlayer )
//		return;
//
//	g_eKillTarget1 = 0;
//	g_eKillTarget2 = 0;
//	g_nKillCamTarget1 = 0;
//	g_nKillCamTarget1 = 0;
//
//	long iEncodedEHandle = msg.ReadLong();
//
//	if( iEncodedEHandle == INVALID_NETWORKED_EHANDLE_VALUE )
//		return;	
//
//	int iEntity = iEncodedEHandle & ((1 << MAX_EDICT_BITS) - 1);
//	int iSerialNum = iEncodedEHandle >> MAX_EDICT_BITS;
//
//	EHANDLE hEnt1( iEntity, iSerialNum );
//
//	iEncodedEHandle = msg.ReadLong();
//
//	if( iEncodedEHandle == INVALID_NETWORKED_EHANDLE_VALUE )
//		return;	
//
//	iEntity = iEncodedEHandle & ((1 << MAX_EDICT_BITS) - 1);
//	iSerialNum = iEncodedEHandle >> MAX_EDICT_BITS;
//
//	EHANDLE hEnt2( iEntity, iSerialNum );
//
//	g_eKillTarget1 = hEnt1;
//	g_eKillTarget2 = hEnt2;
//
//	if ( g_eKillTarget1.Get() )
//	{
//		g_nKillCamTarget1	= g_eKillTarget1.Get()->entindex();
//	}
//
//	if ( g_eKillTarget2.Get() )
//	{
//		g_nKillCamTarget2	= g_eKillTarget2.Get()->entindex();
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
private:
	DECLARE_CLASS_SIMPLE( CHudViewport, CBaseViewport );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		gHUD.InitColors( pScheme );

		SetPaintBackgroundEnabled( false );
	}

	virtual void CreateDefaultPanels(void) { /* don't create any panels yet*/ };
};


//-----------------------------------------------------------------------------
// ClientModePortalNormal implementation
//-----------------------------------------------------------------------------
ClientModePortalNormal::ClientModePortalNormal()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start(gameuifuncs, gameeventmanager);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModePortalNormal::~ClientModePortalNormal()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModePortalNormal::Init()
{
	BaseClass::Init();

	// Load up the combine control panel scheme
	g_hVGuiCombineScheme = vgui::scheme()->LoadSchemeFromFileEx(enginevgui->GetPanel(PANEL_CLIENTDLL), IsXbox() ? "resource/ClientScheme.res" : "resource/CombinePanelScheme.res", "CombineScheme");
	if (!g_hVGuiCombineScheme)
	{
		Warning("Couldn't load combine panel scheme!\n");
	}
}

bool ClientModePortalNormal::ShouldDrawCrosshair(void)
{
	return (g_bRollingCredits == false);
}

ClientModePortalNormal* GetClientModePortalNormal()
{
	Assert( dynamic_cast< ClientModePortalNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModePortalNormal* >( GetClientModeNormal() );
}
