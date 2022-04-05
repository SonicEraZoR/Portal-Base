#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Panel.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numbers.h"

#include "convar.h"

#define INIT_HEALTH -1

#define FADE_TIME	100
#define MIN_ALPHA	100

class CHudHealth : public CHudElement, public CHudNumbers
{
	DECLARE_CLASS_SIMPLE(CHudHealth, CHudNumbers);

public:
	CHudHealth(const char *pElementName);

	void			Init(void);
	void			VidInit(void);
	void			Reset(void);
	void			OnThink();
	void			MsgFunc_Damage(bf_read &msg);

private:
	void	Paint(void);
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

	CHudTexture		*icon_cross;
	int				m_iHealth;
	float			m_flFade;
	int				m_bitsDamage;
};

DECLARE_HUDELEMENT(CHudHealth);
DECLARE_HUD_MESSAGE(CHudHealth, Damage);

CHudHealth::CHudHealth(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHealth")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE(CHudHealth, Damage);
	Reset();
}

void CHudHealth::Reset()
{
	m_iHealth = INIT_HEALTH;
	m_flFade = 0;
	m_bitsDamage = 0;
}

void CHudHealth::VidInit()
{
	Reset();
	BaseClass::VidInit();
}

void CHudHealth::OnThink()
{
	int i = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		i = max(local->GetHealth(), 0);
	}

	if (i == m_iHealth)
	{
		return;
	}

	m_flFade = FADE_TIME;
	m_iHealth = i;
}

void CHudHealth::Paint()
{
	Color	clrHealth;
	int		a;
	int		x;
	int		y;

	BaseClass::Paint();

	if (!icon_cross)
	{
		icon_cross = gHUD.GetIcon("cross");
	}

	if (!icon_cross)
	{
		return;
	}

	if (m_flFade)
	{
		m_flFade -= (gpGlobals->frametime * 20);
		if (m_flFade <= 0)
		{
			a = MIN_ALPHA;
			m_flFade = 0;
		}
		else
		{
			a = MIN_ALPHA + (m_flFade / FADE_TIME) * 128;
		}
	}
	else
	{
		a = MIN_ALPHA;
	}

	if (m_iHealth <= 15)
		a = 255;

	if (m_iHealth > 25)
	{
		int r, g, b, nUnused;

		(gHUD.m_clrYellowish).GetColor(r, g, b, nUnused);
		clrHealth.SetColor(r, g, b, a);
	}
	else
	{
		clrHealth.SetColor(250, 0, 0, a);
	}

	int nFontWidth = GetNumberFontWidth();
	int nFontHeight = GetNumberFontHeight();
	int nCrossWidth = icon_cross->Width();

	x = nCrossWidth / 2;
	y = GetTall() - (nFontHeight * 1.5);

	icon_cross->DrawSelf(x, y, clrHealth);

	x = nCrossWidth + (nFontWidth / 2);

	x = DrawHudNumber(x, y, m_iHealth, clrHealth);

	x += nFontWidth / 2;

	int iHeight = nFontHeight;
	int iWidth = nFontWidth / 10;

	clrHealth.SetColor(255, 160, 0, a);
	vgui::surface()->DrawSetColor(clrHealth);
	vgui::surface()->DrawFilledRect(x, y, x + iWidth, y + iHeight);
}

void CHudHealth::MsgFunc_Damage(bf_read &msg)
{
	msg.ReadByte();
	msg.ReadByte();
	msg.ReadLong();

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();
}

void CHudHealth::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}