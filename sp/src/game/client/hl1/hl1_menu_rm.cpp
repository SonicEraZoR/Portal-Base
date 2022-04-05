#include "cbase.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>

class CRMMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CRMMenu, vgui::Frame);
	
public:
	CRMMenu(vgui::VPANEL parent);
	~CRMMenu() {};

	virtual void Activate();

protected:
	virtual void OnCommand(const char* pcCommand);
	virtual void OnTick();
	virtual void OnClose();

	Button* cancelButton;
	Button* hcButton;
	Button* ch00Button;
	Button* ch01Button;
	Button* ch02Button;
	Button* ch03Button;
	Button* ch04Button;
	Button* ch05Button;
	Button* ch06Button;
	Button* ch07Button;
	Button* ch08Button;
	Button* ch09Button;
	Button* ch10Button;
	Button* ch11Button;
	Button* ch12Button;
	//Button* ch13Button;
	Button* ch14Button;
	//Button* ch15Button;
	//Button* ch16Button;
	Button* ch17Button;
	//Button* ch18Button;
};

CRMMenu::CRMMenu(vgui::VPANEL parent) : BaseClass(NULL, "RMMenu")
{
	SetDeleteSelfOnClose(true);
	SetSizeable(false);
	MoveToCenterOfScreen();

	SetTitle("#hls_fix_rm_title", true);

	cancelButton = new Button(this, "Cancel", "#GameUI_Cancel");
	cancelButton->SetCommand("Close");
	hcButton = new Button(this, "loadhc", "Hazard Course");
	hcButton->SetCommand("loadhc");
	ch00Button = new Button(this, "load00", "Black Mesa Inbound");
	ch00Button->SetCommand("load00");
	ch01Button = new Button(this, "load01", "Anomalous Materials");
	ch01Button->SetCommand("load01");
	ch02Button = new Button(this, "load02", "Unforseen Consequences");
	ch02Button->SetCommand("load02");
	ch03Button = new Button(this, "load03", "Office Complex");
	ch03Button->SetCommand("load03");
	ch04Button = new Button(this, "load04", "We've Got Hostiles");
	ch04Button->SetCommand("load04");
	ch05Button = new Button(this, "load05", "Blast Pit");
	ch05Button->SetCommand("load05");
	ch06Button = new Button(this, "load06", "Power Up");
	ch06Button->SetCommand("load06");
	ch07Button = new Button(this, "load07", "On A Rail");
	ch07Button->SetCommand("load07");
	ch08Button = new Button(this, "load08", "Apprehension");
	ch08Button->SetCommand("load08");
	ch09Button = new Button(this, "load09", "Residue Processing");
	ch09Button->SetCommand("load09");
	ch10Button = new Button(this, "load10", "Questionable Ethics");
	ch10Button->SetCommand("load10");
	ch11Button = new Button(this, "load11", "Surface Tension");
	ch11Button->SetCommand("load11");
	ch12Button = new Button(this, "load12", "Forget About Freeman");
	ch12Button->SetCommand("load12");
	//ch13Button = new Button(this, "load13", "Lambda Core");
	//ch13Button->SetCommand("load13");
	ch14Button = new Button(this, "load14", "Xen");
	ch14Button->SetCommand("load14");
	//ch15Button = new Button(this, "load15", "Gonarch's Lair");
	//ch15Button->SetCommand("load15");
	//ch16Button = new Button(this, "load16", "Interloper");
	//ch16Button->SetCommand("load16");
	ch17Button = new Button(this, "load17", "Nihilanth");
	ch17Button->SetCommand("load17");
	//ch18Button = new Button(this, "load18", "Endgame");
	//ch18Button->SetCommand("load18");

	LoadControlSettings("resource/ui/hlsrmmenu.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CRMMenu::Activate()
{
	BaseClass::Activate();
}

void CRMMenu::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	SetVisible(false);
	Close();
	if (!Q_stricmp(pcCommand, "loadhc"))
	{
		engine->ClientCmd("map hlsu_hc");
	}
	else if (!Q_stricmp(pcCommand, "load00"))
	{
		engine->ClientCmd("map hls00amrl");
	}
	else if (!Q_stricmp(pcCommand, "load01"))
	{
		engine->ClientCmd("map hls01amrl");
	}
	else if (!Q_stricmp(pcCommand, "load02"))
	{
		engine->ClientCmd("map hls02amrl");
	}
	else if (!Q_stricmp(pcCommand, "load03"))
	{
		engine->ClientCmd("map hls03amrl");
	}
	else if (!Q_stricmp(pcCommand, "load04"))
	{
		engine->ClientCmd("map hls04amrl");
	}
	else if (!Q_stricmp(pcCommand, "load05"))
	{
		engine->ClientCmd("map hls05amrl");
	}
	else if (!Q_stricmp(pcCommand, "load06"))
	{
		engine->ClientCmd("map hls06amrl");
	}
	else if (!Q_stricmp(pcCommand, "load07"))
	{
		engine->ClientCmd("map hls07amrl");
	}
	else if (!Q_stricmp(pcCommand, "load07"))
	{
		engine->ClientCmd("map hls07amrl");
	}
	else if (!Q_stricmp(pcCommand, "load08"))
	{
		engine->ClientCmd("map hls08amrl");
	}
	else if (!Q_stricmp(pcCommand, "load09"))
	{
		engine->ClientCmd("map hls09amrl");
	}
	else if (!Q_stricmp(pcCommand, "load10"))
	{
		engine->ClientCmd("map hls10amrl");
	}
	else if (!Q_stricmp(pcCommand, "load11"))
	{
		engine->ClientCmd("map hls11amrl");
	}
	else if (!Q_stricmp(pcCommand, "load12"))
	{
		engine->ClientCmd("map hls12amrl");
	}
	//else if (!Q_stricmp(pcCommand, "load13"))
	//{
	//	engine->ClientCmd("");
	//}
	else if (!Q_stricmp(pcCommand, "load14"))
	{
		engine->ClientCmd("map hls14amrl");
	}
	//else if (!Q_stricmp(pcCommand, "load15"))
	//{
	//	engine->ClientCmd("");
	//}
	//else if (!Q_stricmp(pcCommand, "load16"))
	//{
	//	engine->ClientCmd("");
	//}
	else if (!Q_stricmp(pcCommand, "load17"))
	{
		engine->ClientCmd("map hls14cmrl");
	}
	//else if (!Q_stricmp(pcCommand, "load18"))
	//{
	//	engine->ClientCmd("");
	//}
}

void CRMMenu::OnTick()
{
	BaseClass::OnTick();
	if (engine->IsPaused() || engine->IsLevelMainMenuBackground() || !engine->IsInGame())
		SetVisible(true);
	else
		SetVisible(false);

	SetBgColor(Color(125, 125, 125, 255));
}

bool isRMActive = false;

void CRMMenu::OnClose()
{
	BaseClass::OnClose();
	isRMActive = false;
}

CRMMenu* RMMenu = NULL;

CON_COMMAND(EnableRMPanel, "Turns on the RM Panel")
{
	VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL_TOOLS);
	if (!isRMActive)
	{
		RMMenu = new CRMMenu(gameToolParent);
		isRMActive = true;
	}

	RMMenu->Activate();
}