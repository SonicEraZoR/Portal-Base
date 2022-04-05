#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"hl1_monster_scientist.h"
#include	"hl1_monster_barnacle.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"effect_dispatch_data.h"
#include	"te_effect_dispatch.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"engine/IEngineSound.h"
#include	"ai_navigator.h"
#include	"ai_behavior_follow.h"
#include	"AI_Criteria.h"
#include	"SoundEmitterSystem/isoundemittersystembase.h"

#define SC_PLFEAR	"SC_PLFEAR"
#define SC_FEAR		"SC_FEAR"
#define SC_HEAL		"SC_HEAL"
#define SC_SCREAM	"SC_SCREAM"
#define SC_POK		"SC_POK"

#define		NUM_SCIENTIST_HEADS		4
enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_LUTHER = 2, HEAD_SLICK = 3 };

ConVar	sk_scientist_health("sk_scientist_health", "20");
ConVar	sk_scientist_heal("sk_scientist_heal", "25");

int ACT_EXCITED;

string_t m_iszBarnacle;

#define SCIENTIST_HEAL (1)
#define SCIENTIST_NEEDLEON (2)
#define SCIENTIST_NEEDLEOFF (3)

BEGIN_DATADESC(CHL1Scientist)
DEFINE_FIELD(m_flFearTime, FIELD_TIME),
DEFINE_FIELD(m_flHealTime, FIELD_TIME),
DEFINE_FIELD(m_flPainTime, FIELD_TIME),
DEFINE_THINKFUNC(SUB_LVFadeOut),
DEFINE_USEFUNC(FollowerUse),
END_DATADESC()

LINK_ENTITY_TO_CLASS(monster_scientist, CHL1Scientist);

void CHL1Scientist::Precache(void)
{
	PrecacheModel("models/scientist.mdl");

	PrecacheScriptSound("Scientist.Pain");

	TalkInit();

	BaseClass::Precache();
}

void CHL1Scientist::Spawn(void)
{
	if (m_nBody == -1)
		m_nBody = random->RandomInt(0, NUM_SCIENTIST_HEADS - 1);


	SetRenderColor(255, 255, 255, 255);

	Precache();

	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	SetModel("models/scientist.mdl");

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	ClearEffects();
	m_iHealth = sk_scientist_health.GetFloat();
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE);
	CapabilitiesAdd(bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);

	m_nSkin = 0;

	if (m_nBody == HEAD_LUTHER)
		m_nSkin = 1;

	NPCInit();

	CreateVPhysics();

	SetUse(&CHL1Scientist::FollowerUse);

	BaseClass::Spawn();
}

void CHL1Scientist::Activate()
{
	m_iszBarnacle = FindPooledString("monster_barnacle");
	BaseClass::Activate();
}

Class_T	CHL1Scientist::Classify(void)
{
	return	CLASS_HUMAN_PASSIVE;
}

int CHL1Scientist::GetSoundInterests(void)
{
	return	SOUND_WORLD |
			SOUND_COMBAT |
			SOUND_DANGER |
			SOUND_PLAYER;
}

void CHL1Scientist::ModifyOrAppendCriteria(AI_CriteriaSet& criteriaSet)
{
	BaseClass::ModifyOrAppendCriteria(criteriaSet);

	bool predisaster = FBitSet(m_spawnflags, SF_PRE_DISASTER) ? true : false;

	criteriaSet.AppendCriteria("disaster", predisaster ? "[disaster::pre]" : "[disaster::post]");
}

float CHL1Scientist::MaxYawSpeed(void)
{
	switch (GetActivity())
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 160;
		break;
	case ACT_RUN:
		return 160;
		break;
	default:
		return 60;
		break;
	}
}

float CHL1Scientist::TargetDistance(void)
{
	CBaseEntity *pFollowTarget = GetFollowTarget();

	if (pFollowTarget == NULL || !pFollowTarget->IsAlive())
		return 1e6;

	return (pFollowTarget->WorldSpaceCenter() - WorldSpaceCenter()).Length();
}

bool CHL1Scientist::IsValidEnemy(CBaseEntity *pEnemy)
{
	if (pEnemy->m_iClassname == m_iszBarnacle)
	{
		return false;
	}

	return BaseClass::IsValidEnemy(pEnemy);
}

