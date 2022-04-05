#include "cbase.h"
#include "eventqueue.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "actanimating.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"

#define XEN_PLANT_GLOW_SPRITE		"sprites/flare3.spr"
#define XEN_PLANT_HIDE_TIME			5

class CXenPLight : public CActAnimating
{
	DECLARE_CLASS(CXenPLight, CActAnimating);
	DECLARE_DATADESC();

public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		Think(void);

	void		LightOn(void);
	void		LightOff(void);

private:
	CSprite* m_pGlow;
	float	m_flDmgTime;
};

LINK_ENTITY_TO_CLASS(xen_plantlight, CXenPLight);

BEGIN_DATADESC(CXenPLight)
DEFINE_FIELD(m_pGlow, FIELD_CLASSPTR),
DEFINE_FIELD(m_flDmgTime, FIELD_FLOAT),
END_DATADESC()

void CXenPLight::Spawn(void)
{
	Precache();

	SetModel("models/light.mdl");
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);

	UTIL_SetSize(this, Vector(-80, -80, 0), Vector(80, 80, 32));
	SetActivity(ACT_IDLE);
	SetNextThink(gpGlobals->curtime + 0.1);
	SetCycle(random->RandomFloat(0, 1));

	m_pGlow = CSprite::SpriteCreate(XEN_PLANT_GLOW_SPRITE, GetLocalOrigin() + Vector(0, 0, (WorldAlignMins().z + WorldAlignMaxs().z) * 0.5), FALSE);
	m_pGlow->SetTransparency(kRenderGlow, GetRenderColor().r, GetRenderColor().g, GetRenderColor().b, GetRenderColor().a, m_nRenderFX);
	m_pGlow->SetAttachment(this, 1);
}

void CXenPLight::Precache(void)
{
	PrecacheModel("models/light.mdl");
	PrecacheModel(XEN_PLANT_GLOW_SPRITE);
}

void CXenPLight::Think(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);

	switch (GetActivity())
	{
	case ACT_CROUCH:
		if (IsSequenceFinished())
		{
			SetActivity(ACT_CROUCHIDLE);
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if (gpGlobals->curtime > m_flDmgTime)
		{
			SetActivity(ACT_STAND);
			LightOn();
		}
		break;

	case ACT_STAND:
		if (IsSequenceFinished())
			SetActivity(ACT_IDLE);
		break;

	case ACT_IDLE:
	default:
		break;
	}
}

void CXenPLight::Touch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer())
	{
		m_flDmgTime = gpGlobals->curtime + XEN_PLANT_HIDE_TIME;
		if (GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND)
		{
			SetActivity(ACT_CROUCH);
		}
	}
}

void CXenPLight::LightOn(void)
{
	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOn", Value, 0, this, this);

	if (m_pGlow)
		m_pGlow->RemoveEffects(EF_NODRAW);
}


void CXenPLight::LightOff(void)
{
	variant_t Value;
	g_EventQueue.AddEvent(STRING(m_target), "TurnOff", Value, 0, this, this);

	if (m_pGlow)
		m_pGlow->AddEffects(EF_NODRAW);
}

#define SF_HAIR_SYNC		0x0001

class CXenHair : public CActAnimating
{
	DECLARE_CLASS(CXenHair, CActAnimating)
public:
	void		Spawn(void);
	void		Precache(void);
	void		Think(void);
};

LINK_ENTITY_TO_CLASS(xen_hair, CXenHair);

void CXenHair::Spawn(void)
{
	Precache();
	SetModel("models/hair.mdl");
	UTIL_SetSize(this, Vector(-4, -4, 0), Vector(4, 4, 32));
	SetSequence(0);

	if (!HasSpawnFlags(SF_HAIR_SYNC))
	{
		SetCycle(random->RandomFloat(0, 1));
		m_flPlaybackRate = random->RandomFloat(0.7, 1.4);
	}
	ResetSequenceInfo();

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
	SetNextThink(gpGlobals->curtime + random->RandomFloat(0.1, 0.4));
}

void CXenHair::Think(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);
}


void CXenHair::Precache(void)
{
	PrecacheModel("models/hair.mdl");
}

class CXenTreeTrigger : public CBaseEntity
{
	DECLARE_CLASS(CXenTreeTrigger, CBaseEntity)
public:
	void		Touch(CBaseEntity* pOther);
	static CXenTreeTrigger* TriggerCreate(CBaseEntity* pOwner, const Vector& position);
};

LINK_ENTITY_TO_CLASS(xen_ttrigger, CXenTreeTrigger);

CXenTreeTrigger* CXenTreeTrigger::TriggerCreate(CBaseEntity* pOwner, const Vector& position)
{
	CXenTreeTrigger* pTrigger = CREATE_ENTITY(CXenTreeTrigger, "xen_ttrigger");
	pTrigger->SetAbsOrigin(position);

	pTrigger->SetSolid(SOLID_BBOX);
	pTrigger->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	pTrigger->SetMoveType(MOVETYPE_NONE);
	pTrigger->SetOwnerEntity(pOwner);

	return pTrigger;
}

