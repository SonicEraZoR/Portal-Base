#include "cbase.h"
#include "hl1_monster.h"
#include "scripted.h"
#include "soundent.h"
#include "animation.h"
#include "entitylist.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "player.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "npcevent.h"

#include "hl1_prop_ragdoll.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "cplane.h"
#include "ai_squad.h"

#define HUMAN_GIBS 1
#define ALIEN_GIBS 2

void CHL1BaseMonster::Precache(void)
{
	PrecacheModel("models/gibs/agibs.mdl");
	PrecacheModel("models/gibs/hgibs.mdl");

	PrecacheScriptSound("Player.FallGib");

	BaseClass::Precache();
}

void CHL1BaseMonster::Spawn(void)
{
	m_iStartHealth = m_iHealth;

	BaseClass::Spawn();
}

void CHL1BaseMonster::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	TraceAttack(info, vecDir, ptr);
}


void CHL1BaseMonster::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
	if (info.GetDamage() >= 1.0 && !(info.GetDamageType() & DMG_SHOCK))
	{
		UTIL_BloodSpray(ptr->endpos, vecDir, BloodColor(), 4, FX_BLOODSPRAY_ALL);
	}
	BaseClass::TraceAttack(info, vecDir, ptr, 0);
}

bool CHL1BaseMonster::BecomeRagdoll(const CTakeDamageInfo& info, const Vector& forceVector)
{
	if ((info.GetDamageType() & DMG_VEHICLE) && !g_pGameRules->IsMultiplayer())
	{
		CTakeDamageInfo info2 = info;
		info2.SetDamageForce(forceVector);
		Vector pos = info2.GetDamagePosition();
		float flAbsMinsZ = GetAbsOrigin().z + WorldAlignMins().z;
		if ((pos.z - flAbsMinsZ) < 24)
		{
			// HACKHACK: Make sure the vehicle impact is at least 2ft off the ground
			pos.z = flAbsMinsZ + 24;
			info2.SetDamagePosition(pos);
		}

		// in single player create ragdolls on the server when the player hits someone
		// with their vehicle - for more dramatic death/collisions
		CHL1Ragdoll* pRagdoll = CreateHL1Ragdoll(this, m_nForceBone, info2, COLLISION_GROUP_INTERACTIVE_DEBRIS, true);
		
		pRagdoll->m_iRagdollHealth = m_iStartHealth / 2;
		if (HasAlienGibs())
			pRagdoll->m_bHumanGibs = 0;
		else if (HasHumanGibs())
			pRagdoll->m_bHumanGibs = 1;
		if (BloodColor() == DONT_BLEED)
			pRagdoll->m_bShouldNotGib = 1;
		
		FixupBurningServerRagdoll(pRagdoll);
		RemoveDeferred();
		return true;
	}

	//Fix up the force applied to server side ragdolls. This fixes magnets not affecting them.
	CTakeDamageInfo newinfo = info;
	newinfo.SetDamageForce(forceVector);

	
	if (CanBecomeRagdoll() == false)
		return false;

	CHL1Ragdoll* pRagdoll = CreateHL1Ragdoll(this, m_nForceBone, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true);
	
	pRagdoll->m_iRagdollHealth = m_iStartHealth / 2;
	if (HasAlienGibs())
		pRagdoll->m_bHumanGibs = 0;
	else if (HasHumanGibs())
		pRagdoll->m_bHumanGibs = 1;
	if (BloodColor() == DONT_BLEED)
		pRagdoll->m_bShouldNotGib = 1;
	
	FixupBurningServerRagdoll(pRagdoll);
	PhysSetEntityGameFlags(pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS);
	RemoveDeferred();
	
	return true;
}

bool CHL1BaseMonster::ShouldGib(const CTakeDamageInfo &info)
{
	if (info.GetDamageType() & DMG_NEVERGIB)
		return false;

	if ((g_pGameRules->Damage_ShouldGibCorpse(info.GetDamageType()) && m_iHealth < GIB_HEALTH_VALUE) || (info.GetDamageType() & DMG_ALWAYSGIB))
		return true;

	return false;

}

