#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "hl1_items.h"
#include "hl1_player.h"

class CHL1Longjump : public CHL1Item
{
	DECLARE_CLASS(CHL1Longjump, CHL1Item);
public:
	void Spawn(void)
	{
		Precache();
		SetModel("models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_longjump.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CHL1_Player *pHL1Player = (CHL1_Player*)pPlayer;

		if (pHL1Player->m_bHasLongJump == true)
		{
			return false;
		}

		if (pHL1Player->IsSuitEquipped())
		{
			pHL1Player->m_bHasLongJump = true;

			CSingleUserRecipientFilter user(pHL1Player);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			WRITE_STRING(STRING(m_iClassname));
			MessageEnd();

			UTIL_EmitSoundSuit(pHL1Player->edict(), "!HEV_A1");
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CHL1Longjump);
PRECACHE_REGISTER(item_longjump);