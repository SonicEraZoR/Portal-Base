#include "cbase.h"
#include "hl1_monster_talker.h"
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
#include "ai_interactions.h"
#include "doors.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "hl1_monster.h"
#include "hl1_prop_ragdoll.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

ConVar hl1_fixup_sentence_sndlevel("hl1_fixup_sentence_sndlevel", "1");

extern ConVar hl1_ragdoll_gib;

bool forceObeyFollow = false;

BEGIN_DATADESC(CHL1Talker)
DEFINE_ENTITYFUNC(Touch),
DEFINE_FIELD(m_bInBarnacleMouth, FIELD_BOOLEAN),
END_DATADESC()

void CHL1Talker::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("Barney.Close");
	PrecacheScriptSound("Player.FallGib");
}

void CHL1Talker::Spawn(void)
{
	m_iStartHealth = m_iHealth;

	BaseClass::Spawn();
}

//Creating VPhysics with the below settings is what allows Scientists and Barneys to collide with the Tentacle Monster.
bool CHL1Talker::CreateVPhysics(void)
{
	VPhysicsDestroyObject();
	
	CPhysCollide* pModel = PhysCreateBbox(Vector(-16, -16, 0), Vector(16, 16, 72));
	IPhysicsObject* pPhysics = PhysModelCreateCustom(this, pModel, GetAbsOrigin(), GetAbsAngles(), "barney_hull", false);

	VPhysicsSetObject(pPhysics);
	if (pPhysics)
	{
		pPhysics->SetCallbackFlags(CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION);

		PhysAddShadow(this);

		pPhysics->EnableCollisions(true);
	}

	return true;
}

//Sometimes the VPhysics do not create properly and the best way I could find to fix this was to rebuild the VPhysics on every think.
void CHL1Talker::NPCThink(void)
{
	BaseClass::NPCThink();
	
	CreateVPhysics();
}

