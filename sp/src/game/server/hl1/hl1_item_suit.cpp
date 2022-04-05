#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "hl1_items.h"

#define SF_SHORT_LOGON	0x0001

class CHL1Suit : public CHL1Item
{
	DECLARE_CLASS(CHL1Suit, CHL1Item);
public:
	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer *pPlayer);
};

LINK_ENTITY_TO_CLASS(item_suit, CHL1Suit);
PRECACHE_REGISTER(item_suit);

void CHL1Suit::Spawn(void)
{
	Precache();
	SetModel("models/w_suit.mdl");
	BaseClass::Spawn();

	CollisionProp()->UseTriggerBounds(true, 12.0f);
}

void CHL1Suit::Precache(void)
{
	PrecacheModel("models/w_suit.mdl");
}

bool CHL1Suit::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer->IsSuitEquipped())
		return false;

	if (HasSpawnFlags(SF_SHORT_LOGON))
		UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_A0");
	else
		UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_AAx");

	pPlayer->EquipSuit();
	return true;
}