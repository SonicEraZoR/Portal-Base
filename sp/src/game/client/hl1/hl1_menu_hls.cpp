#include "cbase.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/Button.h>
#include "ienginevgui.h"

extern ConVar hl1_mp5_recoil;
extern ConVar hl1_ragdoll_gib;
extern ConVar hl1_bullsquid_spit;
extern ConVar hl1_bigmomma_splash;
extern ConVar hl1_movement;
extern ConVar hl1_move_sounds;

//Crowbar Sounds
extern ConVar hl1_crowbar_sound;
extern ConVar hl1_crowbar_concrete;
extern ConVar hl1_crowbar_metal;
extern ConVar hl1_crowbar_dirt;
extern ConVar hl1_crowbar_vent;
extern ConVar hl1_crowbar_grate;
extern ConVar hl1_crowbar_tile;
extern ConVar hl1_crowbar_wood;
extern ConVar hl1_crowbar_glass;
extern ConVar hl1_crowbar_computer;

extern ConVar hl1_crowbar_concrete_vol;
extern ConVar hl1_crowbar_metal_vol;
extern ConVar hl1_crowbar_dirt_vol;
extern ConVar hl1_crowbar_vent_vol;
extern ConVar hl1_crowbar_grate_vol;
extern ConVar hl1_crowbar_tile_vol;
extern ConVar hl1_crowbar_wood_vol;
extern ConVar hl1_crowbar_glass_vol;
extern ConVar hl1_crowbar_computer_vol;

class CSubOptionsHLS : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CSubOptionsHLS, vgui::PropertyPage);

public:
	CSubOptionsHLS(vgui::Panel* parent);
	~CSubOptionsHLS() {}

	virtual void OnApplyChanges();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

	CheckButton* recoilButton;
	CheckButton* ragdollButton;
	CheckButton* bullsquidButton;
	CheckButton* bigmommaButton;
	CheckButton* movementButton;
	CheckButton* moveSoundButton;
};

CSubOptionsHLS::CSubOptionsHLS(vgui::Panel* parent) : PropertyPage(parent, NULL)
{
	recoilButton = new CheckButton(this, "RecoilButton", "Enable hl1 recoil for mp5");
	recoilButton->SetSelected(hl1_mp5_recoil.GetBool());

	ragdollButton = new CheckButton(this, "RagdollButton", "Enable ragdoll gibbing");
	ragdollButton->SetSelected(hl1_ragdoll_gib.GetBool());

	bullsquidButton = new CheckButton(this, "BullSquidButton", "Use hl1 texture for Bullsquid spit");
	bullsquidButton->SetSelected(hl1_bullsquid_spit.GetBool());

	bigmommaButton = new CheckButton(this, "BigMommaButton", "Use hl1 texture for Gonarch splash");
	bigmommaButton->SetSelected(hl1_bigmomma_splash.GetBool());

	movementButton = new CheckButton(this, "MovementButton", "Use hl1 style movement.");
	movementButton->SetSelected(hl1_movement.GetBool());

	moveSoundButton = new CheckButton(this, "MoveSoundButton", "Use hl1 movement sounds.");
	moveSoundButton->SetSelected(hl1_move_sounds.GetBool());

	LoadControlSettings("resource/ui/hlssubmenu.res");
}

void CSubOptionsHLS::OnApplyChanges()
{
	hl1_mp5_recoil.SetValue(recoilButton->IsSelected());
	hl1_ragdoll_gib.SetValue(ragdollButton->IsSelected());
	hl1_bullsquid_spit.SetValue(bullsquidButton->IsSelected());
	hl1_bigmomma_splash.SetValue(bigmommaButton->IsSelected());
	hl1_movement.SetValue(movementButton->IsSelected());
	hl1_move_sounds.SetValue(moveSoundButton->IsSelected());
}

void CSubOptionsHLS::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(Color(125, 125, 125, 255));
}

void CSubOptionsHLS::OnCheckButtonChecked(Panel* panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

class CSubOptionsCrowbar : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CSubOptionsCrowbar, vgui::PropertyPage);

