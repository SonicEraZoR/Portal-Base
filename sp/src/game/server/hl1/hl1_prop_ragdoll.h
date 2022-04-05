#ifndef HL1_PROP_RAGDOLL_H
#define HL1_PROP_RAGDOLL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "physics_prop_ragdoll.h"

class CHL1Ragdoll : public CRagdollProp
{
	DECLARE_CLASS(CHL1Ragdoll, CRagdollProp);
public:
	virtual int	OnTakeDamage(const CTakeDamageInfo& info);
	void		RagdollGib(const CTakeDamageInfo& info);
	
	float		m_iRagdollHealth;
	float		m_iNoGibTime;
	bool		m_bHumanGibs;
	bool		m_bShouldNotGib;
};

CHL1Ragdoll* CreateHL1Ragdoll(CBaseAnimating* pAnimating, int forceBone, const CTakeDamageInfo& info, int collisionGroup, bool bUseLRURetirement = false);

#endif // HL1_PROP_RAGDOLL_H