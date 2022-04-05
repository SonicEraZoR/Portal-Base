#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"decals.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"hl1_monster.h"
#include	"ai_navigator.h"
#include	"decals.h"
#include	"effect_dispatch_data.h"
#include	"te_effect_dispatch.h"
#include	"Sprite.h"
#include	"physics_bone_follower.h"

#define	BIG_AE_STEP1				1
#define	BIG_AE_STEP2				2
#define	BIG_AE_STEP3				3
#define	BIG_AE_STEP4				4
#define BIG_AE_SACK					5
#define BIG_AE_DEATHSOUND			6

#define	BIG_AE_MELEE_ATTACKBR		8
#define	BIG_AE_MELEE_ATTACKBL		9
#define	BIG_AE_MELEE_ATTACK1		10
#define BIG_AE_MORTAR_ATTACK1		11
#define BIG_AE_LAY_CRAB				12
#define BIG_AE_JUMP_FORWARD			13
#define BIG_AE_SCREAM				14
#define BIG_AE_PAIN_SOUND			15
#define BIG_AE_ATTACK_SOUND			16
#define BIG_AE_BIRTH_SOUND			17
#define BIG_AE_EARLY_TARGET			50

#define	BIG_ATTACKDIST		170
#define BIG_MORTARDIST		800
#define BIG_MAXCHILDREN		6


#define bits_MEMORY_CHILDPAIR		(bits_MEMORY_CUSTOM1)
#define bits_MEMORY_ADVANCE_NODE	(bits_MEMORY_CUSTOM2)
#define bits_MEMORY_COMPLETED_NODE	(bits_MEMORY_CUSTOM3)
#define bits_MEMORY_FIRED_NODE		(bits_MEMORY_CUSTOM4)

#define SF_INFOBM_RUN		0x0001
#define SF_INFOBM_WAIT		0x0002

#define BIG_CHILDCLASS		"monster_babycrab"

ConVar sk_bigmomma_health_factor("sk_bigmomma_health_factor", "1");
ConVar sk_bigmomma_dmg_slash("sk_bigmomma_dmg_slash", "50");
ConVar sk_bigmomma_dmg_blast("sk_bigmomma_dmg_blast", "100");
ConVar sk_bigmomma_radius_blast("sk_bigmomma_radius_blast", "250");

extern ConVar sv_gravity;
extern ConVar hl1_bigmomma_splash;

enum
{
	SCHED_NODE_FAIL = LAST_SHARED_SCHEDULE,
	SCHED_BIG_NODE,
};

enum
{
	TASK_MOVE_TO_NODE_RANGE = LAST_SHARED_TASK,
	TASK_FIND_NODE,
	TASK_PLAY_NODE_PRESEQUENCE,
	TASK_PLAY_NODE_SEQUENCE,
	TASK_PROCESS_NODE,
	TASK_WAIT_NODE,
	TASK_NODE_DELAY,
	TASK_NODE_YAW,
	TASK_CHECK_NODE_PROXIMITY,
};

int gSpitSprite, gSpitDebrisSprite;

Vector VecCheckSplatToss(CBaseEntity* pEnt, const Vector& vecSpot1, Vector vecSpot2, float maxHeight)
{
	trace_t			tr;
	Vector			vecMidPoint;
	Vector			vecApex;
	Vector			vecScale;
	Vector			vecGrenadeVel;
	Vector			vecTemp;
	float			flGravity = sv_gravity.GetFloat();

	vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	UTIL_TraceLine(vecMidPoint, vecMidPoint + Vector(0, 0, maxHeight), MASK_SOLID_BRUSHONLY, pEnt, COLLISION_GROUP_NONE, &tr);
	vecApex = tr.endpos;

	UTIL_TraceLine(vecSpot1, vecApex, MASK_SOLID, pEnt, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		return vec3_origin;
	}

	float height = (vecApex.z - vecSpot1.z) - 15;

	if (height < 0)
		height *= -1;

	float speed = sqrt(2 * flGravity * height);

	float time = speed / flGravity;
	vecGrenadeVel = (vecSpot2 - vecSpot1);
	vecGrenadeVel.z = 0;

	vecGrenadeVel = vecGrenadeVel * (0.5 / time);
	vecGrenadeVel.z = speed;

	return vecGrenadeVel;
}

