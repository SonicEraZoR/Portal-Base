//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "weapon_portalbasecombatweapon.h"
#include "fx.h"
#include "particles_localspace.h"
#include "view.h"
#include "particles_attractor.h"

#include "vcollide_parse.h"
#include "engine/ivdebugoverlay.h"
#include "iviewrender_beams.h"
#include "beamdraw.h"
#include "c_te_effect_dispatch.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#include "fx_interpvalue.h"
#include "c_weapon_portalgun.h"

#define PHYSCANNON_BEAM_SPRITE "sprites/orangelight1.vmt"
#define PHYSCANNON_BEAM_PORTAL_SPRITE "sprites/orangelight1_portal.vmt"  //mygamepedia: fixed sprite without $ignorez
#define PHYSCANNON_GLOW_SPRITE "sprites/glow04_noz.vmt"
#define PHYSCANNON_ENDCAP_SPRITE "sprites/orangeflare1.vmt"
#define PHYSCANNON_CENTER_GLOW "sprites/orangecore1.vmt"
#define PHYSCANNON_BLAST_SPRITE "sprites/orangecore2.vmt"

#define MEGACANNON_BEAM_SPRITE "sprites/lgtning_noz.vmt"
#define MEGACANNON_BEAM_PORTAL_SPRITE "sprites/lgtning_portal.vmt" //mygamepedia: fixed sprite without $ignorez
#define MEGACANNON_GLOW_SPRITE "sprites/blueflare1_noz.vmt"
#define MEGACANNON_ENDCAP_SPRITE "sprites/blueflare1_noz.vmt"
#define MEGACANNON_CENTER_GLOW "effects/fluttercore.vmt"
#define MEGACANNON_BLAST_SPRITE "effects/fluttercore.vmt"

#define MEGACANNON_RAGDOLL_BOOGIE_SPRITE "sprites/lgtning_noz.vmt"

#define	MEGACANNON_MODEL "models/weapons/v_superphyscannon.mdl"
#define	MEGACANNON_SKIN	1

#define	SPRITE_SCALE	128.0f

enum
{
	ELEMENT_STATE_NONE = -1,
	ELEMENT_STATE_OPEN,
	ELEMENT_STATE_CLOSED,
};

enum
{
	EFFECT_NONE,
	EFFECT_CLOSED,
	EFFECT_READY,
	EFFECT_HOLDING,
	EFFECT_LAUNCH,
};

enum EffectType_t
{
	PHYSCANNON_CORE = 0,

	PHYSCANNON_BLAST,

	PHYSCANNON_GLOW1,	// Must be in order!
	PHYSCANNON_GLOW2,
	PHYSCANNON_GLOW3,
	PHYSCANNON_GLOW4,
	PHYSCANNON_GLOW5,
	PHYSCANNON_GLOW6,

	PHYSCANNON_ENDCAP1,	// Must be in order!
	PHYSCANNON_ENDCAP2,
	PHYSCANNON_ENDCAP3,	// Only used in third-person!

	NUM_PHYSCANNON_PARAMETERS	// Must be last!
};

#define	NUM_GLOW_PHYSCANNON_SPRITES ((PHYSCANNON_GLOW6-PHYSCANNON_GLOW1)+1)
#define NUM_ENDCAP_PHYSCANNON_SPRITES ((PHYSCANNON_ENDCAP3-PHYSCANNON_ENDCAP1)+1)

#define	NUM_PHYSCANNON_BEAMS	3

extern void FX_GaussExplosion(const Vector& pos, const Vector& dir, int type);
extern void FormatViewModelAttachment(Vector& vOrigin, bool bInverse);

class C_WeaponPhysCannon: public CBasePortalCombatWeapon
{
	DECLARE_CLASS( C_WeaponPhysCannon, CBasePortalCombatWeapon );
public:
	C_WeaponPhysCannon( void );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	virtual int		DrawModel( int flags );
	virtual void	OnDataChanged(DataUpdateType_t updateType);

	void			StartEffects();
	float			SpriteScaleFactor();
	void			DoEffect(int effectType, Vector* pos = NULL, CBaseEntity* pEntity = NULL);
	void			DoMegaEffect(int effectType, Vector* pos = NULL, CBaseEntity* pEntity = NULL) {}
	void			GetEffectParameters(EffectType_t effectID, color32& color, float& scale, IMaterial** pMaterial, Vector& vecAttachment);
	bool			IsEffectVisible(EffectType_t effectID);
	void			DrawEffectSprite(EffectType_t effectID);
	void			DrawEffects();

	// Physgun effects
	void			DoEffectClosed(void);
	void			DoMegaEffectClosed(void);

	void			DoEffectReady(void);
	void			DoMegaEffectReady(void);

