//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for .
//
//===========================================================================//

#include "cbase.h"
#include "vcollide_parse.h"
#include "c_portal_player.h"
#include "view.h"
#include "c_basetempentity.h"
#include "takedamageinfo.h"
#include "in_buttons.h"
#include "iviewrender_beams.h"
#include "r_efx.h"
#include "dlight.h"
#include "PortalRender.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "tier1/KeyValues.h"
#include "ScreenSpaceEffects.h"
#include "portal_shareddefs.h"
#include "ivieweffects.h"		// for screenshake
#include "prop_portal_shared.h"

// NVNT for fov updates
#include "haptics/ihaptics.h"


// Don't alias here
#if defined( CPortal_Player )
#undef CPortal_Player	
#endif

#define REORIENTATION_RATE 120.0f
#define REORIENTATION_ACCELERATION_RATE 400.0f

ConVar cl_reorient_in_air("cl_reorient_in_air", "1", FCVAR_ARCHIVE, "Allows the player to only reorient from being upside down while in the air.");

LINK_ENTITY_TO_CLASS(player, C_Portal_Player);

IMPLEMENT_CLIENTCLASS_DT(C_Portal_Player, DT_Portal_Player, CPortal_Player)
RecvPropEHandle(RECVINFO(m_hPortalEnvironment)),
RecvPropEHandle(RECVINFO(m_hSurroundingLiquidPortal)),
RecvPropBool(RECVINFO(m_bSuppressingCrosshair)),
RecvPropBool(RECVINFO(m_bHeldObjectOnOppositeSideOfPortal)),
RecvPropBool(RECVINFO(m_bPitchReorientation)),
RecvPropEHandle(RECVINFO(m_pHeldObjectPortal)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_Portal_Player)
END_PREDICTION_DATA()

