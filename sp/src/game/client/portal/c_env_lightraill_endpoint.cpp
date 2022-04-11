//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "particles_simple.h"
#include "env_lightrail_endpoint_shared.h"
#include "particles_attractor.h"
#include "fx_quad.h"

class C_Env_Lightrail_Endpoint : public C_BaseEntity
{
	DECLARE_CLASS( C_Env_Lightrail_Endpoint, C_BaseEntity );
	DECLARE_CLIENTCLASS();

public:
	void			OnDataChanged( DataUpdateType_t updateType );
	RenderGroup_t	GetRenderGroup( void );

	void			ClientThink( void );
	void			NotifyShouldTransmit( ShouldTransmitState_t state );

	void			UpdateIdle( float percentage );
	void			UpdateCharging( float percentage );		//Update the charging state
	void			UpdateLargeFX( void );					//Update Discharging Large FX
	void			UpdateSmallFX( void );					//Update Discharging Small FX

private:	

	bool			SetupEmitters( void );
	inline float	GetStateDurationPercentage( void );

	float			m_flSmallScale;
	float			m_flLargeScale;
	int				m_nState;
	float			m_flDuration;
	float			m_flStartTime;
	int				m_spawnflags;
	
	CSmartPtr<CSimpleEmitter>		m_pSimpleEmitter;
	CSmartPtr<CParticleAttractor>	m_pAttractorEmitter;
};

