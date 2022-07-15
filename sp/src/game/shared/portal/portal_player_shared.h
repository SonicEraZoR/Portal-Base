//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef PORTAL_PLAYER_SHARED_H
#define PORTAL_PLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// Shared header file for players
#if defined( CLIENT_DLL )
#define CPortal_Player C_Portal_Player
#include "c_portal_player.h"
#else
#include "portal_player.h"
#endif


#endif //PORTAL_PLAYER_SHARED_h
