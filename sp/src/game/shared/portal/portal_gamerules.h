//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game rules for Portal.
//
//=============================================================================//

#ifdef PORTAL_MP



#include "portal_mp_gamerules.h" //redirect to multiplayer gamerules in multiplayer builds



#else

#ifndef PORTAL_GAMERULES_H
#define PORTAL_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "gamerules.h"
#include "hl2_gamerules.h"

#ifdef CLIENT_DLL
	#define CPortalGameRules C_PortalGameRules
	#define CPortalGameRulesProxy C_PortalGameRulesProxy
#endif

#if defined ( CLIENT_DLL )
#include "steam/steam_api.h"
#endif


class CPortalGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CPortalGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};


class CPortalGameRules : public CHalfLife2
{
public:
	DECLARE_CLASS(CPortalGameRules, CHalfLife2); //mygamepedia: it was CSingleplayRules rules instead of CHalfLife2
												 //this should fix multiple issues, such as mega physcannon, hope nothing will be broken

	virtual bool	Init();

	CNetworkVar(bool, m_bTeamPlayEnabled);

#ifdef CLIENT_DLL
	virtual bool IsBonusChallengeTimeBased( void );
	virtual bool IsMultiplayer();
	virtual bool FAllowFlashlight();
	virtual bool ShouldCollide(int collisionGroup0, int collisionGroup1);
#endif

private:
#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

	CPortalGameRules();
	virtual ~CPortalGameRules() {}

	virtual void			Think( void );

	virtual bool			ClientCommand( CBaseEntity *pEdict, const CCommand &args );

	virtual const char*		GetGameDescription( void ) { return "Portal"; }

	virtual void			PlayerThink( CBasePlayer *pPlayer );

	//mygamepedia: these are modded for dif tasks we need with portal-base
	virtual bool			IsMultiplayer();											//logical way to find out if we are in mp
	virtual bool			ShouldBurningPropsEmitLight();								//orig does nothing, this is why i made it optional
	virtual bool			ShouldRemoveRadio();										//remove radio depending on cvar val
	virtual bool			FAllowFlashlight();											//use mp_flashlight to allow flashlight in mp
	virtual void			PlayerSpawn(CBasePlayer* pPlayer);							//add optional def items spawn for mp
	virtual bool			ShouldCollide(int collisionGroup0, int collisionGroup1);	//make wpns & plr collide optional
	virtual	bool			IsTeamplay() { return m_bTeamPlayEnabled; }					//is teamplay on?

public:

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );

private:

	int						DefaultFOV( void ) { return 75; }
#endif
};


//-----------------------------------------------------------------------------
// Gets us at the Half-Life 2 game rules
//-----------------------------------------------------------------------------
inline CPortalGameRules* PortalGameRules()
{
	return static_cast<CPortalGameRules*>(g_pGameRules);
}

#endif // PORTAL_GAMERULES_H
#endif