IMPLEMENT_CLIENTCLASS_DT( C_Env_Lightrail_Endpoint, DT_Env_Lightrail_Endpoint, CEnv_Lightrail_Endpoint )
	RecvPropFloat( RECVINFO(m_flSmallScale) ),
	RecvPropFloat( RECVINFO(m_flLargeScale) ),
	RecvPropInt( RECVINFO(m_nState) ),
	RecvPropFloat( RECVINFO(m_flDuration) ),
	RecvPropFloat( RECVINFO(m_flStartTime) ),
	RecvPropInt( RECVINFO(m_spawnflags) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_Env_Lightrail_Endpoint::GetRenderGroup( void )
{
	return RENDER_GROUP_TRANSLUCENT_ENTITY;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
		SetupEmitters();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_Env_Lightrail_Endpoint::SetupEmitters( void )
{
	// Setup the basic core emitter
	if ( m_pSimpleEmitter.IsValid() == false )
	{
		m_pSimpleEmitter = CSimpleEmitter::Create( "energycore" );

		if ( m_pSimpleEmitter.IsValid() == false )
			return false;
	}

	// Setup the attractor emitter
	if ( m_pAttractorEmitter.IsValid() == false )
	{
		m_pAttractorEmitter = CParticleAttractor::Create( GetAbsOrigin(), "energyattractor" );

		if ( m_pAttractorEmitter.IsValid() == false )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : percentage - 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::UpdateIdle( float percentage )
{
	// Only do these particles if required
	//if ( m_spawnflags & SF_ENERGYCORE_NO_PARTICLES )
	//	return;

	// Must be active
	if ( percentage >= 1.0f )
		return;

	// Emitters must be valid
	if ( SetupEmitters() == false )
		return;

	// Reset our sort origin
	m_pSimpleEmitter->SetSortOrigin( GetAbsOrigin() );

	SimpleParticle *sParticle;

	// Do the charging particles
	m_pAttractorEmitter->SetAttractorOrigin( GetAbsOrigin() );

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	Vector	offset;
	float	dist;

	int numParticles = floor( 4.0f * percentage );

	for ( int i = 0; i < numParticles; i++ )
	{
		dist = random->RandomFloat( 4.0f * percentage, 64.0f * percentage );

		offset = forward * dist;

		dist = RemapValClamped( dist, 4.0f * percentage, 64.0f * percentage, 6.0f, 1.0f );
		offset += right * random->RandomFloat( -4.0f * dist, 4.0f * dist );
		offset += up * random->RandomFloat( -4.0f * dist, 4.0f * dist );

		offset += GetAbsOrigin();

		sParticle = (SimpleParticle *) m_pAttractorEmitter->AddParticle( sizeof(SimpleParticle), m_pAttractorEmitter->GetPMaterial( "effects/strider_muzzle" ), offset );

		if ( sParticle == NULL )
			return;
		
		sParticle->m_vecVelocity	= Vector(0,0,8);
		sParticle->m_flDieTime		= 0.5f;
		sParticle->m_flLifetime		= 0.0f;

		sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= 0.0f;

		float alpha = 255 * percentage;

		sParticle->m_uchColor[0]	= alpha;
		sParticle->m_uchColor[1]	= alpha;
		sParticle->m_uchColor[2]	= alpha;
		sParticle->m_uchStartAlpha	= alpha;
		sParticle->m_uchEndAlpha	= 0;

		sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 );
		sParticle->m_uchEndSize		= 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : percentage - 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::UpdateCharging( float percentage )
{
	// Emitters must be valid
	if ( SetupEmitters() == false )
		return;

	if ( percentage <= 0.0f )
		return;

	// Reset our sort origin
	m_pSimpleEmitter->SetSortOrigin( GetAbsOrigin() );

	float flLargeScale = 4.0f * m_flLargeScale * percentage;
	//float flSmallScale = 4.0f * m_flSmallScale * percentage;

	SimpleParticle *sParticle;

	// Do the core effects
	for ( int i = 0; i < 2; i++ )
	{
		sParticle = (SimpleParticle *) m_pSimpleEmitter->AddParticle( sizeof(SimpleParticle), m_pSimpleEmitter->GetPMaterial( "effects/strider_muzzle" ), GetAbsOrigin() );

		if ( sParticle == NULL )
			return;
		
		sParticle->m_vecVelocity	= vec3_origin;
		sParticle->m_flDieTime		= 0.1f;
		sParticle->m_flLifetime		= 0.0f;

		sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= 0.0f;

		float alpha = 255;

		sParticle->m_uchColor[0]	= alpha;
		sParticle->m_uchColor[1]	= alpha;
		sParticle->m_uchColor[2]	= alpha;
		sParticle->m_uchStartAlpha	= alpha;
		sParticle->m_uchEndAlpha	= 0;

		if ( i < 2 )
		{
			sParticle->m_uchStartSize	= flLargeScale * (i+1);
			sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;
		}
		else
		{
			if ( random->RandomInt( 0, 20 ) == 0 )
			{
				sParticle->m_uchStartSize	= flLargeScale * (i+1);
				sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 4.0f;
				sParticle->m_flDieTime		= 0.25f;
			}
			else
			{
				sParticle->m_uchStartSize	= flLargeScale * (i+1);
				sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;
			}
		}
	}

	// Only do these particles if required
	//if ( m_spawnflags & SF_ENERGYCORE_NO_PARTICLES )
	//	return;

	// Do the charging particles
	m_pAttractorEmitter->SetAttractorOrigin( GetAbsOrigin() );

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	Vector	offset;
	float	dist;

	int numParticles = floor( 4.0f * percentage );

	for ( int i = 0; i < numParticles; i++ )
	{
		dist = random->RandomFloat( 4.0f * percentage, 64.0f * percentage );

		offset = forward * dist;

		dist = RemapValClamped( dist, 4.0f * percentage, 64.0f * percentage, 6.0f, 1.0f );
		offset += right * random->RandomFloat( -4.0f * dist, 4.0f * dist );
		offset += up * random->RandomFloat( -4.0f * dist, 4.0f * dist );

		offset += GetAbsOrigin();

		sParticle = (SimpleParticle *) m_pAttractorEmitter->AddParticle( sizeof(SimpleParticle), m_pAttractorEmitter->GetPMaterial( "effects/strider_muzzle" ), offset );

		if ( sParticle == NULL )
			return;
		
		sParticle->m_vecVelocity	= Vector(0,0,8);
		sParticle->m_flDieTime		= 0.5f;
		sParticle->m_flLifetime		= 0.0f;

		sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= 0.0f;

		float alpha = 255 * percentage;

		sParticle->m_uchColor[0]	= alpha;
		sParticle->m_uchColor[1]	= alpha;
		sParticle->m_uchColor[2]	= alpha;
		sParticle->m_uchStartAlpha	= alpha;
		sParticle->m_uchEndAlpha	= 0;

		sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 );
		sParticle->m_uchEndSize		= 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : percentage - 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::UpdateSmallFX( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : percentage - 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::UpdateLargeFX( void )
{
	// Emitters must be valid
	if ( SetupEmitters() == false )
		return;

	// Reset our sort origin
	m_pSimpleEmitter->SetSortOrigin( GetAbsOrigin() );

	float flLargeScale = 8.0f * m_flLargeScale;

	Vector forward, right, up;
	AngleVectors( GetAbsAngles(), &forward, &right, &up );

	SimpleParticle *sParticle;

	// Base of the core effect
	sParticle = (SimpleParticle *) m_pSimpleEmitter->AddParticle( sizeof(SimpleParticle), m_pSimpleEmitter->GetPMaterial( "effects/strider_muzzle" ), GetAbsOrigin() );

	if ( sParticle == NULL )
		return;
	
	sParticle->m_vecVelocity	= forward * 32.0f;
	sParticle->m_flDieTime		= 0.2f;
	sParticle->m_flLifetime		= 0.0f;

	sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
	sParticle->m_flRollDelta	= 0.0f;

	float alpha = 128;

	sParticle->m_uchColor[0]	= alpha;
	sParticle->m_uchColor[1]	= alpha;
	sParticle->m_uchColor[2]	= alpha;
	sParticle->m_uchStartAlpha	= alpha;
	sParticle->m_uchEndAlpha	= 0;

	sParticle->m_uchStartSize	= flLargeScale * 2.0f;
	sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;

	// Make sure we encompass the complete particle here!
	m_pSimpleEmitter->SetParticleCullRadius( sParticle->m_uchEndSize );

	// Do the core effects
	for ( int i = 0; i < 2; i++ )
	{
		sParticle = (SimpleParticle *) m_pSimpleEmitter->AddParticle( sizeof(SimpleParticle), m_pSimpleEmitter->GetPMaterial( "effects/combinemuzzle2" ), GetAbsOrigin() );

		if ( sParticle == NULL )
			return;
		
		sParticle->m_vecVelocity	= forward * ( 32.0f * (i+1) );
		sParticle->m_flDieTime		= 0.2f;
		sParticle->m_flLifetime		= 0.0f;

		sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= 0.0f;

		float alpha = 100;

		sParticle->m_uchColor[0]	= alpha;
		sParticle->m_uchColor[1]	= alpha;
		sParticle->m_uchColor[2]	= alpha;
		sParticle->m_uchStartAlpha	= alpha;
		sParticle->m_uchEndAlpha	= 0;

		if ( i < 1 )
		{
			sParticle->m_uchStartSize	= flLargeScale * (i+1);
			sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;
		}
		else
		{
			if ( random->RandomInt( 0, 20 ) == 0 )
			{
				sParticle->m_uchStartSize	= flLargeScale * (i+1);
				sParticle->m_uchEndSize		= 0.0f;
				sParticle->m_flDieTime		= 0.25f;
			}
			else
			{
				sParticle->m_uchStartSize	= flLargeScale * (i+1);
				sParticle->m_uchEndSize		= 0.0f;
			}
		}
	}

	// Only do these particles if required
	//if ( m_spawnflags & SF_ENERGYCORE_NO_PARTICLES )
	//	return;

	// Do the charging particles
	m_pAttractorEmitter->SetAttractorOrigin( GetAbsOrigin() );

	Vector	offset;
	float	dist;

	for ( int i = 0; i < 4; i++ )
	{
		dist = random->RandomFloat( 4.0f * m_flLargeScale, 64.0f * m_flLargeScale );

		offset = forward * dist;

		dist = RemapValClamped( dist, 4.0f * m_flLargeScale, 64.0f * m_flLargeScale, 6.0f, 1.0f );
		offset += right * random->RandomFloat( -2.0f * dist * m_flLargeScale, 2.0f * dist * m_flLargeScale );
		offset += up * random->RandomFloat( -2.0f * dist * m_flLargeScale, 2.0f * dist * m_flLargeScale );

		offset += GetAbsOrigin();

		sParticle = (SimpleParticle *) m_pAttractorEmitter->AddParticle( sizeof(SimpleParticle), m_pAttractorEmitter->GetPMaterial( "effects/combinemuzzle2_dark" ), offset );

		if ( sParticle == NULL )
			return;
		
		sParticle->m_vecVelocity	= Vector(0,0,2);
		sParticle->m_flDieTime		= 0.5f;
		sParticle->m_flLifetime		= 0.0f;

		sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= 0.0f;

		float alpha = 255;

		sParticle->m_uchColor[0]	= alpha;
		sParticle->m_uchColor[1]	= alpha;
		sParticle->m_uchColor[2]	= alpha;
		sParticle->m_uchStartAlpha	= alpha;
		sParticle->m_uchEndAlpha	= 0;

		sParticle->m_uchStartSize	= 1;
		sParticle->m_uchEndSize		= 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : inline float
//-----------------------------------------------------------------------------
inline float C_Env_Lightrail_Endpoint::GetStateDurationPercentage( void )
{
	if ( m_flDuration == 0 )
		return 0.0f;

	return RemapValClamped( ( gpGlobals->curtime - m_flStartTime ), 0, m_flDuration, 0, 1.0f );;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit( state );

	// Turn off
	if ( state == SHOULDTRANSMIT_END )
	{
		SetNextClientThink( CLIENT_THINK_NEVER );
	}

	// Turn on
	if ( state == SHOULDTRANSMIT_START )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Env_Lightrail_Endpoint::ClientThink( void )
{
	if ( gpGlobals->frametime <= 0.0f )
		return;

	float flDuration = GetStateDurationPercentage();

	switch( m_nState )
	{
	case ENDPOINT_STATE_OFF:
		UpdateIdle( 1.0f - flDuration );
		break;

	case ENDPOINT_STATE_CHARGING:
		UpdateCharging( flDuration );
		break;

	case ENDPOINT_STATE_SMALLFX:
		UpdateSmallFX( );
		break;

	case ENDPOINT_STATE_LARGEFX:
		UpdateLargeFX( );
		break;
	}
}
