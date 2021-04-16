//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PORTAL_PLAYERANIMSTATE_H
#define PORTAL_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "iplayeranimstate.h"
#include "base_playeranimstate.h"


#if defined( CLIENT_DLL )
	class C_Portal_Player;
	#define CPortal_Player C_Portal_Player
#else
	class CPortal_Player;
#endif

#ifdef CLIENT_DLL
	class C_BaseAnimatingOverlay;
	class C_Portal_Player;
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
#define CPortal_Player C_Portal_Player
#else
	class CBaseAnimatingOverlay;
	class CPortal_Player;
#endif

	// When moving this fast, he plays run anim.
#define ARBITRARY_RUN_SPEED		175.0f

	enum PlayerAnimEvent_t
	{
		PLAYERANIMEVENT_FIRE_GUN_PRIMARY = 0,
		PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
		PLAYERANIMEVENT_THROW_GRENADE,
		PLAYERANIMEVENT_JUMP,
		PLAYERANIMEVENT_RELOAD,

		PLAYERANIMEVENT_ATTACK_PRIMARY,
		PLAYERANIMEVENT_ATTACK_SECONDARY,
		PLAYERANIMEVENT_ATTACK_GRENADE,
		PLAYERANIMEVENT_RELOAD,
		PLAYERANIMEVENT_RELOAD_LOOP,
		PLAYERANIMEVENT_RELOAD_END,
		PLAYERANIMEVENT_JUMP,
		PLAYERANIMEVENT_SWIM,
		PLAYERANIMEVENT_DIE,
		PLAYERANIMEVENT_FLINCH_CHEST,
		PLAYERANIMEVENT_FLINCH_HEAD,
		PLAYERANIMEVENT_FLINCH_LEFTARM,
		PLAYERANIMEVENT_FLINCH_RIGHTARM,
		PLAYERANIMEVENT_FLINCH_LEFTLEG,
		PLAYERANIMEVENT_FLINCH_RIGHTLEG,
		PLAYERANIMEVENT_DOUBLEJUMP,

		// Cancel.
		PLAYERANIMEVENT_CANCEL,
		PLAYERANIMEVENT_SPAWN,

		// Snap to current yaw exactly
		PLAYERANIMEVENT_SNAP_YAW,

		PLAYERANIMEVENT_CUSTOM,				// Used to play specific activities
		PLAYERANIMEVENT_CUSTOM_GESTURE,
		PLAYERANIMEVENT_CUSTOM_SEQUENCE,	// Used to play specific sequences
		PLAYERANIMEVENT_CUSTOM_GESTURE_SEQUENCE,

		// TF Specific. Here until there's a derived game solution to this.
		PLAYERANIMEVENT_ATTACK_PRE,
		PLAYERANIMEVENT_ATTACK_POST,
		PLAYERANIMEVENT_GRENADE1_DRAW,
		PLAYERANIMEVENT_GRENADE2_DRAW,
		PLAYERANIMEVENT_GRENADE1_THROW,
		PLAYERANIMEVENT_GRENADE2_THROW,
		PLAYERANIMEVENT_VOICE_COMMAND_GESTURE,
		PLAYERANIMEVENT_DOUBLEJUMP_CROUCH,
		PLAYERANIMEVENT_STUN_BEGIN,
		PLAYERANIMEVENT_STUN_MIDDLE,
		PLAYERANIMEVENT_STUN_END,

		PLAYERANIMEVENT_ATTACK_PRIMARY_SUPER,

		PLAYERANIMEVENT_COUNT
	};

	class IPortalPlayerAnimState : virtual public IPlayerAnimState
	{
	public:
		// This is called by both the client and the server in the same way to trigger events for
		// players firing, jumping, throwing grenades, etc.
		virtual void DoAnimationEvent(PlayerAnimEvent_t event, int nData = 0) = 0;

		// Returns true if we're playing the grenade prime or throw animation.
		virtual bool IsThrowingGrenade() = 0;
	};

	IPortalPlayerAnimState* CreatePlayerAnimState(CBaseAnimatingOverlay *pEntity, LegAnimType_t legAnimType, bool bUseAimSequences);

	// If this is set, then the game code needs to make sure to send player animation events
	// to the local player if he's the one being watched.
	extern ConVar cl_showanimstate;

#endif // PORTAL_PLAYERANIMSTATE_H
