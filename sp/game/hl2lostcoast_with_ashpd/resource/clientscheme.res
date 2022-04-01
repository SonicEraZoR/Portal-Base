///////////////////////////////////////////////////////////
// Tracker scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
///////////////////////////////////////////////////////////
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	// color details
	// this is a list of all the colors used by the scheme
	Colors
	{
	}
	
	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"FgColor"			"255 220 0 100"
		"FgColor_vrmode"	"255 220 0 200"
		"BgColor"			"0 0 0 76"

		"Panel.FgColor"			"255 220 0 100"
		"Panel.BgColor"			"0 0 0 76"
		
		"BrightFg"		"255 220 0 255"

		"DamagedBg"			"180 0 0 200"
		"DamagedFg"			"180 0 0 230"
		"BrightDamagedFg"		"255 0 0 255"

		// weapon selection colors
		"SelectionNumberFg"		"255 220 0 255"
		"SelectionTextFg"		"255 220 0 255"
		"SelectionEmptyBoxBg" 	"0 0 0 80"
		"SelectionBoxBg" 		"0 0 0 80"
		"SelectionSelectedBoxBg" "0 0 0 80"
		
		"ZoomReticleColor"	"255 220 0 255"

		// HL1-style HUD colors
		"Yellowish"			"255 160 0 255"
		"Normal"			"255 208 64 255"
		"Caution"			"255 48 0 255"

		// Top-left corner of the "Half-Life 2" on the main screen
		"Main.Title1.X"			"53"
		"Main.Title1.Y"			"170"
		"Main.Title1.Y_hidef"	"184"
		"Main.Title1.Color"	"255 255 255 255"

		// Top-left corner of secondary title e.g. "DEMO" on the main screen
		"Main.Title2.X"				"291"
		"Main.Title2.Y"				"207"
		"Main.Title2.Y_hidef"		"242"
		"Main.Title2.Color"	"255 255 255 200"

		// Top-left corner of the menu on the main screen
		"Main.Menu.X"			"53"
		"Main.Menu.X_hidef"		"76"
		"Main.Menu.Y"			"240"

		// Blank space to leave beneath the menu on the main screen
		"Main.BottomBorder"	"32"

		// Deck colors
		"SteamDeckLoadingBar"			"250 128 20 255"
		"SteamDeckSpinner"				"201 100 0 255"
		"SteamDeckLoadingText"			"181 179 175 255"
	}

	//////////////////////// BITMAP FONT FILES /////////////////////////////
	//
	// Bitmap Fonts are ****VERY*** expensive static memory resources so they are purposely sparse
	BitmapFontFiles
	{
		// UI buttons, custom font, (256x64)
		"Buttons"		"materials/vgui/fonts/buttons_32.vbf"
	}

	
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		// fonts are used in order that they are listed
		// fonts are used in order that they are listed
		"DebugFixed"
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"antialias" "1"
			}
		}
		// fonts are used in order that they are listed
		"DebugFixedSmall"
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"antialias" "1"
			}
		}
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		Default
		{
			"1"	[$X360]
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
			}
			"1"	[$WIN32]
			{
				"name"		"Verdana"
				"tall"		"16" [$DECK]
				"tall"		"9"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"22" [$DECK]
				"tall"		"12" [!$LINUX]
				"tall"		"16" [$LINUX]
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"26" [$DECK]
				"tall"		"14" [!$LINUX]
				"tall"		"19" [$LINUX]
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"30" [$DECK]
				"tall"		"20" [!$LINUX]
				"tall"		"24" [$LINUX]
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5" // Proportional - Josh
			{
				"name"		"Verdana"
				"tall"		"14" [$DECK]
				"tall"		"9" [!$LINUX]
				"tall"		"11" [$LINUX]
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5" // Proportional - Josh
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Arial"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"  // Proportional - Josh
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Verdana"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
			"7"
			{
				"name"		"Arial"
				"tall"		"11"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		WeaponIcons
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"70" [$DECK]
				"tall"		"64"
				"tall_hidef"	"58"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		WeaponIconsSelected
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"70" [$DECK]
				"tall"		"64"
				"tall_hidef"	"58"
				"weight"	"0"
				"antialias" "1"
				"blur"		"5"
				"scanlines"	"2"
				"additive"	"1"
				"custom"	"1"
			}
		}
		WeaponIconsSmall
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"36" [$DECK]
				"tall"		"32"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		FlashlightDeck
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"46"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		Crosshairs
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"40" [!$OSX]
				"tall"		"41" [$OSX]
				"weight"	"0"
				"antialias" "0"
				"additive"	"1"
				"custom"	"1"
				"yres"		"1 1599" [!$DECK]
				"yres"		"1 1439" [$DECK]
			}
			"2"
			{
				"name"		"HalfLife2"
				"tall"		"80"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
				"yres"		"1600 3199" [!$DECK]
				"yres"		"1440 3199" [$DECK]
			}
			"3"
			{
				"name"		"HalfLife2"
				"tall"		"120"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
				"yres"		"3200 4799"
			}
			"4"
			{
				"name"		"HalfLife2"
				"tall"		"17"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		QuickInfo
		{
			"1"	[$X360]
			{
				"name"		"HL2cross"
				"tall"		"57"
				"weight"	"0"
				"antialias" 	"1"
				"additive"	"1"
				"custom"	"1"
			}
			"1"	[$WIN32]
			{
				"name"		"HL2cross"
				"tall"		"36" [$DECK]
				"tall"		"28" [!$OSX]
				"tall"		"50" [$OSX]
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1" [!$OSX]
			}
		}
		HudNumbers
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"	[!$DECK]
				"tall"		"40"	[$DECK]
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		SquadIcon	[$X360]
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"50"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbersGlow
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"	[!$DECK]
				"tall"		"40"	[$DECK]
				"weight"	"0"
				"blur"		"4"
				"scanlines" "2"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbersSmall
		{
			"1"
			{
				"name"		"HalfLife2" [!$OSX]
				"name"		"Helvetica Bold" [$OSX]
				"tall"		"16"	[!$DECK]
				"tall"		"26"	[$DECK]
				"weight"	"1000"
				"additive"	"1"
				"antialias" "1"
				"custom"	"1"
			}
		}
		HudSelectionNumbers
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"16" [$DECK] 
				"tall"		"11"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextLarge
		{
			"1"	[$X360]
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"1.0"
				"scaley"	"1.0"
			}
			"1"	[$WIN32]
			{
				"name"		"Verdana" [!$OSX]
				"name"		"Helvetica Bold" [$OSX]
				"tall"		"22" [$DECK]
				"tall"		"14"
				"weight"	"1000"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextSmall
		{
			"1"	[$WIN32]
			{
				"name"		"Verdana" [!$OSX]
				"name"		"Helvetica" [$OSX]
				"tall"		"18" [$DECK]
				"tall"		"11"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
			}
			"1"	[$X360]
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudSelectionText
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"10" [$DECK]
				"tall"		"8"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
				"additive"	"1"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"14" [$DECK]
				"tall"		"10"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
				"additive"	"1"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"18" [$DECK]
				"tall"		"16" [$LINUX]
				"tall"		"12"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
				"additive"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"22" [$DECK]
				"tall"		"20" [$LINUX]
				"tall"		"16"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
				"additive"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"9" [$DECK]
				"tall"		"8" [$LINUX]
				"tall"		"7"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		GameUIButtons
		{
			"1"	[$X360]
			{
				"bitmap"	"1"
				"name"		"Buttons"
				"scalex"	"0.63"
				"scaley"	"0.63"
				"scalex_hidef"	"1.0"
				"scaley_hidef"	"1.0"
			}
		}
		BudgetLabel
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
		}
		DebugOverlay
		{
			"1"	[$WIN32]
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
			"1"	[$X360]
			{
				"name"		"Tahoma"
				"tall"		"18"
				"weight"	"200"
				"outline"	"1"
			}
		}
		"CloseCaption_Normal"
		{
			"1"
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana" [$OSX]
				"tall"		"15" [$DECK]
				"tall"		"12"
				"weight"	"500"
				"antialias"	"1"
			}
		}
		"CloseCaption_Italic"
		{
			"1"
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana Italic" [$OSX]
				"tall"		"15" [$DECK]
				"tall"		"12"
				"weight"	"500"
				"italic"	"1"
				"antialias"	"1"
			}
		}
		"CloseCaption_Bold"
		{
			"1"
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana Bold" [$OSX]
				"tall"		"15" [$DECK]
				"tall"		"12"
				"weight"	"900"
				"antialias"	"1"
			}
		}
		"CloseCaption_BoldItalic"
		{
			"1"
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana Bold Italic" [$OSX]
				"tall"		"15" [$DECK]
				"tall"		"12"
				"weight"	"900"
				"italic"	"1"
				"antialias"	"1"
			}
		}
		"CloseCaption_Small"
		{
			"1"
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana" [$OSX]
				"tall"		"15" [$DECK]
				"tall"		"12"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"antialias"	"1"
			}
		}
		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"14"
				"weight"	"0"
				"symbol"	"1"
			}
		}
		"Trebuchet24"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"900"
				"range"		"0x0000 0x007F"	//	Basic Latin
				"antialias" "1"
				"additive"	"1"
			}
		}
		"Trebuchet18"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"18"
				"weight"	"900"
			}
		}
		ClientTitleFont
		{
			"1"
			{
				"name"  "LostCoast"
				"tall"			"32"
				"tall_hidef"	"46"
				"weight" "0"
				"additive" "0"
				"antialias" "1"
				"custom"	"1" [$OSX]
			}
		}
		CreditsLogo
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"34"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsIcons
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"34"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsText
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
				"yres"	"480 899"
			}
			"2"
			{
				"name"		"Trebuchet MS"
				"tall"		"12"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		CreditsOutroLogos
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"34"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsOutroValve
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"48"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsOutroText
		{
			"1"
			{
				"name"		"Verdana" [!$OSX]
				"name"		"Courier Bold" [$OSX]
				"tall"		"16"
				"weight"	"900"
				"antialias" "1"
			}
		}
		CenterPrintText
		{
			// note that this scales with the screen resolution
			"1"
			{
				"name"		"Trebuchet MS" [!$OSX]
				"name"		"Helvetica" [$OSX]
				"tall"		"18"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HDRDemoText
		{
			// note that this scales with the screen resolution
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"24"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		"AchievementNotification"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"14"
				"weight"	"900"
				"antialias" "1"
			}
		}
		"CommentaryDefault"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"	[$WIN32]
				"tall"		"20"	[$X360]
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"900"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1200 6000"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Verdana"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"900"
			}
			"7"
			{
				"name"		"Arial"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"800"
			}
			
		}
		"SteamDeckLoadingText"
		{
			"7"
			{
				"name"		"Alte DIN 1451 Mittelschrift"
				"tall"		"24"
				"weight"	"800"
			}
		}
	}

	
	//////////////////////// CUSTOM FONT FILES /////////////////////////////
	//
	// specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
	CustomFontFiles
	{
		"1"		"resource/HALFLIFE2.ttf"
		"2"		"resource/HL2crosshairs.ttf"
		"1"		"resource/LostCoast.ttf"
		"4"		"resource/linux_fonts/DejaVuSans.ttf"
		"5"		"resource/linux_fonts/DejaVuSans-Bold.ttf"
		"6"		"resource/linux_fonts/DejaVuSans-BoldOblique.ttf"
		"7"		"resource/linux_fonts/DejaVuSans-Oblique.ttf"
		"8"		"resource/linux_fonts/LiberationSans-Regular.ttf"
		"9"		"resource/linux_fonts/LiberationSans-Bold.ttf"
		"10"	"resource/linux_fonts/LiberationMono-Regular.ttf"
		"11"	"gamepadui/fonts/din1451alt.ttf"
	}

}
