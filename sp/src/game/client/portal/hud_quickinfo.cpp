//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "../hud_crosshair.h"
#include "c_portal_player.h"
#include "c_weapon_portalgun.h"
#include "IGameUIFuncs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	HEALTH_WARNING_THRESHOLD	25


static ConVar	hud_quickinfo( "hud_quickinfo", "1", FCVAR_ARCHIVE );
static ConVar	hud_quickinfo_swap( "hud_quickinfo_swap", "0", FCVAR_ARCHIVE );
static ConVar	beta_quickinfo_older_gun("beta_quickinfo_older_gun", "1", FCVAR_ARCHIVE);

extern ConVar	crosshair;

ConVar	beta_quickinfo("beta_quickinfo", "0", FCVAR_ARCHIVE);

#define BaseLPOpacity	48.0f //Minimum opacity for the last placed portal indicator (What it fades out to)

#define QUICKINFO_EVENT_DURATION	1.0f
#define	QUICKINFO_BRIGHTNESS_FULL	255
#define	QUICKINFO_BRIGHTNESS_DIM	64
#define	QUICKINFO_FADE_IN_TIME		0.5f
#define QUICKINFO_FADE_OUT_TIME		2.0f

/*
==================================================
CHUDQuickInfo 
==================================================
*/

using namespace vgui;

class CHUDQuickInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDQuickInfo, vgui::Panel );
public:
	CHUDQuickInfo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	bool ShouldDraw( void );
	virtual void OnThink();
	virtual void Paint();
	
	virtual void ApplySchemeSettings( IScheme *scheme );
private:
	
	void	DrawWarning( int x, int y, CHudTexture *icon, float &time );
	void	UpdateEventTime( void );
	bool	EventTimeElapsed( void );

	int		m_lastAmmo;
	int		m_lastHealth;

	float	m_ammoFade;
	float	m_healthFade;

	bool	m_warnAmmo;
	bool	m_warnHealth;

	bool	m_bFadedOut;

	bool	m_bDimmed;			// Whether or not we are dimmed down
	float	m_flLastEventTime;	// Last active event (controls dimmed state)
	
	float	m_fLastPlacedAlpha[2];
	bool	m_bLastPlacedAlphaCountingUp[2];

	CPanelAnimationVar(float, m_fCBlend1, "CBlend1", "0");
	CPanelAnimationVar(float, m_fCBlend2, "CBlend2", "0");

	CHudTexture	*m_icon_c;

	CHudTexture	*m_icon_rbn;	// right bracket
	CHudTexture	*m_icon_lbn;	// left bracket

	CHudTexture	*m_icon_rb;		// right bracket, full
	CHudTexture	*m_icon_lb;		// left bracket, full
	CHudTexture	*m_icon_rbe;	// right bracket, empty
	CHudTexture	*m_icon_lbe;	// left bracket, empty

	CHudTexture* m_icon_rb_portal; // right bracket, valid
	CHudTexture* m_icon_lb_portal; // left bracket, valid
	CHudTexture* m_icon_rbe_portal; // right bracket, last placed
	CHudTexture* m_icon_lbe_portal; // left bracket, last placed
	CHudTexture* m_icon_rbn_portal; // right bracket, invalid
	CHudTexture* m_icon_lbn_portal; // left bracket, invalid
};

DECLARE_HUDELEMENT( CHUDQuickInfo );

CHUDQuickInfo::CHUDQuickInfo( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HUDQuickInfo" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_CROSSHAIR );

	m_fLastPlacedAlpha[0] = m_fLastPlacedAlpha[1] = 80;
	m_bLastPlacedAlphaCountingUp[0] = m_bLastPlacedAlphaCountingUp[1] = true;
}

void CHUDQuickInfo::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
}


void CHUDQuickInfo::Init( void )
{
	m_ammoFade = 0.0f;
	m_healthFade = 0.0f;

	m_lastAmmo = 0;
	m_lastHealth = 100;

	m_warnAmmo = false;
	m_warnHealth = false;

	m_bFadedOut = false;
	m_bDimmed = false;
	m_flLastEventTime   = 0.0f;
}