void CXenTreeTrigger::Touch(CBaseEntity* pOther)
{
	if (GetOwnerEntity())
	{
		GetOwnerEntity()->Touch(pOther);
	}
}

#define TREE_AE_ATTACK		1

class CXenTree : public CActAnimating
{
	DECLARE_CLASS(CXenTree, CActAnimating);
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		Think(void);
	int			OnTakeDamage(const CTakeDamageInfo& info) { Attack(); return 0; }
	void		HandleAnimEvent(animevent_t* pEvent);
	void		Attack(void);
	Class_T			Classify(void) { return CLASS_ALIEN_PREDATOR; }

	DECLARE_DATADESC();

private:
	CXenTreeTrigger* m_pTrigger;
};

LINK_ENTITY_TO_CLASS(xen_tree, CXenTree);

BEGIN_DATADESC(CXenTree)
DEFINE_FIELD(m_pTrigger, FIELD_CLASSPTR),
END_DATADESC()

void CXenTree::Spawn(void)
{
	Precache();

	SetModel("models/tree.mdl");
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);

	m_takedamage = DAMAGE_YES;

	UTIL_SetSize(this, Vector(-30, -30, 0), Vector(30, 30, 188));
	SetActivity(ACT_IDLE);
	SetNextThink(gpGlobals->curtime + 0.1);
	SetCycle(random->RandomFloat(0, 1));
	m_flPlaybackRate = random->RandomFloat(0.7, 1.4);

	Vector triggerPosition, vForward;

	AngleVectors(GetAbsAngles(), &vForward);
	triggerPosition = GetAbsOrigin() + (vForward * 64);

	m_pTrigger = CXenTreeTrigger::TriggerCreate(this, triggerPosition);
	UTIL_SetSize(m_pTrigger, Vector(-24, -24, 0), Vector(24, 24, 128));
}

void CXenTree::Precache(void)
{
	PrecacheModel("models/tree.mdl");
	PrecacheModel(XEN_PLANT_GLOW_SPRITE);

	PrecacheScriptSound("XenTree.AttackMiss");
	PrecacheScriptSound("XenTree.AttackHit");
}

void CXenTree::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer() && FClassnameIs(pOther, "monster_bigmomma"))
		return;

	Attack();
}


void CXenTree::Attack(void)
{
	if (GetActivity() == ACT_IDLE)
	{
		SetActivity(ACT_MELEE_ATTACK1);
		m_flPlaybackRate = random->RandomFloat(1.0, 1.4);

		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "XenTree.AttackMiss");
	}
}


void CXenTree::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case TREE_AE_ATTACK:
	{
		CBaseEntity* pList[8];
		BOOL sound = FALSE;
		int count = UTIL_EntitiesInBox(pList, 8, m_pTrigger->GetAbsOrigin() + m_pTrigger->WorldAlignMins(), m_pTrigger->GetAbsOrigin() + m_pTrigger->WorldAlignMaxs(), FL_NPC | FL_CLIENT);

		Vector forward;
		AngleVectors(GetAbsAngles(), &forward);

		for (int i = 0; i < count; i++)
		{
			if (pList[i] != this)
			{
				if (pList[i]->GetOwnerEntity() != this)
				{
					sound = true;
					pList[i]->TakeDamage(CTakeDamageInfo(this, this, 25, DMG_CRUSH | DMG_SLASH));
					pList[i]->ViewPunch(QAngle(15, 0, 18));

					pList[i]->SetAbsVelocity(pList[i]->GetAbsVelocity() + forward * 100);
				}
			}
		}

		if (sound)
		{
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "XenTree.AttackHit");
		}
	}
	return;
	}

	BaseClass::HandleAnimEvent(pEvent);
}

void CXenTree::Think(void)
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1);
	DispatchAnimEvents(this);

	switch (GetActivity())
	{
	case ACT_MELEE_ATTACK1:
		if (IsSequenceFinished())
		{
			SetActivity(ACT_IDLE);
			m_flPlaybackRate = random->RandomFloat(0.6f, 1.4f);
		}
		break;

	default:
	case ACT_IDLE:
		break;

	}
}

class CXenSpore : public CActAnimating
{
	DECLARE_CLASS(CXenSpore, CActAnimating);
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther) {}
	void		Attack(void) {}

	static const char* pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
	DECLARE_CLASS(CXenSporeSmall, CXenSpore);
	void		Spawn(void);
};

class CXenSporeMed : public CXenSpore
{
	DECLARE_CLASS(CXenSporeMed, CXenSpore);
	void		Spawn(void);
};