int CHL1Talker::OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo)
{
	if (IsAlive())
	{
		if (inputInfo.GetAttacker() && m_NPCState != NPC_STATE_PRONE && FBitSet(inputInfo.GetAttacker()->GetFlags(), FL_CLIENT))
		{
			CBaseEntity* pFriend = FindNearestFriend(FALSE);

			if (pFriend && pFriend->IsAlive())
			{
				CHL1Talker* pTalkMonster = (CHL1Talker*)pFriend;
				pTalkMonster->SetSchedule(SCHED_TALKER_IDLE_STOP_SHOOTING);
			}
		}
	}

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

void CHL1Talker::Event_Killed(const CTakeDamageInfo& info)
{
	if (info.GetAttacker()->GetFlags() & FL_CLIENT)
	{
		TellFriends();
		LimitFollowers(info.GetAttacker(), 0);
	}

	BaseClass::Event_Killed(info);
}

void CHL1Talker::TellFriends(void)
{
	CBaseEntity* pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for (i = 0; i < TLK_CFRIENDS; i++)
	{
		while ((pFriend = EnumFriends(pFriend, i, true)) != NULL)
		{
			CAI_BaseNPC* pMonster = pFriend->MyNPCPointer();
			if (pMonster->IsAlive())
			{
				// don't provoke a friend that's playing a death animation. They're a goner
				pMonster->Remember(bits_MEMORY_PROVOKED);
			}
		}
	}
}

int CHL1Talker::FIdleSpeak(void)
{
	// try to start a conversation, or make statement
	int pitch;

	if (!IsOkToSpeak())
		return false;

	Assert(GetExpresser()->SemaphoreIsAvailable(this));

	pitch = GetExpresser()->GetVoicePitch();

	// player using this entity is alive and wounded?
	CBaseEntity* pTarget = GetTarget();

	if (pTarget != NULL)
	{
		if (pTarget->IsPlayer())
		{
			if (pTarget->IsAlive())
			{
				SetSpeechTarget(GetTarget());
				if (GetExpresser()->CanSpeakConcept(TLK_PLHURT3) &&
					(GetTarget()->m_iHealth <= GetTarget()->m_iMaxHealth / 8))
				{
					Speak(TLK_PLHURT3);
					return true;
				}
				else if (GetExpresser()->CanSpeakConcept(TLK_PLHURT2) &&
					(GetTarget()->m_iHealth <= GetTarget()->m_iMaxHealth / 4))
				{
					Speak(TLK_PLHURT2);
					return true;
				}
				else if (GetExpresser()->CanSpeakConcept(TLK_PLHURT1) &&
					(GetTarget()->m_iHealth <= GetTarget()->m_iMaxHealth / 2))
				{
					Speak(TLK_PLHURT1);
					return true;
				}
			}
			else
			{
				//!!!KELLY - here's a cool spot to have the talkNPC talk about the dead player if we want.
				// "Oh dear, Gordon Freeman is dead!" -Scientist
				// "Damn, I can't do this without you." -Barney
			}
		}
	}

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	CBaseEntity *pFriend = FindNearestFriend(false);

	// 75% chance of talking to another citizen if one is available.
	if (pFriend && !(pFriend->IsMoving()) && random->RandomInt( 0, 3 ) != 0 )
	{
		if ( SpeakQuestionFriend( pFriend ) )
		{
			// force friend to answer
			CHL1Talker *pTalkNPC = dynamic_cast<CHL1Talker *>(pFriend);
			if (pTalkNPC && !pTalkNPC->HasSpawnFlags(SF_NPC_GAG) && !pTalkNPC->IsInAScript() )
			{
				SetSpeechTarget( pFriend );
				pTalkNPC->SetAnswerQuestion( this );
				pTalkNPC->GetExpresser()->BlockSpeechUntil( GetExpresser()->GetTimeSpeechComplete() );
				
				m_nSpeak++;
			}

			// Don't let anyone else butt in.
			DeferAllIdleSpeech( random->RandomFloat( TALKER_DEFER_IDLE_SPEAK_MIN, TALKER_DEFER_IDLE_SPEAK_MAX ), pTalkNPC );
			return true;
		}
	}

	// Otherwise, play an idle statement, try to face client when making a statement.
	pFriend = FindNearestFriend(true);
	if (pFriend)
	{
		SetSpeechTarget(pFriend);
		// If we're about to talk to the player, and we've never said hello, say hello first
		if (!GetSpeechFilter() || !GetSpeechFilter()->NeverSayHello())
		{
			if (GetExpresser()->CanSpeakConcept(TLK_HELLO) && !GetExpresser()->SpokeConcept(TLK_HELLO))
			{
				SayHelloToPlayer(pFriend);
				return true;
			}
		}
		if (Speak(TLK_IDLE))
		{
			DeferAllIdleSpeech(random->RandomFloat(TALKER_DEFER_IDLE_SPEAK_MIN, TALKER_DEFER_IDLE_SPEAK_MAX));
			m_nSpeak++;
		}
		else
		{
			// We failed to speak. Don't try again for a bit.
			m_flNextIdleSpeechTime = gpGlobals->curtime + 3;
		}
		return true;
	}

	// didn't speak
	m_flNextIdleSpeechTime = gpGlobals->curtime + 3;
	return false;
}

void CHL1Talker::SetAnswerQuestion(CNPCSimpleTalker* pSpeaker)
{
	if (!m_hCine)
	{
		SetSchedule(SCHED_TALKER_IDLE_RESPONSE);
	}

	SetSpeechTarget((CAI_BaseNPC*)pSpeaker);
}

bool CHL1Talker::BecomeRagdoll(const CTakeDamageInfo& info, const Vector& forceVector)
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


	if (CanBecomeServerRagdoll() == false)
		return false;

	CHL1Ragdoll* pRagdoll = CreateHL1Ragdoll(this, m_nForceBone, newinfo, COLLISION_GROUP_INTERACTIVE_DEBRIS, true);

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

bool CHL1Talker::ShouldGib(const CTakeDamageInfo& info)
{
	if (info.GetDamageType() & DMG_NEVERGIB)
		return false;

	if ((g_pGameRules->Damage_ShouldGibCorpse(info.GetDamageType()) && m_iHealth < GIB_HEALTH_VALUE) || (info.GetDamageType() & DMG_ALWAYSGIB))
		return true;

	return false;
}

bool CHL1Talker::CorpseGib(const CTakeDamageInfo& info)
{
	CEffectData	data;

	data.m_vOrigin = WorldSpaceCenter();
	data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
	VectorNormalize(data.m_vNormal);

	data.m_flScale = RemapVal(m_iHealth, 0, -500, 1, 3);
	data.m_flScale = clamp(data.m_flScale, 1, 3);

	data.m_nMaterial = 1;

	data.m_nColor = BloodColor();

	DispatchEffect("HL1Gib", data);

	EmitSound("Player.FallGib");

	CSoundEnt::InsertSound(SOUND_MEAT, GetAbsOrigin(), 256, 0.5f, this);

	return true;
}

void CHL1Talker::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HL1TALKER_FOLLOW_WALK_PATH_FOR_UNITS:
	{
		GetNavigator()->SetMovementActivity(ACT_WALK);
		break;
	}
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CHL1Talker::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HL1TALKER_FOLLOW_WALK_PATH_FOR_UNITS:
	{
		float distance;

		distance = (m_vecLastPosition - GetLocalOrigin()).Length2D();

		if (distance > pTask->flTaskData ||
			GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		{
			TaskComplete();
			GetNavigator()->ClearGoal();
		}
		break;
	}



	case TASK_TALKER_CLIENT_STARE:
	case TASK_TALKER_LOOK_AT_CLIENT:
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (m_NPCState == NPC_STATE_IDLE		&&
			!IsMoving() &&
			!GetExpresser()->IsSpeaking())
		{

			if (pPlayer)
			{
				IdleHeadTurn(pPlayer);
			}
		}
		else
		{
			TaskFail("moved away");
			return;
		}

		if (pTask->iTask == TASK_TALKER_CLIENT_STARE)
		{
			if ((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length2D() > TALKER_STARE_DIST)
			{
				TaskFail(NO_TASK_FAILURE);
			}

			Vector vForward;
			AngleVectors(GetAbsAngles(), &vForward);
			if (UTIL_DotPoints(pPlayer->GetAbsOrigin(), GetAbsOrigin(), vForward) < m_flFieldOfView)
			{
				TaskFail("looked away");
			}
		}

		if (gpGlobals->curtime > m_flWaitFinished)
		{
			TaskComplete(NO_TASK_FAILURE);
		}

		break;
	}

	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (GetExpresser()->IsSpeaking() && GetSpeechTarget() != NULL)
		{
			IdleHeadTurn(GetSpeechTarget(), GetExpresser()->GetTimeSpeechComplete() - gpGlobals->curtime);
		}
		else if (GetEnemy())
		{
			IdleHeadTurn(GetEnemy());
		}

		BaseClass::RunTask(pTask);

		break;
	}

	case TASK_FACE_PLAYER:
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (pPlayer)
		{
			IdleHeadTurn(pPlayer);
			if (gpGlobals->curtime > m_flWaitFinished && GetMotor()->DeltaIdealYaw() < 10)
			{
				TaskComplete();
			}
		}
		else
		{
			TaskFail(FAIL_NO_PLAYER);
		}

		break;
	}

	case TASK_TALKER_EYECONTACT:
	{
		if (!IsMoving() && GetExpresser()->IsSpeaking() && GetSpeechTarget() != NULL)
		{
			IdleHeadTurn(GetSpeechTarget(), GetExpresser()->GetTimeSpeechComplete() - gpGlobals->curtime);
		}

		BaseClass::RunTask(pTask);

		break;

	}


	default:
	{
		if (GetExpresser()->IsSpeaking() && GetSpeechTarget() != NULL)
		{
			IdleHeadTurn(GetSpeechTarget(), GetExpresser()->GetTimeSpeechComplete() - gpGlobals->curtime);
		}
		else if (GetEnemy() && m_NPCState == NPC_STATE_COMBAT)
		{
			IdleHeadTurn(GetEnemy());
		}
		else if (GetFollowTarget())
		{
			IdleHeadTurn(GetFollowTarget());
		}

		BaseClass::RunTask(pTask);
		break;
	}
	}
}

