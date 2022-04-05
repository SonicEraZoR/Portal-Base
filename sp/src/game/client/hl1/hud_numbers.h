#ifndef HUD_NUMBERS_H
#define HUD_NUMBERS_H
#ifdef  _WIN32
#pragma once
#endif

#include "cdll_util.h"
#include <vgui_controls/Panel.h>

class CHudNumbers : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudNumbers, vgui::Panel);
	
public:
	CHudNumbers(vgui::Panel *parent, const char *name);
	void VidInit(void);

protected:
	int		DrawHudNumber(int x, int y, int iNumber, Color &clrDraw);
	int		GetNumberFontHeight(void);
	int		GetNumberFontWidth(void);

private:
	CHudTexture *digits[10];
};

#endif