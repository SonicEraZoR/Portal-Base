#ifndef HL1_MONSTER_HORNET_H
#define HL1_MONSTER_HORNET_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster.h"

extern int iHornetPuff;

class CHL1Hornet : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Hornet, CHL1BaseMonster);
public:

	void Spawn(void);
	void Precache(void);
	Class_T	 Classify(void);
	Disposition_t		IRelationType(CBaseEntity* pTarget);

	void DieTouch(CBaseEntity* pOther);
	void Event_Killed(const CTakeDamageInfo& info) { return; }
	void DartTouch(CBaseEntity* pOther);
	void TrackTouch(CBaseEntity* pOther);
	void TrackTarget(void);
	void StartDart(void);
	void IgniteTrail(void);
	void StartTrack(void);


	virtual unsigned int PhysicsSolidMaskForEntity(void) const;
	virtual bool ShouldGib(const CTakeDamageInfo& info) { return false; }

	float			m_flStopAttack;
	int				m_iHornetType;
	float			m_flFlySpeed;
	int				m_flDamage;

	DECLARE_DATADESC();

	Vector			m_vecEnemyLKP;
};

#endif // HL1_MONSTER_HORNET_H