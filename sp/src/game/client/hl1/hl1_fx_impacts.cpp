#include "cbase.h"
#include "fx_impact.h"

void ImpactCallback(const CEffectData& data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity* pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
		return;

	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("Impact", ImpactCallback);