void MortarSpray(const Vector& position, const Vector& direction, int spriteModel, int count)
{
	CPVSFilter filter(position);

	te->SpriteSpray(filter, 0.0, &position, &direction, spriteModel, 200, 80, count);
}

class CBMortar : public CBaseAnimating
{
	DECLARE_CLASS(CBMortar, CBaseAnimating);
public:
	void Spawn(void);

	virtual void Precache();

	static CBMortar* Shoot(CBaseEntity* pOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther);
	void Animate(void);

	float	m_flDmgTime;

	DECLARE_DATADESC();

	int  m_maxFrame;
	int  m_iFrame;

	CSprite* pSprite;
};

LINK_ENTITY_TO_CLASS(bmortar, CBMortar);

BEGIN_DATADESC(CBMortar)
DEFINE_FIELD(m_maxFrame, FIELD_INTEGER),
DEFINE_FIELD(m_flDmgTime, FIELD_FLOAT),
DEFINE_FUNCTION(Animate),
DEFINE_FIELD(m_iFrame, FIELD_INTEGER),
DEFINE_FIELD(pSprite, FIELD_CLASSPTR),
END_DATADESC()

void CBMortar::Spawn(void)
{
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetClassname("bmortar");

	SetSolid(SOLID_BBOX);

	pSprite = CSprite::SpriteCreate("sprites/mommaspit.vmt", GetAbsOrigin(), true);

	if (pSprite)
	{
		pSprite->SetAttachment(this, 0);
		pSprite->m_flSpriteFramerate = 5;

		pSprite->m_nRenderMode = kRenderTransAlpha;
		pSprite->SetBrightness(255);

		m_iFrame = 0;

		pSprite->SetScale(2.5f);
	}

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));

	m_maxFrame = (float)modelinfo->GetModelFrameCount(GetModel()) - 1;
	m_flDmgTime = gpGlobals->curtime + 0.4;
}

void CBMortar::Animate(void)
{
	SetNextThink(gpGlobals->curtime + 0.1);

	Vector vVelocity = GetAbsVelocity();

	VectorNormalize(vVelocity);

	if (gpGlobals->curtime > m_flDmgTime)
	{
		m_flDmgTime = gpGlobals->curtime + 0.2;
		MortarSpray(GetAbsOrigin() + Vector(0, 0, 15), -vVelocity, gSpitSprite, 3);
	}
	if (m_iFrame++)
	{
		if (m_iFrame > m_maxFrame)
		{
			m_iFrame = 0;
		}
	}
}

CBMortar* CBMortar::Shoot(CBaseEntity* pOwner, Vector vecStart, Vector vecVelocity)
{
	CBMortar* pSpit = CREATE_ENTITY(CBMortar, "bmortar");
	pSpit->Spawn();

	UTIL_SetOrigin(pSpit, vecStart);
	pSpit->SetAbsVelocity(vecVelocity);
	pSpit->SetOwnerEntity(pOwner);
	pSpit->SetThink(&CBMortar::Animate);
	pSpit->SetNextThink(gpGlobals->curtime + 0.1);

	return pSpit;
}

void CBMortar::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("NPC_BigMomma.SpitTouch1");
	PrecacheScriptSound("NPC_BigMomma.SpitHit1");
	PrecacheScriptSound("NPC_BigMomma.SpitHit2");
}

void CBMortar::Touch(CBaseEntity* pOther)
{
	trace_t tr;
	int		iPitch;

	iPitch = random->RandomFloat(90, 110);

	EmitSound("NPC_BigMomma.SpitTouch1");

	switch (random->RandomInt(0, 1))
	{
	case 0:
		EmitSound("NPC_BigMomma.SpitHit1");
		break;
	case 1:
		EmitSound("NPC_BigMomma.SpitHit2");
		break;
	}

	if (pOther->IsBSPModel())
	{
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		if(hl1_bigmomma_splash.GetBool())
			UTIL_DecalTrace(&tr, "BigMommaSplash1");
		else
			UTIL_DecalTrace(&tr, "BigMommaSplash2");
	}
	else
	{
		tr.endpos = GetAbsOrigin();

		Vector vVelocity = GetAbsVelocity();
		VectorNormalize(vVelocity);

		tr.plane.normal = -1 * vVelocity;
	}
	MortarSpray(tr.endpos + Vector(0, 0, 15), tr.plane.normal, gSpitSprite, 24);

	CBaseEntity* pOwner = GetOwnerEntity();

	RadiusDamage(CTakeDamageInfo(this, pOwner, sk_bigmomma_dmg_blast.GetFloat(), DMG_ACID), GetAbsOrigin(), sk_bigmomma_radius_blast.GetFloat(), CLASS_NONE, NULL);

	UTIL_Remove(pSprite);
	UTIL_Remove(this);
}