class CXenSporeLarge : public CXenSpore
{
	DECLARE_CLASS(CXenSporeLarge, CXenSpore);
	void		Spawn(void);

	static const Vector m_hullSizes[];
};

const Vector CXenSporeLarge::m_hullSizes[] =
{
	Vector(90, -25, 0),
	Vector(25, 75, 0),
	Vector(-15, -100, 0),
	Vector(-90, -35, 0),
	Vector(-90, 60, 0),
};

class CXenHull : public CPointEntity
{
	DECLARE_CLASS(CXenHull, CPointEntity);
public:
	static CXenHull* CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset);
	Class_T			Classify(void) { return CLASS_ALIEN_PREDATOR; }
};

CXenHull* CXenHull::CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset)
{
	CXenHull* pHull = CREATE_ENTITY(CXenHull, "xen_hull");

	UTIL_SetOrigin(pHull, source->GetAbsOrigin() + offset);
	pHull->SetSolid(SOLID_BBOX);
	pHull->SetMoveType(MOVETYPE_NONE);
	pHull->SetOwnerEntity(source);
	UTIL_SetSize(pHull, mins, maxs);
	pHull->SetRenderColorA(0);
	pHull->m_nRenderMode = kRenderTransTexture;
	return pHull;
}

LINK_ENTITY_TO_CLASS(xen_spore_small, CXenSporeSmall);
LINK_ENTITY_TO_CLASS(xen_spore_medium, CXenSporeMed);
LINK_ENTITY_TO_CLASS(xen_spore_large, CXenSporeLarge);
LINK_ENTITY_TO_CLASS(xen_hull, CXenHull);

void CXenSporeSmall::Spawn(void)
{
	m_nSkin = 0;
	CXenSpore::Spawn();
	UTIL_SetSize(this, Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CXenSporeMed::Spawn(void)
{
	m_nSkin = 1;
	CXenSpore::Spawn();
	UTIL_SetSize(this, Vector(-40, -40, 0), Vector(40, 40, 120));
}

void CXenSporeLarge::Spawn(void)
{
	m_nSkin = 2;
	CXenSpore::Spawn();
	UTIL_SetSize(this, Vector(-48, -48, 110), Vector(48, 48, 240));

	Vector forward, right;

	AngleVectors(GetAbsAngles(), &forward, &right, NULL);

	for (int i = 0; i < ARRAYSIZE(m_hullSizes); i++)
	{
		CXenHull::CreateHull(this, Vector(-12, -12, 0), Vector(12, 12, 120), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right));
	}
}

void CXenSpore::Spawn(void)
{
	Precache();

	SetModel(pModelNames[m_nSkin]);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_BBOX);
	m_takedamage = DAMAGE_NO;

	SetSequence(0);
	SetCycle(random->RandomFloat(0.0f, 1.0f));
	m_flPlaybackRate = random->RandomFloat(0.7f, 1.4f);
	ResetSequenceInfo();
	SetNextThink(gpGlobals->curtime + random->RandomFloat(0.1f, 0.4f));
}

const char* CXenSpore::pModelNames[] =
{
	"models/fungus(small).mdl",
	"models/fungus.mdl",
	"models/fungus(large).mdl",
};


void CXenSpore::Precache(void)
{
	PrecacheModel((char*)pModelNames[m_nSkin]);
}

class CHEVDead : public CAI_BaseNPC
{
	DECLARE_CLASS(CHEVDead, CAI_BaseNPC);
public:
	void Spawn(void);
	Class_T	Classify(void) { return	CLASS_NONE; }
	float MaxYawSpeed(void) { return 8.0f; }

	bool KeyValue(const char* szKeyName, const char* szValue);

	int	m_iPose;
	static char* m_szPoses[4];
};

char* CHEVDead::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

bool CHEVDead::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "pose"))
		m_iPose = atoi(szValue);
	else
		CAI_BaseNPC::KeyValue(szKeyName, szValue);

	return true;
}

LINK_ENTITY_TO_CLASS(monster_hevsuit_dead, CHEVDead);

void CHEVDead::Spawn(void)
{
	PrecacheModel("models/player.mdl");
	SetModel("models/player.mdl");

	ClearEffects();
	SetSequence(0);
	m_nBody = 1;
	m_bloodColor = BLOOD_COLOR_RED;

	SetSequence(LookupSequence(m_szPoses[m_iPose]));

	if (GetSequence() == -1)
	{
		Msg("Dead hevsuit with bad pose\n");
		SetSequence(0);
		ClearEffects();
		AddEffects(EF_BRIGHTLIGHT);
	}

	m_iHealth = 8;

	NPCInitDead();

	SetSolid(SOLID_BBOX);

	Vector mins, maxs;
	ExtractBbox(LookupSequence(m_szPoses[m_iPose]), mins, maxs);
	UTIL_SetSize(this, mins, maxs);

	SetCollisionGroup(COLLISION_GROUP_DEBRIS);
}