void CHUDQuickInfo::VidInit( void )
{
	Init();

	m_icon_c = gHUD.GetIcon( "crosshair" );
	m_icon_rb = gHUD.GetIcon("crosshair_right_full");
	m_icon_lb = gHUD.GetIcon("crosshair_left_full");
	m_icon_rbe = gHUD.GetIcon("crosshair_right_empty");
	m_icon_lbe = gHUD.GetIcon("crosshair_left_empty");
	m_icon_rbn = gHUD.GetIcon("crosshair_right");
	m_icon_lbn = gHUD.GetIcon("crosshair_left");

	if ( IsX360() )
	{
		m_icon_rb_portal = gHUD.GetIcon( "portal_crosshair_right_valid_x360" );
		m_icon_lb_portal = gHUD.GetIcon("portal_crosshair_left_valid_x360");
		m_icon_rbe_portal = gHUD.GetIcon("portal_crosshair_last_placed_x360");
		m_icon_lbe_portal = gHUD.GetIcon("portal_crosshair_last_placed_x360");
		m_icon_rbn_portal = gHUD.GetIcon("portal_crosshair_right_invalid_x360");
		m_icon_lbn_portal = gHUD.GetIcon("portal_crosshair_left_invalid_x360");
	}
	else
	{
		m_icon_rb_portal = gHUD.GetIcon("portal_crosshair_right_valid");
		m_icon_lb_portal = gHUD.GetIcon("portal_crosshair_left_valid");
		m_icon_rbe_portal = gHUD.GetIcon("portal_crosshair_last_placed");
		m_icon_lbe_portal = gHUD.GetIcon("portal_crosshair_last_placed");
		m_icon_rbn_portal = gHUD.GetIcon("portal_crosshair_right_invalid");
		m_icon_lbn_portal = gHUD.GetIcon("portal_crosshair_left_invalid");
	}
}


void CHUDQuickInfo::DrawWarning( int x, int y, CHudTexture *icon, float &time )
{
	float scale	= (int)( fabs(sin(gpGlobals->curtime*8.0f)) * 128.0);

	// Only fade out at the low point of our blink
	if ( time <= (gpGlobals->frametime * 200.0f) )
	{
		if ( scale < 40 )
		{
			time = 0.0f;
			return;
		}
		else
		{
			// Counteract the offset below to survive another frame
			time += (gpGlobals->frametime * 200.0f);
		}
	}
	
	// Update our time
	time -= (gpGlobals->frametime * 200.0f);
	Color caution = gHUD.m_clrCaution;
	caution[3] = scale * 255;

	icon->DrawSelf( x, y, caution );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::ShouldDraw( void )
{
	if ( !m_icon_c || !m_icon_rb || !m_icon_rbe || !m_icon_lb || !m_icon_lbe )
		return false;

	C_Portal_Player *player = ToPortalPlayer(C_BasePlayer::GetLocalPlayer());
	if ( player == NULL )
		return false;

	if ( !crosshair.GetBool() )
		return false;

	if ( player->IsSuppressingCrosshair() )
		return false;

	return ( CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage() );
}

//-----------------------------------------------------------------------------
// Purpose: Checks if the hud element needs to fade out
//-----------------------------------------------------------------------------
void CHUDQuickInfo::OnThink()
{
	BaseClass::OnThink();

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL)
		return;

	C_WeaponPortalgun *pPortalgun = dynamic_cast<C_WeaponPortalgun*>(pWeapon);

	if (!hud_quickinfo.GetInt() || !pPortalgun || (!pPortalgun->CanFirePortal1() && !pPortalgun->CanFirePortal2()))
	{
		// see if we should fade in/out
		bool bFadeOut = player->IsZoomed();

		// check if the state has changed
		if (m_bFadedOut != bFadeOut)
		{
			m_bFadedOut = bFadeOut;

			m_bDimmed = false;

			if (bFadeOut)
			{
				g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", 0.0f, 0.0f, 0.25f, vgui::AnimationController::INTERPOLATOR_LINEAR);
			}
			else
			{
				g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
			}
		}
		else if (!m_bFadedOut)
		{
			// If we're dormant, fade out
			if (EventTimeElapsed())
			{
				if (!m_bDimmed)
				{
					m_bDimmed = true;
					g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_DIM, 0.0f, QUICKINFO_FADE_OUT_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
				}
			}
			else if (m_bDimmed)
			{
				// Fade back up, we're active
				m_bDimmed = false;
				g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
			}
		}
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
	}
}