class CInfoBM : public CPointEntity
{
	DECLARE_CLASS(CInfoBM, CPointEntity);
public:
	void Spawn(void);
	bool KeyValue(const char* szKeyName, const char* szValue);

	DECLARE_DATADESC();

	float   m_flRadius;
	float   m_flDelay;
	string_t m_iszReachTarget;
	string_t m_iszReachSequence;
	string_t m_iszPreSequence;

	COutputEvent m_OnAnimationEvent;
};

LINK_ENTITY_TO_CLASS(info_bigmomma, CInfoBM);

BEGIN_DATADESC(CInfoBM)
DEFINE_FIELD(m_flRadius, FIELD_FLOAT),
DEFINE_FIELD(m_flDelay, FIELD_FLOAT),
DEFINE_KEYFIELD(m_iszReachTarget, FIELD_STRING, "reachtarget"),
DEFINE_KEYFIELD(m_iszReachSequence, FIELD_STRING, "reachsequence"),
DEFINE_KEYFIELD(m_iszPreSequence, FIELD_STRING, "presequence"),
DEFINE_OUTPUT(m_OnAnimationEvent, "OnAnimationEvent"),
END_DATADESC()

void CInfoBM::Spawn(void)
{
	BaseClass::Spawn();
}

bool CInfoBM::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "radius"))
	{
		m_flRadius = atof(szValue);
		return true;
	}
	else if (FStrEq(szKeyName, "reachdelay"))
	{
		m_flDelay = atof(szValue);
		return true;
	}
	else if (FStrEq(szKeyName, "health"))
	{
		m_iHealth = atoi(szValue);
		return true;
	}

	return BaseClass::KeyValue(szKeyName, szValue);
}

class CHL1BigMomma : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1BigMomma, CHL1BaseMonster);
public:

	void Spawn(void);
	void Precache(void);

	Class_T	Classify(void) { return CLASS_ALIEN_MONSTER; };
	void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);
	int	OnTakeDamage(const CTakeDamageInfo& info);
	void HandleAnimEvent(animevent_t* pEvent);
	void LayHeadcrab(void);
	void LaunchMortar(void);
	void DeathNotice(CBaseEntity* pevChild);

	int MeleeAttack1Conditions(float flDot, float flDist);
	int MeleeAttack2Conditions(float flDot, float flDist);
	int RangeAttack1Conditions(float flDot, float flDist);

	BOOL CanLayCrab(void)
	{
		if (m_crabTime < gpGlobals->curtime && m_crabCount < BIG_MAXCHILDREN)
		{
			Vector mins = GetAbsOrigin() - Vector(32, 32, 0);
			Vector maxs = GetAbsOrigin() + Vector(32, 32, 0);

			CBaseEntity* pList[2];
			int count = UTIL_EntitiesInBox(pList, 2, mins, maxs, FL_NPC);
			for (int i = 0; i < count; i++)
			{
				if (pList[i] != this)
					return COND_NONE;
			}
			return COND_CAN_MELEE_ATTACK2;
		}

		return COND_NONE;
	}

	void Activate(void);

	void NodeReach(void);
	void NodeStart(string_t iszNextNode);
	bool ShouldGoToNode(void);

	const char* GetNodeSequence(void)
	{
		CInfoBM* pTarget = (CInfoBM*)GetTarget();

		if (pTarget && FClassnameIs(pTarget, "info_bigmomma"))
		{
			return STRING(pTarget->m_iszReachSequence);
		}

		return NULL;
	}


	const char* GetNodePresequence(void)
	{
		CInfoBM* pTarget = (CInfoBM*)GetTarget();

		if (pTarget && FClassnameIs(pTarget, "info_bigmomma"))
		{
			return STRING(pTarget->m_iszPreSequence);
		}
		return NULL;
	}

	float GetNodeDelay(void)
	{
		CInfoBM* pTarget = (CInfoBM*)GetTarget();

		if (pTarget && FClassnameIs(pTarget, "info_bigmomma"))
		{
			return pTarget->m_flDelay;
		}
		return 0;
	}

	float GetNodeRange(void)
	{
		CInfoBM* pTarget = (CInfoBM*)GetTarget();

		if (pTarget && FClassnameIs(pTarget, "info_bigmomma"))
		{
			return pTarget->m_flRadius;
		}

		return 1e6;
	}

	float GetNodeYaw(void)
	{
		CBaseEntity* pTarget = GetTarget();

		if (pTarget)
		{
			if (pTarget->GetAbsAngles().y != 0)
				return pTarget->GetAbsAngles().y;
		}

		return GetAbsAngles().y;
	}

	void OnRestore(void)
	{
		BaseClass::OnRestore();
		m_crabCount = 0;
	}

	int SelectSchedule(void);
	void StartTask(const Task_t* pTask);
	void RunTask(const Task_t* pTask);

	float MaxYawSpeed(void);

	DECLARE_DATADESC();

	DEFINE_CUSTOM_AI;

