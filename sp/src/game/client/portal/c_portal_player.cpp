//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for .
//
//===========================================================================//

#include "cbase.h"
#include "c_portal_player.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "portal_shareddefs.h"
#include "ivieweffects.h"		// for screenshake
#include "prop_portal_shared.h"

// NVNT for fov updates
#include "haptics/ihaptics.h"


// Don't alias here
#if defined( CPortal_Player )
#undef CPortal_Player	
#endif

ConVar cl_reorient_rate( "cl_reorient_rate", "1000", FCVAR_ARCHIVE, "Rate at which the player reorients to the camera" );
ConVar cl_reorient_acceleration_rate( "cl_reorient_acceleration_rate", "5000", FCVAR_ARCHIVE, "Acceleration rate at which the player reorients to the camera" );

ConVar cl_reorient_in_air("cl_reorient_in_air", "1", FCVAR_ARCHIVE, "Allows the player to only reorient from being upside down while in the air." ); 

LINK_ENTITY_TO_CLASS( player, C_Portal_Player );

IMPLEMENT_CLIENTCLASS_DT(C_Portal_Player, DT_Portal_Player, CPortal_Player)
RecvPropBool( RECVINFO( m_bHeldObjectOnOppositeSideOfPortal ) ),
RecvPropEHandle( RECVINFO( m_pHeldObjectPortal ) ),
RecvPropBool( RECVINFO( m_bPitchReorientation ) ),
RecvPropEHandle( RECVINFO( m_hPortalEnvironment ) ),
RecvPropEHandle( RECVINFO( m_hSurroundingLiquidPortal ) ),
RecvPropBool( RECVINFO( m_bSuppressingCrosshair ) ),
END_RECV_TABLE()


BEGIN_PREDICTION_DATA( C_Portal_Player )
END_PREDICTION_DATA()

extern bool g_bUpsideDown;

C_Portal_Player::C_Portal_Player()
: m_iv_angEyeAngles( "C_Portal_Player::m_iv_angEyeAngles" )
{
	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;
	m_bPitchReorientation = false;
	m_fReorientationRate = 0.0f;
	m_angEyeAngles.Init();

	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
}

C_Portal_Player::~C_Portal_Player( void )
{
}

void C_Portal_Player::ClientThink( void )
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