bool CHL1BaseMonster::CorpseGib(const CTakeDamageInfo &info)
{
	CEffectData	data;

	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize(data.m_vNormal);

	data.m_flScale = RemapVal(m_iHealth, 0, -500, 1, 3);
	data.m_flScale = clamp(data.m_flScale, 1, 3);

	if (HasAlienGibs())
		data.m_nMaterial = ALIEN_GIBS;
	else if (HasHumanGibs())
		data.m_nMaterial = HUMAN_GIBS;

	data.m_nColor = BloodColor();

	DispatchEffect("HL1Gib", data);

	EmitSound("Player.FallGib");

	CSoundEnt::InsertSound(SOUND_MEAT, GetAbsOrigin(), 256, 0.5f, this);

	return true;
}

bool CHL1BaseMonster::HasAlienGibs(void)
{
	Class_T myClass = Classify();

	if (myClass == CLASS_ALIEN_MILITARY ||
		myClass == CLASS_ALIEN_MONSTER ||
		myClass == CLASS_INSECT ||
		myClass == CLASS_ALIEN_PREDATOR ||
		myClass == CLASS_ALIEN_PREY)

		return true;

	return false;
}

bool CHL1BaseMonster::HasHumanGibs(void)
{
	Class_T myClass = Classify();

	if (myClass == CLASS_HUMAN_MILITARY ||
		myClass == CLASS_PLAYER_ALLY ||
		myClass == CLASS_HUMAN_PASSIVE ||
		myClass == CLASS_PLAYER)

		return true;

	return false;
}

int	CHL1BaseMonster::IRelationPriority(CBaseEntity *pTarget)
{
	return BaseClass::IRelationPriority(pTarget);
}

bool CHL1BaseMonster::NoFriendlyFire(void)
{
	if (!m_pSquad)
	{
		return true;
	}

	CPlane	backPlane;
	CPlane  leftPlane;
	CPlane	rightPlane;

	Vector	vecLeftSide;
	Vector	vecRightSide;
	Vector	v_left;

	Vector  vForward, vRight, vUp;
	QAngle  vAngleToEnemy;

	if (GetEnemy() != NULL)
	{
		VectorAngles((GetEnemy()->WorldSpaceCenter() - GetAbsOrigin()), vAngleToEnemy);

		AngleVectors(vAngleToEnemy, &vForward, &vRight, &vUp);
	}
	else
	{
		return false;
	}

	vecLeftSide = GetAbsOrigin() - (vRight * (WorldAlignSize().x * 1.5));
	vecRightSide = GetAbsOrigin() + (vRight * (WorldAlignSize().x * 1.5));
	v_left = vRight * -1;

	leftPlane.InitializePlane(vRight, vecLeftSide);
	rightPlane.InitializePlane(v_left, vecRightSide);
	backPlane.InitializePlane(vForward, GetAbsOrigin());

	AISquadIter_t iter;
	for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember(&iter); pSquadMember; pSquadMember = m_pSquad->GetNextMember(&iter))
	{
		if (pSquadMember == NULL)
			continue;

		if (pSquadMember == this)
			continue;

		if (backPlane.PointInFront(pSquadMember->GetAbsOrigin()) &&
			leftPlane.PointInFront(pSquadMember->GetAbsOrigin()) &&
			rightPlane.PointInFront(pSquadMember->GetAbsOrigin()))
		{
			return false;
		}
	}

	return true;
}

void CHL1BaseMonster::EjectShell(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int iType)
{
	CEffectData	data;
	data.m_vStart = vecVelocity;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = QAngle(0, rotation, 0);
	data.m_fFlags = iType;

	DispatchEffect("HL1ShellEject", data);
}

int CHL1BaseMonster::SelectDeadSchedule()
{
	if (m_lifeState == LIFE_DEAD)
		return SCHED_NONE;

	CleanupOnDeath();
	return SCHED_DIE;
}