#ifndef HL1_MONSTER_BARNACLE_H
#define HL1_MONSTER_BARNACLE_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster.h"
#include "rope_physics.h"

#define	BARNACLE_BODY_HEIGHT	44
#define BARNACLE_PULL_SPEED			8
#define BARNACLE_KILL_VICTIM_DELAY	5

#define	BARNACLE_AE_PUKEGIB	2

class CHL1Barnacle : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Barnacle, CHL1BaseMonster);

public:
	void			Spawn(void);
	void			Precache(void);
	CBaseEntity* TongueTouchEnt(float* pflLength);
	Class_T			Classify(void);
	void			HandleAnimEvent(animevent_t* pEvent);
	void			BarnacleThink(void);
	void			WaitTillDead(void);
	void			Event_Killed(const CTakeDamageInfo& info);
	int				OnTakeDamage_Alive(const CTakeDamageInfo& info);

	DECLARE_DATADESC();

	float			m_flAltitude;

	float			m_flKillVictimTime;
	int				m_cGibs;
	bool			m_fLiftingPrey;
	float			m_flTongueAdj;
	float			m_flIgnoreTouchesUntil;

public:
	DEFINE_CUSTOM_AI;
};

#endif // HL1_MONSTER_BARNACLE_H