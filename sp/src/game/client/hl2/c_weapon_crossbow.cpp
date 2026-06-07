//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"
#include "c_basehlcombatweapon.h"
#include "c_weapon__stubs.h"
#include "c_weapon_portalgun.h"
#include "fx_interpvalue.h"  
#include "enginesprite.h"
#include "fx_sparks.h"

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()

//
// Crossbow bolt
//

class C_CrossbowBolt : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_CrossbowBolt, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_CrossbowBolt( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

private:

	C_CrossbowBolt( const C_CrossbowBolt & ); // not defined, not accessible

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_CrossbowBolt, DT_CrossbowBolt, CCrossbowBolt )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CrossbowBolt::C_CrossbowBolt( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_CrossbowBolt::DrawModel( int flags )
{
	// See if we're drawing the motion blur
	if ( flags & STUDIO_TRANSPARENCY )
	{
		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		//mygamepedia: added check for skin here cuz we can reenable motion for cooled down bolt
		if (speed > 0 && m_nSkin == 1)
		{
			float		color[3];
			IMaterial* pBlurMaterial = materials->FindMaterial("effects/muzzleflash1", NULL, false);

			float	stepSize = MIN( ( speed * 0.5f ), 4.0f );

			Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
			Vector	spawnStep = -vecDir * stepSize;

			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->Bind( pBlurMaterial );

			float	alpha;

			// Draw the motion blurred trail
			for ( int i = 0; i < 20; i++ )
			{
				spawnPos += spawnStep;

				alpha = RemapValClamped( i, 5, 11, 0.25f, 0.05f );

				color[0] = color[1] = color[2] = alpha;

				DrawHalo( pBlurMaterial, spawnPos, 3.0f, color );
			}
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}

	// Draw the normal portion
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::ClientThink( void )
{
	m_bUpdated = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CrosshairLoadCallback( const CEffectData &data )
{
	IClientRenderable *pRenderable = data.GetRenderable( );
	if ( !pRenderable )
		return;
	
	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if ( pRenderable->GetAttachment( data.m_nAttachmentIndex, position, angles ) )
	{
		FX_ElectricSpark( position, 1.0f, 1.0f, NULL );
	}
}

DECLARE_CLIENT_EFFECT( "CrossbowLoad", CrosshairLoadCallback );

/*MyGamepedia:
This is our client side code for crossnbow. We network int value state and execute logic depending on it.
If we will ever support mp, this code should be reworked, using prediction.
How is this working ? See the code, the main idea is to catch in which view we render sprite in AND if we should render
given sprite in this view, we use two, 3rd person (i call it "portal view"), and main (viewmodel).

I have to note that it looks a little bit different from server sprite version, it's a little brighter.
You have convars for settings, but I didn't managed to make 1:1 look.
*/

ConVar cl_portalbase_crossbow_sprite_scale(
	"cl_portalbase_crossbow_sprite_scale", "64.0",
	FCVAR_NONE,
	"World-unit multiplier for crossbow charger sprite (texture pixel size).");

ConVar cl_portalbase_crossbow_charge_alpha(
	"cl_portalbase_crossbow_charge_alpha", "32.0",
    FCVAR_NONE,
	"Alpha (0-255) of charger sprite in START_CHARGE state.");

ConVar cl_portalbase_crossbow_charge_scale(
	"cl_portalbase_crossbow_charge_scale", "0.025",
    FCVAR_NONE,
	"Scale of charger sprite in START_CHARGE state (before sprite_scale multiply).");

ConVar cl_portalbase_crossbow_charge_alpha_time(
	"cl_portalbase_crossbow_charge_alpha_time", "0.5",
    FCVAR_NONE,
	"Interpolation time for alpha in START_CHARGE state.");

ConVar cl_portalbase_crossbow_charge_scale_time(
	"cl_portalbase_crossbow_charge_scale_time", "0.5",
    FCVAR_NONE,
	"Interpolation time for scale in START_CHARGE state.");

ConVar cl_portalbase_crossbow_ready_alpha(
	"cl_portalbase_crossbow_ready_alpha", "80.0",
    FCVAR_NONE,
	"Alpha (0-255) of charger sprite in READY state.");

ConVar cl_portalbase_crossbow_ready_scale(
	"cl_portalbase_crossbow_ready_scale", "0.1",
    FCVAR_NONE,
	"Scale of charger sprite in READY state (before sprite_scale multiply).");

ConVar cl_portalbase_crossbow_ready_alpha_time(
	"cl_portalbase_crossbow_ready_alpha_time", "1.0",
    FCVAR_NONE,
	"Interpolation time for alpha in READY state.");

ConVar cl_portalbase_crossbow_ready_scale_time(
	"cl_portalbase_crossbow_ready_scale_time", "0.5",
    FCVAR_NONE,
	"Interpolation time for scale in READY state.");

extern void FormatViewModelAttachment(Vector& vOrigin, bool bInverse);

#define CROSSBOW_GLOW_SPRITE    "sprites/light_glow02_noz_crossbow_portalbase.vmt"
#define	CROSSBOW_GLOW_SPRITE2	"sprites/blueflare1_crossbow_portalbase.vmt"
#define BOLT_SPARK_ATTACHMENT   1
#define BOLT_TIP_ATTACHMENT     2

#define EVENT_WEAPON_THROW		3005

#define	SPARK_ELECTRIC_SPREAD	0.0f
#define	SPARK_ELECTRIC_MINSPEED	64.0f
#define	SPARK_ELECTRIC_MAXSPEED	300.0f
#define	SPARK_ELECTRIC_GRAVITY	800.0f
#define	SPARK_ELECTRIC_DAMPEN	0.3f

static PMaterialHandle g_Material_CrossbowSpark = NULL;

//MyGamepedia: FX_ElectricSpark version for main view
static void FX_ElectricSparkVM(const Vector& pos, int nMagnitude, int nTrailLength, const Vector* vecDir)
{
    CSmartPtr<CTrailParticles> pSparkEmitter = CTrailParticles::Create("FX_ElectricSparkVM 1");
    if (!pSparkEmitter) return;

    if (g_Material_CrossbowSpark == NULL)
        g_Material_CrossbowSpark = pSparkEmitter->GetPMaterial("effects/spark");

    pSparkEmitter->SetViewModelOnly(true);   // main view only  

    pSparkEmitter->Setup((Vector&)pos, NULL,
        SPARK_ELECTRIC_SPREAD, SPARK_ELECTRIC_MINSPEED, SPARK_ELECTRIC_MAXSPEED,
        SPARK_ELECTRIC_GRAVITY, SPARK_ELECTRIC_DAMPEN,
        bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);
    pSparkEmitter->SetSortOrigin(pos);

    Vector dir;
    int numSparks = nMagnitude * nMagnitude * random->RandomFloat(2, 4);
    for (int i = 0; i < numSparks; i++)
    {
        TrailParticle* pParticle = (TrailParticle*)pSparkEmitter->AddParticle(sizeof(TrailParticle), g_Material_CrossbowSpark, pos);
        if (!pParticle) return;
        pParticle->m_flLifetime = 0.0f;
        pParticle->m_flDieTime = nMagnitude * random->RandomFloat(1.0f, 2.0f);
        dir.Random(-1.0f, 1.0f);
        dir[2] = random->RandomFloat(0.5f, 1.0f);
        if (vecDir) { dir += 2 * (*vecDir); VectorNormalize(dir); }
        pParticle->m_flWidth = random->RandomFloat(2.0f, 5.0f);
        pParticle->m_flLength = nTrailLength * random->RandomFloat(0.02f, 0.05f);
        pParticle->m_vecVelocity = dir * random->RandomFloat(SPARK_ELECTRIC_MINSPEED, SPARK_ELECTRIC_MAXSPEED);
        Color32Init(pParticle->m_color, 255, 255, 255, 255);
    }

    CSmartPtr<CTrailParticles> pSparkEmitter2 = CTrailParticles::Create("FX_ElectricSparkVM 2");
    if (!pSparkEmitter2) return;

    pSparkEmitter2->SetViewModelOnly(true);  // main view only  

    pSparkEmitter2->SetSortOrigin(pos);
    pSparkEmitter2->m_ParticleCollision.SetGravity(400.0f);
    pSparkEmitter2->SetFlag(bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);

    numSparks = nMagnitude * random->RandomInt(16, 32);
    for (int i = 0; i < numSparks; i++)
    {
        TrailParticle* pParticle = (TrailParticle*)pSparkEmitter2->AddParticle(sizeof(TrailParticle), g_Material_CrossbowSpark, pos);
        if (!pParticle) return;
        pParticle->m_flLifetime = 0.0f;
        dir.Random(-1.0f, 1.0f);
        if (vecDir) { dir += *vecDir; VectorNormalize(dir); }
        pParticle->m_flWidth = random->RandomFloat(2.0f, 4.0f);
        pParticle->m_flLength = nTrailLength * random->RandomFloat(0.02f, 0.03f);
        pParticle->m_flDieTime = nMagnitude * random->RandomFloat(0.1f, 0.2f);
        pParticle->m_vecVelocity = dir * random->RandomFloat(128, 256);
        Color32Init(pParticle->m_color, 255, 255, 255, 255);
    }

    CSmartPtr<CSimpleGlowEmitter> pSimple = CSimpleGlowEmitter::Create("FX_ElectricSparkVM 3", pos, gpGlobals->curtime + 0.2f);
    pSimple->SetViewModelOnly(true);         // main view only  <-- NEW  

    SimpleParticle* sParticle;

    // Inner glow  
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("effects/yellowflare_noz"), pos);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 0.2f;
    sParticle->m_vecVelocity.Init();
    sParticle->m_uchColor[0] = sParticle->m_uchColor[1] = sParticle->m_uchColor[2] = 255;
    sParticle->m_uchStartAlpha = 255; sParticle->m_uchEndAlpha = 255;
    sParticle->m_uchStartSize = nMagnitude * random->RandomInt(4, 8); sParticle->m_uchEndSize = 0;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = 0.0f;

    // Outer glow  
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("effects/yellowflare_noz"), pos);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 0.2f;
    sParticle->m_vecVelocity.Init();
    float fColor = random->RandomInt(32, 64);
    sParticle->m_uchColor[0] = sParticle->m_uchColor[1] = sParticle->m_uchColor[2] = (byte)fColor;
    sParticle->m_uchStartAlpha = (byte)fColor; sParticle->m_uchEndAlpha = 0;
    sParticle->m_uchStartSize = nMagnitude * random->RandomInt(32, 64); sParticle->m_uchEndSize = 0;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);

    // Smoke  
    Vector sOffs(pos[0] + random->RandomFloat(-4.0f, 4.0f), pos[1] + random->RandomFloat(-4.0f, 4.0f), pos[2]);
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_DustPuff[1], sOffs);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 1.0f;
    sParticle->m_vecVelocity.Init();
    sParticle->m_vecVelocity[2] = 16.0f;
    sParticle->m_vecVelocity[0] = random->RandomFloat(-16.0f, 16.0f);
    sParticle->m_vecVelocity[1] = random->RandomFloat(-16.0f, 16.0f);
    sParticle->m_uchColor[0] = 255; sParticle->m_uchColor[1] = 255; sParticle->m_uchColor[2] = 200;
    sParticle->m_uchStartAlpha = random->RandomInt(16, 32); sParticle->m_uchEndAlpha = 0;
    sParticle->m_uchStartSize = random->RandomInt(4, 8);
    sParticle->m_uchEndSize = sParticle->m_uchStartSize * 4.0f;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = random->RandomFloat(-2.0f, 2.0f);
}