void C_Portal_Player::FixTeleportationRoll( void )
{
	if( IsInAVehicle() ) //HL2 compatibility fix. do absolutely nothing to the view in vehicles
		return;

	if( !IsLocalPlayer() )
		return;

	// Normalize roll from odd portal transitions
	QAngle vAbsAngles = EyeAngles();


	Vector vCurrentForward, vCurrentRight, vCurrentUp;
	AngleVectors( vAbsAngles, &vCurrentForward, &vCurrentRight, &vCurrentUp );

	if ( vAbsAngles[ROLL] == 0.0f )
	{
		m_fReorientationRate = 0.0f;
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
		return;
	}

	bool bForcePitchReorient = ( vAbsAngles[ROLL] > 175.0f && vCurrentForward.z > 0.99f );
	bool bOnGround = ( GetGroundEntity() != NULL );

	if ( bForcePitchReorient )
	{
		m_fReorientationRate = cl_reorient_rate.GetFloat() * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
	}
	else
	{
		// Don't reorient in air if they don't want to
		if ( !cl_reorient_in_air.GetBool() && !bOnGround )
		{
			g_bUpsideDown = ( vCurrentUp.z < 0.0f );
			return;
		}
	}

	if ( vCurrentUp.z < 0.75f )
	{
		m_fReorientationRate += gpGlobals->frametime * cl_reorient_acceleration_rate.GetFloat();

		// Upright faster if on the ground
		float fMaxReorientationRate = cl_reorient_rate.GetFloat() * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
		if ( m_fReorientationRate > fMaxReorientationRate )
			m_fReorientationRate = fMaxReorientationRate;
	}
	else
	{
		if ( m_fReorientationRate > cl_reorient_rate.GetFloat() * 0.5f )
		{
			m_fReorientationRate -= gpGlobals->frametime * cl_reorient_acceleration_rate.GetFloat();
			if ( m_fReorientationRate < cl_reorient_rate.GetFloat() * 0.5f )
				m_fReorientationRate = cl_reorient_rate.GetFloat() * 0.5f;
		}
		else if ( m_fReorientationRate < cl_reorient_rate.GetFloat() * 0.5f )
		{
			m_fReorientationRate += gpGlobals->frametime * cl_reorient_acceleration_rate.GetFloat();
			if ( m_fReorientationRate > cl_reorient_rate.GetFloat() * 0.5f )
				m_fReorientationRate = cl_reorient_rate.GetFloat() * 0.5f;
		}
	}

	if ( !m_bPitchReorientation && !bForcePitchReorient )
	{
		// Randomize which way we roll if we're completely upside down
		if ( vAbsAngles[ROLL] == 180.0f && RandomInt( 0, 1 ) == 1 )
		{
			vAbsAngles[ROLL] = -180.0f;
		}

		if ( vAbsAngles[ROLL] < 0.0f )
		{
			vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ROLL] > 0.0f )
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles( vAbsAngles );
		}
		else if ( vAbsAngles[ROLL] > 0.0f )
		{
			vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ROLL] < 0.0f )
				vAbsAngles[ROLL] = 0.0f;
			engine->SetViewAngles( vAbsAngles );
			m_angEyeAngles = vAbsAngles;
			m_iv_angEyeAngles.Reset();
		}
	}
	else
	{
		if ( vAbsAngles[ROLL] != 0.0f )
		{
			if ( vCurrentUp.z < 0.2f )
			{
				float fDegrees = gpGlobals->frametime * m_fReorientationRate;
				if ( vCurrentForward.z > 0.0f )
				{
					fDegrees = -fDegrees;
				}

				// Rotate around the right axis
				VMatrix mAxisAngleRot = SetupMatrixAxisRot( vCurrentRight, fDegrees );

				vCurrentUp = mAxisAngleRot.VMul3x3( vCurrentUp );
				vCurrentForward = mAxisAngleRot.VMul3x3( vCurrentForward );

				VectorAngles( vCurrentForward, vCurrentUp, vAbsAngles );

				engine->SetViewAngles( vAbsAngles );
				m_angEyeAngles = vAbsAngles;
				m_iv_angEyeAngles.Reset();
			}
			else
			{
				if ( vAbsAngles[ROLL] < 0.0f )
				{
					vAbsAngles[ROLL] += gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ROLL] > 0.0f )
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles( vAbsAngles );
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
				else if ( vAbsAngles[ROLL] > 0.0f )
				{
					vAbsAngles[ROLL] -= gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ROLL] < 0.0f )
						vAbsAngles[ROLL] = 0.0f;
					engine->SetViewAngles( vAbsAngles );
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
			}
		}
	}

	// Keep track of if we're upside down for look control
	vAbsAngles = EyeAngles();
	AngleVectors( vAbsAngles, NULL, NULL, &vCurrentUp );

	if ( bForcePitchReorient )
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
	else
		g_bUpsideDown = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_Portal_Player::DrawModel( int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

	if( IsLocalPlayer() )
	{
		if ( !C_BasePlayer::ShouldDrawThisPlayer() )
		{
			if ( !g_pPortalRender->IsRenderingPortal() )
				return 0;

			if( (g_pPortalRender->GetViewRecursionLevel() == 1) && (m_iForceNoDrawInPortalSurface != -1) ) //CPortalRender::s_iRenderingPortalView )
				return 0;
		}
	}

	return BaseClass::DrawModel(flags);
}

void C_Portal_Player::DoImpactEffect( trace_t &tr, int nDamageType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->DoImpactEffect( tr, nDamageType );
		return;
	}

	BaseClass::DoImpactEffect( tr, nDamageType );
}

