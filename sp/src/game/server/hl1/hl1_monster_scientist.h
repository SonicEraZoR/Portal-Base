#ifndef	HL1_SCIENTIST_H
#define	HL1_SCIENTIST_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster_talker.h"

class CHL1Scientist : public CHL1Talker
{
public:
	DECLARE_CLASS(CHL1Scientist, CHL1Talker);
	DECLARE_DATADESC();

	CHL1Scientist(void) {}

	void	Precache(void);
	void	Spawn(void);
	void	Activate();
	Class_T Classify(void);
	int		GetSoundInterests(void);

	virtual void ModifyOrAppendCriteria(AI_CriteriaSet& set);

	virtual int ObjectCaps(void) { return UsableNPCObjectCaps(BaseClass::ObjectCaps()); }
	float	MaxYawSpeed(void);

	float	TargetDistance(void);
	bool	IsValidEnemy(CBaseEntity *pEnemy);


	int		OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	void	Event_Killed(const CTakeDamageInfo &info);

	void	Heal(void);
	bool	CanHeal(void);

	int		TranslateSchedule(int scheduleType);
	void	HandleAnimEvent(animevent_t *pEvent);
	int		SelectSchedule(void);
	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	NPC_STATE SelectIdealState(void);

	int		FriendNumber(int arrayNumber);

	bool	DisregardEnemy(CBaseEntity *pEnemy) { return !pEnemy->IsAlive() || (gpGlobals->curtime - m_flFearTime) > 15; }

	void	TalkInit(void);

	void	DeclineFollowing(void);

	bool	CanBecomeRagdoll(void);
	bool	ShouldGib(const CTakeDamageInfo &info);

	void	SUB_StartLVFadeOut(float delay = 10.0f, bool bNotSolid = true);
	void	SUB_LVFadeOut(void);

	void	Scream(void);

	Activity GetStoppedActivity(void);
	Activity NPC_TranslateActivity(Activity newActivity);

	void DeathSound(const CTakeDamageInfo &info);

	enum
	{
		SCHED_SCI_HEAL = BaseClass::NEXT_SCHEDULE,
		SCHED_SCI_FOLLOWTARGET,
		SCHED_SCI_STOPFOLLOWING,
		SCHED_SCI_FACETARGET,
		SCHED_SCI_COVER,
		SCHED_SCI_HIDE,
		SCHED_SCI_IDLESTAND,
		SCHED_SCI_PANIC,
		SCHED_SCI_FOLLOWSCARED,
		SCHED_SCI_FACETARGETSCARED,
		SCHED_SCI_FEAR,
		SCHED_SCI_STARTLE,
	};

	enum
	{
		TASK_SAY_HEAL = BaseClass::NEXT_TASK,
		TASK_HEAL,
		TASK_SAY_FEAR,
		TASK_RUN_PATH_SCARED,
		TASK_SCREAM,
		TASK_RANDOM_SCREAM,
		TASK_MOVE_TO_TARGET_RANGE_SCARED,
	};

	DEFINE_CUSTOM_AI;

private:
	float m_flFearTime;
	float m_flHealTime;
	float m_flPainTime;
};

class CHL1DeadScientist : public CAI_BaseNPC
{
	DECLARE_CLASS(CHL1DeadScientist, CAI_BaseNPC);
public:
	void Spawn(void);
	Class_T	Classify(void) { return	CLASS_NONE; }
	
	bool KeyValue(const char *szKeyName, const char *szValue);
	float MaxYawSpeed(void) { return 8.0f; }

	int	m_iPose;
	int m_iDesiredSequence;
	static char *m_szPoses[7];
};

class CHL1SittingScientist : public CHL1Scientist
{
	DECLARE_CLASS(CHL1SittingScientist, CHL1Scientist);
public:
	DECLARE_DATADESC();

	void  Spawn(void);
	void  Precache(void);

	int FriendNumber(int arrayNumber);

	void SittingThink(void);

	virtual void SetAnswerQuestion(CNPCSimpleTalker *pSpeaker);

	int		FIdleSpeak(void);

	int		m_baseSequence;
	int		m_iHeadTurn;
	float	m_flResponseDelay;
};

#endif