int CHL1Scientist::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	if (inputInfo.GetInflictor() && inputInfo.GetInflictor()->GetFlags() & FL_CLIENT)
	{
		Remember(bits_MEMORY_PROVOKED);
		StopFollowing();
	}

	if (gpGlobals->curtime >= m_flPainTime)
	{
		m_flPainTime = gpGlobals->curtime + random->RandomFloat(0.5, 0.75);

		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Scientist.Pain");
	}

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

void CHL1Scientist::Event_Killed(const CTakeDamageInfo &info)
{
	SetUse(NULL);
	BaseClass::Event_Killed(info);

	if (UTIL_IsLowViolence())
	{
		SUB_StartLVFadeOut(0.0f);
	}
}

void CHL1Scientist::Heal(void)
{
	//Scientist would not heal player until I removed this first condition.
	//This was because it was already checking if he could heal before running this function.
	//if (!CanHeal())
	//	return;
	
	Vector target = GetTarget()->GetAbsOrigin() -GetAbsOrigin();
	if (target.Length() > 100)
		return;
	
	GetTarget()->TakeHealth(sk_scientist_heal.GetFloat(), DMG_GENERIC);
	m_flHealTime = gpGlobals->curtime + 60;
}

bool CHL1Scientist::CanHeal(void)
{
	CBaseEntity *pTarget = GetFollowTarget();

	if (pTarget == NULL)
		return false;
	
	if (pTarget->IsPlayer() == false)
		return false;
	
	if ((m_flHealTime > gpGlobals->curtime) || (pTarget->m_iHealth > (pTarget->m_iMaxHealth * 0.5)))
		return false;
	
	return true;
}

int CHL1Scientist::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_TARGET_FACE:
	{
		int baseType;

		baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_TARGET_FACE;
		else
			return baseType;
	}
	break;

	case SCHED_TARGET_CHASE:
		return SCHED_SCI_FOLLOWTARGET;
		break;

	case SCHED_IDLE_STAND:
	{
		int baseType;

		baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_SCI_IDLESTAND;
		else
			return baseType;
	}
	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