void C_Portal_Player::PreThink( void )
{
	QAngle vTempAngles = GetLocalAngles();

	if ( IsLocalPlayer() )
	{
		vTempAngles[PITCH] = EyeAngles()[PITCH];
	}
	else
	{
		vTempAngles[PITCH] = m_angEyeAngles[PITCH];
	}

	if ( vTempAngles[YAW] < 0.0f )
	{
		vTempAngles[YAW] += 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();

	//HandleSpeedChanges();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Portal_Player::AddEntity( void )
{
	BaseClass::AddEntity();

	//needed for player animation, This line will prevent the model of the player from rotating when looking up & firing
	// this needs to be here and not in C_BasePlayer because otherwise it won't work on player's shadowclone
	SetLocalAnglesDim(X_INDEX, 0);
}


bool C_Portal_Player::ShouldDraw( void )
{
	if ( !IsAlive() )
		return false;

	//return true;

	//	if( GetTeamNumber() == TEAM_SPECTATOR )
	//		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;

	if ( IsRagdoll() )
		return false;

	return C_BaseAnimating::ShouldDraw(); //skip C_BasePlayer to C_BaseAnimating
}

void C_Portal_Player::PlayerPortalled( C_Prop_Portal *pEnteredPortal )
{
	if( pEnteredPortal )
	{
		m_bPortalledMessagePending = true;
		m_PendingPortalMatrix = pEnteredPortal->MatrixThisToLinked();

		if( IsLocalPlayer() )
			g_pPortalRender->EnteredPortal( pEnteredPortal );
	}
}

void C_Portal_Player::OnPreDataChanged( DataUpdateType_t type )
{
	//Assert(m_pPortalEnvironment_LastCalcView == m_hPortalEnvironment.Get());
	//PreDataChanged_Backup.m_hPortalEnvironment = m_hPortalEnvironment;
	PreDataChanged_Backup.m_hSurroundingLiquidPortal = m_hSurroundingLiquidPortal;
	PreDataChanged_Backup.m_qEyeAngles = m_iv_angEyeAngles.GetCurrent();

	BaseClass::OnPreDataChanged( type );
}

void C_Portal_Player::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if( m_hSurroundingLiquidPortal != PreDataChanged_Backup.m_hSurroundingLiquidPortal )
	{
		CLiquidPortal_InnerLiquidEffect *pLiquidEffect = (CLiquidPortal_InnerLiquidEffect *)g_pScreenSpaceEffects->GetScreenSpaceEffect( "LiquidPortal_InnerLiquid" );
		if( pLiquidEffect )
		{
			C_Func_LiquidPortal *pSurroundingPortal = m_hSurroundingLiquidPortal.Get();
			if( pSurroundingPortal != NULL )
			{
				C_Func_LiquidPortal *pOldSurroundingPortal = PreDataChanged_Backup.m_hSurroundingLiquidPortal.Get();
				if( pOldSurroundingPortal != pSurroundingPortal->m_hLinkedPortal.Get() )
				{
					pLiquidEffect->m_pImmersionPortal = pSurroundingPortal;
					pLiquidEffect->m_bFadeBackToReality = false;
				}
				else
				{
					pLiquidEffect->m_bFadeBackToReality = true;
					pLiquidEffect->m_fFadeBackTimeLeft = pLiquidEffect->s_fFadeBackEffectTime;
				}
			}
			else
			{
				pLiquidEffect->m_pImmersionPortal = NULL;
				pLiquidEffect->m_bFadeBackToReality = false;
			}
		}		
	}

	DetectAndHandlePortalTeleportation();

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

//CalcView() gets called between OnPreDataChanged() and OnDataChanged(), and these changes need to be known about in both before CalcView() gets called, and if CalcView() doesn't get called
bool C_Portal_Player::DetectAndHandlePortalTeleportation( void )
{
	if( m_bPortalledMessagePending )
	{
		m_bPortalledMessagePending = false;

		//C_Prop_Portal *pOldPortal = PreDataChanged_Backup.m_hPortalEnvironment.Get();
		//Assert( pOldPortal );
		//if( pOldPortal )
		{
			Vector ptNewPosition = GetNetworkOrigin();

			UTIL_Portal_PointTransform( m_PendingPortalMatrix, m_vEyePosition, m_vEyePosition );

			UTIL_Portal_AngleTransform( m_PendingPortalMatrix, m_qEyeAngles_LastCalcView, m_angEyeAngles );
			m_angEyeAngles.x = AngleNormalize( m_angEyeAngles.x );
			m_angEyeAngles.y = AngleNormalize( m_angEyeAngles.y );
			m_angEyeAngles.z = AngleNormalize( m_angEyeAngles.z );
			m_iv_angEyeAngles.Reset(); //copies from m_angEyeAngles

			if( engine->IsPlayingDemo() )
			{				
				pl.v_angle = m_angEyeAngles;		
				engine->SetViewAngles( pl.v_angle );
			}

			engine->ResetDemoInterpolation();
			if( IsLocalPlayer() ) 
			{
				//DevMsg( "FPT: %.2f %.2f %.2f\n", m_angEyeAngles.x, m_angEyeAngles.y, m_angEyeAngles.z );
				SetLocalAngles( m_angEyeAngles );
			}

			// Reorient last facing direction to fix pops in view model lag
			for ( int i = 0; i < MAX_VIEWMODELS; i++ )
			{
				CBaseViewModel *vm = GetViewModel( i );
				if ( !vm )
					continue;

				UTIL_Portal_VectorTransform( m_PendingPortalMatrix, vm->m_vecLastFacing, vm->m_vecLastFacing );
			}
		}
		m_bPortalledMessagePending = false;
	}

	return false;
}

void C_Portal_Player::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	DetectAndHandlePortalTeleportation();
	//if( DetectAndHandlePortalTeleportation() )
	//	DevMsg( "Teleported within OnDataChanged\n" );

	m_iForceNoDrawInPortalSurface = -1;
	bool bEyeTransform_Backup = m_bEyePositionIsTransformedByPortal;
	m_bEyePositionIsTransformedByPortal = false; //assume it's not transformed until it provably is
	m_vEyePosition = EyePosition();

	QAngle qEyeAngleBackup = EyeAngles();
	Vector ptEyePositionBackup = EyePosition();
	//C_Prop_Portal *pPortalBackup = m_hPortalEnvironment.Get();

	if ( m_lifeState != LIFE_ALIVE )
	{
		if ( g_nKillCamMode != 0 )
		{
			return;
		}

		Vector origin = EyePosition();

		IRagdoll* pRagdoll = GetRepresentativeRagdoll();

		if ( pRagdoll )
		{
			origin = pRagdoll->GetRagdollOrigin();
#if !PORTAL_HIDE_PLAYER_RAGDOLL
			origin.z += VEC_DEAD_VIEWHEIGHT_SCALED( this ).z; // look over ragdoll, not through
#endif //PORTAL_HIDE_PLAYER_RAGDOLL
		}

		BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );

		eyeOrigin = origin;

		Vector vForward; 
		AngleVectors( eyeAngles, &vForward );

		VectorNormalize( vForward );
#if !PORTAL_HIDE_PLAYER_RAGDOLL
		VectorMA( origin, -CHASE_CAM_DISTANCE_MAX, vForward, eyeOrigin );
#endif //PORTAL_HIDE_PLAYER_RAGDOLL

		Vector WALL_MIN( -WALL_OFFSET, -WALL_OFFSET, -WALL_OFFSET );
		Vector WALL_MAX( WALL_OFFSET, WALL_OFFSET, WALL_OFFSET );

		trace_t trace; // clip against world
		C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull( origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );
		C_BaseEntity::PopEnableAbsRecomputations();

		if (trace.fraction < 1.0)
		{
			eyeOrigin = trace.endpos;
		}
	}
	else
	{
		IClientVehicle *pVehicle; 
		pVehicle = GetVehicle();

		if ( !pVehicle )
		{
			if ( IsObserver() )
			{
				CalcObserverView( eyeOrigin, eyeAngles, fov );
			}
			else
			{
				CalcPlayerView( eyeOrigin, eyeAngles, fov );
				if( m_hPortalEnvironment.Get() != NULL )
				{
					//time for hax
					m_bEyePositionIsTransformedByPortal = bEyeTransform_Backup;
					CalcPortalView( eyeOrigin, eyeAngles );
				}
			}
		}
		else
		{
			CalcVehicleView( pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov );
		}
	}

	m_qEyeAngles_LastCalcView = qEyeAngleBackup;
	//m_ptEyePosition_LastCalcView = ptEyePositionBackup;
	//m_pPortalEnvironment_LastCalcView = pPortalBackup;

#ifdef WIN32
	// NVNT Inform haptics module of fov
	if(IsLocalPlayer())
		haptics->UpdatePlayerFOV(fov);
#endif
}

