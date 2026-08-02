//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Half-Life 2 game rules, such as the relationship tables and ammo
//			damage cvars.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "portal_gamerules.h"
#include "ammodef.h"
#include "hl2_shareddefs.h"
#include "portal_shareddefs.h"

#ifdef CLIENT_DLL
#else
	#include "player.h"
	#include "game.h"
	#include "gamerules.h"
	#include "teamplay_gamerules.h"
	#include "portal_player.h"
	#include "globalstate.h"
	#include "ai_basenpc.h"
	#include "portal/weapon_physcannon.h"
	#include "props.h"		// For props flags used in making the portal weight box
	#include "datacache/imdlcache.h"	// For precaching box model
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


REGISTER_GAMERULES_CLASS( CPortalGameRules );

BEGIN_NETWORK_TABLE_NOBASE( CPortalGameRules, DT_PortalGameRules )
	#ifdef CLIENT_DLL
		RecvPropBool(RECVINFO(m_bMegaPhysgun)),
		RecvPropBool(RECVINFO(m_bTeamPlayEnabled)),
	#else
		SendPropBool(SENDINFO(m_bMegaPhysgun)),
		SendPropBool(SENDINFO(m_bTeamPlayEnabled)),
	#endif
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( portal_gamerules, CPortalGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( PortalGameRulesProxy, DT_PortalGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_PortalGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CPortalGameRules *pRules = PortalGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CPortalGameRulesProxy, DT_PortalGameRulesProxy )
		RecvPropDataTable( "portal_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_PortalGameRules ), RecvProxy_PortalGameRules )
	END_RECV_TABLE()
#else
	void* SendProxy_PortalGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CPortalGameRules *pRules = PortalGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CPortalGameRulesProxy, DT_PortalGameRulesProxy )
		SendPropDataTable( "portal_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_PortalGameRules ), SendProxy_PortalGameRules )
	END_SEND_TABLE()
#endif


extern ConVar	sv_robust_explosions;
extern ConVar	sk_allow_autoaim;
extern ConVar	sk_autoaim_scale1;
extern ConVar	sk_autoaim_scale2;

#if !defined ( CLIENT_DLL )
extern ConVar	sv_alternateticks;
extern ConVar	sv_portalbase_portal_trace_filter_file;
#endif // !CLIENT_DLL

#define PORTAL_WEIGHT_BOX_MODEL_NAME "models/props/metal_box.mdl"

// Portal-only con commands

#ifndef CLIENT_DLL
// Create the box used for portal puzzles, named 'box'. Used for easy debugging of portal puzzles.
void CC_Create_PortalWeightBox( void )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Try to create entity
	CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName("prop_physics") );
	if (entity)
	{
		entity->PrecacheModel( PORTAL_WEIGHT_BOX_MODEL_NAME );
		entity->SetModel( PORTAL_WEIGHT_BOX_MODEL_NAME );
		entity->SetName( MAKE_STRING("box") );
		entity->AddSpawnFlags( SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
		entity->Precache();
		DispatchSpawn(entity);

		// Now attempt to drop into the world
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors( &forward );
		UTIL_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH,MASK_SOLID, 
			pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			tr.endpos.z += 12;
			entity->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( entity, MASK_SOLID );
		}
	}
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand ent_create_portal_weight_box("ent_create_portal_weight_box", CC_Create_PortalWeightBox, "Creates a weight box used in portal puzzles at the location the player is looking.", FCVAR_GAMEDLL | FCVAR_CHEAT);
#endif // CLIENT_DLL


#define PORTAL_METAL_SPHERE_MODEL_NAME "models/props/sphere.mdl"

#ifndef CLIENT_DLL
// Create a very reflective bouncy metal sphere
void CC_Create_PortalMetalSphere( void )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	// Try to create entity
	CBaseEntity *entity = dynamic_cast< CBaseEntity * >( CreateEntityByName("prop_physics") );
	if (entity)
	{
		entity->PrecacheModel( PORTAL_METAL_SPHERE_MODEL_NAME );
		entity->SetModel( PORTAL_METAL_SPHERE_MODEL_NAME );
		entity->SetName( MAKE_STRING("sphere") );
		entity->AddSpawnFlags( SF_PHYSPROP_ENABLE_PICKUP_OUTPUT );
		entity->Precache();
		DispatchSpawn(entity);

		// Now attempt to drop into the world
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors( &forward );
		UTIL_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH,MASK_SOLID, 
			pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			tr.endpos.z += 12;
			entity->Teleport( &tr.endpos, NULL, NULL );
			UTIL_DropToFloor( entity, MASK_SOLID );
		}
	}
	CBaseEntity::SetAllowPrecache( allowPrecache );
}
static ConCommand ent_create_portal_metal_sphere("ent_create_portal_metal_sphere", CC_Create_PortalMetalSphere, "Creates a reflective metal sphere where the player is looking.", FCVAR_GAMEDLL | FCVAR_CHEAT);
#endif // CLIENT_DLL