void CHL1Scientist::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case SCIENTIST_HEAL:
		Heal();
		break;
	case SCIENTIST_NEEDLEON:
	{
		int oldBody = m_nBody;
		m_nBody = (oldBody % NUM_SCIENTIST_HEADS) + NUM_SCIENTIST_HEADS * 1;
	}
	break;
	case SCIENTIST_NEEDLEOFF:
	{
		int oldBody = m_nBody;
		m_nBody = (oldBody % NUM_SCIENTIST_HEADS) + NUM_SCIENTIST_HEADS * 0;
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

int CHL1Scientist::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_PRONE)
	{
		return BaseClass::SelectSchedule();
	}

	CBaseEntity *pEnemy = GetEnemy();

	if (GetFollowTarget())
	{
		CBaseEntity *pEnemy = GetEnemy();

		int relationship = D_NU;

		if (pEnemy != NULL)
			relationship = IRelationType(pEnemy);

		if (relationship != D_HT && relationship != D_FR)
		{
			if (TargetDistance() <= 128)
			{
				if (CanHeal())
				{
					SetTarget(GetFollowTarget());
					return SCHED_SCI_HEAL;
				}
			}
		}
	}
	else if (HasCondition(COND_PLAYER_PUSHING) && !(GetSpawnFlags() & SF_PRE_DISASTER))
	{
		return SCHED_HL1TALKER_FOLLOW_MOVE_AWAY;
	}

	if (BehaviorSelectSchedule())
	{
		return BaseClass::SelectSchedule();
	}



	if (HasCondition(COND_HEAR_DANGER) && m_NPCState != NPC_STATE_PRONE)
	{
		CSound *pSound;
		pSound = GetBestSound();

		if (pSound && pSound->IsSoundType(SOUND_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	switch (m_NPCState)
	{

	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:

		if (pEnemy)
		{
			if (HasCondition(COND_SEE_ENEMY))
				m_flFearTime = gpGlobals->curtime;
			else if (DisregardEnemy(pEnemy))
			{
				SetEnemy(NULL);
				pEnemy = NULL;
			}
		}

		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			return SCHED_SMALL_FLINCH;
		}

		if (HasCondition(COND_HEAR_DANGER) || HasCondition(COND_HEAR_COMBAT))
		{
			CSound *pSound;
			pSound = GetBestSound();

			if (pSound)
			{
				if (pSound->IsSoundType(SOUND_DANGER | SOUND_COMBAT))
				{
					if (gpGlobals->curtime - m_flFearTime > 3)
					{
						m_flFearTime = gpGlobals->curtime;
						return SCHED_SCI_STARTLE;
					}
				}
			}
		}

		if (GetFollowTarget())
		{
			if (!GetFollowTarget()->IsAlive())
			{
				StopFollowing();
				break;
			}

			int relationship = D_NU;

			if (pEnemy != NULL)
				relationship = IRelationType(pEnemy);

			if (relationship != D_HT)
			{
				return SCHED_TARGET_FACE;
			}
			else
			{
				if (HasCondition(COND_NEW_ENEMY))
					return SCHED_SCI_FEAR;
				return SCHED_SCI_FACETARGETSCARED;
			}
		}

		TrySmellTalk();
		break;


	case NPC_STATE_COMBAT:

		if (HasCondition(COND_NEW_ENEMY))
			return SCHED_SCI_FEAR;
		if (HasCondition(COND_SEE_ENEMY))
			return SCHED_SCI_COVER;

		if (HasCondition(COND_HEAR_COMBAT) || HasCondition(COND_HEAR_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;

		return SCHED_SCI_COVER;
		break;
	}

	return BaseClass::SelectSchedule();
}

void CHL1Scientist::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SAY_HEAL:

		GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 2);
		SetSpeechTarget(GetTarget());
		Speak(SC_HEAL);

		TaskComplete();
		break;

	case TASK_SCREAM:
		Scream();
		TaskComplete();
		break;

	case TASK_RANDOM_SCREAM:
		if (random->RandomFloat(0, 1) < pTask->flTaskData)
			Scream();
		TaskComplete();
		break;

	case TASK_SAY_FEAR:
		if (IsOkToSpeak())
		{
			GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 2);
			SetSpeechTarget(GetEnemy());
			if (GetEnemy() && GetEnemy()->IsPlayer())
				Speak(SC_PLFEAR);
			else
				Speak(SC_FEAR);
		}
		TaskComplete();
		break;

	case TASK_HEAL:
		SetIdealActivity(ACT_MELEE_ATTACK1);
		break;

	case TASK_RUN_PATH_SCARED:
		GetNavigator()->SetMovementActivity(ACT_RUN_SCARED);
		break;

	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
	{
		if (GetTarget() == NULL)
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else if ((GetTarget()->GetAbsOrigin() - GetAbsOrigin()).Length() < 1)
		{
			TaskComplete();
		}
	}
	break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CHL1Scientist::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RUN_PATH_SCARED:
		if (!IsMoving())
			TaskComplete();
		if (random->RandomInt(0, 31) < 8)
			Scream();
		break;

	case TASK_MOVE_TO_TARGET_RANGE_SCARED:
	{
		float distance;

		if (GetTarget() == NULL)
		{
			TaskFail(FAIL_NO_TARGET);
		}
		else
		{
			distance = (GetNavigator()->GetPath()->ActualGoalPosition() - GetAbsOrigin()).Length2D();
			if ((distance < pTask->flTaskData) || (GetNavigator()->GetPath()->ActualGoalPosition() - GetTarget()->GetAbsOrigin()).Length() > pTask->flTaskData * 0.5)
			{
				GetNavigator()->GetPath()->ResetGoalPosition(GetTarget()->GetAbsOrigin());
				distance = (GetNavigator()->GetPath()->ActualGoalPosition() - GetAbsOrigin()).Length2D();
				GetNavigator()->SetGoal(GOALTYPE_TARGETENT);
			}

			if (distance < pTask->flTaskData)
			{
				TaskComplete();
				GetNavigator()->GetPath()->Clear();
			}
			else
			{
				if (distance < 190 && GetNavigator()->GetMovementActivity() != ACT_WALK_SCARED)
					GetNavigator()->SetMovementActivity(ACT_WALK_SCARED);
				else if (distance >= 270 && GetNavigator()->GetMovementActivity() != ACT_RUN_SCARED)
					GetNavigator()->SetMovementActivity(ACT_RUN_SCARED);
			}
		}
	}
	break;

	case TASK_HEAL:
		if (IsSequenceFinished())
		{
			TaskComplete();
		}
		else
		{
			if (TargetDistance() > 90)
				TaskComplete();

			if (GetTarget())
				GetMotor()->SetIdealYaw(UTIL_VecToYaw(GetTarget()->GetAbsOrigin() - GetAbsOrigin()));
		}
		break;
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

NPC_STATE CHL1Scientist::SelectIdealState(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		if (HasCondition(COND_NEW_ENEMY))
		{
			if (GetFollowTarget() && GetEnemy())
			{
				int relationship = IRelationType(GetEnemy());
				if (relationship != D_FR || relationship != D_HT && (!HasCondition(COND_LIGHT_DAMAGE) || !HasCondition(COND_HEAVY_DAMAGE)))
				{
					return NPC_STATE_ALERT;
				}
				StopFollowing();
			}
		}
		else if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			if (GetFollowTarget())
				StopFollowing();
		}
		break;

	case NPC_STATE_COMBAT:
	{
		CBaseEntity *pEnemy = GetEnemy();
		if (pEnemy != NULL)
		{
			if (DisregardEnemy(pEnemy))
			{
				SetEnemy(NULL);
				return NPC_STATE_ALERT;
			}
			if (GetFollowTarget())
			{
				return NPC_STATE_ALERT;
			}

			if (HasCondition(COND_SEE_ENEMY))
			{
				m_flFearTime = gpGlobals->curtime;
				return NPC_STATE_COMBAT;
			}

		}
	}
	break;
	}

	return BaseClass::SelectIdealState();
}

