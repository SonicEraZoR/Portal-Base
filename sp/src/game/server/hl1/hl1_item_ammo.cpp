#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "items.h"
#include "hl1_items.h"

#define AMMO_MP5_GIVE		50
#define AMMO_MP5_MODEL		"models/w_9mmARclip.mdl"

class CMP5AmmoClip : public CHL1Item
{
public:
	DECLARE_CLASS(CMP5AmmoClip, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_MP5_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel(AMMO_MP5_MODEL);
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->GiveAmmo(AMMO_MP5_GIVE, "9mmRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5clip, CMP5AmmoClip);
PRECACHE_REGISTER(ammo_mp5clip);
LINK_ENTITY_TO_CLASS(ammo_9mmar, CMP5AmmoClip);
PRECACHE_REGISTER(ammo_9mmar);

#define AMMO_MP5CHAIN_GIVE		200
#define AMMO_MP5CHAIN_MODEL		"models/w_chainammo.mdl"

class CMP5Chainammo : public CHL1Item
{
public:
	DECLARE_CLASS(CMP5Chainammo, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_MP5CHAIN_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel(AMMO_MP5CHAIN_MODEL);
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->GiveAmmo(AMMO_MP5CHAIN_GIVE, "9mmRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_9mmbox, CMP5Chainammo);
PRECACHE_REGISTER(ammo_9mmbox);

#define AMMO_MP5GRENADE_GIVE		2
#define AMMO_MP5GRENADE_MODEL		"models/w_ARgrenade.mdl"

class CMP5AmmoGrenade : public CHL1Item
{
public:
	DECLARE_CLASS(CMP5AmmoGrenade, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_MP5GRENADE_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel(AMMO_MP5GRENADE_MODEL);
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->GiveAmmo(AMMO_MP5GRENADE_GIVE, "MP5_Grenade"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5grenades, CMP5AmmoGrenade);
PRECACHE_REGISTER(ammo_mp5grenades);
LINK_ENTITY_TO_CLASS(ammo_argrenades, CMP5AmmoGrenade);
PRECACHE_REGISTER(ammo_argrenades);

class C9mmClip : public CHL1Item
{
public:
	DECLARE_CLASS(C9mmClip, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel("models/w_9mmclip.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_9mmclip.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->GiveAmmo(17, "9mmRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_glockclip, C9mmClip);
PRECACHE_REGISTER(ammo_glockclip);
LINK_ENTITY_TO_CLASS(ammo_9mmclip, C9mmClip);
PRECACHE_REGISTER(ammo_9mmclip);

class CShotgunAmmo : public CHL1Item
{
public:
	DECLARE_CLASS(CShotgunAmmo, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel("models/w_shotbox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_shotbox.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->GiveAmmo(12, "Buckshot"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_buckshot, CShotgunAmmo);
PRECACHE_REGISTER(ammo_buckshot);

class C357Ammo : public CHL1Item
{
public:
	DECLARE_CLASS(C357Ammo, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel("models/w_357ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_357ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->GiveAmmo(6, "357Round"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_357, C357Ammo);
PRECACHE_REGISTER(ammo_357);

class CCrossbowAmmo : public CHL1Item
{
public:
	DECLARE_CLASS(CCrossbowAmmo, CHL1Item);
	
	void Spawn(void)
	{
		Precache();
		SetModel("models/w_crossbow_clip.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_crossbow_clip.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->GiveAmmo(5, "XBowBolt"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_crossbow, CCrossbowAmmo);
PRECACHE_REGISTER(ammo_crossbow);

class CRocketAmmo : public CHL1Item
{
public:
	DECLARE_CLASS(CRocketAmmo, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel("models/w_rpgammo.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_rpgammo.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->GiveAmmo(1, "RPG_Rocket"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRocketAmmo);
PRECACHE_REGISTER(ammo_rpgclip);

class CGaussAmmo : public CHL1Item
{
public:
	DECLARE_CLASS(CGaussAmmo, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel("models/w_gaussammo.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/w_gaussammo.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->GiveAmmo(20, "Uranium"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_gaussclip, CGaussAmmo);
PRECACHE_REGISTER(ammo_gaussclip);