private:
	float	m_nodeTime;
	float	m_crabTime;
	float	m_mortarTime;
	float	m_painSoundTime;
	int		m_crabCount;
	float	m_flDmgTime;

	bool	m_bDoneWithPath;

	string_t m_iszTarget;
	string_t m_iszNetName;
	float	m_flWait;

	Vector m_vTossDir;
};

LINK_ENTITY_TO_CLASS(monster_bigmomma, CHL1BigMomma);

BEGIN_DATADESC(CHL1BigMomma)
DEFINE_FIELD(m_nodeTime, FIELD_TIME),
DEFINE_FIELD(m_crabTime, FIELD_TIME),
DEFINE_FIELD(m_mortarTime, FIELD_TIME),
DEFINE_FIELD(m_painSoundTime, FIELD_TIME),
DEFINE_KEYFIELD(m_iszNetName, FIELD_STRING, "netname"),
DEFINE_FIELD(m_flWait, FIELD_TIME),
DEFINE_FIELD(m_iszTarget, FIELD_STRING),
DEFINE_FIELD(m_crabCount, FIELD_INTEGER),
DEFINE_FIELD(m_flDmgTime, FIELD_TIME),
DEFINE_FIELD(m_bDoneWithPath, FIELD_BOOLEAN),
DEFINE_FIELD(m_vTossDir, FIELD_VECTOR),
END_DATADESC()

void CHL1BigMomma::Spawn()
{
	Precache();

	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	SetSolid(SOLID_BBOX);

	AddSolidFlags(FSOLID_CUSTOMRAYTEST);

	SetModel("models/big_mom.mdl");
	UTIL_SetSize(this, Vector(-32, -32, 0), Vector(32, 32, 64));

	//Vector vecSurroundingMins(-95, -95, 0);
	//Vector vecSurroundingMaxs(95, 95, 190);
	//CollisionProp()->SetSurroundingBoundsType(USE_SPECIFIED_BOUNDS, &vecSurroundingMins, &vecSurroundingMaxs);
	CollisionProp()->SetSurroundingBoundsType(USE_HITBOXES);

	SetNavType(NAV_GROUND);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	m_bloodColor = BLOOD_COLOR_GREEN;
	m_iHealth = 150 * sk_bigmomma_health_factor.GetFloat();

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2);

	m_flFieldOfView = 0.3;
	m_NPCState = NPC_STATE_NONE;

	SetRenderColor(255, 255, 255, 255);

	m_bDoneWithPath = false;

	m_nodeTime = 0.0f;

	m_iszTarget = m_iszNetName;

	NPCInit();

	BaseClass::Spawn();
}