//MyGamepedia: FX_ElectricSpark version for portal view
static void FX_ElectricSparkPortal(const Vector& pos, int nMagnitude, int nTrailLength, const Vector* vecDir)
{
    CSmartPtr<CTrailParticles> pSparkEmitter = CTrailParticles::Create("FX_ElectricSparkPortal 1");
    if (!pSparkEmitter) return;

    if (g_Material_CrossbowSpark == NULL)
        g_Material_CrossbowSpark = pSparkEmitter->GetPMaterial("effects/spark");

    pSparkEmitter->SetSuppressInFirstPerson(true);  // portal view only  

    pSparkEmitter->Setup((Vector&)pos, NULL,
        SPARK_ELECTRIC_SPREAD, SPARK_ELECTRIC_MINSPEED, SPARK_ELECTRIC_MAXSPEED,
        SPARK_ELECTRIC_GRAVITY, SPARK_ELECTRIC_DAMPEN,
        bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);
    pSparkEmitter->SetSortOrigin(pos);

    Vector dir;
    int numSparks = nMagnitude * nMagnitude * random->RandomFloat(2, 4);
    for (int i = 0; i < numSparks; i++)
    {
        TrailParticle* pParticle = (TrailParticle*)pSparkEmitter->AddParticle(sizeof(TrailParticle), g_Material_CrossbowSpark, pos);
        if (!pParticle) return;
        pParticle->m_flLifetime = 0.0f;
        pParticle->m_flDieTime = nMagnitude * random->RandomFloat(1.0f, 2.0f);
        dir.Random(-1.0f, 1.0f);
        dir[2] = random->RandomFloat(0.5f, 1.0f);
        if (vecDir) { dir += 2 * (*vecDir); VectorNormalize(dir); }
        pParticle->m_flWidth = random->RandomFloat(2.0f, 5.0f);
        pParticle->m_flLength = nTrailLength * random->RandomFloat(0.02f, 0.05f);
        pParticle->m_vecVelocity = dir * random->RandomFloat(SPARK_ELECTRIC_MINSPEED, SPARK_ELECTRIC_MAXSPEED);
        Color32Init(pParticle->m_color, 255, 255, 255, 255);
    }

    CSmartPtr<CTrailParticles> pSparkEmitter2 = CTrailParticles::Create("FX_ElectricSparkPortal 2");
    if (!pSparkEmitter2) return;

    pSparkEmitter2->SetSuppressInFirstPerson(true);  // portal view only  

    pSparkEmitter2->SetSortOrigin(pos);
    pSparkEmitter2->m_ParticleCollision.SetGravity(400.0f);
    pSparkEmitter2->SetFlag(bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);

    numSparks = nMagnitude * random->RandomInt(16, 32);
    for (int i = 0; i < numSparks; i++)
    {
        TrailParticle* pParticle = (TrailParticle*)pSparkEmitter2->AddParticle(sizeof(TrailParticle), g_Material_CrossbowSpark, pos);
        if (!pParticle) return;
        pParticle->m_flLifetime = 0.0f;
        dir.Random(-1.0f, 1.0f);
        if (vecDir) { dir += *vecDir; VectorNormalize(dir); }
        pParticle->m_flWidth = random->RandomFloat(2.0f, 4.0f);
        pParticle->m_flLength = nTrailLength * random->RandomFloat(0.02f, 0.03f);
        pParticle->m_flDieTime = nMagnitude * random->RandomFloat(0.1f, 0.2f);
        pParticle->m_vecVelocity = dir * random->RandomFloat(128, 256);
        Color32Init(pParticle->m_color, 255, 255, 255, 255);
    }

    CSmartPtr<CSimpleGlowEmitter> pSimple = CSimpleGlowEmitter::Create("FX_ElectricSparkPortal 3", pos, gpGlobals->curtime + 0.2f);
    pSimple->SetSuppressInFirstPerson(true);         // portal view only  <-- NEW  

    SimpleParticle* sParticle;

    // Inner glow  
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("effects/yellowflare_noz"), pos);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 0.2f;
    sParticle->m_vecVelocity.Init();
    sParticle->m_uchColor[0] = sParticle->m_uchColor[1] = sParticle->m_uchColor[2] = 255;
    sParticle->m_uchStartAlpha = 255; sParticle->m_uchEndAlpha = 255;
    sParticle->m_uchStartSize = nMagnitude * random->RandomInt(4, 8); sParticle->m_uchEndSize = 0;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = 0.0f;

    // Outer glow  
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("effects/yellowflare_noz"), pos);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 0.2f;
    sParticle->m_vecVelocity.Init();
    float fColor = random->RandomInt(32, 64);
    sParticle->m_uchColor[0] = sParticle->m_uchColor[1] = sParticle->m_uchColor[2] = (byte)fColor;
    sParticle->m_uchStartAlpha = (byte)fColor; sParticle->m_uchEndAlpha = 0;
    sParticle->m_uchStartSize = nMagnitude * random->RandomInt(32, 64); sParticle->m_uchEndSize = 0;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);

    // Smoke  
    Vector sOffs(pos[0] + random->RandomFloat(-4.0f, 4.0f), pos[1] + random->RandomFloat(-4.0f, 4.0f), pos[2]);
    sParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_DustPuff[1], sOffs);
    if (!sParticle) return;
    sParticle->m_flLifetime = 0.0f; sParticle->m_flDieTime = 1.0f;
    sParticle->m_vecVelocity.Init();
    sParticle->m_vecVelocity[2] = 16.0f;
    sParticle->m_vecVelocity[0] = random->RandomFloat(-16.0f, 16.0f);
    sParticle->m_vecVelocity[1] = random->RandomFloat(-16.0f, 16.0f);
    sParticle->m_uchColor[0] = 255; sParticle->m_uchColor[1] = 255; sParticle->m_uchColor[2] = 200;
    sParticle->m_uchStartAlpha = random->RandomInt(16, 32); sParticle->m_uchEndAlpha = 0;
    sParticle->m_uchStartSize = random->RandomInt(4, 8);
    sParticle->m_uchEndSize = sParticle->m_uchStartSize * 4.0f;
    sParticle->m_flRoll = random->RandomInt(0, 360); sParticle->m_flRollDelta = random->RandomFloat(-2.0f, 2.0f);
}

