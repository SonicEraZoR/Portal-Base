//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PORTAL_CLIENTMODE_H
#define PORTAL_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui/Cursor.h>

class CHudViewport;

namespace vgui
{
	typedef unsigned long HScheme;
}

class ClientModePortalNormal : public ClientModeShared 
{
public:
	DECLARE_CLASS(ClientModePortalNormal, ClientModeShared);

					ClientModePortalNormal();
	virtual			~ClientModePortalNormal();

	virtual void	Init();
	virtual bool	ShouldDrawCrosshair(void);
};


extern IClientMode *GetClientModeNormal();
extern ClientModePortalNormal* GetClientModePortalNormal();

extern vgui::HScheme g_hVGuiCombineScheme;


#endif // PORTAL_CLIENTMODE_H
