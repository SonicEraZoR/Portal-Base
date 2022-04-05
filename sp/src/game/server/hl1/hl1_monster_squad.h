#ifndef HL1_MONSTER_SQUAD_H
#define HL1_MONSTER_SQUAD_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster.h"

#define	SF_SQUADMONSTER_LEADER	32


#define bits_NO_SLOT		0

#define bits_SLOT_HGRUNT_ENGAGE1	( 1 << 0 )
#define bits_SLOT_HGRUNT_ENGAGE2	( 1 << 1 )
#define bits_SLOTS_HGRUNT_ENGAGE	( bits_SLOT_HGRUNT_ENGAGE1 | bits_SLOT_HGRUNT_ENGAGE2 )

#define bits_SLOT_HGRUNT_GRENADE1	( 1 << 2 ) 
#define bits_SLOT_HGRUNT_GRENADE2	( 1 << 3 ) 
#define bits_SLOTS_HGRUNT_GRENADE	( bits_SLOT_HGRUNT_GRENADE1 | bits_SLOT_HGRUNT_GRENADE2 )

#define bits_SLOT_AGRUNT_HORNET1	( 1 << 4 )
#define bits_SLOT_AGRUNT_HORNET2	( 1 << 5 )
#define bits_SLOT_AGRUNT_CHASE		( 1 << 6 )
#define bits_SLOTS_AGRUNT_HORNET	( bits_SLOT_AGRUNT_HORNET1 | bits_SLOT_AGRUNT_HORNET2 )

#define bits_SLOT_HOUND_ATTACK1		( 1 << 7 )
#define bits_SLOT_HOUND_ATTACK2		( 1 << 8 )
#define bits_SLOT_HOUND_ATTACK3		( 1 << 9 )
#define bits_SLOTS_HOUND_ATTACK		( bits_SLOT_HOUND_ATTACK1 | bits_SLOT_HOUND_ATTACK2 | bits_SLOT_HOUND_ATTACK3 )

#define bits_SLOT_SQUAD_SPLIT		( 1 << 10 )

#define NUM_SLOTS			11

#define	HL1_MAX_SQUAD_MEMBERS	5

class CHL1SquadMonster : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1SquadMonster, CHL1BaseMonster);
public:
	EHANDLE	m_hSquadLeader;
	EHANDLE	m_hSquadMember[HL1_MAX_SQUAD_MEMBERS - 1];
	int		m_afSquadSlots;
	float	m_flLastEnemySightTime;
	bool	m_fEnemyEluded;

	int		m_iMySlot;

	virtual void  GatherEnemyConditions(CBaseEntity* pEnemy);
	void StartNPC(void);
	void VacateSlot(void);
	void OnScheduleChange(void);
	void Event_Killed(const CTakeDamageInfo& info);
	bool OccupySlot(int iDesiredSlot);

	CHL1SquadMonster* MySquadLeader()
	{
		CHL1SquadMonster* pSquadLeader = (CHL1SquadMonster*)((CBaseEntity*)m_hSquadLeader);
		if (pSquadLeader != NULL)
			return pSquadLeader;
		return this;
	}
	CHL1SquadMonster* MySquadMember(int i)
	{
		if (i >= HL1_MAX_SQUAD_MEMBERS - 1)
			return this;
		else
			return (CHL1SquadMonster*)((CBaseEntity*)m_hSquadMember[i]);
	}
	int	InSquad(void) { return m_hSquadLeader != NULL; }
	int IsLeader(void) { return m_hSquadLeader == this; }
	int SquadJoin(int searchRadius);
	virtual int SquadRecruit(int searchRadius, int maxMembers);
	int	SquadCount(void);
	void SquadRemove(CHL1SquadMonster* pRemove);
	void SquadUnlink(void);
	bool SquadAdd(CHL1SquadMonster* pAdd);
	void SquadDisband(void);
	void SquadAddConditions(int iConditions);
	void SquadMakeEnemy(CBaseEntity* pEnemy);
	void SquadPasteEnemyInfo(void);
	void SquadCopyEnemyInfo(void);
	bool SquadEnemySplit(void);
	bool SquadMemberInRange(const Vector& vecLocation, float flDist);

	virtual CHL1SquadMonster* MySquadMonsterPointer(void) { return this; }

	int	Save(CSave& save);
	int Restore(CRestore& restore);

	virtual bool IsValidCover(const Vector& vecCoverLocation, CAI_Hint const* pHint);

	NPC_STATE GetIdealState(void);

	Vector	m_vecEnemyLKP;

	DECLARE_DATADESC();
};

#endif // HL1_MONSTER_SQUAD_H