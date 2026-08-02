//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//


#ifndef SECURITYCAMERA_SEVER_H
#define SECURITYCAMERA_SEVER_H

#include "rope.h"

#ifdef _WIN32
#pragma once
#endif

#define	SECURITY_CAMERA_MODEL		"models/props/security_camera.mdl"
#define SECURITY_CAMERA_BC_YAW		"aim_yaw"
#define SECURITY_CAMERA_BC_PITCH	"aim_pitch"
#define	SECURITY_CAMERA_RANGE		1500
#define SECURITY_CAMERA_SPREAD		VECTOR_CONE_2DEGREES
#define	SECURITY_CAMERA_MAX_WAIT	5
#define	SECURITY_CAMERA_PING_TIME	1.0f	//LPB!!

#define SECURITY_CAMERA_NUM_ROPES 2
#define SECURITY_CAMERA_GLOW_SPRITE	"sprites/glow1.vmt"

//Aiming variables
#define	SECURITY_CAMERA_MAX_NOHARM_PERIOD	0.0f
#define	SECURITY_CAMERA_MAX_GRACE_PERIOD	3.0f

//Spawnflags
#define SF_SECURITY_CAMERA_AUTOACTIVATE		0x00000020
#define SF_SECURITY_CAMERA_STARTINACTIVE	0x00000040
#define SF_SECURITY_CAMERA_NEVERRETIRE		0x00000080
#define SF_SECURITY_CAMERA_OUT_OF_AMMO		0x00000100

#define CAMERA_DESTROYED_SCENE_1			"scenes/general/generic_security_camera_destroyed-1.vcd"
#define CAMERA_DESTROYED_SCENE_2			"scenes/general/generic_security_camera_destroyed-2.vcd"
#define CAMERA_DESTROYED_SCENE_3			"scenes/general/generic_security_camera_destroyed-3.vcd"
#define CAMERA_DESTROYED_SCENE_4			"scenes/general/generic_security_camera_destroyed-4.vcd"
#define CAMERA_DESTROYED_SCENE_5			"scenes/general/generic_security_camera_destroyed-5.vcd"

//Heights
#define	SECURITY_CAMERA_YAW_SPEED	7.0f

#define SECURITY_CAMERA_TOTAL_TO_KNOCK_DOWN 33

//Turret states
enum turretState_e
{
	TURRET_SEARCHING,
	TURRET_AUTO_SEARCHING,
	TURRET_ACTIVE,
	TURRET_DEPLOYING,
	TURRET_RETIRING,
	TURRET_DEAD,
};

// Forces glados actor to play reaction scenes when player dismounts camera.
void PlayDismountSounds(void);


//
// Security Camera
//

class CNPC_SecurityCamera : public CNPCBaseInteractive<CAI_BaseNPC>, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS(CNPC_SecurityCamera, CNPCBaseInteractive<CAI_BaseNPC>);
public:

	CNPC_SecurityCamera(void);
	~CNPC_SecurityCamera(void);

	void			Precache(void);
	virtual void	CreateSounds(void);
	virtual void	StopLoopingSounds(void);
	virtual void	Spawn(void);
	virtual void	Activate(void);
	bool			CreateVPhysics(void);
	virtual void	UpdateOnRemove(void);
	virtual void	NotifySystemEvent(CBaseEntity* pNotify, notify_system_event_t eventType, const notify_system_event_params_t& params);
	virtual int		ObjectCaps(void);
	void			Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual void	OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);
	virtual void	OnPhysGunDrop(CBasePlayer* pPhysGunUser, PhysGunDrop_t Reason);
	//virtual bool	HasPreferredCarryAnglesForPlayer(CBasePlayer* pPlayer);
	//virtual QAngle	PreferredCarryAngles(void);
	//virtual bool	OnAttemptPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);

	bool IsHeldByPhyscannon() { return VPhysicsGetObject() && (VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD); }
	bool IsBeingCarriedByPlayer(void) { return m_bCarriedByPlayer; }

	// Think functions
	void	Retire(void);
	void	Deploy(void);
	void	ActiveThink(void);
	void	SearchThink(void);
	void	DeathThink(void);

	// Inputs
	void	InputToggle(inputdata_t& inputdata);
	void	InputEnable(inputdata_t& inputdata);
	void	InputDisable(inputdata_t& inputdata);
	void	InputRagdoll(inputdata_t& inputdata);

	void	SetLastSightTime();

	int		OnTakeDamage(const CTakeDamageInfo& inputInfo);
	virtual void	PlayerPenetratingVPhysics(void);
	virtual bool	OnAttemptPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);

	bool	ShouldSavePhysics() { return true; }

	virtual	bool	IsSecurityCamera() const { return true; }

	virtual bool CanBeAnEnemyOf(CBaseEntity* pEnemy);

	Class_T	Classify(void)
	{
		if (m_bEnabled)
			return CLASS_COMBINE;

		return CLASS_NONE;
	}

	bool	FVisible(CBaseEntity* pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity** ppBlocker = NULL);

	Vector	EyeOffset(Activity nActivity)
	{
		Vector vForward;

		GetVectors(&vForward, 0, 0);

		return vForward * 10.0f;
	}

	Vector	EyePosition(void)
	{
		return GetAbsOrigin() + EyeOffset(GetActivity());
	}

	bool	m_bDetached;	//mygamepedia: is it detached ?


protected:

	bool	PreThink(turretState_e state);
	void	Ping(void);
	void	Toggle(void);
	void	Enable(void);
	void	Disable(void);

	void	RopesOn(void);
	void	RopesOff(void);
	void	EyeOn(void);
	void	EyeOff(void);

	bool	UpdateFacing(void);

	// physics influence
	CHandle<CBasePlayer>	m_hPhysicsAttacker;
	CHandle<CBaseCombatCharacter> m_hLastNPCToKickMe;		// Stores the last NPC who tried to knock me over
	float					m_flLastPhysicsInfluenceTime;

	bool	m_bCarriedByPlayer;

private:

	CHandle<CRopeKeyframe>	m_hRopes[SECURITY_CAMERA_NUM_ROPES];
	CHandle<CSprite>		m_hEyeGlow;

	bool	m_bAutoStart;
	bool	m_bActive;		//Denotes the turret is deployed and looking for targets
	bool	m_bBlinkState;
	bool	m_bEnabled;		//Denotes whether the turret is able to deploy or not

	float	m_flLastSight;
	float	m_flPingTime;

	QAngle	m_vecGoalAngles;
	QAngle	m_vecCurrentAngles;
	Vector	m_vNoisePos;
	int		m_iTicksTillNextNoise;

	CSoundPatch* m_pMovementSound;

	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	COutputEvent m_OnPhysGunPickup;
	COutputEvent m_OnPhysGunDrop;

	DECLARE_DATADESC();
};

#endif // SECURITYCAMERA_SEVER_H