int CHL1Talker::SelectSchedule(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_PRONE:
	{
		if (m_bInBarnacleMouth)
		{
			return SCHED_HL1TALKER_BARNACLE_CHOMP;
		}
		else
		{
			return SCHED_HL1TALKER_BARNACLE_HIT;
		}
	}
	}

	return BaseClass::SelectSchedule();
}

bool CHL1Talker::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
	{
		ClearSchedule("Talker is being eaten by a barnacle");
		m_bInBarnacleMouth = true;
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimReleased)
	{
		SetState(NPC_STATE_IDLE);

		CPASAttenuationFilter filter(this);

		CSoundParameters params;

		if (GetParametersForSound("Barney.Close", params, NULL))
		{
			EmitSound_t ep(params);
			ep.m_nPitch = GetExpresser()->GetVoicePitch();

			EmitSound(filter, entindex(), ep);
		}

		m_bInBarnacleMouth = false;
		SetAbsVelocity(vec3_origin);
		SetMoveType(MOVETYPE_STEP);
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimGrab)
	{
		if (GetFlags() & FL_ONGROUND)
		{
			SetGroundEntity(NULL);
		}

		if (GetState() == NPC_STATE_SCRIPT)
		{
			if (m_hCine)
			{
				m_hCine->CancelScript();
			}
		}

		SetState(NPC_STATE_PRONE);
		ClearSchedule("Talker was grabbed by a barnacle");

		CTakeDamageInfo info;
		PainSound(info);
		return true;
	}
	return false;
}