void C_Portal_Player::CalcPortalView( Vector &eyeOrigin, QAngle &eyeAngles )
{
	//although we already ran CalcPlayerView which already did these copies, they also fudge these numbers in ways we don't like, so recopy
	VectorCopy( EyePosition(), eyeOrigin );
	VectorCopy( EyeAngles(), eyeAngles );

	//Re-apply the screenshake (we just stomped it)
	vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );

	C_Prop_Portal *pPortal = m_hPortalEnvironment.Get();
	assert( pPortal );

	C_Prop_Portal *pRemotePortal = pPortal->m_hLinkedPortal;
	if( !pRemotePortal )
	{
		return; //no hacks possible/necessary
	}

	Vector ptPortalCenter;
	Vector vPortalForward;

	ptPortalCenter = pPortal->GetNetworkOrigin();
	pPortal->GetVectors( &vPortalForward, NULL, NULL );
	float fPortalPlaneDist = vPortalForward.Dot( ptPortalCenter );

	bool bOverrideSpecialEffects = false; //sometimes to get the best effect we need to kill other effects that are simply for cleanliness

	float fEyeDist = vPortalForward.Dot( eyeOrigin ) - fPortalPlaneDist;
	bool bTransformEye = false;
	if( fEyeDist < 0.0f ) //eye behind portal
	{
		if( pPortal->m_PortalSimulator.EntityIsInPortalHole( this ) ) //player standing in portal
		{
			bTransformEye = true;
		}
		else if( vPortalForward.z < -0.01f ) //there's a weird case where the player is ducking below a ceiling portal. As they unduck their eye moves beyond the portal before the code detects that they're in the portal hole.
		{
			Vector ptPlayerOrigin = GetAbsOrigin();
			float fOriginDist = vPortalForward.Dot( ptPlayerOrigin ) - fPortalPlaneDist;

			if( fOriginDist > 0.0f )
			{
				float fInvTotalDist = 1.0f / (fOriginDist - fEyeDist); //fEyeDist is negative
				Vector ptPlaneIntersection = (eyeOrigin * fOriginDist * fInvTotalDist) - (ptPlayerOrigin * fEyeDist * fInvTotalDist);
				Assert( fabs( vPortalForward.Dot( ptPlaneIntersection ) - fPortalPlaneDist ) < 0.01f );

				Vector vIntersectionTest = ptPlaneIntersection - ptPortalCenter;

				Vector vPortalRight, vPortalUp;
				pPortal->GetVectors( NULL, &vPortalRight, &vPortalUp );

				if( (vIntersectionTest.Dot( vPortalRight ) <= PORTAL_HALF_WIDTH) &&
					(vIntersectionTest.Dot( vPortalUp ) <= PORTAL_HALF_HEIGHT) )
				{
					bTransformEye = true;
				}
			}
		}		
	}

	if( bTransformEye )
	{
		m_bEyePositionIsTransformedByPortal = true;

		//DevMsg( 2, "transforming portal view from <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		VMatrix matThisToLinked = pPortal->MatrixThisToLinked();
		UTIL_Portal_PointTransform( matThisToLinked, eyeOrigin, eyeOrigin );
		UTIL_Portal_AngleTransform( matThisToLinked, eyeAngles, eyeAngles );

		//DevMsg( 2, "transforming portal view to   <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		if ( IsToolRecording() )
		{
			static EntityTeleportedRecordingState_t state;

			KeyValues *msg = new KeyValues( "entity_teleported" );
			msg->SetPtr( "state", &state );
			state.m_bTeleported = false;
			state.m_bViewOverride = true;
			state.m_vecTo = eyeOrigin;
			state.m_qaTo = eyeAngles;
			MatrixInvert( matThisToLinked.As3x4(), state.m_teleportMatrix );

			// Post a message back to all IToolSystems
			Assert( (int)GetToolHandle() != 0 );
			ToolFramework_PostToolMessage( GetToolHandle(), msg );

			msg->deleteThis();
		}

		bOverrideSpecialEffects = true;
	}
	else
	{
		m_bEyePositionIsTransformedByPortal = false;
	}

	if( bOverrideSpecialEffects )
	{		
		m_iForceNoDrawInPortalSurface = ((pRemotePortal->m_bIsPortal2)?(2):(1));
		pRemotePortal->m_fStaticAmount = 0.0f;
	}
}