// Must match server-side enum in weapon_crossbow.cpp  
enum CrossbowChargerState_t
{
	CHARGER_STATE_START_LOAD = 0,
	CHARGER_STATE_START_CHARGE,
	CHARGER_STATE_READY,
	CHARGER_STATE_DISCHARGE,
	CHARGER_STATE_OFF,
};

class C_WeaponCrossbow : public C_BasePortalCombatWeapon
{
    DECLARE_CLASS(C_WeaponCrossbow, C_BasePortalCombatWeapon);

public:
    DECLARE_CLIENTCLASS();
    DECLARE_PREDICTABLE();

    RenderGroup_t GetRenderGroup() { return RENDER_GROUP_TWOPASS; }

    void    OnDataChanged(DataUpdateType_t updateType);
    int     DrawModel(int flags);
    void    ViewModelDrawn(C_BaseViewModel* pBaseViewModel);


    //virtual bool    OnFireEvent(C_BaseViewModel* pViewModel, const Vector& origin, const QAngle& angles, int event, const char* options);

    C_WeaponCrossbow();

private:
    void    DrawChargerSprite(CPortalgunEffect& effect, bool b3rdPerson, bool bBlast);
    void    UpdateEffectState();

    //these are presets that contain params we will use to draw the sprite, CPortalgunEffect is not a sprite itself,
    //it only contains values
    