int CHL1Scientist::FriendNumber(int arrayNumber)
{
	static int array[3] = { 1, 2, 0 };
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}

void CHL1Scientist::TalkInit()
{
	BaseClass::TalkInit();

	m_szFriends[0] = "monster_scientist";
	m_szFriends[1] = "monster_sitting_scientist";
	m_szFriends[2] = "monster_barney";

	switch (m_nBody % 3)
	{
	default:
	case HEAD_GLASSES:	GetExpresser()->SetVoicePitch(105);	break;
	case HEAD_EINSTEIN: GetExpresser()->SetVoicePitch(100);	break;
	case HEAD_LUTHER:	GetExpresser()->SetVoicePitch(95);	break;
	case HEAD_SLICK:	GetExpresser()->SetVoicePitch(100);	break;
	}
}

void CHL1Scientist::DeclineFollowing(void)
{
	if (CanSpeakAfterMyself())
	{
		Speak(SC_POK);
	}
}

bool CHL1Scientist::CanBecomeRagdoll(void)
{
	if (UTIL_IsLowViolence())
	{
		return false;
	}

	return BaseClass::CanBecomeRagdoll();
}

bool CHL1Scientist::ShouldGib(const CTakeDamageInfo &info)
{
	if (UTIL_IsLowViolence())
	{
		return false;
	}

	return BaseClass::ShouldGib(info);
}

void CHL1Scientist::SUB_StartLVFadeOut(float delay, bool notSolid)
{
	SetThink(&CHL1Scientist::SUB_LVFadeOut);
	SetNextThink(gpGlobals->curtime + delay);
	SetRenderColorA(255);
	m_nRenderMode = kRenderNormal;

	if (notSolid)
	{
		AddSolidFlags(FSOLID_NOT_SOLID);
		SetLocalAngularVelocity(vec3_angle);
	}
}