	void			DoMegaEffectHolding(void);
	void			DoEffectHolding(void);

	void			DoMegaEffectLaunch(Vector* pos, CBaseEntity* pPuntEntity = NULL);
	void			DoEffectLaunch(Vector* pos, CBaseEntity* pPuntEntity = NULL);

	void			DoEffectNone(void);
	void			DoEffectIdle(void);

	// Do we have the super-phys gun?
	inline bool	IsMegaPhysCannon()
	{
		return (g_pGameRules->MegaPhyscannonActive() == true);
	}

private:

	bool	SetupEmitter( void );

	bool	m_bIsCurrentlyUpgrading;
	bool	m_bWasUpgraded;

	CNetworkVar(int, m_EffectState);		// Current state of the effects on the gun

	CSmartPtr<CLocalSpaceEmitter>	m_pLocalEmitter;
	CSmartPtr<CSimpleEmitter>		m_pEmitter;
	CSmartPtr<CParticleAttractor>	m_pAttractor;

protected:

	CPortalgunEffect		m_Parameters[NUM_PHYSCANNON_PARAMETERS];	// Interpolated parameters for the effects
	CPortalgunEffectBeam	m_Beams[NUM_PHYSCANNON_BEAMS];				// Beams

	int				m_nOldEffectState;	// Used for parity checks
};

IMPLEMENT_CLIENTCLASS_DT(C_WeaponPhysCannon, DT_WeaponPhysCannon, CWeaponPhysCannon)
	RecvPropBool(RECVINFO(m_bIsCurrentlyUpgrading)),
	RecvPropInt(RECVINFO(m_EffectState)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_WeaponPhysCannon)
	DEFINE_PRED_FIELD(m_EffectState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_physcannon, C_WeaponPhysCannon);
PRECACHE_WEAPON_REGISTER(weapon_physcannon);

//Precahce the effects
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectPhysCannon)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_BEAM_SPRITE)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_BEAM_PORTAL_SPRITE)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_GLOW_SPRITE)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_ENDCAP_SPRITE)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_CENTER_GLOW)
	CLIENTEFFECT_MATERIAL(PHYSCANNON_BLAST_SPRITE)

	CLIENTEFFECT_MATERIAL(MEGACANNON_BEAM_SPRITE)
	CLIENTEFFECT_MATERIAL(MEGACANNON_BEAM_PORTAL_SPRITE)
	CLIENTEFFECT_MATERIAL(MEGACANNON_GLOW_SPRITE)
	CLIENTEFFECT_MATERIAL(MEGACANNON_ENDCAP_SPRITE)
	CLIENTEFFECT_MATERIAL(MEGACANNON_CENTER_GLOW)
	CLIENTEFFECT_MATERIAL(MEGACANNON_BLAST_SPRITE)

	CLIENTEFFECT_MATERIAL(MEGACANNON_RAGDOLL_BOOGIE_SPRITE)
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_WeaponPhysCannon::C_WeaponPhysCannon( void )
{
	m_bWasUpgraded = false;
	m_EffectState = (int)EFFECT_NONE;
	m_nOldEffectState = EFFECT_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Update effects when needed - MyGamepedia
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		// Start thinking (Baseclass stops it)
		SetNextClientThink(CLIENT_THINK_ALWAYS);

		{
			C_BaseAnimating::AutoAllowBoneAccess boneaccess(true, true);
			StartEffects();
		}

		DoEffect(m_EffectState);
	}

	// Update effect state when out of parity with the server
	else if (m_nOldEffectState != m_EffectState)
	{
		DoEffect(m_EffectState);
		m_nOldEffectState = m_EffectState;
	}
}

void C_WeaponPhysCannon::DoEffect(int effectType, Vector* pos, CBaseEntity* pEntity)
{
	// Make sure we're active
	StartEffects();

	m_EffectState = effectType;

	// Save predicted state
	m_nOldEffectState = m_EffectState;

	// Do different effects when upgraded
	if (IsMegaPhysCannon())
	{
		DoMegaEffect(effectType, pos, pEntity);
		return;
	}

	switch (effectType)
	{
	case EFFECT_CLOSED:
		DoEffectClosed();
		break;

	case EFFECT_READY:
		DoEffectReady();
		break;

	case EFFECT_HOLDING:
		DoEffectHolding();
		break;

	case EFFECT_LAUNCH:
		DoEffectNone();
		break;

	default:
	case EFFECT_NONE:
		DoEffectNone();
		break;
	}
}

