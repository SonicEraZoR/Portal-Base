#include	"cbase.h"
#include	"hl1_monster.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"ai_motor.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include    "ai_baseactor.h"

class CGMan : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CGMan, CAI_BaseActor);

	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void) { return 90; }
	Class_T  Classify(void);
	void HandleAnimEvent(animevent_t *pEvent);

	bool IsInC5A1();

	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);
	int	 OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	void TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType);

	int GetSoundInterests(void);

	virtual int PlayScriptedSentence(const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener);

	EHANDLE m_hPlayer;
	EHANDLE m_hTalkTarget;
	float m_flTalkTime;
};

LINK_ENTITY_TO_CLASS(monster_gman, CGMan);
PRECACHE_REGISTER(monster_gman);

bool CGMan::IsInC5A1()
{
	const char *pMapName = STRING(gpGlobals->mapname);

	if (pMapName)
	{
		return !Q_strnicmp(pMapName, "c5a1", 4);
	}

	return false;
}

Class_T	CGMan::Classify(void)
{
	return	CLASS_NONE;
}

void CGMan::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CGMan::Spawn()
{
	Precache();

	SetModel("models/gman.mdl");
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_MECH);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_USE_WEAPONS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);

	NPCInit();
}

void CGMan::Precache()
{
	PrecacheModel("models/gman.mdl");
}

void CGMan::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_WAIT:
		if (m_hPlayer == NULL)
		{
			m_hPlayer = gEntList.FindEntityByClassname(NULL, "player");
		}
		break;
	}
	BaseClass::StartTask(pTask);
}

void CGMan::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_WAIT:
		if (m_flTalkTime > gpGlobals->curtime && m_hTalkTarget != NULL)
		{
			AddLookTarget(m_hTalkTarget->GetAbsOrigin(), 1.0, 2.0);
		}
		else if (m_hPlayer != NULL && (GetSequence() == 0 || IsInC5A1()))
		{
			AddLookTarget(m_hPlayer->EyePosition(), 1.0, 3.0);
		}
		else
		{
			Vector forward;
			GetVectors(&forward, NULL, NULL);

			AddLookTarget(GetAbsOrigin() + forward * 12.0f, 1.0, 1.0);
			SetBoneController(0, 0);
		}
		BaseClass::RunTask(pTask);
		break;
	}

	SetBoneController(0, 0);
	BaseClass::RunTask(pTask);
}

int CGMan::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	m_iHealth = m_iMaxHealth / 2;

	if (inputInfo.GetDamage() > 0)
		SetCondition(COND_LIGHT_DAMAGE);

	if (inputInfo.GetDamage() >= 20)
		SetCondition(COND_HEAVY_DAMAGE);

	return TRUE;
}

void CGMan::TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType)
{
	g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
}

int CGMan::GetSoundInterests(void)
{
	return	NULL;
}

int CGMan::PlayScriptedSentence(const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, bool bConcurrent, CBaseEntity *pListener)
{
	BaseClass::PlayScriptedSentence(pszSentence, delay, volume, soundlevel, bConcurrent, pListener);

	m_flTalkTime = gpGlobals->curtime + delay;
	m_hTalkTarget = pListener;

	return 1;
}