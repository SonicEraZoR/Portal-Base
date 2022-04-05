#include "cbase.h"
#include "bone_setup.h"
#include "hl1_prop_ragdoll.h"
#include "te_effect_dispatch.h"
#include "soundent.h"

#define HUMAN_GIBS 1
#define ALIEN_GIBS 2

#define SF_RAGDOLLPROP_USE_LRU_RETIREMENT	0x1000

extern ConVar hl1_ragdoll_gib;

LINK_ENTITY_TO_CLASS(prop_hl1_ragdoll, CHL1Ragdoll);

int CHL1Ragdoll::OnTakeDamage(const CTakeDamageInfo& info)
{
	if (hl1_ragdoll_gib.GetBool() && !m_bShouldNotGib && gpGlobals->curtime > m_iNoGibTime)
	{
		if (!(info.GetDamageType() & DMG_NEVERGIB || info.GetDamageType() & DMG_BULLET || info.GetDamageType() & DMG_FALL) && (info.GetDamageType() & DMG_ALWAYSGIB || info.GetDamageType() & DMG_CLUB || info.GetDamageType() & DMG_BLAST))
		{
			float damage = info.GetDamage();
			m_iRagdollHealth = m_iRagdollHealth - damage;

			if (m_iRagdollHealth <= 0)
				RagdollGib(info);
		}
	}

	return BaseClass::OnTakeDamage(info);
}

void CHL1Ragdoll::RagdollGib(const CTakeDamageInfo& info)
{
	CEffectData	data;

	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize(data.m_vNormal);

	data.m_flScale = RemapVal(m_iHealth, 0, -500, 1, 3);
	data.m_flScale = clamp(data.m_flScale, 1, 3);

	if (m_bHumanGibs)
	{
		data.m_nMaterial = HUMAN_GIBS;
		data.m_nColor = BLOOD_COLOR_RED;
	}
	else
	{
		data.m_nMaterial = ALIEN_GIBS;
		data.m_nColor = BLOOD_COLOR_YELLOW;
	}

	DispatchEffect("HL1Gib", data);

	EmitSound("Player.FallGib");

	CSoundEnt::InsertSound(SOUND_MEAT, GetAbsOrigin(), 256, 0.5f, this);

	RemoveDeferred();
}

static void SyncAnimatingWithPhysics(CBaseAnimating* pAnimating)
{
	IPhysicsObject* pPhysics = pAnimating->VPhysicsGetObject();
	if (pPhysics)
	{
		Vector pos;
		pPhysics->GetShadowPosition(&pos, NULL);
		pAnimating->SetAbsOrigin(pos);
	}
}