//-----------------------------------------------------------------------------
// Sprite scale factor 
//-----------------------------------------------------------------------------
inline float C_WeaponPhysCannon::SpriteScaleFactor()
{
	return IsMegaPhysCannon() ? 1.5f : 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::StartEffects()
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	//todo: add a proper logic for NPCs when we will have working code for their portalgun
	if (pOwner == NULL)
		return;

	//check for active weapon to fix glitchy effects on gun on restore/levelchange when a different weapon is active
	CBaseEntity* pModelView = ((pOwner->GetActiveWeapon() == this) ? (pOwner->GetViewModel()) : (0));

	CBaseEntity* pModelWorld = this;

	if (!pModelView)
	{
		pModelView = pModelWorld;
	}

	//float flScaleFactor = SpriteScaleFactor();

	// ------------------------------------------
	// Core
	// ------------------------------------------

	if (m_Parameters[PHYSCANNON_CORE].GetMaterial() == NULL)
	{
		m_Parameters[PHYSCANNON_CORE].GetScale().Init(0.0f, 1.0f, 0.1f);
		m_Parameters[PHYSCANNON_CORE].GetAlpha().Init(255.0f, 255.0f, 0.1f);
		m_Parameters[PHYSCANNON_CORE].SetAttachment(1);

		if (m_Parameters[PHYSCANNON_CORE].SetMaterial(PHYSCANNON_CENTER_GLOW) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// Blast
	// ------------------------------------------

	if (m_Parameters[PHYSCANNON_BLAST].GetMaterial() == NULL)
	{
		m_Parameters[PHYSCANNON_BLAST].GetScale().Init(0.0f, 1.0f, 0.1f);
		m_Parameters[PHYSCANNON_BLAST].GetAlpha().Init(255.0f, 255.0f, 0.1f);
		m_Parameters[PHYSCANNON_BLAST].SetAttachment(1);
		m_Parameters[PHYSCANNON_BLAST].SetVisible(false);

		if (m_Parameters[PHYSCANNON_BLAST].SetMaterial(PHYSCANNON_BLAST_SPRITE) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// Glows
	// ------------------------------------------

	const char* attachNamesGlowThirdPerson[NUM_GLOW_PHYSCANNON_SPRITES] =
	{
		"fork1m",
		"fork1t",
		"fork2m",
		"fork2t",
		"fork3m",
		"fork3t",
	};

	const char* attachNamesGlow[NUM_GLOW_PHYSCANNON_SPRITES] =
	{
		"fork1b",
		"fork1m",
		"fork1t",
		"fork2b",
		"fork2m",
		"fork2t"
	};

	//Create the glow sprites
	for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
	{
		if (m_Parameters[i].GetMaterial() != NULL)
		{
			//reattach the sprite in case to fix misplaced sprites on vm when weapon picked up
			if (ShouldDrawUsingViewModel())
			{
				m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesGlow[i - PHYSCANNON_GLOW1]));
			}
			else
			{
				m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesGlowThirdPerson[i - PHYSCANNON_GLOW1]));
			}

			continue;
		}

		m_Parameters[i].GetScale().SetAbsolute(0.05f * SPRITE_SCALE);
		m_Parameters[i].GetAlpha().SetAbsolute(64.0f);

		// Different for different views
		if (ShouldDrawUsingViewModel())
		{
			m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesGlow[i - PHYSCANNON_GLOW1]));
		}
		else
		{
			m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesGlowThirdPerson[i - PHYSCANNON_GLOW1]));
		}
		m_Parameters[i].SetColor(Vector(255, 128, 0));

		if (m_Parameters[i].SetMaterial(PHYSCANNON_GLOW_SPRITE) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// End caps
	// ------------------------------------------

	const char* attachNamesEndCap[NUM_ENDCAP_PHYSCANNON_SPRITES] =
	{
		"fork1t",
		"fork2t",
		"fork3t"
	};

	//Create the glow sprites
	for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
	{
		//reattach the sprite in case to fix misplaced sprites on vm when weapon picked ups
		if (m_Parameters[i].GetMaterial() != NULL)
		{
			m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesEndCap[i - PHYSCANNON_ENDCAP1]));
			continue;
		}

		m_Parameters[i].GetScale().SetAbsolute(0.05f * SPRITE_SCALE);
		m_Parameters[i].GetAlpha().SetAbsolute(255.0f);
		m_Parameters[i].SetAttachment(pModelView->LookupAttachment(attachNamesEndCap[i - PHYSCANNON_ENDCAP1]));
		m_Parameters[i].SetVisible(false);

		if (m_Parameters[i].SetMaterial(PHYSCANNON_ENDCAP_SPRITE) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Closing effects
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DoEffectClosed(void)
{

#ifdef CLIENT_DLL

	// Turn off the end-caps
	for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
	{
		m_Parameters[i].SetVisible(false);
	}

#endif

}

//-----------------------------------------------------------------------------
// Purpose: Ready effects
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DoEffectReady(void)
{

#ifdef CLIENT_DLL

	// Special POV case
	if (ShouldDrawUsingViewModel())
	{
		//Turn on the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent(14.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent(128.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}
	else
	{
		//Turn off the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent(8.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent(0.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}

	// Turn on the glow sprites
	for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
	{
		m_Parameters[i].GetScale().InitFromCurrent(0.4f * SPRITE_SCALE, 0.2f);
		m_Parameters[i].GetAlpha().InitFromCurrent(64.0f, 0.2f);
		m_Parameters[i].SetVisible();
	}

	// Turn on the glow sprites
	for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
	{
		m_Parameters[i].SetVisible(false);
	}
#endif

}


//-----------------------------------------------------------------------------
// Holding effects
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DoEffectHolding(void)
{

#ifdef CLIENT_DLL

	if (ShouldDrawUsingViewModel())
	{
		// Scale up the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent(16.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent(255.0f, 0.1f);
		m_Parameters[PHYSCANNON_CORE].SetVisible();

		// Prepare for scale up
		m_Parameters[PHYSCANNON_BLAST].SetVisible(false);

		// Turn on the glow sprites
		for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
		{
			m_Parameters[i].GetScale().InitFromCurrent(0.5f * SPRITE_SCALE, 0.2f);
			m_Parameters[i].GetAlpha().InitFromCurrent(64.0f, 0.2f);
			m_Parameters[i].SetVisible();
		}

		// Turn on the glow sprites
		// NOTE: The last glow is left off for first-person
		for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES - 1); i++)
		{
			m_Parameters[i].SetVisible();
		}

		// Create our beams
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		CBaseEntity* pBeamEnt = pOwner->GetViewModel();

		// Setup the beams
		m_Beams[0].Init(LookupAttachment("fork1t"), 1, pBeamEnt, true);
		m_Beams[1].Init(LookupAttachment("fork2t"), 1, pBeamEnt, true);

		// Set them visible
		m_Beams[0].SetBrightness(255);
		m_Beams[1].SetBrightness(255);
	}
	else
	{
		// Scale up the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent(14.0f, 0.2f);
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent(255.0f, 0.1f);
		m_Parameters[PHYSCANNON_CORE].SetVisible();

		// Prepare for scale up
		m_Parameters[PHYSCANNON_BLAST].SetVisible(false);

		// Turn on the glow sprites
		for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
		{
			m_Parameters[i].GetScale().InitFromCurrent(0.5f * SPRITE_SCALE, 0.2f);
			m_Parameters[i].GetAlpha().InitFromCurrent(64.0f, 0.2f);
			m_Parameters[i].SetVisible();
		}

		// Turn on the glow sprites
		for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
		{
			m_Parameters[i].SetVisible();
		}

		// Setup the beams
		m_Beams[0].Init(LookupAttachment("fork1t"), 1, this, false);
		m_Beams[1].Init(LookupAttachment("fork2t"), 1, this, false);
		m_Beams[2].Init(LookupAttachment("fork3t"), 1, this, false);

		// Set them visible
		m_Beams[0].SetBrightness(255);
		m_Beams[1].SetBrightness(255);
		m_Beams[2].SetBrightness(255);
	}

#endif

}

//-----------------------------------------------------------------------------
// Purpose: Shutdown for the weapon when it's holstered
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DoEffectNone(void)
{
#ifdef CLIENT_DLL

	//Turn off main glows
	m_Parameters[PHYSCANNON_CORE].SetVisible(false);
	m_Parameters[PHYSCANNON_BLAST].SetVisible(false);

	for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
	{
		m_Parameters[i].SetVisible(false);
	}

	// Turn on the glow sprites
	for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
	{
		m_Parameters[i].SetVisible(false);
	}

	m_Beams[0].SetBrightness(0);
	m_Beams[1].SetBrightness(0);
	m_Beams[2].SetBrightness(0);
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_WeaponPhysCannon::SetupEmitter( void )
{
	if ( !m_pLocalEmitter.IsValid() )
	{
		m_pLocalEmitter = CLocalSpaceEmitter::Create( "physpowerup", GetRefEHandle(), LookupAttachment( "core" ) );

		if ( m_pLocalEmitter.IsValid() == false )
			return false;
	}

	if ( !m_pAttractor.IsValid() )
	{
		m_pAttractor = CParticleAttractor::Create( vec3_origin, "physpowerup_att" );

		if ( m_pAttractor.IsValid() == false )
			return false;
	}

	if ( !m_pEmitter.IsValid() )
	{
		m_pEmitter = CSimpleEmitter::Create( "physpowerup_glow" );

		if ( m_pEmitter.IsValid() == false )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Sorts the components of a vector
//-----------------------------------------------------------------------------
static inline void SortAbsVectorComponents( const Vector& src, int* pVecIdx )
{
	Vector absVec( fabs(src[0]), fabs(src[1]), fabs(src[2]) );

	int maxIdx = (absVec[0] > absVec[1]) ? 0 : 1;
	if (absVec[2] > absVec[maxIdx])
	{
		maxIdx = 2;
	}

	// always choose something right-handed....
	switch(	maxIdx )
	{
	case 0:
		pVecIdx[0] = 1;
		pVecIdx[1] = 2;
		pVecIdx[2] = 0;
		break;
	case 1:
		pVecIdx[0] = 2;
		pVecIdx[1] = 0;
		pVecIdx[2] = 1;
		break;
	case 2:
		pVecIdx[0] = 0;
		pVecIdx[1] = 1;
		pVecIdx[2] = 2;
		break;
	}
}

//-----------------------------------------------------------------------------
// Compute the bounding box's center, size, and basis
//-----------------------------------------------------------------------------
void ComputeRenderInfo( mstudiobbox_t *pHitBox, const matrix3x4_t &hitboxToWorld, 
										 Vector *pVecAbsOrigin, Vector *pXVec, Vector *pYVec )
{
	// Compute the center of the hitbox in worldspace
	Vector vecHitboxCenter;
	VectorAdd( pHitBox->bbmin, pHitBox->bbmax, vecHitboxCenter );
	vecHitboxCenter *= 0.5f;
	VectorTransform( vecHitboxCenter, hitboxToWorld, *pVecAbsOrigin );

	// Get the object's basis
	Vector vec[3];
	MatrixGetColumn( hitboxToWorld, 0, vec[0] );
	MatrixGetColumn( hitboxToWorld, 1, vec[1] );
	MatrixGetColumn( hitboxToWorld, 2, vec[2] );
//	vec[1] *= -1.0f;

	Vector vecViewDir;
	VectorSubtract( CurrentViewOrigin(), *pVecAbsOrigin, vecViewDir );
	VectorNormalize( vecViewDir );

	// Project the shadow casting direction into the space of the hitbox
	Vector localViewDir;
	localViewDir[0] = DotProduct( vec[0], vecViewDir );
	localViewDir[1] = DotProduct( vec[1], vecViewDir );
	localViewDir[2] = DotProduct( vec[2], vecViewDir );

	// Figure out which vector has the largest component perpendicular
	// to the view direction...
	// Sort by how perpendicular it is
	int vecIdx[3];
	SortAbsVectorComponents( localViewDir, vecIdx );

	// Here's our hitbox basis vectors; namely the ones that are
	// most perpendicular to the view direction
	*pXVec = vec[vecIdx[0]];
	*pYVec = vec[vecIdx[1]];

	// Project them into a plane perpendicular to the view direction
	*pXVec -= vecViewDir * DotProduct( vecViewDir, *pXVec );
	*pYVec -= vecViewDir * DotProduct( vecViewDir, *pYVec );
	VectorNormalize( *pXVec );
	VectorNormalize( *pYVec );

	// Compute the hitbox size
	Vector boxSize;
	VectorSubtract( pHitBox->bbmax, pHitBox->bbmin, boxSize );

	// We project the two longest sides into the vectors perpendicular
	// to the projection direction, then add in the projection of the perp direction
	Vector2D size( boxSize[vecIdx[0]], boxSize[vecIdx[1]] );
	size.x *= fabs( DotProduct( vec[vecIdx[0]], *pXVec ) );
	size.y *= fabs( DotProduct( vec[vecIdx[1]], *pYVec ) );

	// Add the third component into x and y
	size.x += boxSize[vecIdx[2]] * fabs( DotProduct( vec[vecIdx[2]], *pXVec ) );
	size.y += boxSize[vecIdx[2]] * fabs( DotProduct( vec[vecIdx[2]], *pYVec ) );

	// Bloat a bit, since the shadow wants to extend outside the model a bit
	size *= 2.0f;

	// Clamp the minimum size
	Vector2DMax( size, Vector2D(10.0f, 10.0f), size );

	// Factor the size into the xvec + yvec
	(*pXVec) *= size.x * 0.5f;
	(*pYVec) *= size.y * 0.5f;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the complete list of values needed to render an effect from an
//			effect parameter
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::GetEffectParameters(EffectType_t effectID, color32& color, float& scale, IMaterial** pMaterial, Vector& vecAttachment)
{
	const float dt = gpGlobals->curtime;

	// Get alpha
	float alpha = m_Parameters[effectID].GetAlpha().Interp(dt);

	// Get scale
	scale = m_Parameters[effectID].GetScale().Interp(dt);

	// Get material
	*pMaterial = (IMaterial*)m_Parameters[effectID].GetMaterial();

	// Setup the color
	color.r = (int)m_Parameters[effectID].GetColor().x;
	color.g = (int)m_Parameters[effectID].GetColor().y;
	color.b = (int)m_Parameters[effectID].GetColor().z;
	color.a = (int)alpha;

	// Setup the attachment
	int		attachment = m_Parameters[effectID].GetAttachment();
	QAngle	angles;

	// Format for first-person
	if (ShouldDrawUsingViewModel())
	{
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());

		if (pOwner != NULL)
		{
			pOwner->GetViewModel()->GetAttachment(attachment, vecAttachment, angles);
			::FormatViewModelAttachment(vecAttachment, true);
		}
	}
	else
	{
		GetAttachment(attachment, vecAttachment, angles);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not an effect is set to display
//-----------------------------------------------------------------------------
bool C_WeaponPhysCannon::IsEffectVisible(EffectType_t effectID)
{
	float flAlpha = m_Parameters[effectID].GetAlpha().Interp(gpGlobals->curtime);
	return (flAlpha > 0) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: Draws the effect sprite, given an effect parameter ID
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DrawEffectSprite(EffectType_t effectID)
{
	color32 color;
	float scale;
	IMaterial* pMaterial;
	Vector	vecAttachment;

	// Don't draw invisible effects
	if (IsEffectVisible(effectID) == false)
		return;

	// Get all of our parameters
	GetEffectParameters(effectID, color, scale, &pMaterial, vecAttachment);

	// Msg( "Scale: %.2f\tAlpha: %.2f\n", scale, alpha );

	// Don't render fully translucent objects
	if (color.a <= 0.0f)
		return;

	// Draw the sprite
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(pMaterial, this);
	DrawSprite(vecAttachment, scale, scale, color);
}

//-----------------------------------------------------------------------------
// Purpose: Render our third-person effects
//-----------------------------------------------------------------------------
void C_WeaponPhysCannon::DrawEffects(void)
{
	// Draw the core effects
	DrawEffectSprite(PHYSCANNON_CORE);
	DrawEffectSprite(PHYSCANNON_BLAST);

	// Draw the glows
	for (int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1 + NUM_GLOW_PHYSCANNON_SPRITES); i++)
	{
		DrawEffectSprite((EffectType_t)i);
	}

	// Draw the endcaps
	for (int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1 + NUM_ENDCAP_PHYSCANNON_SPRITES); i++)
	{
		DrawEffectSprite((EffectType_t)i);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_WeaponPhysCannon::DrawModel( int flags )
{
	// If we're not ugrading, don't do anything special
	if ( m_bIsCurrentlyUpgrading == false && m_bWasUpgraded == false )
		return BaseClass::DrawModel( flags );

	if ( gpGlobals->frametime == 0 )
		return BaseClass::DrawModel( flags );

	if ( !m_bReadyToDraw )
		return 0;

	m_bWasUpgraded = true;

	// Create the particle emitter if it's not already
	if ( SetupEmitter() )
	{
		// Add the power-up particles

		// See if we should draw
		if ( m_bReadyToDraw == false )
			return 0;

		C_BaseAnimating *pAnimating = GetBaseAnimating();
		if (!pAnimating)
			return 0;

		matrix3x4_t	*hitboxbones[MAXSTUDIOBONES];
		if ( !pAnimating->HitboxToWorldTransforms( hitboxbones ) )
			return 0;

		studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( pAnimating->GetModel() );
		if (!pStudioHdr)
			return false;

		mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( pAnimating->GetHitboxSet() );
		if ( !set )
			return false;

		int i;

		float fadePerc = 1.0f;

		if ( m_bIsCurrentlyUpgrading )
		{
			Vector	vecSkew = vec3_origin;

			// Skew the particles in front or in back of their targets
			vecSkew = CurrentViewForward() * 4.0f;

			float spriteScale = 1.0f;
			spriteScale = clamp( spriteScale, 0.75f, 1.0f );

			SimpleParticle *sParticle;

			for ( i = 0; i < set->numhitboxes; ++i )
			{
				Vector vecAbsOrigin, xvec, yvec;
				mstudiobbox_t *pBox = set->pHitbox(i);
				ComputeRenderInfo( pBox, *hitboxbones[pBox->bone], &vecAbsOrigin, &xvec, &yvec );

				Vector offset;
				Vector	xDir, yDir;

				xDir = xvec;
				float xScale = VectorNormalize( xDir ) * 0.75f;

				yDir = yvec;
				float yScale = VectorNormalize( yDir ) * 0.75f;

				int numParticles = clamp( 4.0f * fadePerc, 1, 3 );

				for ( int j = 0; j < numParticles; j++ )
				{
					offset = xDir * Helper_RandomFloat( -xScale*0.5f, xScale*0.5f ) + yDir * Helper_RandomFloat( -yScale*0.5f, yScale*0.5f );
					offset += vecSkew;

					sParticle = (SimpleParticle *) m_pEmitter->AddParticle( sizeof(SimpleParticle), m_pEmitter->GetPMaterial( "effects/combinemuzzle1" ), vecAbsOrigin + offset );

					if ( sParticle == NULL )
						return 1;
					
					sParticle->m_vecVelocity	= vec3_origin;
					sParticle->m_uchStartSize	= 16.0f * spriteScale;
					sParticle->m_flDieTime		= 0.2f;
					sParticle->m_flLifetime		= 0.0f;

					sParticle->m_flRoll			= Helper_RandomInt( 0, 360 );
					sParticle->m_flRollDelta	= Helper_RandomFloat( -2.0f, 2.0f );

					float alpha = 40;

					sParticle->m_uchColor[0]	= alpha;
					sParticle->m_uchColor[1]	= alpha;
					sParticle->m_uchColor[2]	= alpha;
					sParticle->m_uchStartAlpha	= alpha;
					sParticle->m_uchEndAlpha	= 0;
					sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
				}
			}
		}
	}

	int		attachment = LookupAttachment( "core" );
	Vector	coreOrigin;
	QAngle	coreAngles;

	GetAttachment( attachment, coreOrigin, coreAngles );

	SimpleParticle *sParticle;

	// Do the core effects
	for ( int i = 0; i < 4; i++ )
	{
		sParticle = (SimpleParticle *) m_pLocalEmitter->AddParticle( sizeof(SimpleParticle), m_pLocalEmitter->GetPMaterial( "effects/strider_muzzle" ), vec3_origin );

		if ( sParticle == NULL )
			return 1;
		
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
			sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 ) * (i+1);
			sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;
		}
		else
		{
			if ( random->RandomInt( 0, 20 ) == 0 )
			{
				sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 ) * (i+1);
				sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 4.0f;
				sParticle->m_flDieTime		= 0.25f;
			}
			else
			{
				sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 ) * (i+1);
				sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2.0f;
			}
		}
	}

	if ( m_bWasUpgraded && m_bIsCurrentlyUpgrading )
	{
		// Update our attractor point
		m_pAttractor->SetAttractorOrigin( coreOrigin );

		Vector offset;

		for ( int i = 0; i < 4; i++ )
		{
			offset = coreOrigin + RandomVector( -32.0f, 32.0f );

			sParticle = (SimpleParticle *) m_pAttractor->AddParticle( sizeof(SimpleParticle), m_pAttractor->GetPMaterial( "effects/strider_muzzle" ), offset );

			if ( sParticle == NULL )
				return 1;
			
			sParticle->m_vecVelocity	= Vector(0,0,8);
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

			sParticle->m_uchStartSize	= random->RandomFloat( 1, 2 );
			sParticle->m_uchEndSize		= 0;
		}
	}

	return BaseClass::DrawModel( flags );
}

void CallbackPhyscannonImpact(const CEffectData& data)
{
	C_BaseEntity* pEnt = data.GetEntity();
	if (pEnt == NULL)
		return;

	Vector	vecAttachment;
	QAngle	vecAngles;

	C_BaseCombatWeapon* pWeapon = dynamic_cast<C_BaseCombatWeapon*>(pEnt);

	if (pWeapon == NULL)
		return;

	pWeapon->GetAttachment(1, vecAttachment, vecAngles);

	Vector	dir = (data.m_vOrigin - vecAttachment);
	VectorNormalize(dir);

	// Do special first-person fix-up
	if (pWeapon->GetOwner() == CBasePlayer::GetLocalPlayer())
	{
		// Translate the attachment entity to the viewmodel
		C_BasePlayer* pPlayer = dynamic_cast<C_BasePlayer*>(pWeapon->GetOwner());

		if (pPlayer)
		{
			pEnt = pPlayer->GetViewModel();
		}

		// Format attachment for first-person view!
		FormatViewModelAttachment(vecAttachment, true);

		// Explosions at the impact point
		if (data.m_nDamageType != 1)
			FX_GaussExplosion(data.m_vOrigin, -dir, 0);

		// Draw a beam
		BeamInfo_t beamInfo;

		beamInfo.m_pStartEnt = pEnt;
		beamInfo.m_nStartAttachment = 1;
		beamInfo.m_pEndEnt = NULL;
		beamInfo.m_nEndAttachment = -1;
		beamInfo.m_vecStart = vec3_origin;
		beamInfo.m_vecEnd = data.m_vOrigin;
		beamInfo.m_pszModelName = "sprites/orangelight1.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.1f;
		beamInfo.m_flWidth = 12.0f;
		beamInfo.m_flEndWidth = 4.0f;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 0;
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0f;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 30.0;
		beamInfo.m_flRed = 255.0;
		beamInfo.m_flGreen = 255.0;
		beamInfo.m_flBlue = 255.0;
		beamInfo.m_nSegments = 16;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE;

		beams->CreateBeamEntPoint(beamInfo);
	}
	else
	{
		// Explosion at the starting point
		if (data.m_nDamageType != 1)
			FX_GaussExplosion(vecAttachment, dir, 0);
	}
}

DECLARE_CLIENT_EFFECT("PhyscannonImpact", CallbackPhyscannonImpact);

void CallbackMegaPhyscannonImpact(const CEffectData& data)
{
	C_BaseEntity* pEnt = data.GetEntity();
	if (pEnt == NULL)
		return;

	Vector vecAttachment;
	QAngle vecAngles;

	C_BaseCombatWeapon* pWeapon = dynamic_cast<C_BaseCombatWeapon*>(pEnt);
	if (pWeapon == NULL)
		return;

	pWeapon->GetAttachment(1, vecAttachment, vecAngles);

	Vector dir = (data.m_vOrigin - vecAttachment);
	VectorNormalize(dir);

	if (pWeapon->GetOwner() == CBasePlayer::GetLocalPlayer())
	{
		C_BasePlayer* pPlayer = dynamic_cast<C_BasePlayer*>(pWeapon->GetOwner());
		if (pPlayer)
			pEnt = pPlayer->GetViewModel();

		FormatViewModelAttachment(vecAttachment, true);

		if (data.m_nDamageType != 1)
			FX_GaussExplosion(data.m_vOrigin, -dir, 0);

		// Primary straight beam (no noise)  
		BeamInfo_t beamInfo;
		beamInfo.m_pStartEnt = pEnt;
		beamInfo.m_nStartAttachment = 1;
		beamInfo.m_pEndEnt = NULL;
		beamInfo.m_nEndAttachment = -1;
		beamInfo.m_vecStart = vec3_origin;
		beamInfo.m_vecEnd = data.m_vOrigin;
		beamInfo.m_pszModelName = "sprites/lgtning_noz.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.1f;
		beamInfo.m_flWidth = 12.0f;   // wide at vm attachment  
		beamInfo.m_flEndWidth = 2.0f;    // narrow at impact  
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 0;       // straight, no noise  
		beamInfo.m_flBrightness = 255.0f;
		beamInfo.m_flSpeed = 0.0f;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 30.0f;
		beamInfo.m_flRed = 255.0f;
		beamInfo.m_flGreen = 255.0f;
		beamInfo.m_flBlue = 255.0f;
		beamInfo.m_nSegments = 16;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE;
		beams->CreateBeamEntPoint(beamInfo);

		// 1-2 extra noisy beams  
		int numBeams = random->RandomInt(1, 2);
		for (int i = 0; i < numBeams; i++)
		{
			BeamInfo_t extra;
			extra.m_pStartEnt = pEnt;
			extra.m_nStartAttachment = 1;
			extra.m_pEndEnt = NULL;
			extra.m_nEndAttachment = -1;
			extra.m_vecStart = vec3_origin;
			extra.m_vecEnd = data.m_vOrigin;
			extra.m_pszModelName = "sprites/lgtning_noz.vmt";
			extra.m_flHaloScale = 0.0f;
			extra.m_flLife = 0.1f;
			extra.m_flWidth = (float)random->RandomInt(1, 2); // at vm  
			extra.m_flEndWidth = 2.0f;                           // at impact  
			extra.m_flFadeLength = 0.0f;
			extra.m_flAmplitude = (float)random->RandomInt(8, 12);
			extra.m_flBrightness = 255.0f;
			extra.m_flSpeed = 0.0f;
			extra.m_nStartFrame = 0;
			extra.m_flFrameRate = 30.0f;
			extra.m_flRed = 255.0f;
			extra.m_flGreen = 255.0f;
			extra.m_flBlue = 255.0f;
			extra.m_nSegments = 16;
			extra.m_bRenderable = true;
			extra.m_nFlags = FBEAM_ONLYNOISEONCE;
			beams->CreateBeamEntPoint(extra);
		}
	}
	else
	{
		if (data.m_nDamageType != 1)
			FX_GaussExplosion(vecAttachment, dir, 0);
	}
}

DECLARE_CLIENT_EFFECT("MegaPhyscannonImpact", CallbackMegaPhyscannonImpact);