#ifdef CLIENT_DLL //{


#else //}{

	extern bool		g_fGameOver;
	
	//-----------------------------------------------------------------------------
	// Purpose:
	// Input  :
	// Output :
	//-----------------------------------------------------------------------------
	CPortalGameRules::CPortalGameRules()
	{
		m_bMegaPhysgun = false;

		m_flLastHealthDropTime = 0.0f;
		m_flLastGrenadeDropTime = 0.0f;

		m_bMegaPhysgun = false;

		m_bTeamPlayEnabled = teamplay.GetBool();

		g_pCVar->FindVar( "sv_maxreplay" )->SetValue( "1.5" );
	}


	//-----------------------------------------------------------------------------
	// Purpose: called each time a player uses a "cmd" command
	// Input  : *pEdict - the player who issued the command
	//			Use engine.Cmd_Argv,  engine.Cmd_Argv, and engine.Cmd_Argc to get 
	//			pointers the character string command.
	//-----------------------------------------------------------------------------
	bool CPortalGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
	{
		if( CSingleplayRules::ClientCommand( pEdict, args ) ) //mygamepedia: CSingleplayRules!!! NOT BaseClass!!!
			return true;

		CPortal_Player *pPlayer = (CPortal_Player *) pEdict;

		if ( pPlayer->ClientCommand( args ) )
			return true;

		return false;
	}

	ConVar	sv_portalbase_portalgamerules_mp_equip_players ("sv_portalbase_portalgamerules_mp_equip_players", "1",
		FCVAR_REPLICATED,
		"Equip players with default set of items in multiplayer.",
		true, 0, true, 1);

	//-----------------------------------------------------------------------------
	// Purpose: Player has just spawned. Equip them in multiplayer if enabled. Orig does nothing. - MyGamepedia
	//-----------------------------------------------------------------------------
	void CPortalGameRules::PlayerSpawn( CBasePlayer *pPlayer )
	{
		//not in mp or feature is disabled
		if (gpGlobals->maxClients < 2 || !sv_portalbase_portalgamerules_mp_equip_players.GetBool())
			return;

		bool		addDefault;
		CBaseEntity* pWeaponEntity = NULL;

		pPlayer->EquipSuit();

		addDefault = true;

		while ((pWeaponEntity = gEntList.FindEntityByClassname(pWeaponEntity, "game_player_equip")) != NULL)
		{
			pWeaponEntity->Touch(pPlayer);
			addDefault = false;
		}
	}

	//MyGamepedia: make it more flexible, orig does nothing
	ConVar	sv_portalbase_portalgamerules_mp_stop_player_while_game_over("sv_portalbase_portalgamerules_mp_stop_player_while_game_over", "0",
		FCVAR_REPLICATED,
		"Game over prevents movement for all players in multiplayer.",
		true, 0, true, 1);

	void CPortalGameRules::PlayerThink( CBasePlayer *pPlayer )
	{
		if (!sv_portalbase_portalgamerules_mp_stop_player_while_game_over.GetBool())
			return;

		if (g_fGameOver)
		{
			// clear attack/use commands from player
			pPlayer->m_afButtonPressed = 0;
			pPlayer->m_nButtons = 0;
			pPlayer->m_afButtonReleased = 0;
		}
	}

	void CPortalGameRules::Think( void )
	{
		BaseClass::Think();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Returns how much damage the given ammo type should do to the victim
	//			when fired by the attacker.
	// Input  : pAttacker - Dude what shot the gun.
	//			pVictim - Dude what done got shot.
	//			nAmmoType - What been shot out.
	// Output : How much hurt to put on dude what done got shot (pVictim).
	//-----------------------------------------------------------------------------

	//MyGamepedia: make it more flexible, taked portal, sp, mp versions
	ConVar	sv_portalbase_portalgamerules_fall_damage_type("sv_portalbase_portalgamerules_fall_damage_type", "1",
		FCVAR_REPLICATED,
		"The way the player will receive fall damage.\n 0 - Portal, no fall damage.\n 1 - Half Life 2 singleplayer, full fall damage.\n 2 - Half Life 2 multiplayer, full fall damage or 10 points, depending on mp_falldamage.",
		true, 0, true, 2);

	float CPortalGameRules::FlPlayerFallDamage(CBasePlayer* pPlayer)
	{
		int iType = sv_portalbase_portalgamerules_fall_damage_type.GetInt();

		//Portal
		if (iType == 0)
			return 0.0f;

		//HL2
		if (iType == 1)
		{
			// subtract off the speed at which a player is allowed to fall without being hurt,
			// so damage will be based on speed beyond that, not the entire fall
			pPlayer->m_Local.m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
			return pPlayer->m_Local.m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		}

		//HL2DM
		if (falldamage.GetBool())
		{
			pPlayer->m_Local.m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
			return pPlayer->m_Local.m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		}
		else
			return 10;
	}


#endif //} !CLIENT_DLL


bool bLoadedPortalTraceByGamerules = false;

//-----------------------------------------------------------------------------
// Purpose: On starting a game, make global state changes specific to portal
//-----------------------------------------------------------------------------
bool CPortalGameRules::Init()
{
#if !defined ( CLIENT_DLL )
	// Portal never wants alternate ticks. Some low end hardware sets it in dxsupport.cfg so this will catch those cases.
	sv_alternateticks.SetValue( 0 );

	//mygamepedia: on game start, load portalgun tracer filter here
	UTIL_Porta_LoadPortalTraceFilterList(sv_portalbase_portal_trace_filter_file.GetString());
#endif

	return BaseClass::Init();
}

ConVar	sv_portalbase_portalgamerules_use_classic_shouldcollide("sv_portalbase_portalgamerules_use_classic_shouldcollide", "1",
	FCVAR_REPLICATED,
	"Use classic should collide from Half Life 2 instead of Portal version. For instant, if this is disabled, the player will collide with weapons and other pickups.",
	true, 0, true, 1);

// ------------------------------------------------------------------------------------
// Purpose: Run hl2 ShouldCollide if wanted - MyGamepedia
// ------------------------------------------------------------------------------------
bool CPortalGameRules::ShouldCollide(int collisionGroup0, int collisionGroup1)
{
	// If it's a portal, we want to collide with it!
	/*if ( collisionGroup0 == PORTALCOLLISION_GROUP_PORTAL && collisionGroup1 != PORTALCOLLISION_GROUP_PORTAL ||
		 collisionGroup1 == PORTALCOLLISION_GROUP_PORTAL && collisionGroup0 != PORTALCOLLISION_GROUP_PORTAL )
	{
		return true;
	}*/

	//normal
	if (sv_portalbase_portalgamerules_use_classic_shouldcollide.GetBool())
		return BaseClass::ShouldCollide(collisionGroup0, collisionGroup1);

	//this will run orig method version
	return CSingleplayRules::ShouldCollide(collisionGroup0, collisionGroup1);
}

//-----------------------------------------------------------------------------
// Purpose: Are we in mp ? - MyGamepedia
//-----------------------------------------------------------------------------
bool CPortalGameRules::IsMultiplayer()
{
	if (gpGlobals->maxClients < 2)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Are plrs allowed to use flashlight in mp ? - MyGamepedia
//-----------------------------------------------------------------------------
bool CPortalGameRules::FAllowFlashlight()
{
	ConVarRef flashlight("mp_flashlight");
	return flashlight.GetBool();
}

#ifndef CLIENT_DLL
#ifdef HL2_EPISODIC
//MyGamepedia: make it optional cuz orig portal gamerules always return false
ConVar	sv_portalbase_portalgamerules_should_burning_props_emit_light("sv_portalbase_portalgamerules_should_burning_props_emit_light", "1",
	FCVAR_REPLICATED,
	"Should burning props emit light when Alyx is in darkness mode ? This is episodic feature only.",
	true, 0, true, 1);
#endif // HL2_EPISODIC

//-----------------------------------------------------------------------------
// This takes the long way around to see if a prop should emit a DLIGHT when it
// ignites, to avoid having Alyx-related code in props.cpp.
//-----------------------------------------------------------------------------
bool CPortalGameRules::ShouldBurningPropsEmitLight()
{
#ifdef HL2_EPISODIC
	if (!sv_portalbase_portalgamerules_should_burning_props_emit_light.GetBool())
		return false;

	return BaseClass::IsAlyxInDarknessMode();
#else
	return false;
#endif // HL2_EPISODIC
}

//MyGamepedia: let it be optional
ConVar	sv_portalbase_portalgamerules_use_radio_arg("sv_portalbase_portalgamerules_use_radio_arg", "0",
	FCVAR_REPLICATED,
	"Enable radio ARG from Portal in Portal campaign.",
	true, 0, true, 1.0);

//---------------------------------------------------------
// This is the only way we can silence the radio sound from the first room without touching them map -- jdw
//---------------------------------------------------------
bool CPortalGameRules::ShouldRemoveRadio( void )
{
	if (!sv_portalbase_portalgamerules_use_radio_arg.GetBool())
		return false;

	return true;
}


#endif//CLIENT_DLL

#ifdef CLIENT_DLL

bool CPortalGameRules::IsBonusChallengeTimeBased( void )
{
	CBasePlayer* pPlayer = UTIL_PlayerByIndex( 1 );
	if ( !pPlayer )
		return true;

	int iBonusChallenge = pPlayer->GetBonusChallenge();
	if ( iBonusChallenge == PORTAL_CHALLENGE_TIME || iBonusChallenge == PORTAL_CHALLENGE_NONE )
		return true;

	return false;
}

#endif

// ------------------------------------------------------------------------------------ //
// Global functions.
// ------------------------------------------------------------------------------------ //

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if (!bInitted)
	{
		bInitted = true;

		//MyGamepedia: This is a copy from hl2_gamerules.cpp, it may needs to be shared smh, i also cut a cool story from 1836 line.
		//It's done to fix strider minigun fire when EnableAggressiveBehavior fired (used for ep1)
		def.AddAmmoType("AR2", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_ar2", "sk_npc_dmg_ar2", "sk_max_ar2", BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("AlyxGun", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_alyxgun", "sk_npc_dmg_alyxgun", "sk_max_alyxgun", BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("Pistol", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_pistol", "sk_npc_dmg_pistol", "sk_max_pistol", BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("SMG1", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_smg1", "sk_npc_dmg_smg1", "sk_max_smg1", BULLET_IMPULSE(200, 1225), 0);
		def.AddAmmoType("357", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_357", "sk_npc_dmg_357", "sk_max_357", BULLET_IMPULSE(800, 5000), 0);
		def.AddAmmoType("XBowBolt", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_crossbow", "sk_npc_dmg_crossbow", "sk_max_crossbow", BULLET_IMPULSE(800, 8000), 0);

		def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_LINE, "sk_plr_dmg_buckshot", "sk_npc_dmg_buckshot", "sk_max_buckshot", BULLET_IMPULSE(400, 1200), 0);
		def.AddAmmoType("RPG_Round", DMG_BURN, TRACER_NONE, "sk_plr_dmg_rpg_round", "sk_npc_dmg_rpg_round", "sk_max_rpg_round", 0, 0);
		def.AddAmmoType("SMG1_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_smg1_grenade", "sk_npc_dmg_smg1_grenade", "sk_max_smg1_grenade", 0, 0);
		def.AddAmmoType("SniperRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_plr_dmg_sniper_round", "sk_npc_dmg_sniper_round", "sk_max_sniper_round", BULLET_IMPULSE(650, 6000), 0);
		def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE(150, 6000), 0);
		def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0);
		def.AddAmmoType("Thumper", DMG_SONIC, TRACER_NONE, 10, 10, 2, 0, 0);
		def.AddAmmoType("Gravity", DMG_CLUB, TRACER_NONE, 0, 0, 8, 0, 0);
		//		def.AddAmmoType("Extinguisher",		DMG_BURN,					TRACER_NONE,			0,	0, 100, 0, 0 );
		def.AddAmmoType("Battery", DMG_CLUB, TRACER_NONE, NULL, NULL, NULL, 0, 0);
		def.AddAmmoType("GaussEnergy", DMG_SHOCK, TRACER_NONE, "sk_jeep_gauss_damage", "sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE(650, 8000), 0); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType("CombineCannon", DMG_BULLET, TRACER_LINE, "sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5 * 750 * 12, 0); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType("AirboatGun", DMG_AIRBOAT, TRACER_LINE, "sk_plr_dmg_airboat", "sk_npc_dmg_airboat", NULL, BULLET_IMPULSE(10, 600), 0);

#ifdef HL2_EPISODIC
		def.AddAmmoType("StriderMinigun", DMG_BULLET, TRACER_LINE, 5, 5, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
#else
		def.AddAmmoType("StriderMinigun", DMG_BULLET, TRACER_LINE, 5, 15, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
#endif//HL2_EPISODIC

		def.AddAmmoType("StriderMinigunDirect", DMG_BULLET, TRACER_LINE, 2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("HelicopterGun", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter", "sk_max_smg1", BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER);
		def.AddAmmoType("AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, "sk_max_ar2_altfire", 0, 0);
#ifdef HL2_EPISODIC
		def.AddAmmoType("Hopwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_hopwire", 0, 0);
		def.AddAmmoType("CombineHeavyCannon", DMG_BULLET, TRACER_LINE, 40, 40, NULL, 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType("ammo_proto1", DMG_BULLET, TRACER_LINE, 0, 0, 10, 0, 0);
#endif // HL2_EPISODIC
	}

	return &def;
}

