//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"

#include "portal_playeranimstate.h"
#include "weapon_portalbase.h"

#ifdef CLIENT_DLL
#include "C_Portal_Player.h"
#include "C_Weapon_Portalgun.h"
#include "bone_setup.h"
#include "interpolatedvar.h"
#else
#include "portal_player.h"
#include "weapon_portalgun.h"
#endif

//#define PORTAL_RUN_SPEED			320.0f
//#define PORTAL_CROUCHWALK_SPEED		110.0f

#define ANIM_TOPSPEED_WALK			100
#define ANIM_TOPSPEED_RUN			250
#define ANIM_TOPSPEED_RUN_CROUCH	85

#define DEFAULT_IDLE_NAME "idle_upper_"
#define DEFAULT_CROUCH_IDLE_NAME "crouch_idle_upper_"
#define DEFAULT_CROUCH_WALK_NAME "crouch_walk_upper_"
#define DEFAULT_WALK_NAME "walk_upper_"
#define DEFAULT_RUN_NAME "run_upper_"

#define DEFAULT_FIRE_IDLE_NAME "idle_shoot_"
#define DEFAULT_FIRE_CROUCH_NAME "crouch_idle_shoot_"
#define DEFAULT_FIRE_CROUCH_WALK_NAME "crouch_walk_shoot_"
#define DEFAULT_FIRE_WALK_NAME "walk_shoot_"
#define DEFAULT_FIRE_RUN_NAME "run_shoot_"


#define FIRESEQUENCE_LAYER		(AIMSEQUENCE_LAYER+NUM_AIMSEQUENCE_LAYERS)
#define RELOADSEQUENCE_LAYER	(FIRESEQUENCE_LAYER + 1)
#define GRENADESEQUENCE_LAYER	(RELOADSEQUENCE_LAYER + 1)
#define NUM_LAYERS_WANTED		(GRENADESEQUENCE_LAYER + 1)

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //

class CPortalPlayerAnimState : public CBasePlayerAnimState, public IPortalPlayerAnimState
{
public:
	DECLARE_CLASS(CPortalPlayerAnimState, CBasePlayerAnimState);
	friend IPortalPlayerAnimState* CreatePlayerAnimState(CBaseAnimatingOverlay *pEntity, LegAnimType_t legAnimType, bool bUseAimSequences);

	CPortalPlayerAnimState();

	virtual void DoAnimationEvent(PlayerAnimEvent_t event, int nData);
	virtual bool IsThrowingGrenade();
	virtual int CalcAimLayerSequence(float *flCycle, float *flAimSequenceWeight, bool bForceIdle);
	virtual void ClearAnimationState();
	virtual bool CanThePlayerMove();
	virtual float GetCurrentMaxGroundSpeed();
	virtual Activity CalcMainActivity();
	virtual void DebugShowAnimState(int iStartLine);
	virtual void ComputeSequences(CStudioHdr *pStudioHdr);
	virtual void ClearAnimationLayers();

	void InitPortal(CBaseAnimatingOverlay *pPlayer, LegAnimType_t legAnimType, bool bUseAimSequences);

	/*
	CPortal_Player *GetPortalPlayer(void)							{ return m_pPortalPlayer; }
	virtual Activity TranslateActivity(Activity actDesired);
	void    Teleport(const Vector *pNewOrigin, const QAngle *pNewAngles, CPortal_Player* pPlayer);
	bool	HandleMoving(Activity &idealActivity);
	bool	HandleJumping(Activity &idealActivity);
	bool	HandleDucking(Activity &idealActivity);
	*/

protected:

	int CalcFireLayerSequence(PlayerAnimEvent_t event);
	void ComputeFireSequence(CStudioHdr *pStudioHdr);

	void ComputeReloadSequence(CStudioHdr *pStudioHdr);
	int CalcReloadLayerSequence();

