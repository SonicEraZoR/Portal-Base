#include "cbase.h"
#include "gamemovement.h"
#include "in_buttons.h"
#include <stdarg.h>
#include "movevars_shared.h"
#include "engine/IEngineTrace.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "decals.h"
#include "tier0/vprof.h"
#include "hl1_gamemovement.h"

extern ConVar hl1_movement;

static CHL1GameMovement g_GameMovement;
IGameMovement *g_pGameMovement = (IGameMovement *)&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement );

#ifdef CLIENT_DLL
#include "c_hl1_player.h"
#else
//#include "hl1_player.h"
#endif

bool CHL1GameMovement::CheckJumpButton( void )
{
	m_pHL1Player = ToHL1Player( player );

	Assert( m_pHL1Player );

	if (m_pHL1Player->pl.deadflag)
	{
		mv->m_nOldButtons |= IN_JUMP ;
		return false;
	}

	if (m_pHL1Player->m_flWaterJumpTime)
	{
		m_pHL1Player->m_flWaterJumpTime -= gpGlobals->frametime;
		if (m_pHL1Player->m_flWaterJumpTime < 0)
			m_pHL1Player->m_flWaterJumpTime = 0;
		
		return false;
	}

	if ( m_pHL1Player->GetWaterLevel() >= 2 )
	{	
		SetGroundEntity( NULL );

		if(m_pHL1Player->GetWaterType() == CONTENTS_WATER)
			mv->m_vecVelocity[2] = 100;
		else if (m_pHL1Player->GetWaterType() == CONTENTS_SLIME)
			mv->m_vecVelocity[2] = 80;
		
		if ( m_pHL1Player->m_flSwimSoundTime <= 0 )
		{
			m_pHL1Player->m_flSwimSoundTime = 1000;
			PlaySwimSound();
		}

		return false;
	}

 	if (m_pHL1Player->GetGroundEntity() == NULL)
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;
	}

	if ( mv->m_nOldButtons & IN_JUMP )
		return false;

	SetGroundEntity( NULL );
	
	Vector tempvec = mv->GetAbsOrigin();
	m_pHL1Player->PlayStepSound( tempvec, player->GetSurfaceData(), 1.0, true );
	
	MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );

	float flGroundFactor = 1.0f;
	if ( player->GetSurfaceData() )
	{
		flGroundFactor = player->m_pSurfaceData->game.jumpFactor;
	}

	float startz = mv->m_vecVelocity[2];
	if ( (  m_pHL1Player->m_Local.m_bDucking ) || (  m_pHL1Player->GetFlags() & FL_DUCKING ) )
	{
		if ( m_pHL1Player->m_bHasLongJump &&
			( mv->m_nButtons & IN_DUCK ) &&
			( m_pHL1Player->m_Local.m_flDucktime > 0 ) &&
			mv->m_vecVelocity.Length() > 50 )
		{
			m_pHL1Player->m_Local.m_vecPunchAngle.Set( PITCH, -5 );

			mv->m_vecVelocity = m_vecForward * PLAYER_LONGJUMP_SPEED * 1.6;
			mv->m_vecVelocity.z = sqrt(2 * 800 * 56.0);
		}
		else
		{
			mv->m_vecVelocity[2] = sqrt(45.0 * 800 * 2);
		}
	}
	else
	{
		mv->m_vecVelocity[2] = sqrt(45.0 * 800 * 2);
	}
	FinishGravity();

	mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.1f;
	
	if ( gpGlobals->maxClients > 1 )

	mv->m_nOldButtons |= IN_JUMP;
	return true;
}