void CHL1BigMomma::Precache()
{
	PrecacheModel("models/big_mom.mdl");

	UTIL_PrecacheOther(BIG_CHILDCLASS);

	PrecacheModel("sprites/mommaspit.vmt");
	gSpitSprite = PrecacheModel("sprites/mommaspout.vmt");
	gSpitDebrisSprite = PrecacheModel("sprites/mommablob.vmt");

	PrecacheScriptSound("BigMomma.Pain");
	PrecacheScriptSound("BigMomma.Attack");
	PrecacheScriptSound("BigMomma.AttackHit");
	PrecacheScriptSound("BigMomma.Alert");
	PrecacheScriptSound("BigMomma.Birth");
	PrecacheScriptSound("BigMomma.Sack");
	PrecacheScriptSound("BigMomma.Die");
	PrecacheScriptSound("BigMomma.FootstepLeft");
	PrecacheScriptSound("BigMomma.FootstepRight");
	PrecacheScriptSound("BigMomma.LayHeadcrab");
	PrecacheScriptSound("BigMomma.ChildDie");
	PrecacheScriptSound("BigMomma.LaunchMortar");
}

float CHL1BigMomma::MaxYawSpeed(void)
{
	float ys = 90.0f;

	switch (GetActivity())
	{
	case ACT_IDLE:
		ys = 100.0f;
		break;
	default:
		ys = 90.0f;
	}

	return ys;
}

void CHL1BigMomma::Activate(void)
{
	if (GetTarget() == NULL)
		Remember(bits_MEMORY_ADVANCE_NODE);

	BaseClass::Activate();
}

void CHL1BigMomma::NodeStart(string_t iszNextNode)
{
	m_iszTarget = iszNextNode;

	const char* pTargetName = STRING(m_iszTarget);

	CBaseEntity* pTarget = NULL;

	if (pTargetName)
		pTarget = gEntList.FindEntityByName(NULL, pTargetName);

	if (pTarget == NULL)
	{
		m_bDoneWithPath = true;
		return;
	}

	SetTarget(pTarget);
}

void CHL1BigMomma::NodeReach(void)
{
	CInfoBM* pTarget = (CInfoBM*)GetTarget();

	Forget(bits_MEMORY_ADVANCE_NODE);

	if (!pTarget)
		return;

	if (pTarget->m_iHealth >= 1)
		m_iMaxHealth = m_iHealth = pTarget->m_iHealth * sk_bigmomma_health_factor.GetFloat();

	if (!HasMemory(bits_MEMORY_FIRED_NODE))
	{
		if (pTarget)
		{
			pTarget->m_OnAnimationEvent.FireOutput(this, this);
		}
	}

	Forget(bits_MEMORY_FIRED_NODE);

	m_iszTarget = pTarget->m_target;

	if (pTarget->m_iHealth == 0)
		Remember(bits_MEMORY_ADVANCE_NODE);
	else
	{
		GetNavigator()->ClearGoal();
	}
}


void CHL1BigMomma::TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr)
{
	
	CTakeDamageInfo dmgInfo = info;
	
	if (ptr->hitbox <= 9 && ptr->hitbox > 0)
	{
		if (m_flDmgTime != gpGlobals->curtime || (random->RandomInt(0, 10) < 1))
		{
			g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
			m_flDmgTime = gpGlobals->curtime;
		}

		dmgInfo.SetDamage(0.1);
	}
	else
	{
		SpawnBlood(ptr->endpos + ptr->plane.normal * 15, vecDir, m_bloodColor, 100);

		if (gpGlobals->curtime > m_painSoundTime)
		{
			m_painSoundTime = gpGlobals->curtime + random->RandomInt(1, 3);
			EmitSound("BigMomma.Pain");
		}
	}
	
	BaseClass::TraceAttack(dmgInfo, vecDir, ptr);
}

int CHL1BigMomma::OnTakeDamage(const CTakeDamageInfo& info)
{
	CTakeDamageInfo newInfo = info;

	if (newInfo.GetDamageType() & DMG_ACID)
	{
		newInfo.SetDamage(0);
	}

	if ((GetHealth() - newInfo.GetDamage()) < 1)
	{
		newInfo.SetDamage(0);
		Remember(bits_MEMORY_ADVANCE_NODE);
		DevMsg(2, "BM: Finished node health!!!\n");
	}

	DevMsg(2, "BM Health: %f\n", GetHealth() - newInfo.GetDamage());

	return BaseClass::OnTakeDamage(newInfo);
}

bool CHL1BigMomma::ShouldGoToNode(void)
{
	if (HasMemory(bits_MEMORY_ADVANCE_NODE))
	{
		if (m_nodeTime < gpGlobals->curtime)
			return true;
	}
	return false;
}