	bool IsOuterGrenadePrimed();
	void ComputeGrenadeSequence(CStudioHdr *pStudioHdr);
	int CalcGrenadePrimeSequence();
	int CalcGrenadeThrowSequence();
	int GetOuterGrenadeThrowCounter();

	bool HandleJumping();

	void UpdateLayerSequenceGeneric(CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd);

private:

	// Current state variables.
	bool m_bJumping;			// Set on a jump event.
	float m_flJumpStartTime;
	bool m_bFirstJumpFrame;

	// Aim sequence plays reload while this is on.
	bool m_bReloading;
	float m_flReloadCycle;
	int m_iReloadSequence;

	// This is set to true if ANY animation is being played in the fire layer.
	bool m_bFiring;						// If this is on, then it'll continue the fire animation in the fire layer
	// until it completes.
	int m_iFireSequence;				// (For any sequences in the fire layer, including grenade throw).
	float m_flFireCycle;

	// These control grenade animations.
	bool m_bThrowingGrenade;
	bool m_bPrimingGrenade;
	float m_flGrenadeCycle;
	int m_iGrenadeSequence;
	int m_iLastThrowGrenadeCounter;	// used to detect when the guy threw the grenade.

	/*
	CPortal_Player   *m_pPortalPlayer;
	bool		m_bInAirWalk;
	float		m_flHoldDeployedPoseUntilTime;
	*/
};