public:
	CSubOptionsCrowbar(vgui::Panel* parent);
	~CSubOptionsCrowbar() {}

	void Reset();
	virtual void OnApplyChanges();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
	virtual void OnCommand(const char* pcCommand);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

	CheckButton* crowSoundButton;
	CheckButton* crowConcreteButton;
	CheckButton* crowMetalButton;
	CheckButton* crowDirtButton;
	CheckButton* crowVentButton;
	CheckButton* crowGrateButton;
	CheckButton* crowTileButton;
	CheckButton* crowWoodButton;
	CheckButton* crowGlassButton;
	CheckButton* crowComputerButton;

	Slider* crowConcreteSlider;
	Slider* crowMetalSlider;
	Slider* crowDirtSlider;
	Slider* crowVentSlider;
	Slider* crowGrateSlider;
	Slider* crowTileSlider;
	Slider* crowWoodSlider;
	Slider* crowGlassSlider;
	Slider* crowComputerSlider;

	Button* defaultsButton;
};

CSubOptionsCrowbar::CSubOptionsCrowbar(vgui::Panel* parent) : PropertyPage(parent, NULL)
{
	crowSoundButton = new CheckButton(this, "CrowSoundButton", "Enable HL1 Crowbar Sounds");
	crowSoundButton->SetSelected(hl1_crowbar_sound.GetBool());
	crowConcreteButton = new CheckButton(this, "CrowConcreteButton", "Enable HL1 Crowbar Sound On Concrete");
	crowConcreteButton->SetSelected(hl1_crowbar_concrete.GetBool());
	crowMetalButton = new CheckButton(this, "CrowMetalButton", "Enable HL1 Crowbar Sound On Metal");
	crowMetalButton->SetSelected(hl1_crowbar_metal.GetBool());
	crowDirtButton = new CheckButton(this, "CrowDirtButton", "Enable HL1 Crowbar Sound On Dirt");
	crowDirtButton->SetSelected(hl1_crowbar_dirt.GetBool());
	crowVentButton = new CheckButton(this, "CrowVentButton", "Enable HL1 Crowbar Sound On Vents");
	crowVentButton->SetSelected(hl1_crowbar_vent.GetBool());
	crowGrateButton = new CheckButton(this, "CrowGrateButton", "Enable HL1 Crowbar Sound On Grates");
	crowGrateButton->SetSelected(hl1_crowbar_grate.GetBool());
	crowTileButton = new CheckButton(this, "CrowTileButton", "Enable HL1 Crowbar Sound On Tile");
	crowTileButton->SetSelected(hl1_crowbar_tile.GetBool());
	crowWoodButton = new CheckButton(this, "CrowWoodButton", "Enable HL1 Crowbar Sound On Wood");
	crowWoodButton->SetSelected(hl1_crowbar_wood.GetBool());
	crowGlassButton = new CheckButton(this, "CrowGlassButton", "Enable HL1 Crowbar Sound On Glass");
	crowGlassButton->SetSelected(hl1_crowbar_glass.GetBool());
	crowComputerButton = new CheckButton(this, "CrowComputerButton", "Enable HL1 Crowbar Sound On Computers");
	crowComputerButton->SetSelected(hl1_crowbar_computer.GetBool());
	
	crowConcreteSlider = new Slider(this, "CrowConcreteSlider");
	crowConcreteSlider->SetRange(1, 10);
	crowConcreteSlider->SetNumTicks(10);
	crowConcreteSlider->SetValue(hl1_crowbar_concrete_vol.GetInt());

	crowMetalSlider = new Slider(this, "CrowMetalSlider");
	crowMetalSlider->SetRange(1, 10);
	crowMetalSlider->SetNumTicks(10);
	crowMetalSlider->SetValue(hl1_crowbar_metal_vol.GetInt());

	crowDirtSlider = new Slider(this, "CrowDirtSlider");
	crowDirtSlider->SetRange(1, 10);
	crowDirtSlider->SetNumTicks(10);
	crowDirtSlider->SetValue(hl1_crowbar_dirt_vol.GetInt());

	crowVentSlider = new Slider(this, "CrowVentSlider");
	crowVentSlider->SetRange(1, 10);
	crowVentSlider->SetNumTicks(10);
	crowVentSlider->SetValue(hl1_crowbar_vent_vol.GetInt());

	crowGrateSlider = new Slider(this, "CrowGrateSlider");
	crowGrateSlider->SetRange(1, 10);
	crowGrateSlider->SetNumTicks(10);
	crowGrateSlider->SetValue(hl1_crowbar_grate_vol.GetInt());

	crowTileSlider = new Slider(this, "CrowTileSlider");
	crowTileSlider->SetRange(1, 10);
	crowTileSlider->SetNumTicks(10);
	crowTileSlider->SetValue(hl1_crowbar_tile_vol.GetInt());

	crowWoodSlider = new Slider(this, "CrowWoodSlider");
	crowWoodSlider->SetRange(1, 10);
	crowWoodSlider->SetNumTicks(10);
	crowWoodSlider->SetValue(hl1_crowbar_wood_vol.GetInt());

	crowGlassSlider = new Slider(this, "CrowGlassSlider");
	crowGlassSlider->SetRange(1, 10);
	crowGlassSlider->SetNumTicks(10);
	crowGlassSlider->SetValue(hl1_crowbar_glass_vol.GetInt());

	crowComputerSlider = new Slider(this, "CrowComputerSlider");
	crowComputerSlider->SetRange(1, 10);
	crowComputerSlider->SetNumTicks(10);
	crowComputerSlider->SetValue(hl1_crowbar_computer_vol.GetInt());

	defaultsButton = new Button(this, "reset", "Defaults");
	defaultsButton->SetCommand("reset");
	
	LoadControlSettings("resource/ui/hlssubcrowbarmenu.res");
}