int CHL1BigMomma::SelectSchedule(void)
{
	if (ShouldGoToNode())
	{
		return SCHED_BIG_NODE;
	}

	switch (m_NPCState)
	{
	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
			return BaseClass::SelectSchedule();

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_RANGE_ATTACK1;

		if (HasCondition(COND_CAN_MELEE_ATTACK1))
			return SCHED_MELEE_ATTACK1;

		if (HasCondition(COND_CAN_MELEE_ATTACK2))
			return SCHED_MELEE_ATTACK2;

		return SCHED_CHASE_ENEMY;

		break;
	}
	}

	return BaseClass::SelectSchedule();
}

void CHL1BigMomma::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_CHECK_NODE_PROXIMITY:
	{

	}

	break;
	case TASK_FIND_NODE:
	{
		CBaseEntity* pTarget = GetTarget();

		if (!HasMemory(bits_MEMORY_ADVANCE_NODE))
		{
			if (pTarget)
				m_iszTarget = pTarget->m_target;
		}

		NodeStart(m_iszTarget);
		TaskComplete();
	}
	break;

	case TASK_NODE_DELAY:
		m_nodeTime = gpGlobals->curtime + pTask->flTaskData;
		TaskComplete();
		break;

	case TASK_PROCESS_NODE:
		NodeReach();
		TaskComplete();
		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:
	{
		const char* pSequence = NULL;
		int	iSequence;

		if (pTask->iTask == TASK_PLAY_NODE_SEQUENCE)
			pSequence = GetNodeSequence();
		else
			pSequence = GetNodePresequence();

		if (pSequence)
		{
			iSequence = LookupSequence(pSequence);

			if (iSequence != -1)
			{
				SetIdealActivity(ACT_DO_NOT_DISTURB);
				SetSequence(iSequence);
				SetCycle(0.0f);

				ResetSequenceInfo();
				return;
			}
		}
		TaskComplete();
	}
	break;

	case TASK_NODE_YAW:
		GetMotor()->SetIdealYaw(GetNodeYaw());
		TaskComplete();
		break;

	case TASK_WAIT_NODE:
		m_flWait = gpGlobals->curtime + GetNodeDelay();
		break;


	case TASK_MOVE_TO_NODE_RANGE:
	{
		CBaseEntity* pTarget = GetTarget();

		if (!pTarget)
			TaskFail(FAIL_NO_TARGET);
		else
		{
			if ((pTarget->GetAbsOrigin() - GetAbsOrigin()).Length() < GetNodeRange())
				TaskComplete();
			else
			{
				Activity act = ACT_WALK;
				if (pTarget->GetSpawnFlags() & SF_INFOBM_RUN)
					act = ACT_RUN;

				AI_NavGoal_t goal(GOALTYPE_TARGETENT, vec3_origin, act);

				if (!GetNavigator()->SetGoal(goal))
				{
					TaskFail(NO_TASK_FAILURE);
				}
			}
		}
	}
	break;

	case TASK_MELEE_ATTACK1:
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "BigMomma.Attack");

		BaseClass::StartTask(pTask);
	}

	break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CHL1BigMomma::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_CHECK_NODE_PROXIMITY:
	{
		float distance;

		if (GetTarget() == NULL)
			TaskFail(FAIL_NO_TARGET);
		else
		{
			if (GetNavigator()->IsGoalActive())
			{
				distance = (GetTarget()->GetAbsOrigin() - GetAbsOrigin()).Length2D();

				if (distance < GetNodeRange())
				{
					TaskComplete();
					GetNavigator()->ClearGoal();
				}
			}
			else
				TaskComplete();
		}
	}

	break;

	case TASK_WAIT_NODE:
		if (GetTarget() != NULL && (GetTarget()->GetSpawnFlags() & SF_INFOBM_WAIT))
			return;

		if (gpGlobals->curtime > m_flWaitFinished)
			TaskComplete();

		break;

	case TASK_PLAY_NODE_PRESEQUENCE:
	case TASK_PLAY_NODE_SEQUENCE:

		if (IsSequenceFinished())
		{
			CBaseEntity* pTarget = NULL;

			if (GetTarget())
				pTarget = gEntList.FindEntityByName(NULL, STRING(GetTarget()->m_target));

			if (pTarget)
			{
				SetActivity(ACT_IDLE);
				TaskComplete();
			}
		}

		break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