IPortalPlayerAnimState* CreatePlayerAnimState(CBaseAnimatingOverlay *pEntity, LegAnimType_t legAnimType, bool bUseAimSequences)
{
	CPortalPlayerAnimState *pRet = new CPortalPlayerAnimState;
	pRet->InitPortal(pEntity, legAnimType, bUseAimSequences);
	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : CMultiPlayerAnimState*
//-----------------------------------------------------------------------------
/*
CPortalPlayerAnimState* CreatePortalPlayerAnimState( CPortal_Player *pPlayer )
{
// Setup the movement data.
MultiPlayerMovementData_t movementData;
movementData.m_flBodyYawRate = 720.0f;
movementData.m_flRunSpeed = PORTAL_RUN_SPEED;
movementData.m_flWalkSpeed = -1;
movementData.m_flSprintSpeed = -1.0f;

// Create animation state for this player.
CPortalPlayerAnimState *pRet = new CPortalPlayerAnimState( pPlayer, movementData );

// Specific Portal player initialization.
pRet->InitPortal( pPlayer );

return pRet;
}
*/

// ------------------------------------------------------------------------------------------------ //
// CPortalPlayerAnimState implementation.
// ------------------------------------------------------------------------------------------------ //

CPortalPlayerAnimState::CPortalPlayerAnimState()
{
	m_pOuter = NULL;
	m_bReloading = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
/*
CPortalPlayerAnimState::CPortalPlayerAnimState()
{
m_pPortalPlayer = NULL;
// Don't initialize Portal specific variables here. Init them in InitPortal()
}

*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			&movementData - 
//-----------------------------------------------------------------------------
/*
CPortalPlayerAnimState::CPortalPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData )
: CMultiPlayerAnimState( pPlayer, movementData )
{
	m_pPortalPlayer = NULL;

	// Don't initialize Portal specific variables here. Init them in InitPortal()
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CPortalPlayerAnimState::~CPortalPlayerAnimState()
{
}

void CPortalPlayerAnimState::InitPortal(CBaseAnimatingOverlay *pEntity, LegAnimType_t legAnimType, bool bUseAimSequences)
{
	CModAnimConfig config;
	config.m_flMaxBodyYawDegrees = 90;
	config.m_LegAnimType = legAnimType;
	config.m_bUseAimSequences = bUseAimSequences;

	BaseClass::Init(pEntity, config);
}

//-----------------------------------------------------------------------------
// Purpose: Initialize Portal specific animation state.
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
/*
void CPortalPlayerAnimState::InitPortal( CPortal_Player *pPlayer )
{
m_pPortalPlayer = pPlayer;
m_bInAirWalk = false;
m_flHoldDeployedPoseUntilTime = 0.0f;
}
*/

void CPortalPlayerAnimState::ClearAnimationState()
{
	m_bJumping = false;
	m_bFiring = false;
	m_bReloading = false;
	m_bThrowingGrenade = m_bPrimingGrenade = false;
	m_iLastThrowGrenadeCounter = GetOuterGrenadeThrowCounter();

	BaseClass::ClearAnimationState();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
void CPortalPlayerAnimState::ClearAnimationState( void )
{
m_bInAirWalk = false;

BaseClass::ClearAnimationState();
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : actDesired - 
// Output : Activity
//-----------------------------------------------------------------------------
/*
Activity CPortalPlayerAnimState::TranslateActivity( Activity actDesired )
{
Activity translateActivity = BaseClass::TranslateActivity( actDesired );

if ( GetPortalPlayer()->GetActiveWeapon() )
{
translateActivity = GetPortalPlayer()->GetActiveWeapon()->ActivityOverride( translateActivity, false );
}

return translateActivity;
}
*/

void CPortalPlayerAnimState::DoAnimationEvent(PlayerAnimEvent_t event, int nData)
{
	Assert(event != PLAYERANIMEVENT_THROW_GRENADE);

	if (event == PLAYERANIMEVENT_FIRE_GUN_PRIMARY ||
		event == PLAYERANIMEVENT_FIRE_GUN_SECONDARY)
	{
		// Regardless of what we're doing in the fire layer, restart it.
		m_flFireCycle = 0;
		m_iFireSequence = CalcFireLayerSequence(event);
		m_bFiring = m_iFireSequence != -1;
	}
	else if (event == PLAYERANIMEVENT_JUMP)
	{
		// Play the jump animation.
		m_bJumping = true;
		m_bFirstJumpFrame = true;
		m_flJumpStartTime = gpGlobals->curtime;
	}
	else if (event == PLAYERANIMEVENT_RELOAD)
	{
		m_iReloadSequence = CalcReloadLayerSequence();
		if (m_iReloadSequence != -1)
		{
			m_bReloading = true;
			m_flReloadCycle = 0;
		}
	}
	else
	{
		Assert(!"CPortalPlayerAnimState::DoAnimationEvent");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : event - 
//-----------------------------------------------------------------------------
/*
void CPortalPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
Activity iWeaponActivity = ACT_INVALID;

switch( event )
{
case PLAYERANIMEVENT_ATTACK_PRIMARY:
case PLAYERANIMEVENT_ATTACK_SECONDARY:
{
CPortal_Player *pPlayer = GetPortalPlayer();
if ( !pPlayer )
return;

CWeaponPortalBase *pWpn = pPlayer->GetActivePortalWeapon();

if ( pWpn )
{
// Weapon primary fire.
if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
{
RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE );
}
else
{
RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE );
}

iWeaponActivity = ACT_VM_PRIMARYATTACK;
}
else	// unarmed player
{

}

break;
}

default:
{
BaseClass::DoAnimationEvent( event, nData );
break;
}
}

#ifdef CLIENT_DLL
// Make the weapon play the animation as well
if ( iWeaponActivity != ACT_INVALID )
{
CBaseCombatWeapon *pWeapon = GetPortalPlayer()->GetActiveWeapon();
if ( pWeapon )
{
pWeapon->SendWeaponAnim( iWeaponActivity );
}
}
#endif
}
*/

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*
void CPortalPlayerAnimState::Teleport( const Vector *pNewOrigin, const QAngle *pNewAngles, CPortal_Player* pPlayer )
{
	QAngle absangles = pPlayer->GetAbsAngles();
	m_angRender = absangles;
	m_angRender.x = m_angRender.z = 0.0f;
	if ( pPlayer )
	{
		// Snap the yaw pose parameter lerping variables to face new angles.
		m_flCurrentFeetYaw = m_flGoalFeetYaw = m_flEyeYaw = pPlayer->EyeAngles()[YAW];
	}
}
*/



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPortalPlayerAnimState::HandleMoving( Activity &idealActivity )
{
	float flSpeed = GetOuterXYSpeed();

	// If we move, cancel the deployed anim hold
	if ( flSpeed > MOVING_MINIMUM_SPEED )
	{
		m_flHoldDeployedPoseUntilTime = 0.0;
		idealActivity = ACT_MP_RUN;
	}

	else if ( m_flHoldDeployedPoseUntilTime > gpGlobals->curtime )
	{
		// Unless we move, hold the deployed pose for a number of seconds after being deployed
		idealActivity = ACT_MP_DEPLOYED_IDLE;
	}
	else 
	{
		return BaseClass::HandleMoving( idealActivity );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPortalPlayerAnimState::HandleDucking( Activity &idealActivity )
{
	if ( GetBasePlayer()->m_Local.m_bDucking || GetBasePlayer()->m_Local.m_bDucked )
	{
		if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
			idealActivity = ACT_MP_CROUCH_IDLE;		
		}
		else
		{
			idealActivity = ACT_MP_CROUCHWALK;		
		}

		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
bool CPortalPlayerAnimState::HandleJumping( Activity &idealActivity )
{
	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	if ( ( vecVelocity.z > 300.0f || m_bInAirWalk ) )
	{
		// Check to see if we were in an airwalk and now we are basically on the ground.
		if ( GetBasePlayer()->GetFlags() & FL_ONGROUND )
		{				
			m_bInAirWalk = false;
			RestartMainSequence();
			RestartGesture( GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND );	
		}
		else
		{
			// In an air walk.
			idealActivity = ACT_MP_AIRWALK;
			m_bInAirWalk = true;
		}
	}
	// Jumping.
	else
	{
		if ( m_bJumping )
		{
			if ( m_bFirstJumpFrame )
			{
				m_bFirstJumpFrame = false;
				RestartMainSequence();	// Reset the animation.
			}

			// Don't check if he's on the ground for a sec.. sometimes the client still has the
			// on-ground flag set right when the message comes in.
			else if ( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
			{
				if ( GetBasePlayer()->GetFlags() & FL_ONGROUND )
				{
					m_bJumping = false;
					RestartMainSequence();
					RestartGesture( GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND );					
				}
			}

			// if we're still jumping
			if ( m_bJumping )
			{
				idealActivity = ACT_MP_JUMP_START;
			}
		}	
	}	

	if ( m_bJumping || m_bInAirWalk )
		return true;

	return false;
}

float g_flThrowGrenadeFraction = 0.25;
bool CPortalPlayerAnimState::IsThrowingGrenade()
{
	if (m_bThrowingGrenade)
	{
		// An animation event would be more appropriate here.
		return m_flGrenadeCycle < g_flThrowGrenadeFraction;
	}
	else
	{
		bool bThrowPending = (m_iLastThrowGrenadeCounter != GetOuterGrenadeThrowCounter());
		return bThrowPending || IsOuterGrenadePrimed();
	}
}

int CPortalPlayerAnimState::CalcReloadLayerSequence()
{
	const char *pSuffix = GetWeaponSuffix();
	if (!pSuffix)
		return -1;

	CWeaponPortalBase *pWeapon = m_pHelpers->PortalAnim_GetActiveWeapon();
	if (!pWeapon)
		return -1;

	// First, look for reload_<weapon name>.
	char szName[512];
	Q_snprintf(szName, sizeof(szName), "reload_%s", pSuffix);
	int iReloadSequence = m_pOuter->LookupSequence(szName);
	if (iReloadSequence != -1)
		return iReloadSequence;

	//SDKTODO
	/*
	// Ok, look for generic categories.. pistol, shotgun, rifle, etc.
	if ( pWeapon->GetSDKWpnData().m_WeaponType == WEAPONTYPE_PISTOL )
	{
	Q_snprintf( szName, sizeof( szName ), "reload_pistol" );
	iReloadSequence = m_pOuter->LookupSequence( szName );
	if ( iReloadSequence != -1 )
	return iReloadSequence;
	}
	*/

	// Fall back to reload_m4.
	iReloadSequence = CalcSequenceIndex("reload_m4");
	if (iReloadSequence > 0)
		return iReloadSequence;

	return -1;
}

#ifdef CLIENT_DLL
void CPortalPlayerAnimState::UpdateLayerSequenceGeneric(CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd)
{
	if (!bEnabled)
		return;

	// Increment the fire sequence's cycle.
	flCurCycle += m_pOuter->GetSequenceCycleRate(pStudioHdr, iSequence) * gpGlobals->frametime;
	if (flCurCycle > 1)
	{
		if (bWaitAtEnd)
		{
			flCurCycle = 1;
		}
		else
		{
			// Not firing anymore.
			bEnabled = false;
			iSequence = 0;
			return;
		}
	}

	// Now dump the state into its animation layer.
	C_AnimationLayer *pLayer = m_pOuter->GetAnimOverlay(iLayer);

	pLayer->m_flCycle = flCurCycle;
	pLayer->m_nSequence = iSequence;

	pLayer->m_flPlaybackRate = 1.0;
	pLayer->m_flWeight = 1.0f;
	pLayer->m_nOrder = iLayer;
}
#endif

bool CPortalPlayerAnimState::IsOuterGrenadePrimed()
{
	CBaseCombatCharacter *pChar = m_pOuter->MyCombatCharacterPointer();
	if (pChar)
	{
		CBaseSDKGrenade *pGren = dynamic_cast<CBaseSDKGrenade*>(pChar->GetActiveWeapon());
		return pGren && pGren->IsPinPulled();
	}
	else
	{
		return NULL;
	}
}


void CPortalPlayerAnimState::ComputeGrenadeSequence(CStudioHdr *pStudioHdr)
{
#ifdef CLIENT_DLL
	if (m_bThrowingGrenade)
	{
		UpdateLayerSequenceGeneric(pStudioHdr, GRENADESEQUENCE_LAYER, m_bThrowingGrenade, m_flGrenadeCycle, m_iGrenadeSequence, false);
	}
	else
	{
		// Priming the grenade isn't an event.. we just watch the player for it.
		// Also play the prime animation first if he wants to throw the grenade.
		bool bThrowPending = (m_iLastThrowGrenadeCounter != GetOuterGrenadeThrowCounter());
		if (IsOuterGrenadePrimed() || bThrowPending)
		{
			if (!m_bPrimingGrenade)
			{
				// If this guy just popped into our PVS, and he's got his grenade primed, then
				// let's assume that it's all the way primed rather than playing the prime
				// animation from the start.
				if (TimeSinceLastAnimationStateClear() < 0.4f)
					m_flGrenadeCycle = 1;
				else
					m_flGrenadeCycle = 0;

				m_iGrenadeSequence = CalcGrenadePrimeSequence();
			}

			m_bPrimingGrenade = true;
			UpdateLayerSequenceGeneric(pStudioHdr, GRENADESEQUENCE_LAYER, m_bPrimingGrenade, m_flGrenadeCycle, m_iGrenadeSequence, true);

			// If we're waiting to throw and we're done playing the prime animation...
			if (bThrowPending && m_flGrenadeCycle == 1)
			{
				m_iLastThrowGrenadeCounter = GetOuterGrenadeThrowCounter();

				// Now play the throw animation.
				m_iGrenadeSequence = CalcGrenadeThrowSequence();
				if (m_iGrenadeSequence != -1)
				{
					// Configure to start playing 
					m_bThrowingGrenade = true;
					m_bPrimingGrenade = false;
					m_flGrenadeCycle = 0;
				}
			}
		}
		else
		{
			m_bPrimingGrenade = false;
		}
	}
#endif
}


int CPortalPlayerAnimState::CalcGrenadePrimeSequence()
{
	return CalcSequenceIndex("idle_shoot_gren1");
}


int CPortalPlayerAnimState::CalcGrenadeThrowSequence()
{
	return CalcSequenceIndex("idle_shoot_gren2");
}


int CPortalPlayerAnimState::GetOuterGrenadeThrowCounter()
{
	CPortal_Player *pPlayer = dynamic_cast<CPortal_Player*>(m_pOuter);
	if (pPlayer)
		return pPlayer->m_iThrowGrenadeCounter;
	else
		return 0;
}

void CPortalPlayerAnimState::ComputeReloadSequence(CStudioHdr *pStudioHdr)
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric(pStudioHdr, RELOADSEQUENCE_LAYER, m_bReloading, m_flReloadCycle, m_iReloadSequence, false);
#else
	// Server doesn't bother with different fire sequences.
#endif
}

int CPortalPlayerAnimState::CalcAimLayerSequence(float *flCycle, float *flAimSequenceWeight, bool bForceIdle)
{
	const char *pSuffix = GetWeaponSuffix();
	if (!pSuffix)
		return 0;

	if (bForceIdle)
	{
		switch (GetCurrentMainSequenceActivity())
		{
		case ACT_CROUCHIDLE:
			return CalcSequenceIndex("%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix);

		default:
			return CalcSequenceIndex("%s%s", DEFAULT_IDLE_NAME, pSuffix);
		}
	}
	else
	{
		switch (GetCurrentMainSequenceActivity())
		{
		case ACT_RUN:
			return CalcSequenceIndex("%s%s", DEFAULT_RUN_NAME, pSuffix);

		case ACT_WALK:
		case ACT_RUNTOIDLE:
		case ACT_IDLETORUN:
			return CalcSequenceIndex("%s%s", DEFAULT_WALK_NAME, pSuffix);

		case ACT_CROUCHIDLE:
			return CalcSequenceIndex("%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix);

		case ACT_RUN_CROUCH:
			return CalcSequenceIndex("%s%s", DEFAULT_CROUCH_WALK_NAME, pSuffix);

		case ACT_IDLE:
		default:
			return CalcSequenceIndex("%s%s", DEFAULT_IDLE_NAME, pSuffix);
		}
	}
}

int CPortalPlayerAnimState::CalcFireLayerSequence(PlayerAnimEvent_t event)
{
	// Don't rely on their weapon here because the player has usually switched to their 
	// pistol or rifle by the time the PLAYERANIMEVENT_THROW_GRENADE message gets to the client.

	switch (GetCurrentMainSequenceActivity())
	{
	case ACT_PLAYER_RUN_FIRE:
	case ACT_RUN:
		return CalcSequenceIndex("%s%s", DEFAULT_FIRE_RUN_NAME);

	case ACT_PLAYER_WALK_FIRE:
	case ACT_WALK:
		return CalcSequenceIndex("%s%s", DEFAULT_FIRE_WALK_NAME);

	case ACT_PLAYER_CROUCH_FIRE:
	case ACT_CROUCHIDLE:
		return CalcSequenceIndex("%s%s", DEFAULT_FIRE_CROUCH_NAME);

	case ACT_PLAYER_CROUCH_WALK_FIRE:
	case ACT_RUN_CROUCH:
		return CalcSequenceIndex("%s%s", DEFAULT_FIRE_CROUCH_WALK_NAME);

	default:
	case ACT_PLAYER_IDLE_FIRE:
		return CalcSequenceIndex("%s%s", DEFAULT_FIRE_IDLE_NAME);
	}
}

bool CPortalPlayerAnimState::CanThePlayerMove()
{
	CPortal_Player *pPlayer = 

	return m_pHelpers->SDKAnim_CanMove();
}