void CSubOptionsCrowbar::OnApplyChanges()
{
	hl1_crowbar_sound.SetValue(crowSoundButton->IsSelected());
	if (crowSoundButton->IsSelected())
	{
		hl1_crowbar_concrete.SetValue(crowConcreteButton->IsSelected());
		hl1_crowbar_metal.SetValue(crowMetalButton->IsSelected());
		hl1_crowbar_dirt.SetValue(crowDirtButton->IsSelected());
		hl1_crowbar_vent.SetValue(crowVentButton->IsSelected());
		hl1_crowbar_grate.SetValue(crowGrateButton->IsSelected());
		hl1_crowbar_tile.SetValue(crowTileButton->IsSelected());
		hl1_crowbar_wood.SetValue(crowWoodButton->IsSelected());
		hl1_crowbar_glass.SetValue(crowGlassButton->IsSelected());
		hl1_crowbar_computer.SetValue(crowComputerButton->IsSelected());
		
		hl1_crowbar_concrete_vol.SetValue(crowConcreteSlider->GetValue());
		hl1_crowbar_metal_vol.SetValue(crowMetalSlider->GetValue());
		hl1_crowbar_dirt_vol.SetValue(crowDirtSlider->GetValue());
		hl1_crowbar_vent_vol.SetValue(crowVentSlider->GetValue());
		hl1_crowbar_grate_vol.SetValue(crowGrateSlider->GetValue());
		hl1_crowbar_tile_vol.SetValue(crowTileSlider->GetValue());
		hl1_crowbar_wood_vol.SetValue(crowWoodSlider->GetValue());
		hl1_crowbar_glass_vol.SetValue(crowGlassSlider->GetValue());
		hl1_crowbar_computer_vol.SetValue(crowComputerSlider->GetValue());
		
	}
}

void CSubOptionsCrowbar::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(Color(125, 125, 125, 255));
}

