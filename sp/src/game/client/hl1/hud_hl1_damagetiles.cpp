#include "cbase.h"
#include "hud.h"
#include "text_message.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"
#include "IEffects.h"
#include "hudelement.h"

using namespace vgui;

#include "tier0/memdbgon.h"

#define DMG_IMAGE_LIFE		2
#define NUM_DMG_TYPES		8

typedef struct
{
	float		flExpire;
	float		flBaseline;
	int			x;
	int			y;
	CHudTexture	*icon;
	long		fFlags;
} damagetile_t;

class CHudDamageTiles : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudDamageTiles, vgui::Panel);

public:
	CHudDamageTiles(const char *pElementName);

	void	Init(void);
	void	VidInit(void);
	void	Reset(void);
	bool	ShouldDraw(void);

	void	MsgFunc_Damage(bf_read &msg);

private:
	void	Paint();
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

	CHudTexture	*DamageTileIcon(int i);
	long		DamageTileFlags(int i);
	void		UpdateTiles(long bits);

	long			m_bitsDamage;
	damagetile_t	m_dmgTileInfo[NUM_DMG_TYPES];
};

DECLARE_HUDELEMENT(CHudDamageTiles);
DECLARE_HUD_MESSAGE(CHudDamageTiles, Damage);

CHudDamageTiles::CHudDamageTiles(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDamageTiles")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudDamageTiles::Init(void)
{
	HOOK_HUD_MESSAGE(CHudDamageTiles, Damage);
}


void CHudDamageTiles::VidInit(void)
{
	Reset();
}

void CHudDamageTiles::Reset(void)
{
	m_bitsDamage = 0;
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		m_dmgTileInfo[i].flExpire = 0;
		m_dmgTileInfo[i].flBaseline = 0;
		m_dmgTileInfo[i].x = 0;
		m_dmgTileInfo[i].y = 0;
		m_dmgTileInfo[i].icon = DamageTileIcon(i);
		m_dmgTileInfo[i].fFlags = DamageTileFlags(i);
	}
}

CHudTexture *CHudDamageTiles::DamageTileIcon(int i)
{
	switch (i)
	{
	case 0:
	default:
		return gHUD.GetIcon("dmg_poison");
		break;
	case 1:
		return gHUD.GetIcon("dmg_chem");
		break;
	case 2:
		return gHUD.GetIcon("dmg_cold");
		break;
	case 3:
		return gHUD.GetIcon("dmg_drown");
		break;
	case 4:
		return gHUD.GetIcon("dmg_heat");
		break;
	case 5:
		return gHUD.GetIcon("dmg_gas");
		break;
	case 6:
		return gHUD.GetIcon("dmg_rad");
		break;
	case 7:
		return gHUD.GetIcon("dmg_shock");
		break;
	}
}

long CHudDamageTiles::DamageTileFlags(int i)
{
	switch (i)
	{
	case 0:
	default:
		return DMG_POISON;

	case 1:
		return DMG_ACID;

	case 2:
		return DMG_VEHICLE | DMG_DISSOLVE;

	case 3:
		return DMG_DROWN;

	case 4:
		return DMG_BURN | DMG_SLOWBURN;

	case 5:
		return DMG_NERVEGAS;

	case 6:
		return DMG_RADIATION;

	case 7:
		return DMG_SHOCK;
	}
}

void CHudDamageTiles::MsgFunc_Damage(bf_read &msg)
{
	msg.ReadByte();
	msg.ReadByte();
	long bitsDamage = msg.ReadLong();

	UpdateTiles(bitsDamage);
}

bool CHudDamageTiles::ShouldDraw(void)
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (!m_bitsDamage)
		return false;

	return true;
}

void CHudDamageTiles::Paint(void)
{
	int				r, g, b, a, nUnused;
	damagetile_t	*pDmgTile;
	Color			clrTile;

	(gHUD.m_clrYellowish).GetColor(r, g, b, nUnused);
	a = (int)(fabs(sin(gpGlobals->curtime * 2)) * 256.0);
	clrTile.SetColor(r, g, b, a);

	int i;
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];
		if (m_bitsDamage & pDmgTile->fFlags)
		{
			if (pDmgTile->icon)
			{
				(pDmgTile->icon)->DrawSelf(pDmgTile->x, pDmgTile->y, clrTile);
			}
		}
	}

	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];

		if (m_bitsDamage & pDmgTile->fFlags)
		{
			pDmgTile->flExpire = min(gpGlobals->curtime + DMG_IMAGE_LIFE, pDmgTile->flExpire);

			if (pDmgTile->flExpire <= gpGlobals->curtime && a < 40)
			{
				pDmgTile->flExpire = 0;

				int y = pDmgTile->y;
				pDmgTile->x = 0;
				pDmgTile->y = 0;

				m_bitsDamage &= ~pDmgTile->fFlags;

				for (int j = 0; j < NUM_DMG_TYPES; j++)
				{
					damagetile_t *pDmgTileIter = &m_dmgTileInfo[j];
					if ((pDmgTileIter->y) && (pDmgTileIter->y < y) && (pDmgTileIter->icon))
						pDmgTileIter->y += (pDmgTileIter->icon)->Height();
				}
			}
		}
	}
}


void CHudDamageTiles::UpdateTiles(long bitsDamage)
{
	damagetile_t *pDmgTile;

	long bitsOn = ~m_bitsDamage & bitsDamage;

	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];

		if (m_bitsDamage & pDmgTile->fFlags)
		{
			pDmgTile->flExpire = gpGlobals->curtime + DMG_IMAGE_LIFE;
			if (!pDmgTile->flBaseline)
			{
				pDmgTile->flBaseline = gpGlobals->curtime;
			}
		}

		if (bitsOn & pDmgTile->fFlags)
		{
			if (pDmgTile->icon)
			{
				pDmgTile->x = (pDmgTile->icon)->Width() / 8;
				pDmgTile->y = GetTall() - (pDmgTile->icon)->Height() * 2;
			}
			pDmgTile->flExpire = gpGlobals->curtime + DMG_IMAGE_LIFE;

			for (int j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pDmgTile = &m_dmgTileInfo[j];
				if (pDmgTile->y && pDmgTile->icon)
					pDmgTile->y -= (pDmgTile->icon)->Height();

			}
			pDmgTile = &m_dmgTileInfo[i];
		}
	}

	m_bitsDamage |= bitsDamage;
}

void CHudDamageTiles::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}