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

#define FADE_TIME	100
#define MIN_ALPHA	100	

class CHudBattery : public CHudElement, public CHudNumbers
{
	DECLARE_CLASS_SIMPLE(CHudBattery, CHudNumbers);

public:
	CHudBattery(const char *pElementName);

	void			Init(void);
	void			Reset(void);
	void			VidInit(void);
	void			MsgFunc_Battery(bf_read &msg);

private:
	void	Paint(void);
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

	CHudTexture		*icon_suit_empty;
	CHudTexture		*icon_suit_full;
	int				m_iBattery;
	float			m_flFade;
};

DECLARE_HUDELEMENT(CHudBattery);
DECLARE_HUD_MESSAGE(CHudBattery, Battery);

CHudBattery::CHudBattery(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudSuit")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
}

void CHudBattery::Init()
{
	HOOK_HUD_MESSAGE(CHudBattery, Battery);
	Reset();
}

void CHudBattery::Reset()
{
	m_iBattery = 0;
	m_flFade = 0;
}

void CHudBattery::VidInit()
{
	Reset();

	BaseClass::VidInit();
}

void CHudBattery::Paint()
{
	Color	clrHealth;
	int		a;
	int		x;
	int		y;

	BaseClass::Paint();

	if (!icon_suit_empty)
	{
		icon_suit_empty = gHUD.GetIcon("suit_empty");
	}

	if (!icon_suit_full)
	{
		icon_suit_full = gHUD.GetIcon("suit_full");
	}

	if (!icon_suit_empty || !icon_suit_full)
	{
		return;
	}

	if (m_flFade)
	{
		if (m_flFade > FADE_TIME)
			m_flFade = FADE_TIME;

		m_flFade -= (gpGlobals->frametime * 20);
		if (m_flFade <= 0)
		{
			a = 128;
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

	int r, g, b, nUnused;
	(gHUD.m_clrYellowish).GetColor(r, g, b, nUnused);
	clrHealth.SetColor(r, g, b, a);

	int nFontHeight = GetNumberFontHeight();

	int nHudElemWidth, nHudElemHeight;
	GetSize(nHudElemWidth, nHudElemHeight);

	int iOffset = icon_suit_empty->Height() / 6;

	x = nHudElemWidth / 5;
	y = nHudElemHeight - (nFontHeight * 1.5);

	icon_suit_empty->DrawSelf(x, y - iOffset, clrHealth);

	if (m_iBattery > 0)
	{
		int nSuitOffset = icon_suit_full->Height() * ((float)(100 - (min(100, m_iBattery))) * 0.01);
		icon_suit_full->DrawSelfCropped(x, y - iOffset + nSuitOffset, 0, nSuitOffset, icon_suit_full->Width(), icon_suit_full->Height() - nSuitOffset, clrHealth);
	}

	x += icon_suit_empty->Width();
	DrawHudNumber(x, y, m_iBattery, clrHealth);
}

void CHudBattery::MsgFunc_Battery(bf_read &msg)
{
	int x = msg.ReadShort();

	if (x != m_iBattery)
	{
		m_flFade = FADE_TIME;
		m_iBattery = x;
	}
}

void CHudBattery::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}