void CHL1Scientist::SUB_LVFadeOut(void)
{
	if (VPhysicsGetObject())
	{
		if (VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD || GetEFlags() & EFL_IS_BEING_LIFTED_BY_BARNACLE)
		{
			SetNextThink(gpGlobals->curtime + 5);
			SetRenderColorA(255);
			return;
		}
	}

	float dt = gpGlobals->frametime;
	if (dt > 0.1f)
	{
		dt = 0.1f;
	}
	m_nRenderMode = kRenderTransTexture;
	int speed = max(3, 256 * dt);
	SetRenderColorA(UTIL_Approach(0, m_clrRender->a, speed));
	NetworkStateChanged();

	if (m_clrRender->a == 0)
	{
		UTIL_Remove(this);
	}
	else
	{
		SetNextThink(gpGlobals->curtime);
	}
}

void CHL1Scientist::Scream(void)
{
	if (IsOkToSpeak())
	{
		GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 10);
		SetSpeechTarget(GetEnemy());
		Speak(SC_SCREAM);
	}
}

Activity CHL1Scientist::GetStoppedActivity(void)
{
	if (GetEnemy() != NULL)
		return (Activity)ACT_EXCITED;

	return BaseClass::GetStoppedActivity();
}

Activity CHL1Scientist::NPC_TranslateActivity(Activity newActivity)
{
	if (GetFollowTarget() && GetEnemy())
	{
		CBaseEntity *pEnemy = GetEnemy();

		int relationship = D_NU;

		if (pEnemy != NULL)
			relationship = IRelationType(pEnemy);

		if (relationship == D_HT || relationship == D_FR)
		{
			if (newActivity == ACT_WALK)
				return ACT_WALK_SCARED;
			else if (newActivity == ACT_RUN)
				return ACT_RUN_SCARED;
		}
	}
	return BaseClass::NPC_TranslateActivity(newActivity);
}

void CHL1Scientist::DeathSound(const CTakeDamageInfo &info)
{
	PainSound(info);
}

char *CHL1DeadScientist::m_szPoses[] = { "lying_on_back", "lying_on_stomach", "dead_sitting", "dead_hang", "dead_table1", "dead_table2", "dead_table3" };

bool CHL1DeadScientist::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "pose"))
		m_iPose = atoi(szValue);
	else
		BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

LINK_ENTITY_TO_CLASS(monster_scientist_dead, CHL1DeadScientist);

void CHL1DeadScientist::Spawn(void)
{
	PrecacheModel("models/scientist.mdl");
	SetModel("models/scientist.mdl");

	ClearEffects();
	SetSequence(0);
	m_bloodColor = BLOOD_COLOR_RED;

	SetRenderColor(255, 255, 255, 255);

	if (m_nBody == -1)
	{
		m_nBody = random->RandomInt(0, NUM_SCIENTIST_HEADS - 1);
	}
	if (m_nBody == HEAD_LUTHER)
		m_nSkin = 1;
	else
		m_nSkin = 0;

	SetSequence(LookupSequence(m_szPoses[m_iPose]));

	if (GetSequence() == -1)
	{
		Msg("Dead scientist with bad pose\n");
	}

	m_iHealth = 0.0;

	NPCInitDead();
}

LINK_ENTITY_TO_CLASS(monster_sitting_scientist, CHL1SittingScientist);

BEGIN_DATADESC(CHL1SittingScientist)
DEFINE_FIELD(m_iHeadTurn, FIELD_INTEGER),
DEFINE_FIELD(m_flResponseDelay, FIELD_FLOAT),
DEFINE_THINKFUNC(SittingThink),
END_DATADESC()

typedef enum
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
} SITTING_ANIM;

void CHL1SittingScientist::Spawn()
{
	PrecacheModel("models/scientist.mdl");
	SetModel("models/scientist.mdl");
	Precache();

	InitBoneControllers();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_iHealth = 50;

	m_bloodColor = BLOOD_COLOR_RED;
	m_flFieldOfView = VIEW_FIELD_WIDE;

	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_TURN_HEAD);

	m_spawnflags |= SF_PRE_DISASTER;

	if (m_nBody == -1)
	{
		m_nBody = random->RandomInt(0, NUM_SCIENTIST_HEADS - 1);
	}
	if (m_nBody == HEAD_LUTHER)
		m_nBody = 1;

	UTIL_DropToFloor(this, MASK_SOLID);

	NPCInit();

	SetThink(&CHL1SittingScientist::SittingThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_baseSequence = LookupSequence("sitlookleft");
	SetSequence(m_baseSequence + random->RandomInt(0, 4));
	ResetSequenceInfo();
}

