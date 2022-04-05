#ifndef HL1_MONSTER_H
#define HL1_MONSTER_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "ai_motor.h"

class CHL1BaseMonster : public CAI_BaseNPC
{
	DECLARE_CLASS(CHL1BaseMonster, CAI_BaseNPC);
public:
	CHL1BaseMonster(void) {}

	void	Precache(void);
	void	Spawn(void);

	virtual void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	virtual void    TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);

	bool	BecomeRagdoll(const CTakeDamageInfo& info, const Vector& forceVector);

	bool	ShouldGib(const CTakeDamageInfo &info);
	bool	CorpseGib(const CTakeDamageInfo &info);

	bool	HasAlienGibs(void);
	bool	HasHumanGibs(void);

	int		IRelationPriority(CBaseEntity *pTarget);
	bool	NoFriendlyFire(void);

	void	EjectShell(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int iType);

	virtual int		SelectDeadSchedule();

	int		m_iStartHealth;
};

#endif // HL1_MONSTER_H