void CHL1BigMomma::HandleAnimEvent(animevent_t* pEvent)
{
	CPASAttenuationFilter filter(this);

	Vector vecFwd, vecRight, vecUp;
	QAngle angles;
	angles = GetAbsAngles();
	AngleVectors(angles, &vecFwd, &vecRight, &vecUp);

	switch (pEvent->event)
	{
	case BIG_AE_MELEE_ATTACKBR:
	case BIG_AE_MELEE_ATTACKBL:
	case BIG_AE_MELEE_ATTACK1:
	{
		Vector center = GetAbsOrigin() + vecFwd * 128;
		Vector mins = center - Vector(64, 64, 0);
		Vector maxs = center + Vector(64, 64, 64);

		CBaseEntity* pList[8];
		int count = UTIL_EntitiesInBox(pList, 8, mins, maxs, FL_NPC | FL_CLIENT);
		CBaseEntity* pHurt = NULL;

		for (int i = 0; i < count && !pHurt; i++)
		{
			if (pList[i] != this)
			{
				if (pList[i]->GetOwnerEntity() != this)
				{
					pHurt = pList[i];
				}
			}
		}

		if (pHurt)
		{
			CTakeDamageInfo info(this, this, 15, DMG_CLUB | DMG_SLASH);
			CalculateMeleeDamageForce(&info, (pHurt->GetAbsOrigin() - GetAbsOrigin()), pHurt->GetAbsOrigin());
			pHurt->TakeDamage(info);
			QAngle newAngles = angles;
			newAngles.x = 15;
			if (pHurt->IsPlayer())
			{
				((CBasePlayer*)pHurt)->SetPunchAngle(newAngles);
			}
			switch (pEvent->event)
			{
			case BIG_AE_MELEE_ATTACKBR:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (vecFwd * 150) + Vector(0, 0, 250) - (vecRight * 200));
				break;

			case BIG_AE_MELEE_ATTACKBL:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (vecFwd * 150) + Vector(0, 0, 250) + (vecRight * 200));
				break;

			case BIG_AE_MELEE_ATTACK1:
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (vecFwd * 220) + Vector(0, 0, 200));
				break;
			}

			pHurt->SetGroundEntity(NULL);
			EmitSound(filter, entindex(), "BigMomma.AttackHit");
		}
	}
	break;

	case BIG_AE_SCREAM:
		EmitSound(filter, entindex(), "BigMomma.Alert");
		break;

	case BIG_AE_PAIN_SOUND:
		EmitSound(filter, entindex(), "BigMomma.Pain");
		break;

	case BIG_AE_ATTACK_SOUND:
		EmitSound(filter, entindex(), "BigMomma.Attack");
		break;

	case BIG_AE_BIRTH_SOUND:
		EmitSound(filter, entindex(), "BigMomma.Birth");
		break;

	case BIG_AE_SACK:
		if (RandomInt(0, 100) < 30)
		{
			EmitSound(filter, entindex(), "BigMomma.Sack");
		}
		break;

	case BIG_AE_DEATHSOUND:
		EmitSound(filter, entindex(), "BigMomma.Die");
		break;

	case BIG_AE_STEP1:
	case BIG_AE_STEP3:
		EmitSound(filter, entindex(), "BigMomma.FootstepLeft");
		break;

	case BIG_AE_STEP2:
		EmitSound(filter, entindex(), "BigMomma.FootstepRight");
		break;

	case BIG_AE_MORTAR_ATTACK1:
		LaunchMortar();
		break;

	case BIG_AE_LAY_CRAB:
		LayHeadcrab();
		break;

	case BIG_AE_JUMP_FORWARD:
		SetGroundEntity(NULL);
		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));
		SetAbsVelocity(vecFwd * 200 + vecUp * 500);
		break;

	case BIG_AE_EARLY_TARGET:
	{
		CInfoBM* pTarget = (CInfoBM*)GetTarget();

		if (pTarget)
		{
			pTarget->m_OnAnimationEvent.FireOutput(this, this);
		}

		Remember(bits_MEMORY_FIRED_NODE);
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}


