"GameMenu"
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"InGameOrder" "10"
		"OnlyInGame" "1"
	}
	"5"	
	{
		"label" "#GameUI_GameMenu_NewGame"
		"command" "OpenNewGameDialog"
		"InGameOrder" "20"
		"notmulti" "1"
	}
	"6"
	{
		"label" "#GameUI_GameMenu_LoadGame"
		"command" "OpenLoadGameDialog"
		"InGameOrder" "30"
		"notmulti" "1"
	}
	"7"
	{
		"label" "#GameUI_GameMenu_SaveGame"
		"command" "OpenSaveGameDialog"
		"InGameOrder" "40"
		"notmulti" "1"
		"OnlyInGame" "1"
	}
	"7_5"
	{
		"label" "#GameUI_GameMenu_ActivateVR"
		"command" "engine vr_activate"
		"InGameOrder" "50"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRInactive" "1"
	}
	"7_6"
	{
		"label" "#GameUI_GameMenu_DeactivateVR"
		"command" "engine vr_deactivate"
		"InGameOrder" "60"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRActive" "1"
	}
	"8"
	{
		"label" "#GameUI_Controller"
		"command" "OpenControllerDialog"
		"InGameOrder" "70"
		"ConsoleOnly" "1"
	}
	"9"
	{
		"label" "#GameUI_GameMenu_ModOptions"
		"command" "engine ToggleModSettingsPanel"
		"InGameOrder" "80"
	}
	"10"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
		"InGameOrder" "90"
	}
	"11"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "Quit"
		"InGameOrder" "100"
	}
}