void CHL1SittingScientist::Precache(void)
{
	m_baseSequence = LookupSequence("sitlookleft");
	TalkInit();
}

int CHL1SittingScientist::FriendNumber(int arrayNumber)
{
	static int array[3] = { 2, 1, 0 };
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}

void CHL1SittingScientist::SittingThink(void)
{
	CBaseEntity *pent;

	StudioFrameAdvance();

	if (0 && GetExpresser()->CanSpeakConcept(TLK_HELLO))
	{
		pent = FindNearestFriend(true);
		if (pent)
		{
			float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw > 0)
				SetSequence(m_baseSequence + SITTING_ANIM_sitlookleft);
			else
				SetSequence(m_baseSequence + SITTING_ANIM_sitlookright);

			ResetSequenceInfo();
			SetCycle(0);
			SetBoneController(0, 0);

			GetExpresser()->Speak(TLK_HELLO);
		}
	}
	else if (IsSequenceFinished())
	{
		int i = random->RandomInt(0, 99);
		m_iHeadTurn = 0;

		if (m_flResponseDelay && gpGlobals->curtime > m_flResponseDelay)
		{
			GetExpresser()->Speak(TLK_ANSWER);
			SetSequence(m_baseSequence + SITTING_ANIM_sitscared);
			m_flResponseDelay = 0;
		}
		else if (i < 30)
		{
			SetSequence(m_baseSequence + SITTING_ANIM_sitting3);

			pent = FindNamedEntity("!nearestfriend");

			if (!FIdleSpeak() || !pent)
			{
				m_iHeadTurn = random->RandomInt(0, 8) * 10 - 40;
				SetSequence(m_baseSequence + SITTING_ANIM_sitting3);
			}
			else
			{
				float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

				if (yaw > 180) yaw -= 360;
				if (yaw < -180) yaw += 360;

				if (yaw > 0)
					SetSequence(m_baseSequence + SITTING_ANIM_sitlookleft);
				else
					SetSequence(m_baseSequence + SITTING_ANIM_sitlookright);
			}
		}
		else if (i < 60)
		{
			SetSequence(m_baseSequence + SITTING_ANIM_sitting3);
			m_iHeadTurn = random->RandomInt(0, 8) * 10 - 40;
			if (random->RandomInt(0, 99) < 5)
			{
				FIdleSpeak();
			}
		}
		else if (i < 80)
		{
			SetSequence(m_baseSequence + SITTING_ANIM_sitting2);
		}
		else if (i < 100)
		{
			SetSequence(m_baseSequence + SITTING_ANIM_sitscared);
		}

		ResetSequenceInfo();
		SetCycle(0);
		SetBoneController(0, m_iHeadTurn);
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CHL1SittingScientist::SetAnswerQuestion(CNPCSimpleTalker *pSpeaker)
{
	m_flResponseDelay = gpGlobals->curtime + random->RandomFloat(3, 4);
	SetSpeechTarget((CNPCSimpleTalker *)pSpeaker);
}

int CHL1SittingScientist::FIdleSpeak(void)
{
	int pitch;

	if (!IsOkToSpeak())
		return FALSE;

	GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + random->RandomFloat(4.8, 5.2));

	pitch = GetExpresser()->GetVoicePitch();

	CBaseEntity *pentFriend = FindNamedEntity("!nearestfriend");

	if (pentFriend && random->RandomInt(0, 1))
	{
		SetSchedule(SCHED_TALKER_IDLE_RESPONSE);
		SetSpeechTarget(this);

		Msg("Asking some question!\n");

		IdleHeadTurn(pentFriend);
		SENTENCEG_PlayRndSz(edict(), TLK_PQUESTION, 1.0, SNDLVL_TALKING, 0, pitch);
		GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + random->RandomFloat(4.8, 5.2));
		return TRUE;
	}

	if (random->RandomInt(0, 1))
	{
		Msg("Making idle statement!\n");

		SENTENCEG_PlayRndSz(edict(), TLK_PIDLE, 1.0, SNDLVL_TALKING, 0, pitch);
		GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + random->RandomFloat(4.8, 5.2));
		return TRUE;
	}

	GetExpresser()->BlockSpeechUntil(0);
	return FALSE;
}