void CHL1Talker::IdleHeadTurn(CBaseEntity *pTarget, float flDuration, float flImportance)
{
	if (!(CapabilitiesGet() & bits_CAP_TURN_HEAD))
		return;

	if ((!pTarget) || (m_NPCState == NPC_STATE_SCRIPT))
		return;

	if (flDuration == 0.0f)
	{
		flDuration = random->RandomFloat(2.0, 4.0);
	}

	AddLookTarget(pTarget, 1.0, flDuration);
}

Disposition_t CHL1Talker::IRelationType(CBaseEntity *pTarget)
{
	if (pTarget->IsPlayer())
	{
		if (HasMemory(bits_MEMORY_PROVOKED))
		{
			return D_HT;
		}
	}

	return BaseClass::IRelationType(pTarget);
}

void CHL1Talker::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	if (info.GetDamage() >= 1.0 && !(info.GetDamageType() & DMG_SHOCK))
	{
		UTIL_BloodImpact(ptr->endpos, vecDir, BloodColor(), 4);
	}

	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

void CHL1Talker::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
	if (info.GetDamage() >= 1.0 && !(info.GetDamageType() & DMG_SHOCK))
	{
		UTIL_BloodImpact(ptr->endpos, vecDir, BloodColor(), 4);
	}

	BaseClass::TraceAttack(info, vecDir, ptr, 0);
}

void CHL1Talker::StartFollowing(CBaseEntity *pLeader)
{
	if (!HasSpawnFlags(SF_NPC_GAG))
	{
		if (m_iszUse != NULL_STRING)
		{
			PlaySentence(STRING(m_iszUse), 0.0f);
		}
		else
		{
			Speak(TLK_STARTFOLLOW);
		}

		SetSpeechTarget(pLeader);
	}

	BaseClass::StartFollowing(pLeader);
}

