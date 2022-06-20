//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Single Player animation state 'handler'. This utility is used
//            to evaluate the pose parameter value based on the direction
//            and speed of the player.
//
//====================================================================================//

#ifndef SINGLEPLAYER_ANIMSTATE_H
#define SINGLEPLAYER_ANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "base_playeranimstate.h"

#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#else
#include "player.h"
#endif

class CSinglePlayerAnimState : public CBasePlayerAnimState
{
public:
	enum
	{
		TURN_NONE = 0,
		TURN_LEFT,
		TURN_RIGHT
	};

	CSinglePlayerAnimState(CBasePlayer *pPlayer);
	void Init(CBasePlayer *pPlayer);

	// those 3 methods aren't actually used anywhere I just added them as placeholders because they're pure virtual in the base class
	Activity			CalcMainActivity() { return ACT_IDLE; }
	int					CalcAimLayerSequence(float *flCycle, float *flAimSequenceWeight, bool bForceIdle) { return 0; }
	float				GetCurrentMaxGroundSpeed() { return m_pPlayer->GetPlayerMaxSpeed(); }

	Activity            BodyYawTranslateActivity(Activity activity);

	void                Update();

	const QAngle&        GetRenderAngles();

	void                GetPoseParameters(CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM]);

	CBasePlayer        *GetBasePlayer();

	void Release();

	// all of these need to be public in order for the portal player class to handle player teleportation
	QAngle                m_angRender;

	float                m_flGoalFeetYaw;
	float                m_flCurrentFeetYaw;

	float				 m_flEyeYaw;

private:
	void                GetOuterAbsVelocity(Vector& vel);

	int                    ConvergeAngles(float goal, float maxrate, float dt, float& current);

	void                EstimateYaw(void);
	void                ComputePoseParam_BodyYaw(void);
	void                ComputePoseParam_BodyPitch(CStudioHdr *pStudioHdr);
	void                ComputePoseParam_BodyLookYaw(void);
	void                ComputePoseParam_HeadPitch(CStudioHdr *pStudioHdr);
	void                ComputePlaybackRate();

	CBasePlayer        *m_pPlayer;

	float                m_flGaitYaw;
	float                m_flStoredCycle;

	float                m_flCurrentTorsoYaw;

	float                m_flLastYaw;
	float                m_flLastTurnTime;

	int                    m_nTurningInPlace;

	float                m_flTurnCorrectionTime;
};

CSinglePlayerAnimState *CreatePlayerAnimationState(CBasePlayer *pPlayer);

#endif // SINGLEPLAYER_ANIMSTATE_H
