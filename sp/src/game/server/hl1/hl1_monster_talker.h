#ifndef HL1TALKNPC_H
#define HL1TALKNPC_H

#pragma warning(push)
#include <set>
#pragma warning(pop)

#ifdef _WIN32
#pragma once
#endif

#include "soundflags.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_default.h"
#include "ai_speech.h"
#include "ai_basenpc.h"
#include "AI_Behavior.h"
#include "ai_behavior_follow.h"
#include "npc_talker.h"

#define SF_PRE_DISASTER (1 << 16)

class CHL1Talker : public CNPCSimpleTalker
{
	DECLARE_CLASS(CHL1Talker, CNPCSimpleTalker);
public:
	CHL1Talker(void) {}

	virtual void Precache(void);
	void	Spawn(void);

	bool	CreateVPhysics(void);

	void	NPCThink(void);

	virtual int OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo);
	void		Event_Killed(const CTakeDamageInfo& info);
	void		TellFriends(void);

	virtual int		FIdleSpeak(void);
	virtual void	SetAnswerQuestion(CNPCSimpleTalker* pSpeaker);

	bool	BecomeRagdoll(const CTakeDamageInfo& info, const Vector& forceVector);

	bool	ShouldGib(const CTakeDamageInfo& info);
	bool	CorpseGib(const CTakeDamageInfo& info);

	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	int		SelectSchedule(void);
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	void	IdleHeadTurn(CBaseEntity *pTarget, float flDuration = 0.0, float flImportance = 1.0f);

	Disposition_t IRelationType(CBaseEntity *pTarget);

	void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);

	void	StartFollowing(CBaseEntity *pLeader);
	void	StopFollowing(void);

	void	Touch(CBaseEntity *pOther);

	float	PickLookTarget(bool bExcludePlayers = false, float minTime = 1.5, float maxTime = 2.5);

	bool	OnObstructingDoor(AILocalMoveGoal_t *pMoveGoal, CBaseDoor *pDoor, float distClear, AIMoveResult_t *pResult);

	virtual bool ShouldPlayerAvoid(void) { return false; }

	virtual void OnStoppingFollow(CBaseEntity* pTarget);

	bool	m_bInBarnacleMouth;
	int		m_iStartHealth;

	enum
	{
		SCHED_HL1TALKER_FOLLOW_MOVE_AWAY = BaseClass::NEXT_SCHEDULE,
		SCHED_HL1TALKER_IDLE_SPEAK_WAIT,
		SCHED_HL1TALKER_BARNACLE_HIT,
		SCHED_HL1TALKER_BARNACLE_PULL,
		SCHED_HL1TALKER_BARNACLE_CHOMP,
		SCHED_HL1TALKER_BARNACLE_CHEW,

		NEXT_SCHEDULE,
	};

	enum
	{
		TASK_HL1TALKER_FOLLOW_WALK_PATH_FOR_UNITS = BaseClass::NEXT_TASK,

		NEXT_TASK,
	};

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

protected:
	virtual void 	FollowerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:
	virtual void	DeclineFollowing(void) {}

	virtual int		SelectDeadSchedule(void);
};

#endif