void CSubOptionsCrowbar::OnCheckButtonChecked(Panel* panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	if (crowSoundButton->IsSelected())
	{
		crowConcreteButton->SetEnabled(true);
		crowMetalButton->SetEnabled(true);
		crowDirtButton->SetEnabled(true);
		crowVentButton->SetEnabled(true);
		crowGrateButton->SetEnabled(true);
		crowTileButton->SetEnabled(true);
		crowWoodButton->SetEnabled(true);
		crowGlassButton->SetEnabled(true);
		crowComputerButton->SetEnabled(true);

		crowConcreteSlider->SetEnabled(true);
		crowMetalSlider->SetEnabled(true);
		crowDirtSlider->SetEnabled(true);
		crowVentSlider->SetEnabled(true);
		crowGrateSlider->SetEnabled(true);
		crowTileSlider->SetEnabled(true);
		crowWoodSlider->SetEnabled(true);
		crowGlassSlider->SetEnabled(true);
		crowComputerSlider->SetEnabled(true);
	}
	else
	{
		crowConcreteButton->SetEnabled(false);
		crowMetalButton->SetEnabled(false);
		crowDirtButton->SetEnabled(false);
		crowVentButton->SetEnabled(false);
		crowGrateButton->SetEnabled(false);
		crowTileButton->SetEnabled(false);
		crowWoodButton->SetEnabled(false);
		crowGlassButton->SetEnabled(false);
		crowComputerButton->SetEnabled(false);

		crowConcreteSlider->SetEnabled(false);
		crowMetalSlider->SetEnabled(false);
		crowDirtSlider->SetEnabled(false);
		crowVentSlider->SetEnabled(false);
		crowGrateSlider->SetEnabled(false);
		crowTileSlider->SetEnabled(false);
		crowWoodSlider->SetEnabled(false);
		crowGlassSlider->SetEnabled(false);
		crowComputerSlider->SetEnabled(false);
	}

	if (crowConcreteButton->IsSelected())
		crowConcreteSlider->SetEnabled(true);
	else
		crowConcreteSlider->SetEnabled(false);

	if (crowMetalButton->IsSelected())
		crowMetalSlider->SetEnabled(true);
	else
		crowMetalSlider->SetEnabled(false);

	if (crowDirtButton->IsSelected())
		crowDirtSlider->SetEnabled(true);
	else
		crowDirtSlider->SetEnabled(false);

	if (crowVentButton->IsSelected())
		crowVentSlider->SetEnabled(true);
	else
		crowVentSlider->SetEnabled(false);

	if (crowGrateButton->IsSelected())
		crowGrateSlider->SetEnabled(true);
	else
		crowGrateSlider->SetEnabled(false);

	if (crowTileButton->IsSelected())
		crowTileSlider->SetEnabled(true);
	else
		crowTileSlider->SetEnabled(false);

	if (crowWoodButton->IsSelected())
		crowWoodSlider->SetEnabled(true);
	else
		crowWoodSlider->SetEnabled(false);

	if (crowGlassButton->IsSelected())
		crowGlassSlider->SetEnabled(true);
	else
		crowGlassSlider->SetEnabled(false);

	if (crowComputerButton->IsSelected())
		crowComputerSlider->SetEnabled(true);
	else
		crowComputerSlider->SetEnabled(false);
}

void CSubOptionsCrowbar::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "reset"))
	{
		crowSoundButton->SetSelected(true);
		crowConcreteButton->SetSelected(true);
		crowMetalButton->SetSelected(true);
		crowDirtButton->SetSelected(false);
		crowVentButton->SetSelected(true);
		crowGrateButton->SetSelected(true);
		crowTileButton->SetSelected(false);
		crowWoodButton->SetSelected(false);
		crowGlassButton->SetSelected(false);
		crowComputerButton->SetSelected(false);

		crowConcreteSlider->SetValue(4);
		crowMetalSlider->SetValue(6);
		crowDirtSlider->SetValue(1);
		crowVentSlider->SetValue(3);
		crowGrateSlider->SetValue(5);
		crowTileSlider->SetValue(2);
		crowWoodSlider->SetValue(2);
		crowGlassSlider->SetValue(2);
		crowComputerSlider->SetValue(2);
	}
}

class CHLSMenu : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CHLSMenu, vgui::PropertyDialog);

public:
	CHLSMenu(vgui::VPANEL parent);
	~CHLSMenu() {}

	virtual void Activate();

protected:
	virtual void OnTick();
	virtual void OnClose();

private:
	CSubOptionsHLS* m_pOptionsSubHLS;
	CSubOptionsCrowbar* m_pOptionsSubCrowbar;
};

CHLSMenu::CHLSMenu(vgui::VPANEL parent) : BaseClass(NULL, "HLSMenu")
{
	SetDeleteSelfOnClose(true);
	SetBounds(0, 0, 512, 406);
	SetSizeable(false);
	MoveToCenterOfScreen();

	SetTitle("#hls_fix_hls_title", true);

	m_pOptionsSubHLS = new CSubOptionsHLS(this);
	AddPage(m_pOptionsSubHLS, "#hls_fix_sub_hls");
	m_pOptionsSubCrowbar = new CSubOptionsCrowbar(this);
	AddPage(m_pOptionsSubCrowbar, "#hls_fix_sub_crowbar");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(84);
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CHLSMenu::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);
}

void CHLSMenu::OnTick()
{
	BaseClass::OnTick();
	if (engine->IsPaused() || engine->IsLevelMainMenuBackground() || !engine->IsInGame())
		SetVisible(true);
	else
		SetVisible(false);
}

bool isHLSActive = false;

void CHLSMenu::OnClose()
{
	BaseClass::OnClose();
	isHLSActive = false;
}

CHLSMenu* HLSMenu = NULL;

CON_COMMAND(EnableHLSPanel, "Turns on the HLS Panel")
{
	VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL_TOOLS);
	if (!isHLSActive)
	{
		HLSMenu = new CHLSMenu(gameToolParent);
		isHLSActive = true;
	}

	HLSMenu->Activate();
}