bool CHL1GameMovement::CanUnduck()
{
	if (hl1_movement.GetBool())
	{
		int i;
		trace_t trace;
		Vector newOrigin;

		VectorCopy(mv->m_vecAbsOrigin, newOrigin);

		if (player->GetGroundEntity() != NULL)
		{
			for (i = 0; i < 3; i++)
			{
				newOrigin[i] += (VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i]);
			}
		}
		else
		{
			// If in air and letting go of crouch, make sure we can offset origin to make
			//  up for uncrouching
			Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
			Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

			Vector viewDelta = -0.5f * (hullSizeNormal - hullSizeCrouch);

			VectorAdd(newOrigin, viewDelta, newOrigin);
		}

		bool saveducked = player->m_Local.m_bDucked;
		player->m_Local.m_bDucked = false;
		TracePlayerBBox(mv->m_vecAbsOrigin, newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
		player->m_Local.m_bDucked = saveducked;
		if (trace.startsolid || (trace.fraction != 1.0f))
			return false;

		return true;
	}
	
	return BaseClass::CanUnduck();
}

//-----------------------------------------------------------------------------
// Purpose: Stop ducking
//-----------------------------------------------------------------------------
void CHL1GameMovement::FinishUnDuck(void)
{
	if (hl1_movement.GetBool())
	{
		int i;
		trace_t trace;
		Vector newOrigin;

		VectorCopy(mv->m_vecAbsOrigin, newOrigin);

		if (player->GetGroundEntity() != NULL)
		{
			for (i = 0; i < 3; i++)
			{
				newOrigin[i] += (VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i]);
			}
		}
		else
		{
			// If in air an letting go of croush, make sure we can offset origin to make
			//  up for uncrouching
			Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
			Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

			Vector viewDelta = -0.5f * (hullSizeNormal - hullSizeCrouch);

			VectorAdd(newOrigin, viewDelta, newOrigin);
		}

		player->m_Local.m_bDucked = false;
		player->RemoveFlag(FL_DUCKING);
		player->m_Local.m_bDucking = false;
		player->SetViewOffset(GetPlayerViewOffset(false));
		player->m_Local.m_flDucktime = 0;

		VectorCopy(newOrigin, mv->m_vecAbsOrigin);

		// Recategorize position since ducking can change origin
		CategorizePosition();
	}
	else
	{
		return BaseClass::FinishUnDuck();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CHL1GameMovement::FinishDuck(void)
{
	if (hl1_movement.GetBool())
	{
		int i;

		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

		Vector viewDelta = 0.5f * (hullSizeNormal - hullSizeCrouch);

		player->m_Local.m_bDucked = true;
		player->SetViewOffset(GetPlayerViewOffset(true));
		player->AddFlag(FL_DUCKING);
		player->m_Local.m_bDucking = false;

		// HACKHACK - Fudge for collision bug - no time to fix this properly
		if (player->GetGroundEntity() != NULL)
		{
			for (i = 0; i < 3; i++)
			{
				mv->m_vecAbsOrigin[i] -= (VEC_DUCK_HULL_MIN[i] - VEC_HULL_MIN[i]);
			}
		}
		else
		{
			VectorAdd(mv->m_vecAbsOrigin, viewDelta, mv->m_vecAbsOrigin);
		}

		// See if we are stuck?
		FixPlayerCrouchStuck(true);

		// Recategorize position since ducking can change origin
		CategorizePosition();
	}
	else
	{
		return BaseClass::FinishDuck();
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if duck button is pressed and do the appropriate things
//-----------------------------------------------------------------------------
void CHL1GameMovement::Duck(void)
{
	if (hl1_movement.GetBool())
	{
		int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons);	// These buttons have changed this frame
		int buttonsPressed = buttonsChanged & mv->m_nButtons;			// The changed ones still down are "pressed"
		int buttonsReleased = buttonsChanged & mv->m_nOldButtons;		// The changed ones which were previously down are "released"

		// Check to see if we are in the air.
		bool bInAir = (player->GetGroundEntity() == NULL);
		bool bInDuck = (player->GetFlags() & FL_DUCKING) ? true : false;

		if (mv->m_nButtons & IN_DUCK)
		{
			mv->m_nOldButtons |= IN_DUCK;
		}
		else
		{
			mv->m_nOldButtons &= ~IN_DUCK;
		}

		// Handle death.
		if (IsDead())
		{
			if (bInDuck)
			{
				// Unduck
				FinishUnDuck();
			}
			return;
		}

		HandleDuckingSpeedCrop();

		// If the player is holding down the duck button, the player is in duck transition, ducking, or duck-jumping.
		if ((mv->m_nButtons & IN_DUCK) || player->m_Local.m_bDucking || bInDuck)
		{
			if ((mv->m_nButtons & IN_DUCK))
			{
				// Have the duck button pressed, but the player currently isn't in the duck position.
				if ((buttonsPressed & IN_DUCK) && !bInDuck)
				{
					// Use 1 second so super long jump will work
					player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
					player->m_Local.m_bDucking = true;
				}

				// The player is in duck transition and not duck-jumping.
				if (player->m_Local.m_bDucking)
				{
					float flDuckMilliseconds = max(0.0f, GAMEMOVEMENT_DUCK_TIME - (float)player->m_Local.m_flDucktime);
					float flDuckSeconds = flDuckMilliseconds / GAMEMOVEMENT_DUCK_TIME;

					// Finish in duck transition when transition time is over, in "duck", in air.
					if ((flDuckSeconds > TIME_TO_DUCK) || bInDuck || bInAir)
					{
						FinishDuck();
					}
					else
					{
						// Calc parametric time
						float flDuckFraction = SimpleSpline(flDuckSeconds / TIME_TO_DUCK);
						SetDuckedEyeOffset(flDuckFraction);
					}
				}
			}
			else
			{
				// Try to unduck unless automovement is not allowed
				// NOTE: When not onground, you can always unduck
				if (player->m_Local.m_bAllowAutoMovement || bInAir)
				{
					if ((buttonsReleased & IN_DUCK) && bInDuck)
					{
						// Use 1 second so super long jump will work
						player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
					}

					// Check to see if we are capable of unducking.
					if (CanUnduck())
					{
						// or unducking
						if ((player->m_Local.m_bDucking || player->m_Local.m_bDucked))
						{
							float flDuckMilliseconds = max(0.0f, GAMEMOVEMENT_DUCK_TIME - (float)player->m_Local.m_flDucktime);
							float flDuckSeconds = flDuckMilliseconds / GAMEMOVEMENT_DUCK_TIME;

							// Finish ducking immediately if duck time is over or not on ground
							if (flDuckSeconds > TIME_TO_UNDUCK || (bInAir))
							{
								FinishUnDuck();
							}
							else
							{
								// Calc parametric time
								float flDuckFraction = SimpleSpline(1.0f - (flDuckSeconds / TIME_TO_UNDUCK));
								SetDuckedEyeOffset(flDuckFraction);
								player->m_Local.m_bDucking = true;
							}
						}
					}
					else
					{
						// Still under something where we can't unduck, so make sure we reset this timer so
						//  that we'll unduck once we exit the tunnel, etc.
						player->m_Local.m_flDucktime = GAMEMOVEMENT_DUCK_TIME;
						SetDuckedEyeOffset(1.0f);
					}
				}
			}
		}
	}
	else
	{
		return BaseClass::Duck();
	}
}

void CHL1GameMovement::HandleDuckingSpeedCrop()
{
	if ( !m_iSpeedCropped && ( player->GetFlags() & FL_DUCKING ) )
	{
		float frac = 0.33333333f;
		mv->m_flForwardMove	*= frac;
		mv->m_flSideMove	*= frac;
		mv->m_flUpMove		*= frac;
		m_iSpeedCropped		= true;
	}
}

void CHL1GameMovement::CheckParameters( void )
{
	if ( mv->m_nButtons & IN_SPEED )
	{
		mv->m_flClientMaxSpeed = 100;
	}
	else
	{
		mv->m_flClientMaxSpeed = mv->m_flMaxSpeed;
	}

	CHL1_Player* pHL1Player = dynamic_cast<CHL1_Player*>(player);
	if( pHL1Player && pHL1Player->IsPullingObject() )
	{
		mv->m_flClientMaxSpeed = mv->m_flMaxSpeed * 0.5f;
	}

	BaseClass::CheckParameters();
}
