#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hud_numbers.h"

CHudNumbers::CHudNumbers(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
}

void CHudNumbers::VidInit(void)
{
	for (int i = 0; i < 10; i++)
	{
		char szNumString[10];

		sprintf(szNumString, "number_%x", i);
		digits[i] = gHUD.GetIcon(szNumString);
	}
}

int CHudNumbers::GetNumberFontHeight(void)
{
	if (digits[0])
	{
		return digits[0]->Height();
	}
	else
	{
		return 0;
	}
}

int CHudNumbers::GetNumberFontWidth(void)
{
	if (digits[0])
	{
		return digits[0]->Width();
	}
	else
	{
		return 0;
	}
}

int CHudNumbers::DrawHudNumber(int x, int y, int iNumber, Color &clrDraw)
{
	int iWidth = GetNumberFontWidth();
	int k;

	if (iNumber > 0)
	{
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			digits[k]->DrawSelf(x, y, clrDraw);
			x += iWidth;
		}
		else
		{
			x += iWidth;
		}

		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			digits[k]->DrawSelf(x, y, clrDraw);
			x += iWidth;
		}
		else
		{
			x += iWidth;
		}

		k = iNumber % 10;
		digits[k]->DrawSelf(x, y, clrDraw);
		x += iWidth;
	}
	else
	{
		x += iWidth;

		x += iWidth;

		k = 0;
		digits[k]->DrawSelf(x, y, clrDraw);
		x += iWidth;
	}

	return x;
}
