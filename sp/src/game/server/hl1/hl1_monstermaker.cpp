#include "cbase.h"
#include "entityapi.h"
#include "entityoutput.h"
#include "ai_basenpc.h"
#include "hl1_monstermaker.h"
#include "mapentities.h"

BEGIN_DATADESC(CMonsterMaker)

DEFINE_KEYFIELD(m_iMaxNumNPCs, FIELD_INTEGER, "monstercount"),
DEFINE_KEYFIELD(m_iMaxLiveChildren, FIELD_INTEGER, "MaxLiveChildren"),
DEFINE_KEYFIELD(m_flSpawnFrequency, FIELD_FLOAT, "delay"),
DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_KEYFIELD(m_iszNPCClassname, FIELD_STRING, "monstertype"),
DEFINE_FIELD(m_cLiveChildren, FIELD_INTEGER),
DEFINE_FIELD(m_flGround, FIELD_FLOAT),
DEFINE_INPUTFUNC(FIELD_VOID, "Spawn", InputSpawnNPC),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
DEFINE_OUTPUT(m_OnSpawnNPC, "OnSpawnNPC"),
DEFINE_THINKFUNC(MakerThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monstermaker, CMonsterMaker);

void CMonsterMaker::Spawn(void)
{
	SetSolid(SOLID_NONE);
	m_cLiveChildren = 0;
	Precache();

	if (m_spawnflags & SF_MonsterMaker_INF_CHILD)
	{
		m_spawnflags |= SF_MonsterMaker_FADE;
	}

	if (m_bDisabled == false)
	{
		SetThink(&CMonsterMaker::MakerThink);
		SetNextThink(gpGlobals->curtime + m_flSpawnFrequency);
	}
	else
	{
		SetThink(&CBaseEntity::SUB_DoNothing);
	}
	m_flGround = 0;
}

bool CMonsterMaker::CanMakeNPC(void)
{
	if (m_iMaxLiveChildren > 0 && m_cLiveChildren >= m_iMaxLiveChildren)
	{
		return false;
	}

	if (!m_flGround)
	{ 
		trace_t tr;

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 2048), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		m_flGround = tr.endpos.z;
	}

	Vector mins = GetAbsOrigin() - Vector(34, 34, 0);
	Vector maxs = GetAbsOrigin() + Vector(34, 34, 0);
	maxs.z = GetAbsOrigin().z;

	if ((m_spawnflags & SF_MonsterMaker_NO_DROP) == false)
	{
		mins.z = m_flGround;
	}

	CBaseEntity *pList[128];

	int count = UTIL_EntitiesInBox(pList, 128, mins, maxs, FL_CLIENT | FL_NPC);
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			if (pList[i] == NULL)
				continue;

			if ((pList[i]->GetSolidFlags() & FSOLID_NOT_SOLID) == false)
				return false;
		}
	}

	return true;
}

bool CMonsterMaker::IsDepleted()
{
	if ((m_spawnflags & SF_MonsterMaker_INF_CHILD) || m_iMaxNumNPCs > 0)
		return false;

	return true;
}

void CMonsterMaker::Toggle(void)
{
	if (m_bDisabled)
	{
		Enable();
	}
	else
	{
		Disable();
	}
}

void CMonsterMaker::Enable(void)
{
	if (IsDepleted())
		return;

	m_bDisabled = false;
	SetThink(&CMonsterMaker::MakerThink);
	SetNextThink(gpGlobals->curtime);
}

void CMonsterMaker::Disable(void)
{
	m_bDisabled = true;
	SetThink(NULL);
}

void CMonsterMaker::InputSpawnNPC(inputdata_t &inputdata)
{
	MakeNPC();
}

void CMonsterMaker::InputEnable(inputdata_t &inputdata)
{
	Enable();
}

void CMonsterMaker::InputDisable(inputdata_t &inputdata)
{
	Disable();
}

void CMonsterMaker::InputToggle(inputdata_t &inputdata)
{
	Toggle();
}

void CMonsterMaker::Precache(void)
{
	BaseClass::Precache();
	UTIL_PrecacheOther(STRING(m_iszNPCClassname));
}

void CMonsterMaker::MakeNPC(void)
{
	if (!CanMakeNPC())
	{
		return;
	}

	CBaseEntity	*pent = (CBaseEntity*)CreateEntityByName(STRING(m_iszNPCClassname));

	if (!pent)
	{
		Warning("NULL Ent in MonsterMaker!\n");
		return;
	}

	m_OnSpawnNPC.FireOutput(this, this);

	pent->SetLocalOrigin(GetAbsOrigin());
	pent->SetLocalAngles(GetAbsAngles());

	pent->AddSpawnFlags(SF_NPC_FALL_TO_GROUND);

	if (m_spawnflags & SF_MonsterMaker_FADE)
	{
		pent->AddSpawnFlags(SF_NPC_FADE_CORPSE);
	}


	DispatchSpawn(pent);
	pent->SetOwnerEntity(this);

	m_cLiveChildren++;

	if (!(m_spawnflags & SF_MonsterMaker_INF_CHILD))
	{
		m_iMaxNumNPCs--;

		if (IsDepleted())
		{
			SetThink(NULL);
			SetUse(NULL);
		}
	}
}

void CMonsterMaker::MakerThink(void)
{
	SetNextThink(gpGlobals->curtime + m_flSpawnFrequency);

	MakeNPC();
}

void CMonsterMaker::DeathNotice(CBaseEntity *pVictim)
{
	m_cLiveChildren--;
}