AI_BEGIN_CUSTOM_NPC(monster_scientist, CHL1Scientist)
DECLARE_TASK(TASK_SAY_HEAL)
DECLARE_TASK(TASK_HEAL)
DECLARE_TASK(TASK_SAY_FEAR)
DECLARE_TASK(TASK_RUN_PATH_SCARED)
DECLARE_TASK(TASK_SCREAM)
DECLARE_TASK(TASK_RANDOM_SCREAM)
DECLARE_TASK(TASK_MOVE_TO_TARGET_RANGE_SCARED)
DECLARE_ACTIVITY(ACT_EXCITED)

DEFINE_SCHEDULE
(
SCHED_SCI_HEAL,

"	Tasks"
"		TASK_GET_PATH_TO_TARGET				0"
"		TASK_MOVE_TO_TARGET_RANGE			50"
"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SCI_FOLLOWTARGET"
"		TASK_FACE_IDEAL						0"
"		TASK_SAY_HEAL						0"
"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_ARM"
//"		TASK_HEAL							0"
"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_MELEE_ATTACK1"
"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_DISARM"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_SCI_FOLLOWTARGET,

"	Tasks"
"		TASK_GET_PATH_TO_TARGET			0"
"		TASK_MOVE_TO_TARGET_RANGE		128"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_TARGET_FACE"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
)

DEFINE_SCHEDULE
(
SCHED_SCI_FACETARGET,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_TARGET			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_SCI_FOLLOWTARGET"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
"		COND_GIVE_WAY"
)

DEFINE_SCHEDULE
(
SCHED_SCI_COVER,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_SCI_PANIC"
"		TASK_STOP_MOVING				0"
"		TASK_FIND_COVER_FROM_ENEMY		0"
"		TASK_RUN_PATH_SCARED			0"
"		TASK_TURN_LEFT					179"
"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_SCI_HIDE"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
SCHED_SCI_HIDE,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_SCI_PANIC"
"		TASK_STOP_MOVING			0"
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CROUCHIDLE"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_CROUCHIDLE"
"		TASK_WAIT_RANDOM			10"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_SEE_ENEMY"
"		COND_SEE_HATE"
"		COND_SEE_FEAR"
"		COND_SEE_DISLIKE"
"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
SCHED_SCI_IDLESTAND,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
"		TASK_WAIT					2"
"		TASK_TALKER_HEADRESET		0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_SMELL"
"		COND_PROVOKED"
"		COND_HEAR_COMBAT"
"		COND_GIVE_WAY"
)

DEFINE_SCHEDULE
(
SCHED_SCI_PANIC,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_FACE_ENEMY						0"
"		TASK_SCREAM							0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_EXCITED"
"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
"	"
"	Interrupts"
)

DEFINE_SCHEDULE
(
SCHED_SCI_FOLLOWSCARED,

"	Tasks"
"		TASK_GET_PATH_TO_TARGET				0"
"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SCI_FOLLOWTARGET"
"		TASK_MOVE_TO_TARGET_RANGE_SCARED	128"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
SCHED_SCI_FACETARGETSCARED,

"	Tasks"
"	TASK_FACE_TARGET				0"
"		TASK_SET_ACTIVITY			ACTIVITY:ACT_CROUCHIDLE"
"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_SCI_FOLLOWSCARED"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
SCHED_SCI_FEAR,

"	Tasks"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_ENEMY				0"
"		TASK_SAY_FEAR				0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
SCHED_SCI_STARTLE,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SCI_PANIC"
"		TASK_RANDOM_SCREAM					0.3"
"		TASK_STOP_MOVING					0"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCH"
"		TASK_RANDOM_SCREAM					0.1"
"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCHIDLE"
"		TASK_WAIT_RANDOM					1"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_SEE_ENEMY"
"		COND_SEE_HATE"
"		COND_SEE_FEAR"
"		COND_SEE_DISLIKE"
)

AI_END_CUSTOM_NPC()