CHL1Ragdoll* CreateHL1Ragdoll(CBaseAnimating* pAnimating, int forceBone, const CTakeDamageInfo& info, int collisionGroup, bool bUseLRURetirement)
{
	if (info.GetDamageType() & (DMG_VEHICLE | DMG_CRUSH))
	{
		// if the entity was killed by physics or a vehicle, move to the vphysics shadow position before creating the ragdoll.
		SyncAnimatingWithPhysics(pAnimating);
	}
	CHL1Ragdoll* pRagdoll = (CHL1Ragdoll*)CBaseEntity::CreateNoSpawn("prop_hl1_ragdoll", pAnimating->GetAbsOrigin(), vec3_angle, NULL);
	pRagdoll->CopyAnimationDataFrom(pAnimating);
	pRagdoll->SetOwnerEntity(pAnimating);

	pRagdoll->InitRagdollAnimation();
	matrix3x4_t pBoneToWorld[MAXSTUDIOBONES], pBoneToWorldNext[MAXSTUDIOBONES];

	float dt = 0.1f;

	// Copy over dissolve state...
	if (pAnimating->IsEFlagSet(EFL_NO_DISSOLVE))
	{
		pRagdoll->AddEFlags(EFL_NO_DISSOLVE);
	}

	// NOTE: This currently is only necessary to prevent manhacks from
	// colliding with server ragdolls they kill
	pRagdoll->SetKiller(info.GetInflictor());
	pRagdoll->SetSourceClassName(pAnimating->GetClassname());

	// NPC_STATE_DEAD npc's will have their COND_IN_PVS cleared, so this needs to force SetupBones to happen
	unsigned short fPrevFlags = pAnimating->GetBoneCacheFlags();
	pAnimating->SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	// UNDONE: Extract velocity from bones via animation (like we do on the client)
	// UNDONE: For now, just move each bone by the total entity velocity if set.
	// Get Bones positions before
	// Store current cycle
	float fSequenceDuration = pAnimating->SequenceDuration(pAnimating->GetSequence());
	float fSequenceTime = pAnimating->GetCycle() * fSequenceDuration;

	if (fSequenceTime <= dt && fSequenceTime > 0.0f)
	{
		// Avoid having negative cycle
		dt = fSequenceTime;
	}

	float fPreviousCycle = clamp(pAnimating->GetCycle() - (dt * (1 / fSequenceDuration)), 0.f, 1.f);
	float fCurCycle = pAnimating->GetCycle();
	// Get current bones positions
	pAnimating->SetupBones(pBoneToWorldNext, BONE_USED_BY_ANYTHING);
	// Get previous bones positions
	pAnimating->SetCycle(fPreviousCycle);
	pAnimating->SetupBones(pBoneToWorld, BONE_USED_BY_ANYTHING);
	// Restore current cycle
	pAnimating->SetCycle(fCurCycle);

	// Reset previous bone flags
	pAnimating->ClearBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	pAnimating->SetBoneCacheFlags(fPrevFlags);

	Vector vel = pAnimating->GetAbsVelocity();
	if ((vel.Length() == 0) && (dt > 0))
	{
		// Compute animation velocity
		CStudioHdr* pstudiohdr = pAnimating->GetModelPtr();
		if (pstudiohdr)
		{
			Vector deltaPos;
			QAngle deltaAngles;
			if (Studio_SeqMovement(pstudiohdr,
				pAnimating->GetSequence(),
				fPreviousCycle,
				pAnimating->GetCycle(),
				pAnimating->GetPoseParameterArray(),
				deltaPos,
				deltaAngles))
			{
				VectorRotate(deltaPos, pAnimating->EntityToWorldTransform(), vel);
				vel /= dt;
			}
		}
	}

	if (vel.LengthSqr() > 0)
	{
		int numbones = pAnimating->GetModelPtr()->numbones();
		vel *= dt;
		for (int i = 0; i < numbones; i++)
		{
			Vector pos;
			MatrixGetColumn(pBoneToWorld[i], 3, pos);
			pos -= vel;
			MatrixSetColumn(pos, 3, pBoneToWorld[i]);
		}
	}

	// Is this a vehicle / NPC collision?
	if ((info.GetDamageType() & DMG_VEHICLE) && pAnimating->MyNPCPointer())
	{
		// init the ragdoll with no forces
		pRagdoll->InitRagdoll(vec3_origin, -1, vec3_origin, pBoneToWorld, pBoneToWorldNext, dt, collisionGroup, true);

		// apply vehicle forces
		// Get a list of bones with hitboxes below the plane of impact
		int boxList[128];
		Vector normal(0, 0, -1);
		int count = pAnimating->GetHitboxesFrontside(boxList, ARRAYSIZE(boxList), normal, DotProduct(normal, info.GetDamagePosition()));

		// distribute force over mass of entire character
		float massScale = Studio_GetMass(pAnimating->GetModelPtr());
		massScale = clamp(massScale, 1.f, 1.e4f);
		massScale = 1.f / massScale;

		// distribute the force
		// BUGBUG: This will hit the same bone twice if it has two hitboxes!!!!
		ragdoll_t* pRagInfo = pRagdoll->GetRagdoll();
		for (int i = 0; i < count; i++)
		{
			int physBone = pAnimating->GetPhysicsBone(pAnimating->GetHitboxBone(boxList[i]));
			IPhysicsObject* pPhysics = pRagInfo->list[physBone].pObject;
			pPhysics->ApplyForceCenter(info.GetDamageForce() * pPhysics->GetMass() * massScale);
		}
	}
	else
	{
		pRagdoll->InitRagdoll(info.GetDamageForce(), forceBone, info.GetDamagePosition(), pBoneToWorld, pBoneToWorldNext, dt, collisionGroup, true);
	}

	// Are we dissolving?
	if (pAnimating->IsDissolving())
	{
		pRagdoll->TransferDissolveFrom(pAnimating);
	}
	else if (bUseLRURetirement)
	{
		pRagdoll->AddSpawnFlags(SF_RAGDOLLPROP_USE_LRU_RETIREMENT);
		s_RagdollLRU.MoveToTopOfLRU(pRagdoll);
	}

	// Tracker 22598:  If we don't set the OBB mins/maxs to something valid here, then the client will have a zero sized hull
	//  for the ragdoll for one frame until Vphysics updates the real obb bounds after the first simulation frame.  Having
	//  a zero sized hull makes the ragdoll think it should be faded/alpha'd to zero for a frame, so you get a blink where
	//  the ragdoll doesn't draw initially.
	Vector mins, maxs;
	mins = pAnimating->CollisionProp()->OBBMins();
	maxs = pAnimating->CollisionProp()->OBBMaxs();
	pRagdoll->CollisionProp()->SetCollisionBounds(mins, maxs);

	return pRagdoll;
}