extern float g_fMaxViewModelLag;
void C_Portal_Player::CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	// HACK: Manually adjusting the eye position that view model looking up and down are similar
	// (solves view model "pop" on floor to floor transitions)
	Vector vInterpEyeOrigin = eyeOrigin;

	Vector vForward;
	Vector vRight;
	Vector vUp;
	AngleVectors( eyeAngles, &vForward, &vRight, &vUp );

	if ( vForward.z < 0.0f )
	{
		float fT = vForward.z * vForward.z;
		vInterpEyeOrigin += vRight * ( fT * 4.7f ) + vForward * ( fT * 5.0f ) + vUp * ( fT * 4.0f );
	}

	if ( UTIL_IntersectEntityExtentsWithPortal( this ) )
		g_fMaxViewModelLag = 0.0f;
	else
		g_fMaxViewModelLag = 1.5f;

	for ( int i = 0; i < MAX_VIEWMODELS; i++ )
	{
		CBaseViewModel *vm = GetViewModel( i );
		if ( !vm )
			continue;

		vm->CalcViewModelView( this, vInterpEyeOrigin, eyeAngles );
	}
}

bool LocalPlayerIsCloseToPortal( void )
{
	return C_Portal_Player::GetLocalPlayer()->IsCloseToPortal();
}