void CHL1Talker::StopFollowing(void)
{
	if (!(m_afMemory & bits_MEMORY_PROVOKED))
	{
		if (!HasSpawnFlags(SF_NPC_GAG))
		{
			if (m_iszUnUse != NULL_STRING)
			{
				PlaySentence(STRING(m_iszUnUse), 0.0f);
			}
			else
			{
				Speak(TLK_STOPFOLLOW);
			}

			SetSpeechTarget(GetFollowTarget());
		}
	}

	BaseClass::StopFollowing();
}

void CHL1Talker::Touch(CBaseEntity *pOther)
{
	if (m_NPCState == NPC_STATE_SCRIPT)
		return;

	BaseClass::Touch(pOther);
}

float CHL1Talker::PickLookTarget(bool bExcludePlayers, float minTime, float maxTime)
{
	return random->RandomFloat(5.0f, 10.0f);
}

bool CHL1Talker::OnObstructingDoor(AILocalMoveGoal_t *pMoveGoal, CBaseDoor *pDoor, float distClear, AIMoveResult_t *pResult)
{
	if (BaseClass::OnObstructingDoor(pMoveGoal, pDoor, distClear, pResult))
	{
		if (IsMoveBlocked(*pResult) && pMoveGoal->directTrace.vHitNormal != vec3_origin)
		{
			if (!pDoor->m_bLocked && !pDoor->HasSpawnFlags(SF_DOOR_NONPCS))
			{
				variant_t emptyVariant;
				pDoor->AcceptInput("Open", this, this, emptyVariant, USE_TOGGLE);
				*pResult = AIMR_OK;
			}
		}
		return true;
	}

	return false;
}

void CHL1Talker::FollowerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{

	if (!forceObeyFollow){
		if (GetUseTime() > gpGlobals->curtime)
			return;

		if (m_hCine && !m_hCine->CanInterrupt())
			return;

		if (pCaller != NULL && pCaller->IsPlayer())
		{
			if (m_spawnflags & SF_PRE_DISASTER)
			{
				SetSpeechTarget(pCaller);
				DeclineFollowing();
				return;
			}
		}
	}

	BaseClass::FollowerUse(pActivator, pCaller, useType, value);
}

int CHL1Talker::SelectDeadSchedule()
{
	if (m_lifeState == LIFE_DEAD)
		return SCHED_NONE;

	CleanupOnDeath();
	return SCHED_DIE;
}

void CHL1Talker::OnStoppingFollow(CBaseEntity* pTarget)
{
	BaseClass::StopFollowing();

	BaseClass::OnStoppingFollow(pTarget);
}

AI_BEGIN_CUSTOM_NPC(monster_hl1talker, CHL1Talker)

DECLARE_TASK(TASK_HL1TALKER_FOLLOW_WALK_PATH_FOR_UNITS)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_FOLLOW_MOVE_AWAY,

"	Tasks"
"		 TASK_SET_FAIL_SCHEDULE					SCHEDULE:SCHED_TARGET_FACE"
"		 TASK_STORE_LASTPOSITION				0"
"		 TASK_MOVE_AWAY_PATH					100"
"		 TASK_HL1TALKER_FOLLOW_WALK_PATH_FOR_UNITS	100"
"		 TASK_STOP_MOVING						0"
"		 TASK_FACE_PLAYER						0"
"		 TASK_SET_ACTIVITY						ACT_IDLE"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_IDLE_SPEAK_WAIT,

"	Tasks"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_FACE_PLAYER			0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_BARNACLE_HIT,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_HIT"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_HL1TALKER_BARNACLE_PULL"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_BARNACLE_PULL,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_PULL"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_BARNACLE_CHOMP,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHOMP"
"		 TASK_SET_SCHEDULE			SCHEDULE:SCHED_HL1TALKER_BARNACLE_CHEW"
""
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_HL1TALKER_BARNACLE_CHEW,

"	Tasks"
"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHEW"
)

AI_END_CUSTOM_NPC()