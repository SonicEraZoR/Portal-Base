#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "iclientmode.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

using namespace vgui;

#include "tier0/memdbgon.h"

static const float TARGET_FRAMETIME = 1.0 / 60.0;

class CHudGeiger : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudGeiger, vgui::Panel);
public:
	CHudGeiger(const char *pElementName);
	void Init(void);
	void VidInit(void);
	bool ShouldDraw(void);
	virtual void	ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void	Paint(void);
	void MsgFunc_Geiger(bf_read &msg);

private:
	int m_iGeigerRange;

};

DECLARE_HUDELEMENT(CHudGeiger);
DECLARE_HUD_MESSAGE(CHudGeiger, Geiger);

CHudGeiger::CHudGeiger(const char *pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HudGeiger")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH);
}

void CHudGeiger::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
}

void CHudGeiger::Init(void)
{
	HOOK_HUD_MESSAGE(CHudGeiger, Geiger);

	m_iGeigerRange = 0;
};

void CHudGeiger::VidInit(void)
{
	m_iGeigerRange = 0;
};

void CHudGeiger::MsgFunc_Geiger(bf_read &msg)
{
	m_iGeigerRange = msg.ReadByte();
	m_iGeigerRange = m_iGeigerRange << 2;
}

bool CHudGeiger::ShouldDraw(void)
{
	return (CHudElement::ShouldDraw() && (m_iGeigerRange < 1000 && m_iGeigerRange > 0));
}

void CHudGeiger::Paint()
{
	int pct;
	float flvol = 0;
	bool highsound = false;

	if (m_iGeigerRange > 800)
	{
		pct = 0;
	}
	else if (m_iGeigerRange > 600)
	{
		pct = 2;
		flvol = 0.4;
	}
	else if (m_iGeigerRange > 500)
	{
		pct = 4;
		flvol = 0.5;
	}
	else if (m_iGeigerRange > 400)
	{
		pct = 8;
		flvol = 0.6;
		highsound = true;
	}
	else if (m_iGeigerRange > 300)
	{
		pct = 8;
		flvol = 0.7;
		highsound = true;
	}
	else if (m_iGeigerRange > 200)
	{
		pct = 28;
		flvol = 0.78;
		highsound = true;
	}
	else if (m_iGeigerRange > 150)
	{
		pct = 40;
		flvol = 0.80;
		highsound = true;
	}
	else if (m_iGeigerRange > 100)
	{
		pct = 60;
		flvol = 0.85;
		highsound = true;
	}
	else if (m_iGeigerRange > 75)
	{
		pct = 80;
		flvol = 0.9;
		highsound = true;
	}
	else if (m_iGeigerRange > 50)
	{
		pct = 90;
		flvol = 0.95;
	}
	else
	{
		pct = 95;
		flvol = 1.0;
	}

	int n = random->RandomInt(0, 64);

	float flMultiplier = 1 / (gpGlobals->frametime / TARGET_FRAMETIME);

	if (flMultiplier > 10)
	{
		flMultiplier = 10;
	}
	else if (flMultiplier < 0.1)
	{
		flMultiplier = 0.1;
	}

	n *= flMultiplier;

	if (n < pct)
	{
		char sz[256];
		if (highsound)
		{
			strcpy(sz, "Geiger.BeepHigh");
		}
		else
		{
			strcpy(sz, "Geiger.BeepLow");
		}

		CSoundParameters params;

		if (C_BaseEntity::GetParametersForSound(sz, params, NULL))
		{
			flvol = (flvol * (random->RandomInt(0, 127)) / 255) + 0.25;

			CLocalPlayerFilter filter;

			EmitSound_t ep;
			//ep.m_nChannel = params.channel;
			ep.m_nChannel = 4;
			ep.m_pSoundName = params.soundname;
			ep.m_flVolume = flvol;
			ep.m_SoundLevel = params.soundlevel;
			ep.m_nPitch = params.pitch;

			C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, ep);
		}
	}
}