static ConVar cl_playermodel("cl_playermodel", "none", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Default Player Model");

extern bool g_bUpsideDown;

C_Portal_Player::C_Portal_Player()
	: m_iv_angEyeAngles("C_Portal_Player::m_iv_angEyeAngles")
{
	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;
	m_bPitchReorientation = false;
	m_fReorientationRate = 0.0f;
	m_angEyeAngles.Init();
	AddVar(&m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR);
	m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
}

C_Portal_Player::~C_Portal_Player(void)
{
}

void C_Portal_Player::ClientThink(void)
{
	//PortalEyeInterpolation.m_bNeedToUpdateEyePosition = true;

	FixTeleportationRoll();

	//QAngle vAbsAngles = EyeAngles();

	// Look at the thing that killed you
	//if ( !IsAlive() )
	//{
	//	C_BaseEntity *pEntity1 = g_eKillTarget1.Get();
	//	C_BaseEntity *pEntity2 = g_eKillTarget2.Get();

	//	if ( pEntity2 && pEntity1 )
	//	{
	//		//engine->GetViewAngles( vAbsAngles );

	//		Vector vLook = pEntity1->GetAbsOrigin() - pEntity2->GetAbsOrigin();
	//		VectorNormalize( vLook );

	//		QAngle qLook;
	//		VectorAngles( vLook, qLook );

	//		if ( qLook[PITCH] > 180.0f )
	//		{
	//			qLook[PITCH] -= 360.0f;
	//		}

	//		if ( vAbsAngles[YAW] < 0.0f )
	//		{
	//			vAbsAngles[YAW] += 360.0f;
	//		}

	//		if ( vAbsAngles[PITCH] < qLook[PITCH] )
	//		{
	//			vAbsAngles[PITCH] += gpGlobals->frametime * 120.0f;
	//			if ( vAbsAngles[PITCH] > qLook[PITCH] )
	//				vAbsAngles[PITCH] = qLook[PITCH];
	//		}
	//		else if ( vAbsAngles[PITCH] > qLook[PITCH] )
	//		{
	//			vAbsAngles[PITCH] -= gpGlobals->frametime * 120.0f;
	//			if ( vAbsAngles[PITCH] < qLook[PITCH] )
	//				vAbsAngles[PITCH] = qLook[PITCH];
	//		}

	//		if ( vAbsAngles[YAW] < qLook[YAW] )
	//		{
	//			vAbsAngles[YAW] += gpGlobals->frametime * 240.0f;
	//			if ( vAbsAngles[YAW] > qLook[YAW] )
	//				vAbsAngles[YAW] = qLook[YAW];
	//		}
	//		else if ( vAbsAngles[YAW] > qLook[YAW] )
	//		{
	//			vAbsAngles[YAW] -= gpGlobals->frametime * 240.0f;
	//			if ( vAbsAngles[YAW] < qLook[YAW] )
	//				vAbsAngles[YAW] = qLook[YAW];
	//		}

	//		if ( vAbsAngles[YAW] > 180.0f )
	//		{
	//			vAbsAngles[YAW] -= 360.0f;
	//		}

	//		engine->SetViewAngles( vAbsAngles );
	//	}
	//}
}

void C_Portal_Player::FixTeleportationRoll(void)
{
	if (IsInAVehicle()) //HL2 compatibility fix. do absolutely nothing to the view in vehicles
		return;

	if (!IsLocalPlayer())
		return;

	// Normalize roll from odd portal transitions
	QAngle vAbsAngles = EyeAngles();


	Vector vCurrentForward, vCurrentRight, vCurrentUp;
	AngleVectors(vAbsAngles, &vCurrentForward, &vCurrentRight, &vCurrentUp);

	if (vAbsAngles[ROLL] == 0.0f)
	{
		m_fReorientationRate = 0.0f;
		g_bUpsideDown = (vCurrentUp.z < 0.0f);
		return;
	}

	bool bForcePitchReorient = (vAbsAngles[ROLL] > 175.0f && vCurrentForward.z > 0.99f);
	bool bOnGround = (GetGroundEntity() != NULL);

	if (bForcePitchReorient)
	{
		m_fReorientationRate = REORIENTATION_RATE * ((bOnGround) ? (2.0f) : (1.0f));
	}
	else
	{
		// Don't reorient in air if they don't want to
		if (!cl_reorient_in_air.GetBool() && !bOnGround)
		{
			g_bUpsideDown = (vCurrentUp.z < 0.0f);
			return;
		}
	}

	if (vCurrentUp.z < 0.75f)
	{
		m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;

		// Upright faster if on the ground
		float fMaxReorientationRate = REORIENTATION_RATE * ((bOnGround) ? (2.0f) : (1.0f));
		if (m_fReorientationRate > fMaxReorientationRate)
			m_fReorientationRate = fMaxReorientationRate;
	}
	else
	{
		if (m_fReorientationRate > REORIENTATION_RATE * 0.5f)
		{
			m_fReorientationRate -= gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if (m_fReorientationRate < REORIENTATION_RATE * 0.5f)
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		}
		else if (m_fReorientationRate < REORIENTATION_RATE * 0.5f)
		{
			m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if (m_fReorientationRate > REORIENTATION_RATE * 0.5f)
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		}
	}

	if (!m_bPitchReorientation && !bForcePitchReorient)
	{
		// Randomize which way we roll if we're completely upside down
		if (vAbsAngles[ROLL] == 180.0f && RandomInt(0, 1) == 1)
		{
			vAbsAngles[ROLL] = -180.0f;
		}

		if (vAbsAngles[ROLL] < 0.0f)
		{
			vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
			if (vAbsAngles[ROLL] > 0.0f)
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles(vAbsAngles);
		}
		else if (vAbsAngles[ROLL] > 0.0f)
		{
			vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
			if (vAbsAngles[ROLL] < 0.0f)
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles(vAbsAngles);
			m_angEyeAngles = vAbsAngles;
			m_iv_angEyeAngles.Reset();
		}
	}
	else
	{
		if (vAbsAngles[ROLL] != 0.0f)
		{
			if (vCurrentUp.z < 0.2f)
			{
				float fDegrees = gpGlobals->frametime * m_fReorientationRate;
				if (vCurrentForward.z > 0.0f)
				{
					fDegrees = -fDegrees;
				}

				// Rotate around the right axis
				VMatrix mAxisAngleRot = SetupMatrixAxisRot(vCurrentRight, fDegrees);

				vCurrentUp = mAxisAngleRot.VMul3x3(vCurrentUp);
				vCurrentForward = mAxisAngleRot.VMul3x3(vCurrentForward);

				VectorAngles(vCurrentForward, vCurrentUp, vAbsAngles);

				engine->SetViewAngles(vAbsAngles);
				m_angEyeAngles = vAbsAngles;
				m_iv_angEyeAngles.Reset();
			}
			else
			{
				if (vAbsAngles[ROLL] < 0.0f)
				{
					vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
					if (vAbsAngles[ROLL] > 0.0f)
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles(vAbsAngles);
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
				else if (vAbsAngles[ROLL] > 0.0f)
				{
					vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
					if (vAbsAngles[ROLL] < 0.0f)
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles(vAbsAngles);
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
			}
		}
	}

	// Keep track of if we're upside down for look control
	vAbsAngles = EyeAngles();
	AngleVectors(vAbsAngles, NULL, NULL, &vCurrentUp);

	if (bForcePitchReorient)
		g_bUpsideDown = (vCurrentUp.z < 0.0f);
	else
		g_bUpsideDown = false;
}

void C_Portal_Player::PlayerPortalled(C_Prop_Portal *pEnteredPortal)
{
	if (pEnteredPortal)
	{
		m_bPortalledMessagePending = true;
		m_PendingPortalMatrix = pEnteredPortal->MatrixThisToLinked();

		if (IsLocalPlayer())
			g_pPortalRender->EnteredPortal(pEnteredPortal);
	}
}

bool LocalPlayerIsCloseToPortal(void)
{
	return C_Portal_Player::GetLocalPlayer()->IsCloseToPortal();
}

bool C_Portal_Player::ShouldDraw(void)
{
	if (!IsAlive())
		return false;

	//return true;

	//	if( GetTeamNumber() == TEAM_SPECTATOR )
	//		return false;

	if (IsLocalPlayer() && IsRagdoll())
		return true;

	if (IsRagdoll())
		return false;

	return true;

	return BaseClass::ShouldDraw();
}