void CHL1BigMomma::LayHeadcrab(void)
{
	CBaseEntity* pChild = CBaseEntity::Create(BIG_CHILDCLASS, GetAbsOrigin(), GetAbsAngles(), this);

	pChild->AddSpawnFlags(SF_NPC_FALL_TO_GROUND);

	pChild->SetOwnerEntity(this);

	if (HasMemory(bits_MEMORY_CHILDPAIR))
	{
		m_crabTime = gpGlobals->curtime + RandomFloat(5, 10);
		Forget(bits_MEMORY_CHILDPAIR);
	}
	else
	{
		m_crabTime = gpGlobals->curtime + RandomFloat(0.5, 2.5);
		Remember(bits_MEMORY_CHILDPAIR);
	}

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 100), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
	if (hl1_bigmomma_splash.GetBool())
		UTIL_DecalTrace(&tr, "BigMommaSplash1");
	else
		UTIL_DecalTrace(&tr, "BigMommaSplash2");

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "BigMomma.LayHeadcrab");

	m_crabCount++;
}

void CHL1BigMomma::DeathNotice(CBaseEntity* pevChild)
{
	if (m_crabCount > 0)
	{
		m_crabCount--;
	}
	if (IsAlive())
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "BigMomma.ChildDie");
	}
}


void CHL1BigMomma::LaunchMortar(void)
{
	m_mortarTime = gpGlobals->curtime + RandomFloat(2, 15);

	Vector startPos = GetAbsOrigin();
	startPos.z += 180;

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "BigMomma.LaunchMortar");

	CBMortar* pBomb = CBMortar::Shoot(this, startPos, m_vTossDir);
	pBomb->SetGravity(1.0);
	MortarSpray(startPos, Vector(0, 0, 10), gSpitSprite, 24);
}

int CHL1BigMomma::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDot >= 0.7)
	{
		if (flDist > BIG_ATTACKDIST)
			return COND_TOO_FAR_TO_ATTACK;
		else
			return COND_CAN_MELEE_ATTACK1;
	}
	else
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_NONE;
}

int CHL1BigMomma::MeleeAttack2Conditions(float flDot, float flDist)
{
	return CanLayCrab();
}

int CHL1BigMomma::RangeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > BIG_MORTARDIST)
		return COND_TOO_FAR_TO_ATTACK;

	if (flDist <= BIG_MORTARDIST && m_mortarTime < gpGlobals->curtime)
	{
		CBaseEntity* pEnemy = GetEnemy();

		if (pEnemy)
		{
			Vector startPos = GetAbsOrigin();
			startPos.z += 180;

			m_vTossDir = VecCheckSplatToss(this, startPos, pEnemy->BodyTarget(GetAbsOrigin()), random->RandomFloat(150, 500));

			if (m_vTossDir != vec3_origin)
				return COND_CAN_RANGE_ATTACK1;
		}
	}

	return COND_NONE;
}

AI_BEGIN_CUSTOM_NPC(monster_bigmomma, CHL1BigMomma)

DECLARE_TASK(TASK_MOVE_TO_NODE_RANGE)
DECLARE_TASK(TASK_FIND_NODE)
DECLARE_TASK(TASK_PLAY_NODE_PRESEQUENCE)
DECLARE_TASK(TASK_PLAY_NODE_SEQUENCE)
DECLARE_TASK(TASK_PROCESS_NODE)
DECLARE_TASK(TASK_WAIT_NODE)
DECLARE_TASK(TASK_NODE_DELAY)
DECLARE_TASK(TASK_NODE_YAW)
DECLARE_TASK(TASK_CHECK_NODE_PROXIMITY)

DEFINE_SCHEDULE
(
	SCHED_NODE_FAIL,

	"	Tasks"
	"		TASK_NODE_DELAY						3"
	"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
	"	"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_BIG_NODE,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_NODE_FAIL"
	"		TASK_STOP_MOVING					0"
	"		TASK_FIND_NODE						0"
	"		TASK_PLAY_NODE_PRESEQUENCE			0"
	"		TASK_MOVE_TO_NODE_RANGE				0"
	"		TASK_CHECK_NODE_PROXIMITY			0"
	"		TASK_STOP_MOVING					0"
	"		TASK_NODE_YAW						0"
	"		TASK_FACE_IDEAL						0"
	"		TASK_WAIT_NODE						0"
	"		TASK_PLAY_NODE_SEQUENCE				0"
	"		TASK_PROCESS_NODE					0"

	"	"
	"	Interrupts"
)

AI_END_CUSTOM_NPC()