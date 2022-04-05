#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "engine/IEngineSound.h"
#include "hl1_items.h"
#include "in_buttons.h"

ConVar	sk_healthkit("sk_healthkit", "0");

class CHealthKit : public CHL1Item
{
	DECLARE_CLASS(CHealthKit, CHL1Item);

	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer *pPlayer);
};

LINK_ENTITY_TO_CLASS(item_healthkit, CHealthKit);
PRECACHE_REGISTER(item_healthkit);

void CHealthKit::Spawn(void)
{
	Precache();
	SetModel("models/w_medkit.mdl");

	CHL1Item::Spawn();
}

void CHealthKit::Precache(void)
{
	PrecacheModel("models/w_medkit.mdl");
	PrecacheScriptSound("HealthKit.Touch");
}

bool CHealthKit::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer->TakeHealth(sk_healthkit.GetFloat(), DMG_GENERIC))
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING(GetClassname());
		MessageEnd();

		CPASAttenuationFilter filter(pPlayer, "HealthKit.Touch");
		EmitSound(filter, pPlayer->entindex(), "HealthKit.Touch");

		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return true;
	}

	return false;
}