#ifndef HL1_MONSTER_HGRUNT_H
#define HL1_MONSTER_HGRUNT_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_squad.h"
#include "hl1_monster_squad.h"

class CHGrunt : public CHL1SquadMonster
{
	DECLARE_CLASS(CHGrunt, CHL1SquadMonster);
public:

	void	Precache(void);
	void	Spawn(void);

	bool	CreateVPhysics(void);

	void	NPCThink(void);

	void	JustSpoke(void);
	void	SpeakSentence(void);
	void	PrescheduleThink(void);

	bool	FOkToSpeak(void);

	Class_T	Classify(void);
	int     RangeAttack1Conditions(float flDot, float flDist);
	int		MeleeAttack1Conditions(float flDot, float flDist);
	int     RangeAttack2Conditions(float flDot, float flDist);

	Activity NPC_TranslateActivity(Activity eNewActivity);

	void	ClearAttackConditions(void);

	int		IRelationPriority(CBaseEntity* pTarget);

	int     GetGrenadeConditions(float flDot, float flDist);

	bool	FCanCheckAttacks(void);

	int     GetSoundInterests(void);

	void    TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);
	int		OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo);

	float	MaxYawSpeed(void);

	void    IdleSound(void);

	void	CheckAmmo(void);

	CBaseEntity* Kick(void);

	Vector	Weapon_ShootPosition(void);
	void	HandleAnimEvent(animevent_t* pEvent);

	void	Shoot(void);
	void	Shotgun(void);

	void	StartTask(const Task_t* pTask);
	void	RunTask(const Task_t* pTask);

	int		SelectSchedule(void);
	int		TranslateSchedule(int scheduleType);


	void	PainSound(const CTakeDamageInfo& info);
	void	DeathSound(const CTakeDamageInfo& info);
	void	SetAim(const Vector& aimDir);

	bool	HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt);

	void	Event_Killed(const CTakeDamageInfo& info);


	static const char* pGruntSentences[];

	bool	m_bInBarnacleMouth;

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

private:
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;
	float m_flTalkWaitTime;

	Vector	m_vecTossVelocity;

	int		m_iLastGrenadeCondition;
	bool	m_fStanding;
	bool	m_fFirstEncounter;
	int		m_iClipSize;

	int		m_voicePitch;

	int		m_iSentence;

	float	m_flCheckAttackTime;

	int		m_iAmmoType;

	int		m_iWeapons;
};

#endif // HL1_MONSTER_HGRUNT_H