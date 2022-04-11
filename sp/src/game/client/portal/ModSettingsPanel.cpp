#include "cbase.h"
#include "IModSettingsPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/cvartogglecheckbutton.h>

ConVar cl_showmodsettingspanel("cl_showmodsettingspanel", "0", FCVAR_CLIENTDLL, "Sets the state of ModSettingsPanel <state>");

CON_COMMAND(ToggleModSettingsPanel, "Toggles ModSettingsPanel on or off")
{
	cl_showmodsettingspanel.SetValue(!cl_showmodsettingspanel.GetBool());
	modsettingspanel->Activate();
};

class CModSettingsPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CModSettingsPanel, vgui::Frame);

	CModSettingsPanel(vgui::VPANEL parent); 	// Constructor
	~CModSettingsPanel(){};				// Destructor

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);

private:
	//Other used VGUI control Elements:
	Button *m_pOKButton;
	Button *m_pCancelButton;
	Button *m_pApplyButton;
	CvarToggleCheckButton<ConVarRef> *m_pBetaQuickInfoToggle;
	CvarToggleCheckButton<ConVarRef> *m_pBetaQuickInfoDelayToggle;
	CvarToggleCheckButton<ConVarRef> *m_pBetaQuickInfoOlderGunToggle;
	CvarToggleCheckButton<ConVarRef> *m_pRegenerationEnable;
	CvarToggleCheckButton<ConVarRef> *m_pReceiveFallDamage;
	CheckButton *m_pChellModel;

};

// Constuctor: Initializes the Panel
CModSettingsPanel::CModSettingsPanel(vgui::VPANEL parent) : BaseClass(NULL, "ModSettingsPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(true);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("Resource/UI/modsettingspanel.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	m_pOKButton = dynamic_cast<Button*>( FindChildByName("ButtonOK", true) );
	m_pOKButton->AddActionSignalTarget(this);
	m_pOKButton->SetCommand("ok");
	m_pCancelButton = dynamic_cast<Button*>( FindChildByName("ButtonCancel", true) );
	m_pCancelButton->AddActionSignalTarget(this);
	m_pCancelButton->SetCommand("turnoff");
	m_pApplyButton = dynamic_cast<Button*>( FindChildByName("ButtonApply", true) );
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetCommand("apply");
	m_pBetaQuickInfoToggle = dynamic_cast<CvarToggleCheckButton<ConVarRef>*>( FindChildByName("BetaQuickInfoToggle", true) );
	m_pBetaQuickInfoToggle->SetCvarName("beta_quickinfo");
	m_pBetaQuickInfoToggle->SizeToContents();
	m_pBetaQuickInfoDelayToggle = dynamic_cast<CvarToggleCheckButton<ConVarRef>*>( FindChildByName("BetaQuickInfoDelayToggle", true) );
	m_pBetaQuickInfoDelayToggle->SetCvarName("beta_quickinfo_show_portal_delay");
	m_pBetaQuickInfoDelayToggle->SizeToContents();
	m_pBetaQuickInfoOlderGunToggle = dynamic_cast<CvarToggleCheckButton<ConVarRef>*>( FindChildByName("BetaQuickInfoOlderGunToggle", true) );
	m_pBetaQuickInfoOlderGunToggle->SetCvarName("beta_quickinfo_older_gun");
	m_pBetaQuickInfoOlderGunToggle->SizeToContents();
	m_pRegenerationEnable = dynamic_cast<CvarToggleCheckButton<ConVarRef>*>( FindChildByName("RegenerationEnable", true) );
	m_pRegenerationEnable->SetCvarName("sv_regeneration_enable");
	m_pRegenerationEnable->SizeToContents();
	m_pReceiveFallDamage = dynamic_cast<CvarToggleCheckButton<ConVarRef>*>( FindChildByName("ReceiveFallDamage", true) );
	m_pReceiveFallDamage->SetCvarName("sv_receive_fall_damage");
	m_pReceiveFallDamage->SizeToContents();
	m_pChellModel = dynamic_cast<CheckButton*>( FindChildByName("ChellModel", true) );
	ConVar *cl_playermodel = cvar->FindVar("cl_playermodel");
	if (Q_strcmp(cl_playermodel->GetString(), "models/player/chell.mdl") == 0)
		m_pChellModel->SetSelected(true);
	else
		m_pChellModel->SetSelected(false);

	DevMsg("ModSettingsPanel has been constructed\n");
}

//Class: Used for construction.
class CModSettingsPanelInterface : public IModSettingsPanel
{
private:
	CModSettingsPanel *ModSettingsPanel;
public:
	CModSettingsPanelInterface()
	{
		ModSettingsPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		ModSettingsPanel = new CModSettingsPanel(parent);
	}
	void Activate(void)
	{
		if (ModSettingsPanel)
		{
			ModSettingsPanel->SetPos((ScreenWidth() - ModSettingsPanel->GetWide()) / 2, (ScreenHeight() - ModSettingsPanel->GetTall()) / 2);
			ModSettingsPanel->Activate();
		}
	}

	void Destroy(void)
	{
		if (ModSettingsPanel)
		{
			ModSettingsPanel->SetParent((vgui::Panel *)NULL);
			delete ModSettingsPanel;
		}
	}

};

static CModSettingsPanelInterface g_ModSettingsPanel;
IModSettingsPanel* modsettingspanel = (IModSettingsPanel*)&g_ModSettingsPanel;

void CModSettingsPanel::OnTick()
{
	BaseClass::OnTick();
	SetVisible(cl_showmodsettingspanel.GetBool()); // 1 BY DEFAULT
}

void CModSettingsPanel::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showmodsettingspanel.SetValue(0);
	if (!Q_stricmp(pcCommand, "apply"))
	{
		m_pBetaQuickInfoToggle->ApplyChanges();
		m_pBetaQuickInfoDelayToggle->ApplyChanges();
		m_pBetaQuickInfoOlderGunToggle->ApplyChanges();
		m_pRegenerationEnable->ApplyChanges();
		m_pReceiveFallDamage->ApplyChanges();
		if (!m_pChellModel->IsDepressed())
			engine->ClientCmd("cl_playermodel \"none\"");
		else
			engine->ClientCmd("cl_playermodel \"models/player/chell.mdl\"");
	}
	if (!Q_stricmp(pcCommand, "ok"))
	{
		m_pBetaQuickInfoToggle->ApplyChanges();
		m_pBetaQuickInfoDelayToggle->ApplyChanges();
		m_pBetaQuickInfoOlderGunToggle->ApplyChanges();
		m_pRegenerationEnable->ApplyChanges();
		m_pReceiveFallDamage->ApplyChanges();
		if (!m_pChellModel->IsDepressed())
			engine->ClientCmd("cl_playermodel \"none\"");
		else
			engine->ClientCmd("cl_playermodel \"models/player/chell.mdl\"");
		cl_showmodsettingspanel.SetValue(0);
	}	
}