void CHUDQuickInfo::Paint()
{
	C_Portal_Player *pPortalPlayer = (C_Portal_Player*)( C_BasePlayer::GetLocalPlayer() );
	if ( pPortalPlayer == NULL )
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon == NULL )
		return;

	float fX, fY;
	bool bBehindCamera = false;
	CHudCrosshair::GetDrawPosition(&fX, &fY, &bBehindCamera);

	if (bBehindCamera)
		return;

	int		xCenter = (int)fX;
	int		yCenter = (int)fY - m_icon_lb->Height() / 2;

	int		xCenter_portal = ScreenWidth() / 2;
	int		yCenter_portal = (ScreenHeight() - m_icon_lb_portal->Height()) / 2;

	float	scalar = 138.0f / 255.0f;

	// Check our health for a warning
	int	health = pPortalPlayer->GetHealth();
	if (health != m_lastHealth)
	{
		UpdateEventTime();
		m_lastHealth = health;

		if (health <= HEALTH_WARNING_THRESHOLD)
		{
			if (m_warnHealth == false)
			{
				m_healthFade = 255;
				m_warnHealth = true;

				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowHealth");
			}
		}
		else
		{
			m_warnHealth = false;
		}
	}

	// Check our ammo for a warning
	int	ammo = pWeapon->Clip1();
	if (ammo != m_lastAmmo)
	{
		UpdateEventTime();
		m_lastAmmo = ammo;

		// Find how far through the current clip we are
		float ammoPerc = (float)ammo / (float)pWeapon->GetMaxClip1();

		// Warn if we're below a certain percentage of our clip's size
		if ((pWeapon->GetMaxClip1() > 1) && (ammoPerc <= (1.0f - CLIP_PERC_THRESHOLD)))
		{
			if (m_warnAmmo == false)
			{
				m_ammoFade = 255;
				m_warnAmmo = true;

				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowAmmo");
			}
		}
		else
		{
			m_warnAmmo = false;
		}
	}

	Color clrNormal = gHUD.m_clrNormal;
	clrNormal[3] = 255 * scalar;
	
	SetActive( true );
	
	m_icon_c->DrawSelf(xCenter, yCenter, clrNormal);

	if (IsX360())
	{
		// Because the fixed reticle draws on half-texels, this rather unsightly hack really helps
		// center the appearance of the quickinfo on 360 displays.
		xCenter += 1;
	}

	if (!hud_quickinfo.GetInt())
		return;

	int	sinScale = (int)(fabs(sin(gpGlobals->curtime*8.0f)) * 128.0f);

	C_WeaponPortalgun *pPortalgun = dynamic_cast<C_WeaponPortalgun*>( pWeapon );

	bool bPortalPlacability[2];

	if ( pPortalgun )
	{
		bPortalPlacability[0] = pPortalgun->GetPortal1Placablity() > 0.5f;
		bPortalPlacability[1] = pPortalgun->GetPortal2Placablity() > 0.5f;
	}

	if ( !hud_quickinfo.GetInt() || !pPortalgun || ( !pPortalgun->CanFirePortal1() && !pPortalgun->CanFirePortal2() ) )
	{
		// no quickinfo or we can't fire either portal, just draw the small versions of the crosshairs

		// Update our health
		if (m_healthFade > 0.0f)
		{
			DrawWarning(xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_healthFade);
		}
		else
		{
			float healthPerc = (float)health / 100.0f;
			healthPerc = clamp(healthPerc, 0.0f, 1.0f);

			Color healthColor = m_warnHealth ? gHUD.m_clrCaution : gHUD.m_clrNormal;

			if (m_warnHealth)
			{
				healthColor[3] = 255 * sinScale;
			}
			else
			{
				healthColor[3] = 255 * scalar;
			}

			gHUD.DrawIconProgressBar(xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_icon_lbe, (1.0f - healthPerc), healthColor, CHud::HUDPB_VERTICAL);
		}

		// Update our ammo
		if (m_ammoFade > 0.0f)
		{
			DrawWarning(xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_ammoFade);
		}
		else
		{
			float ammoPerc;

			if (pWeapon->GetMaxClip1() <= 0)
			{
				ammoPerc = 0.0f;
			}
			else
			{
				ammoPerc = 1.0f - ((float)ammo / (float)pWeapon->GetMaxClip1());
				ammoPerc = clamp(ammoPerc, 0.0f, 1.0f);
			}

			Color ammoColor = m_warnAmmo ? gHUD.m_clrCaution : gHUD.m_clrNormal;

			if (m_warnAmmo)
			{
				ammoColor[3] = 255 * sinScale;
			}
			else
			{
				ammoColor[3] = 255 * scalar;
			}

			gHUD.DrawIconProgressBar(xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_icon_rbe, ammoPerc, ammoColor, CHud::HUDPB_VERTICAL);
		}
		return;
	}

	const unsigned char iAlphaStart = 150;	   

	Color portal1Color = UTIL_Portal_Color( 1 );
	Color portal2Color = UTIL_Portal_Color( 2 );

	portal1Color[ 3 ] = iAlphaStart;
	portal2Color[ 3 ] = iAlphaStart;

	const int iBaseLastPlacedAlpha = 128;
	Color lastPlaced1Color = Color( portal1Color[0], portal1Color[1], portal1Color[2], iBaseLastPlacedAlpha );
	Color lastPlaced2Color = Color( portal2Color[0], portal2Color[1], portal2Color[2], iBaseLastPlacedAlpha );

	const float fLastPlacedAlphaLerpSpeed = 300.0f;

	
	float fLeftPlaceBarFill = 0.0f;
	float fRightPlaceBarFill = 0.0f;
	
	if (!beta_quickinfo.GetBool())
	{
		if (pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2())
		{
			int iDrawLastPlaced = 0;

			//do last placed indicator effects
			if (pPortalgun->GetLastFiredPortal() == 1)
			{
				iDrawLastPlaced = 0;
				fLeftPlaceBarFill = 1.0f;
			}
			else if (pPortalgun->GetLastFiredPortal() == 2)
			{
				iDrawLastPlaced = 1;
				fRightPlaceBarFill = 1.0f;
			}

			if (m_bLastPlacedAlphaCountingUp[iDrawLastPlaced])
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] += gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed * 2.0f;
				if (m_fLastPlacedAlpha[iDrawLastPlaced] > 255.0f)
				{
					m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] = false;
					m_fLastPlacedAlpha[iDrawLastPlaced] = 255.0f - (m_fLastPlacedAlpha[iDrawLastPlaced] - 255.0f);
				}
			}
			else
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
				if (m_fLastPlacedAlpha[iDrawLastPlaced] < (float)iBaseLastPlacedAlpha)
				{
					m_fLastPlacedAlpha[iDrawLastPlaced] = (float)iBaseLastPlacedAlpha;
				}
			}

			//reset the last placed indicator on the other side
			m_fLastPlacedAlpha[1 - iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
			if (m_fLastPlacedAlpha[1 - iDrawLastPlaced] < 0.0f)
			{
				m_fLastPlacedAlpha[1 - iDrawLastPlaced] = 0.0f;
			}
			m_bLastPlacedAlphaCountingUp[1 - iDrawLastPlaced] = true;

			if (pPortalgun->GetLastFiredPortal() != 0)
			{
				lastPlaced1Color[3] = m_fLastPlacedAlpha[0];
				lastPlaced2Color[3] = m_fLastPlacedAlpha[1];
			}
			else
			{
				lastPlaced1Color[3] = 0.0f;
				lastPlaced2Color[3] = 0.0f;
			}
		}
		//can't fire both portals, and we want the crosshair to remain somewhat symmetrical without being confusing
		else if (!pPortalgun->CanFirePortal1())
		{
			// clone portal2 info to portal 1
			portal1Color = portal2Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[0] = bPortalPlacability[1];
		}
		else if (!pPortalgun->CanFirePortal2())
		{
			// clone portal1 info to portal 2
			portal2Color = portal1Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[1] = bPortalPlacability[0];
		}

		if (pPortalgun->IsHoldingObject())
		{
			// Change the middle to orange 
			portal1Color = portal2Color = UTIL_Portal_Color(0);
			bPortalPlacability[0] = bPortalPlacability[1] = false;
		}

		if (!hud_quickinfo_swap.GetBool())
		{
			if (bPortalPlacability[0])
				m_icon_lb_portal->DrawSelf(xCenter_portal - (m_icon_lb_portal->Width() * 0.64f), yCenter_portal - (m_icon_rb_portal->Height() * 0.17f), portal1Color);
			else
				m_icon_lbn_portal->DrawSelf(xCenter_portal - (m_icon_lbn_portal->Width() * 0.64f), yCenter_portal - (m_icon_rb_portal->Height() * 0.17f), portal1Color);

			if (bPortalPlacability[1])
				m_icon_rb_portal->DrawSelf(xCenter_portal + (m_icon_rb_portal->Width() * -0.35f), yCenter_portal + (m_icon_rb_portal->Height() * 0.17f), portal2Color);
			else
				m_icon_rbn_portal->DrawSelf(xCenter_portal + (m_icon_rbn_portal->Width() * -0.35f), yCenter_portal + (m_icon_rb_portal->Height() * 0.17f), portal2Color);

			//last placed portal indicator
			m_icon_lbe_portal->DrawSelf(xCenter_portal - (m_icon_lbe_portal->Width() * 1.85f), yCenter_portal, lastPlaced1Color);
			m_icon_rbe_portal->DrawSelf(xCenter_portal + (m_icon_rbe_portal->Width() * 0.75f), yCenter_portal, lastPlaced2Color);
		}
		else
		{
			if (bPortalPlacability[1])
				m_icon_lb_portal->DrawSelf(xCenter_portal - (m_icon_lb_portal->Width() * 0.64f), yCenter_portal - (m_icon_rb_portal->Height() * 0.17f), portal2Color);
			else
				m_icon_lbn_portal->DrawSelf(xCenter_portal - (m_icon_lbn_portal->Width() * 0.64f), yCenter_portal - (m_icon_rb_portal->Height() * 0.17f), portal2Color);

			if (bPortalPlacability[0])
				m_icon_rb_portal->DrawSelf(xCenter_portal + (m_icon_rb_portal->Width() * -0.35f), yCenter_portal + (m_icon_rb_portal->Height() * 0.17f), portal1Color);
			else
				m_icon_rbn_portal->DrawSelf(xCenter_portal + (m_icon_rbn_portal->Width() * -0.35f), yCenter_portal + (m_icon_rb_portal->Height() * 0.17f), portal1Color);

			//last placed portal indicator
			m_icon_lbe_portal->DrawSelf(xCenter_portal - (m_icon_lbe_portal->Width() * 1.85f), yCenter_portal, lastPlaced2Color);
			m_icon_rbe_portal->DrawSelf(xCenter_portal + (m_icon_rbe_portal->Width() * 0.75f), yCenter_portal, lastPlaced1Color);
		}
	}
	else
	{
		if (pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2())
		{
			int iDrawLastPlaced = 0;

			//do last placed indicator effects
			if (pPortalgun->GetLastFiredPortal() == 1)
			{
				iDrawLastPlaced = 0;
				fLeftPlaceBarFill = 1.0f;
			}
			else if (pPortalgun->GetLastFiredPortal() == 2)
			{
				iDrawLastPlaced = 1;
				fRightPlaceBarFill = 1.0f;
			}

			if (m_bLastPlacedAlphaCountingUp[iDrawLastPlaced])
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] += gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed * 2.0f;
				if (m_fLastPlacedAlpha[iDrawLastPlaced] > 255.0f)
				{
					m_bLastPlacedAlphaCountingUp[iDrawLastPlaced] = false;
					m_fLastPlacedAlpha[iDrawLastPlaced] = 255.0f - (m_fLastPlacedAlpha[iDrawLastPlaced] - 255.0f);
				}
			}
			else
			{
				m_fLastPlacedAlpha[iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
				if (m_fLastPlacedAlpha[iDrawLastPlaced] < (float)BaseLPOpacity)
				{
					m_fLastPlacedAlpha[iDrawLastPlaced] = (float)BaseLPOpacity;
				}
			}

			//reset the last placed indicator on the other side
			m_fLastPlacedAlpha[1 - iDrawLastPlaced] -= gpGlobals->absoluteframetime * fLastPlacedAlphaLerpSpeed;
			if (m_fLastPlacedAlpha[1 - iDrawLastPlaced] < BaseLPOpacity)
			{
				m_fLastPlacedAlpha[1 - iDrawLastPlaced] = BaseLPOpacity;
			}
			m_bLastPlacedAlphaCountingUp[1 - iDrawLastPlaced] = true;

			if (pPortalgun->GetLastFiredPortal() != 0)
			{
				lastPlaced1Color[3] = m_fLastPlacedAlpha[0];
				lastPlaced2Color[3] = m_fLastPlacedAlpha[1];
			}
			else
			{
				lastPlaced1Color[3] = BaseLPOpacity;
				lastPlaced2Color[3] = BaseLPOpacity;
			}

		}
		//can't fire both portals, and we want the crosshair to remain somewhat symmetrical without being confusing
		else if (!pPortalgun->CanFirePortal1())
		{
			// clone portal2 info to portal 1
			portal1Color = portal2Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[0] = bPortalPlacability[1];

		}
		else if (!pPortalgun->CanFirePortal2())
		{
			// clone portal1 info to portal 2
			portal2Color = portal1Color;
			lastPlaced1Color[3] = 0.0f;
			lastPlaced2Color[3] = 0.0f;
			bPortalPlacability[1] = bPortalPlacability[0];
		}

		if (pPortalgun->IsHoldingObject())
		{
			// Change the middle to orange 
			portal1Color = portal2Color = UTIL_Portal_Color(0);

			if (!beta_quickinfo_older_gun.GetBool())
			{
				if (lastPlaced1Color[3] != 0)
				{
					lastPlaced1Color[0] = UTIL_Portal_Color(0)[0];
					lastPlaced1Color[1] = UTIL_Portal_Color(0)[1];
					lastPlaced1Color[2] = UTIL_Portal_Color(0)[2];
				}

				if (lastPlaced2Color[3] != 0)
				{
					lastPlaced2Color[0] = UTIL_Portal_Color(0)[0];
					lastPlaced2Color[1] = UTIL_Portal_Color(0)[1];
					lastPlaced2Color[2] = UTIL_Portal_Color(0)[2];
				}
			}
			//The last placed portal indicators do not change color in 2006, only the center. --PelPix
			bPortalPlacability[0] = bPortalPlacability[1] = 1.0f;
		}
		/*
		// Note
		1.0f = full
		0.0 = empty
		We need to make the transistions have a delay and not be instant.
		*/

		float fDelay = 0.0f;
		float fDelay2 = 0.0f;
		float fSnapBackDelay = 0.0f;
		Vector Null_vector(0, 0, 0);
		bool bluePortalPlacabilityControllable = bPortalPlacability[0];
		bool orangePortalPlacabilityControllable = bPortalPlacability[1];
		ConVar *beta_quickinfo_show_portal_delay = cvar->FindVar("beta_quickinfo_show_portal_delay");
		
		if (beta_quickinfo_show_portal_delay->GetBool())
		{
			fDelay = pPortalgun->m_fPortalPlacementDelay;
			fDelay2 = 0.1f;
			fSnapBackDelay = 0.0001f;
			if ((pPortalPlayer->PreDataChanged_Backup.m_qEyeAngles != pPortalPlayer->m_iv_angEyeAngles.GetCurrent()) || (pPortalPlayer->GetAbsVelocity() != Null_vector)) //either camera is moving or player is moving
			{
				bluePortalPlacabilityControllable = false;
				orangePortalPlacabilityControllable = false;
			}
		}

		if (pPortalgun->IsHoldingObject())
		{
			m_fCBlend1 = bPortalPlacability[0];
			m_fCBlend2 = bPortalPlacability[1];
		}
		else
		{
			if (beta_quickinfo_show_portal_delay->GetBool())
			{
				if (bluePortalPlacabilityControllable)
				{
					g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend1", bPortalPlacability[0], 0.0f, fDelay, vgui::AnimationController::INTERPOLATOR_LINEAR);
				}
				else
				{
					if (bPortalPlacability[0])
					{
						g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend1", 0.2f, 0.0f, fSnapBackDelay, vgui::AnimationController::INTERPOLATOR_LINEAR);
					}
					else
					{
						g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend1", 0.0f, 0.0f, fDelay2, vgui::AnimationController::INTERPOLATOR_LINEAR);
					}
				}

				if (orangePortalPlacabilityControllable)
				{
					g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend2", bPortalPlacability[1], 0.0f, fDelay, vgui::AnimationController::INTERPOLATOR_LINEAR);
				}
				else
				{
					if (bPortalPlacability[1])
					{
						g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend2", 0.2f, 0.0f, fSnapBackDelay, vgui::AnimationController::INTERPOLATOR_LINEAR);
					}
					else
					{
						g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "CBlend2", 0.0f, 0.0f, fDelay2, vgui::AnimationController::INTERPOLATOR_LINEAR);
					}
				}
			}
			else
			{
				m_fCBlend1 = bPortalPlacability[0];
				m_fCBlend2 = bPortalPlacability[1];
			}
		}

		gHUD.DrawIconProgressBar(xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_icon_lbe, (1.0f - m_fCBlend1), portal1Color, CHud::HUDPB_VERTICAL);
		gHUD.DrawIconProgressBar(xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_icon_rbe, (1.0f - m_fCBlend2), portal2Color, CHud::HUDPB_VERTICAL);

		//last placed portal indicator
		if (pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2())
		{
			if (pPortalgun->GetLastFiredPortal() == 1)
			{
				m_icon_lb->DrawSelf(xCenter - (m_icon_lb->Width() * 2.60f), yCenter, lastPlaced1Color); //2.75f
			}
			else
			{
				m_icon_lbe->DrawSelf(xCenter - (m_icon_lbe->Width() * 2.60f), yCenter, lastPlaced1Color);//2.75f
			}

			if (pPortalgun->GetLastFiredPortal() == 2)
			{
				m_icon_rb->DrawSelf(xCenter + (m_icon_rb->Width() * 1.60f), yCenter, lastPlaced2Color); //1.75f
			}
			else
			{
				m_icon_rbe->DrawSelf(xCenter + (m_icon_rbe->Width() * 1.60f), yCenter, lastPlaced2Color);//1.75f
			}
		}
		else if (pPortalgun->CanFirePortal1() && !pPortalgun->CanFirePortal2())
		{
			m_icon_lbe->DrawSelf(xCenter - (m_icon_lbe->Width() * 2.60f), yCenter, portal1Color);//2.75f
			m_icon_rbe->DrawSelf(xCenter + (m_icon_rbe->Width() * 1.60f), yCenter, portal1Color);//1.75f

		}
		else if (!pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2())
		{
			m_icon_lbe->DrawSelf(xCenter - (m_icon_lbe->Width() * 2.60f), yCenter, portal2Color);//2.75f
			m_icon_rbe->DrawSelf(xCenter + (m_icon_rbe->Width() * 1.60f), yCenter, portal2Color);//1.75f
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHUDQuickInfo::UpdateEventTime( void )
{
	m_flLastEventTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::EventTimeElapsed( void )
{
	if (( gpGlobals->curtime - m_flLastEventTime ) > QUICKINFO_EVENT_DURATION )
		return true;

	return false;
}

