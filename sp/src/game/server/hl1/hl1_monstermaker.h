#ifndef HL1_MONSTERMAKER_H
#define HL1_MONSTERMAKER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#define	SF_MonsterMaker_START_ON	1
#define SF_MonsterMaker_NPCCLIP		8
#define SF_MonsterMaker_FADE		16
#define SF_MonsterMaker_INF_CHILD	32
#define	SF_MonsterMaker_NO_DROP		64


class CMonsterMaker : public CBaseEntity
{
public:
	DECLARE_CLASS(CMonsterMaker, CBaseEntity);

	CMonsterMaker(void) {}

	void Spawn(void);
	void Precache(void);

	void MakerThink(void);
	bool CanMakeNPC(void);

	void DeathNotice(CBaseEntity* pChild);
	void MakeNPC(void);

	void InputSpawnNPC(inputdata_t& inputdata);
	void InputEnable(inputdata_t& inputdata);
	void InputDisable(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);

	void Toggle(void);
	void Enable(void);
	void Disable(void);

	bool IsDepleted();

	DECLARE_DATADESC();

	int			m_iMaxNumNPCs;
	float		m_flSpawnFrequency;
	int			m_iMaxLiveChildren;
	string_t	m_iszNPCClassname;

	COutputEvent m_OnSpawnNPC;

	int		m_cLiveChildren;

	float	m_flGround;
	bool	m_bDisabled;
};

#endif // HL1_MONSTERMAKER_H