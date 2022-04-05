#ifndef HL1_MONSTER_SNARK_H
#define HL1_MONSTER_SNARK_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster.h"

class CHL1Snark : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Snark, CHL1BaseMonster);
public:

	DECLARE_DATADESC();

	void	Precache(void);
	void	Spawn(void);
	Class_T Classify(void);
	void	Event_Killed(const CTakeDamageInfo& info);
	bool	Event_Gibbed(const CTakeDamageInfo& info);
	void	HuntThink(void);
	void	SuperBounceTouch(CBaseEntity* pOther);

	virtual void ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity);

	virtual unsigned int PhysicsSolidMaskForEntity(void) const;

	virtual bool ShouldGib(const CTakeDamageInfo& info) { return false; }
	static float	m_flNextBounceSoundTime;

	virtual bool IsValidEnemy(CBaseEntity* pEnemy);

private:
	Class_T	m_iMyClass;
	float	m_flDie;
	Vector	m_vecTarget;
	float	m_flNextHunt;
	float	m_flNextHit;
	Vector	m_posPrev;
	EHANDLE m_hOwner;
};

#endif // HL1_MONSTER_SNARK_H