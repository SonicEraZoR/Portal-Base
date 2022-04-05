#include "cbase.h"
#include "usermessages.h"
#include "shake.h"
#include "voice_gamemgr.h"

void RegisterUserMessages(void)
{
	usermessages->Register("Geiger", 1);
	usermessages->Register("Train", 1);
	usermessages->Register("HudText", -1);
	usermessages->Register("SayText", -1);
	usermessages->Register("SayText2", -1);
	usermessages->Register("TextMsg", -1);
	usermessages->Register("HudMsg", -1);
	usermessages->Register("ResetHUD", 1);
	usermessages->Register("GameTitle", 0);
	usermessages->Register("ItemPickup", -1);
	usermessages->Register("ShowMenu", -1);
	usermessages->Register("Shake", 13);
	usermessages->Register("Fade", 10);
	usermessages->Register("VGUIMenu", -1);
	usermessages->Register("Rumble", 3);
	usermessages->Register("Battery", 2);
	usermessages->Register("Damage", 18);
	usermessages->Register("VoiceMask", VOICE_MAX_PLAYERS_DW * 4 * 2 + 1);
	usermessages->Register("RequestState", 0);
	usermessages->Register("CloseCaption", -1);
	usermessages->Register("HintText", -1);
	usermessages->Register("KeyHintText", -1);
	usermessages->Register("AmmoDenied", 2);
	usermessages->Register("SPHapWeapEvent", 4);
	usermessages->Register("HapDmg", -1);
	usermessages->Register("HapPunch", -1);
	usermessages->Register("HapSetDrag", -1);
	usermessages->Register("HapSetConst", -1);
	usermessages->Register("HapMeleeContact", 0);
}