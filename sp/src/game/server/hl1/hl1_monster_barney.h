#ifndef NPC_BARNEY_H
#define NPC_BARNEY_H
#ifdef _WIN32
#pragma once
#endif

#include "hl1_monster_talker.h"

class CHL1Barney : public CHL1Talker
{
	DECLARE_CLASS(CHL1Barney, CHL1Talker);
public:

	DECLARE_DATADESC();

	virtual void ModifyOrAppendCriteria(AI_CriteriaSet& set);

	void	Precache(void);
	void	Spawn(void);

	void	Activate();
	void	TalkInit(void);

	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	int		GetSoundInterests(void);
	Class_T  Classify(void);
	void	AlertSound(void);
	void    SetYawSpeed(void);

	bool    CheckRangeAttack1(float flDot, float flDist);
	void    BarneyFirePistol(void);

	int		OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void	Event_Killed(const CTakeDamageInfo &info);

	void	DeathSound(const CTakeDamageInfo &info);

	void	HandleAnimEvent(animevent_t *pEvent);
	int		TranslateSchedule(int scheduleType);
	int		SelectSchedule(void);

	void	DeclineFollowing(void);

	bool	CanBecomeRagdoll(void);
	bool	ShouldGib(const CTakeDamageInfo &info);

	int		RangeAttack1Conditions(float flDot, float flDist);

	void	SUB_StartLVFadeOut(float delay = 10.0f, bool bNotSolid = true);
	void	SUB_LVFadeOut(void);

	NPC_STATE CHL1Barney::SelectIdealState(void);

	bool	m_fGunDrawn;
	float	m_flPainTime;
	float	m_flCheckAttackTime;
	bool	m_fLastAttackCheck;

	int		m_iAmmoType;

	enum
	{
		SCHED_BARNEY_FOLLOW = BaseClass::NEXT_SCHEDULE,
		SCHED_BARNEY_ENEMY_DRAW,
		SCHED_BARNEY_FACE_TARGET,
		SCHED_BARNEY_IDLE_STAND,
		SCHED_BARNEY_STOP_FOLLOWING,
	};

	DEFINE_CUSTOM_AI;
};

#endif