    //main sprite on tip
    CPortalgunEffect  m_ChargerEffect;       //vm sprite (main view)
    CPortalgunEffect  m_ChargerEffectWorld;  //wr sprite (portal/3rd person view)

    //blast sprite when reloaded
    CPortalgunEffect  m_BlastEffect;       //vm sprite (main view)
    CPortalgunEffect  m_BlastEffectWorld;  //wr sprite (portal/3rd person view)

    int               m_nChargeState;       //current state
    int               m_nPrevChargeState;   //prev state to compare with

    //we can't do bool compare properly, instead, use increment, 
    //also used 4 vals to control ViewModel and WoRldModel separately
    //viewmodel
    int               m_nBlastCountVM;         //current blast count
    int               m_nPrevBlastCountVM;     //prev blast count to compare with

    //worldmodel
    int               m_nBlastCountWR;         //current blast count
    int               m_nPrevBlastCountWR;     //prev blast count to compare with
};

//-----------------------------------------------------------------------------
// Purpose: Set up our sprites when this entity appear on client.
// Attach to tip, set RGB color, custom material, blank alpha/scale values, 
// hide for both portal and main view, off states.
//-----------------------------------------------------------------------------
C_WeaponCrossbow::C_WeaponCrossbow()
{
    m_ChargerEffect.SetAttachment(BOLT_TIP_ATTACHMENT);
    m_ChargerEffect.SetVisibleViewModel(false);
    m_ChargerEffect.SetVisible3rdPerson(false);
    m_ChargerEffect.SetMaterial(CROSSBOW_GLOW_SPRITE);
    m_ChargerEffect.SetColor(Vector(255, 128, 0));
    m_ChargerEffect.GetAlpha().SetAbsolute(0.001f);
    m_ChargerEffect.GetScale().SetAbsolute(0.001f);

    m_ChargerEffectWorld.SetAttachment(BOLT_TIP_ATTACHMENT);
    m_ChargerEffectWorld.SetVisibleViewModel(false);
    m_ChargerEffectWorld.SetVisible3rdPerson(false);
    m_ChargerEffectWorld.SetMaterial(CROSSBOW_GLOW_SPRITE);
    m_ChargerEffectWorld.SetColor(Vector(255, 128, 0));
    m_ChargerEffectWorld.GetAlpha().SetAbsolute(0.001f);
    m_ChargerEffectWorld.GetScale().SetAbsolute(0.001f);

    m_BlastEffect.SetAttachment(BOLT_SPARK_ATTACHMENT);
    m_BlastEffect.SetVisibleViewModel(false);
    m_BlastEffect.SetVisible3rdPerson(false);
    m_BlastEffect.SetMaterial(CROSSBOW_GLOW_SPRITE2);
    m_BlastEffect.SetColor(Vector(255, 255, 255));
    m_BlastEffect.GetAlpha().SetAbsolute(0.001f);
    m_BlastEffect.GetScale().SetAbsolute(0.001f);

    m_BlastEffectWorld.SetAttachment(BOLT_SPARK_ATTACHMENT);
    m_BlastEffectWorld.SetVisibleViewModel(false);
    m_BlastEffectWorld.SetVisible3rdPerson(false);
    m_BlastEffectWorld.SetMaterial(CROSSBOW_GLOW_SPRITE2);
    m_BlastEffectWorld.SetColor(Vector(255, 255, 255));
    m_BlastEffectWorld.GetAlpha().SetAbsolute(0.001f);
    m_BlastEffectWorld.GetScale().SetAbsolute(0.001f);

    m_nChargeState = CHARGER_STATE_OFF;
    m_nPrevChargeState = CHARGER_STATE_OFF;

    m_nBlastCountVM = 0;
    m_nPrevBlastCountVM = 0;
    m_nBlastCountWR = 0;
    m_nPrevBlastCountWR = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Update params for effect presest.
//-----------------------------------------------------------------------------
void C_WeaponCrossbow::UpdateEffectState()
{
    float sprScale = cl_portalbase_crossbow_sprite_scale.GetFloat(); //sprite scale val

    switch (m_nChargeState)
    {
        //Loading started, hide everything
    case CHARGER_STATE_START_LOAD:
        m_ChargerEffect.SetVisibleViewModel(false);
        m_ChargerEffect.SetVisible3rdPerson(false);
        m_ChargerEffectWorld.SetVisibleViewModel(false);
        m_ChargerEffectWorld.SetVisible3rdPerson(false);
        m_ChargerEffect.GetAlpha().SetAbsolute(0.0f);
        m_ChargerEffect.GetScale().SetAbsolute(0.001f);
        m_ChargerEffectWorld.GetAlpha().SetAbsolute(0.0f);
        m_ChargerEffectWorld.GetScale().SetAbsolute(0.001f);
        break;

        //Charged crossbow
    case CHARGER_STATE_START_CHARGE:
        m_ChargerEffect.SetVisibleViewModel(true);
        m_ChargerEffect.SetVisible3rdPerson(false);
        m_ChargerEffectWorld.SetVisibleViewModel(false);
        m_ChargerEffectWorld.SetVisible3rdPerson(true);
        m_ChargerEffect.GetAlpha().InitFromCurrent(             //InitFromCurrent is value that calcs current time and given delay, use for alpha/scale transitions
            cl_portalbase_crossbow_charge_alpha.GetFloat(),
            cl_portalbase_crossbow_charge_alpha_time.GetFloat());
        m_ChargerEffect.GetScale().InitFromCurrent(
            cl_portalbase_crossbow_charge_scale.GetFloat() * sprScale,
            cl_portalbase_crossbow_charge_scale_time.GetFloat());
        m_ChargerEffectWorld.GetAlpha().InitFromCurrent(
            cl_portalbase_crossbow_charge_alpha.GetFloat(),
            cl_portalbase_crossbow_charge_alpha_time.GetFloat());
        m_ChargerEffectWorld.GetScale().InitFromCurrent(
            cl_portalbase_crossbow_charge_scale.GetFloat() * sprScale,
            cl_portalbase_crossbow_charge_scale_time.GetFloat());
        break;

        //Idle state
    case CHARGER_STATE_READY:
        m_ChargerEffect.SetVisibleViewModel(true);
        m_ChargerEffect.SetVisible3rdPerson(false);
        m_ChargerEffectWorld.SetVisibleViewModel(false);
        m_ChargerEffectWorld.SetVisible3rdPerson(true);
        m_ChargerEffect.GetAlpha().InitFromCurrent(
            cl_portalbase_crossbow_ready_alpha.GetFloat(),
            cl_portalbase_crossbow_ready_alpha_time.GetFloat());
        m_ChargerEffect.GetScale().InitFromCurrent(
            cl_portalbase_crossbow_ready_scale.GetFloat() * sprScale,
            cl_portalbase_crossbow_ready_scale_time.GetFloat());
        m_ChargerEffectWorld.GetAlpha().InitFromCurrent(
            cl_portalbase_crossbow_ready_alpha.GetFloat(),
            cl_portalbase_crossbow_ready_alpha_time.GetFloat());
        m_ChargerEffectWorld.GetScale().InitFromCurrent(
            cl_portalbase_crossbow_ready_scale.GetFloat() * sprScale,
            cl_portalbase_crossbow_ready_scale_time.GetFloat());
        break;

    case CHARGER_STATE_DISCHARGE:
        // Charger glow off  
        m_ChargerEffect.SetVisible(false);
        m_ChargerEffectWorld.SetVisible(false);

        // Blast flash: start at 128 alpha, fade to 0 over 0.4 seconds  
        m_BlastEffect.GetAlpha().Init(128.0f, 0.0f, 0.4f);
        m_BlastEffect.GetScale().SetAbsolute(0.2f * sprScale);
        m_BlastEffect.SetVisibleViewModel(true);
        m_BlastEffect.SetVisible3rdPerson(false);

        m_BlastEffectWorld.GetAlpha().Init(128.0f, 0.0f, 0.4f);
        m_BlastEffectWorld.GetScale().SetAbsolute(0.2f * sprScale);
        m_BlastEffectWorld.SetVisibleViewModel(false);
        m_BlastEffectWorld.SetVisible3rdPerson(true);
        break;

        //Disabled crossbow, hide everything
    case CHARGER_STATE_OFF:
    default:
        m_ChargerEffect.SetVisibleViewModel(false);
        m_ChargerEffect.SetVisible3rdPerson(false);
        m_ChargerEffectWorld.SetVisibleViewModel(false);
        m_ChargerEffectWorld.SetVisible3rdPerson(false);
        m_ChargerEffect.GetAlpha().SetAbsolute(0.0f);
        m_ChargerEffectWorld.GetAlpha().SetAbsolute(0.0f);
        break;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Compare old and current states, if != - update.
//-----------------------------------------------------------------------------
void C_WeaponCrossbow::OnDataChanged(DataUpdateType_t updateType)
{
    BaseClass::OnDataChanged(updateType);

    //save blast vals once restored to avoid issues
    if (updateType == DATA_UPDATE_CREATED)
    {
        m_nPrevBlastCountVM = m_nBlastCountVM;
        m_nPrevBlastCountWR = m_nBlastCountWR;
    }

    //update effect params
    if (m_nChargeState != m_nPrevChargeState)
    {
        UpdateEffectState();
        m_nPrevChargeState = m_nChargeState; //we are updated, wait for next !=
    }
}

//-----------------------------------------------------------------------------
// Purpose: Here we render given sprite
// Input:   &effect     - sprite to render
//          b3rdPerson  - should this sprite render only in poortal view or not.
//          bBlast      - we want to draw blast sprite this time.
//-----------------------------------------------------------------------------
void C_WeaponCrossbow::DrawChargerSprite(CPortalgunEffect &effect, bool b3rdPerson, bool bBlast)
{
    bool bVisible = b3rdPerson ? effect.IsVisible3rdPerson() : effect.IsVisibleViewModel();

    //here is the main trick, check context, which call wants to draw this sprite, 
    //ViewModelDrawn() will check IsVisibleViewModel(), and if it's false for this particualar sprite, go out,
    //otherwise, draw the sprite, the same for DrawModel() and IsVisible3rdPerson(), this is how
    //we render those sprites in two different views independently
    if (!bVisible)
        return;

    float alpha = effect.GetAlpha().Interp(gpGlobals->curtime);

    if (!bBlast)
    {
        //fully transparent ? don't draw
        if (alpha <= 0.0f)
            return;
    }
    else
    {
        if (alpha <= 0.0f)
        {
            //for blast sprite, also hide the sprite
            if (b3rdPerson)
                effect.SetVisible3rdPerson(false);
            else
                effect.SetVisibleViewModel(false);
            return;
        }
    }

    //no player owner ? don't draw
    CBasePlayer* pOwner = ToBasePlayer(GetOwner());
    if (!pOwner)
        return;

    //missing material ? don't draw
    IMaterial* pMat = effect.GetMaterial();
    if (!pMat || pMat->IsErrorMaterial())
    {
        return;
    }

    float scale = effect.GetScale().Interp(gpGlobals->curtime);

    Vector  vecAttachment;
    QAngle  angles; //used so we don't argue with method params

    //we want to render in world model
    if (b3rdPerson)
    {
        //The weapon entity normally has its model index
        //pointing to the viewmodel model, but when drawing through a portal, 
        //the model that's actually visible is the world, we need the world model
        //attach point pos and ang, all we do here is save orig for a while, set world model index,
        //get data from world model attach point for the vars, return back orig from stored
        int originalModelIndex = GetModelIndex();
        SetModelIndex(GetWorldModelIndex());
        GetAttachment(effect.GetAttachment(), vecAttachment, angles);
        
        //also create sparks here
        Vector vecPos;
        QAngle angPos;
        if (GetAttachment(BOLT_SPARK_ATTACHMENT, vecPos, angPos))
            FX_ElectricSparkPortal(vecPos, 1, 1, NULL);

        SetModelIndex(originalModelIndex);
    }
    else //we want to render in viewmodel
    {
        C_BaseViewModel* pVM = pOwner->GetViewModel();

        //now viewmodel actually ? go out
        if (!pVM)
            return;

        //get pos with ang qand 
        pVM->GetAttachment(effect.GetAttachment(), vecAttachment, angles);

        //corrects a world-space position that was obtained from a viewmodel attachment for the FOV difference
        ::FormatViewModelAttachment(vecAttachment, true);
    }

    //RGBA
    color32 color;
    color.r = (byte)effect.GetColor().x;
    color.g = (byte)effect.GetColor().y;
    color.b = (byte)effect.GetColor().z;
    color.a = (byte)clamp(alpha, 0.0f, 255.0f);

    //finally draw our sprite
    CMatRenderContextPtr pRenderContext(materials);
    pRenderContext->Bind(pMat, this);
    DrawSprite(vecAttachment, scale, scale, color);
}

//-----------------------------------------------------------------------------
// Purpose: Worldmodel draw call, render our portal view sprite if it's the pass we want.
//-----------------------------------------------------------------------------
int C_WeaponCrossbow::DrawModel(int flags)
{
    int ret = BaseClass::DrawModel(flags);

    //before we try to draw sprite, make sure the world model itself is rendered
    if (ret)
    {
        DrawChargerSprite(m_ChargerEffectWorld, true, false);

        //the amount of blast sprites should be increased - create one more blast sprite
        if (m_nPrevBlastCountWR < m_nBlastCountWR)
        {
            //HACK! reset blast sprite values for the reload moment
            float sprScale = cl_portalbase_crossbow_sprite_scale.GetFloat();
            m_BlastEffectWorld.GetAlpha().Init(128.0f, 0.0f, 0.4f);
            m_BlastEffectWorld.GetScale().SetAbsolute(0.2f * sprScale);
            m_BlastEffectWorld.SetVisible3rdPerson(true);  //must be set before DrawChargerSprite  
            DrawChargerSprite(m_BlastEffectWorld, true, true);

            m_nPrevBlastCountWR = m_nBlastCountWR;
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Viewmodel draw call, render our main view sprite if it's the pass we want.
//-----------------------------------------------------------------------------
void C_WeaponCrossbow::ViewModelDrawn(C_BaseViewModel* pBaseViewModel)
{
    DrawChargerSprite(m_ChargerEffect, false, false);

    //the amount of blast sprites should be increased - create one more blast sprite
    if (m_nPrevBlastCountVM < m_nBlastCountVM)
    {
        //HACK! reset blast sprite values for the reload moment
        float sprScale = cl_portalbase_crossbow_sprite_scale.GetFloat();
        m_BlastEffect.GetAlpha().Init(128.0f, 0.0f, 0.4f);
        m_BlastEffect.GetScale().SetAbsolute(0.2f * sprScale);
        m_BlastEffect.SetVisibleViewModel(true);   //must be set before DrawChargerSprite  
        DrawChargerSprite(m_BlastEffect, false, true);

        //also create sparks
        Vector vecPos; QAngle angPos;
        if (pBaseViewModel->GetAttachment(BOLT_SPARK_ATTACHMENT, vecPos, angPos))
        {
            ::FormatViewModelAttachment(vecPos, true);
            FX_ElectricSparkVM(vecPos, 1, 1, NULL);
        }

        m_nPrevBlastCountVM = m_nBlastCountVM;
    }

    BaseClass::ViewModelDrawn(pBaseViewModel);
}

STUB_WEAPON_CLASS_IMPLEMENT(weapon_crossbow, C_WeaponCrossbow);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponCrossbow, DT_WeaponCrossbow, CWeaponCrossbow)
    RecvPropInt(RECVINFO(m_nChargeState)), //get from server our state
    RecvPropInt(RECVINFO(m_nBlastCountWR)),
    RecvPropInt(RECVINFO(m_nBlastCountVM)),
END_RECV_TABLE()