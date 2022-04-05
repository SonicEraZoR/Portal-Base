// Patch 2.1

#include "cbase.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_route.h"
#include "ai_memory.h"
#include "hl1_monster.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "animation.h"
#include "basecombatweapon.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "shake.h"
#include "decals.h"
#include "particle_smokegrenade.h"
#include "gib.h"
#include "hl2_shareddefs.h"
#include "func_break.h"

extern short	g_sModelIndexFireball;
int				gGargGibModel;

#define	STOMP_INTERVAL 0.025

#define GARG_ATTACKDIST 120.0f

#define GARG_AE_SLASH_LEFT			1
#define GARG_AE_LEFT_FOOT			3
#define GARG_AE_RIGHT_FOOT			4
#define GARG_AE_STOMP				5
#define GARG_AE_BREATHE				6

#define GARG_DAMAGE					( DMG_ENERGYBEAM | DMG_CRUSH | DMG_MISSILEDEFENSE | DMG_BLAST )
#define GARG_EYE_SPRITE_NAME		"sprites/gargeye1.vmt"
#define GARG_BEAM_SPRITE_NAME		"sprites/xbeam3.vmt"
#define GARG_BEAM_SPRITE2			"sprites/xbeam3.vmt"
#define GARG_STOMP_SPRITE_NAME		"sprites/gargeye1.vmt"
#define GARG_FLAME_LENGTH			330
#define GARG_GIB_MODEL				"models/metalplategibs.mdl"

#define STOMP_SPRITE_COUNT			10

#define ATTACH_EYE					1

ConVar sk_gargantua_health("sk_gargantua_health", "800");
ConVar sk_gargantua_dmg_slash("sk_gargantua_dmg_slash", "10");
ConVar sk_gargantua_dmg_fire("sk_gargantua_dmg_fire", "3");
ConVar sk_gargantua_dmg_stomp("sk_gargantua_dmg_stomp", "50");

enum
{
	TASK_SOUND_ATTACK = LAST_SHARED_TASK,
	TASK_FLAME_SWEEP,
};

enum
{
	SCHED_GARG_FLAME = LAST_SHARED_SCHEDULE,
	SCHED_GARG_SWIPE,
	SCHED_GARG_CHASE_ENEMY,
	SCHED_GARG_CHASE_ENEMY_FAILED,
};

static void MoveToGround(Vector* position, CBaseEntity* ignore, const Vector& mins, const Vector& maxs)
{
	trace_t tr;

	Vector floor = *position;
	floor.z -= 1024;
	UTIL_TraceHull(*position, floor, mins, maxs, MASK_NPCSOLID, ignore, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1)
	{
		position->z = tr.endpos.z;
	}
}

class CGargantuaStomp : public CBaseEntity
{
	DECLARE_CLASS(CGargantuaStomp, CBaseEntity);

public:
	DECLARE_DATADESC();

	virtual void Precache();

	void Spawn(void);
	void Think(void);
	static CGargantuaStomp* StompCreate(Vector& origin, Vector& end, float speed, CBaseEntity* pOwner);

private:
	Vector			m_vecMoveDir;
	float			m_flScale;
	float			m_flSpeed;
	unsigned int	m_uiFramerate;
	float			m_flDmgTime;
	CBaseEntity* m_pOwner;
};

LINK_ENTITY_TO_CLASS(garg_stomp, CGargantuaStomp);

BEGIN_DATADESC(CGargantuaStomp)
DEFINE_FIELD(m_vecMoveDir, FIELD_VECTOR),
DEFINE_FIELD(m_flScale, FIELD_FLOAT),
DEFINE_FIELD(m_flSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_uiFramerate, FIELD_INTEGER),
DEFINE_FIELD(m_flDmgTime, FIELD_TIME),
DEFINE_FIELD(m_pOwner, FIELD_CLASSPTR),
END_DATADESC()

CGargantuaStomp* CGargantuaStomp::StompCreate(Vector& origin, Vector& end, float speed, CBaseEntity* pOwner)
{
	CGargantuaStomp* pStomp = (CGargantuaStomp*)CreateEntityByName("garg_stomp");

	pStomp->SetAbsOrigin(origin);
	Vector dir = (end - origin);

	pStomp->m_flScale = 2048;
	pStomp->m_vecMoveDir = dir;
	VectorNormalize(pStomp->m_vecMoveDir);
	pStomp->m_flSpeed = speed;
	pStomp->m_pOwner = pOwner;
	pStomp->Spawn();

	return pStomp;
}

void CGargantuaStomp::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("Garg.Stomp");
	PrecacheModel(GARG_STOMP_SPRITE_NAME);
}

void CGargantuaStomp::Spawn(void)
{
	Precache();

	SetNextThink(gpGlobals->curtime);
	SetClassname("garg_stomp");
	m_flDmgTime = gpGlobals->curtime;

	m_uiFramerate = 30;
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Garg.Stomp");
}

void CGargantuaStomp::Think(void)
{
	trace_t tr;

	SetNextThink(gpGlobals->curtime + 0.1);

	Vector vecStart = GetAbsOrigin();
	vecStart.z += 30;
	Vector vecEnd = vecStart + (m_vecMoveDir * m_flSpeed * gpGlobals->frametime);

	UTIL_TraceHull(vecStart, vecEnd, Vector(-32, -32, -32), Vector(32, 32, 32), MASK_SOLID, m_pOwner, COLLISION_GROUP_NONE, &tr);
	if (tr.m_pEnt)
	{
		CBaseEntity* pEntity = tr.m_pEnt;
		CTakeDamageInfo info(this, this, 50, DMG_SONIC);
		CalculateMeleeDamageForce(&info, m_vecMoveDir, tr.endpos);
		pEntity->TakeDamage(info);
	}

	m_flSpeed += (gpGlobals->frametime) * m_uiFramerate;
	m_uiFramerate += (gpGlobals->frametime) * 2000;

	if (gpGlobals->curtime - m_flDmgTime > 0.2f)
	{
		m_flDmgTime = gpGlobals->curtime - 0.2f;
	}

	while (gpGlobals->curtime - m_flDmgTime > STOMP_INTERVAL)
	{
		SetAbsOrigin(GetAbsOrigin() + m_vecMoveDir * m_flSpeed * STOMP_INTERVAL);
		for (int i = 0; i < 2; i++)
		{
			CSprite* pSprite = CSprite::SpriteCreate(GARG_STOMP_SPRITE_NAME, GetAbsOrigin(), TRUE);
			if (pSprite)
			{
				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 500), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
				pSprite->SetAbsOrigin(tr.endpos);
				pSprite->SetNextThink(gpGlobals->curtime + 0.3);
				pSprite->SetThink(&CSprite::SUB_Remove);
				pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxFadeFast);
			}
			g_pEffects->EnergySplash(tr.endpos, tr.plane.normal);
		}
		m_flDmgTime += STOMP_INTERVAL;
		m_flScale -= STOMP_INTERVAL * m_flSpeed;
		if (m_flScale <= 0)
		{
			UTIL_Remove(this);
			CPASAttenuationFilter filter(this);
			StopSound(entindex(), CHAN_STATIC, "Garg.Stomp");
		}
	}
}

class CHL1Gargantua : public CHL1BaseMonster
{
	DECLARE_CLASS(CHL1Gargantua, CHL1BaseMonster);
public:

	void Spawn(void);
	void Precache(void);
	Class_T  Classify(void);

	float MaxYawSpeed(void);

	int MeleeAttack1Conditions(float flDot, float flDist);
	int MeleeAttack2Conditions(float flDot, float flDist);
	int RangeAttack1Conditions(float flDot, float flDist);

	void HandleAnimEvent(animevent_t* pEvent);
	int  TranslateSchedule(int scheduleType);

	void StartTask(const Task_t* pTask);
	void RunTask(const Task_t* pTask);

	bool CanBecomeRagdoll() { return false; }

	void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);
	int	 OnTakeDamage_Alive(const CTakeDamageInfo& info);

	void PrescheduleThink(void);
	void Event_Killed(const CTakeDamageInfo& info);
	void DeathEffect(void);

	bool ShouldGib(const CTakeDamageInfo& info);

	void EyeOff(void);
	void EyeOn(int level);
	void EyeUpdate(void);

	void StompAttack(void);
	void FlameCreate(void);
	void FlameUpdate(void);
	void FlameControls(float angleX, float angleY);
	void FlameDestroy(void);
	inline BOOL FlameIsOn(void) { return m_pFlame[0] != NULL; }

	void FlameDamage(Vector vecStart, Vector vecEnd, CBaseEntity* pevInflictor, CBaseEntity* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType);

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:
	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	CSprite* m_pEyeGlow;
	CBeam* m_pFlame[4];

	int			m_eyeBrightness;
	float		m_seeTime;
	float		m_flameTime;
	float		m_painSoundTime;
	float		m_streakTime;
	float		m_flameX;
	float		m_flameY;

	float		m_flDmgTime;
};

LINK_ENTITY_TO_CLASS(monster_gargantua, CHL1Gargantua);

BEGIN_DATADESC(CHL1Gargantua)
DEFINE_FIELD(m_pEyeGlow, FIELD_CLASSPTR),
DEFINE_FIELD(m_eyeBrightness, FIELD_INTEGER),
DEFINE_FIELD(m_seeTime, FIELD_TIME),
DEFINE_FIELD(m_flameTime, FIELD_TIME),
DEFINE_FIELD(m_streakTime, FIELD_TIME),
DEFINE_ARRAY(m_pFlame, FIELD_CLASSPTR, 4),
DEFINE_FIELD(m_flameX, FIELD_FLOAT),
DEFINE_FIELD(m_flameY, FIELD_FLOAT),
DEFINE_FIELD(m_flDmgTime, FIELD_TIME),
DEFINE_FIELD(m_painSoundTime, FIELD_TIME),
END_DATADESC()

void CHL1Gargantua::Spawn()
{
	Precache();

	SetModel("models/garg.mdl");

	SetNavType(NAV_GROUND);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);

	Vector vecSurroundingMins(-80, -80, 0);
	Vector vecSurroundingMaxs(80, 80, 214);
	CollisionProp()->SetSurroundingBoundsType(USE_SPECIFIED_BOUNDS, &vecSurroundingMins, &vecSurroundingMaxs);

	m_bloodColor = BLOOD_COLOR_GREEN;
	m_iHealth = sk_gargantua_health.GetFloat();
	SetViewOffset(Vector(0, 0, 96));
	m_flFieldOfView = -0.2;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2);

	SetHullType(HULL_LARGE);
	SetHullSizeNormal();

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	m_pEyeGlow = CSprite::SpriteCreate(GARG_EYE_SPRITE_NAME, GetAbsOrigin(), FALSE);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(this, 1);
	EyeOff();

	m_seeTime = gpGlobals->curtime + 5;
	m_flameTime = gpGlobals->curtime + 2;

	NPCInit();

	BaseClass::Spawn();

	GetEnemies()->SetFreeKnowledgeDuration(59.0f);
}

void CHL1Gargantua::Precache()
{
	PrecacheModel("models/garg.mdl");
	PrecacheModel(GARG_EYE_SPRITE_NAME);
	PrecacheModel(GARG_BEAM_SPRITE_NAME);
	PrecacheModel(GARG_BEAM_SPRITE2);
	gGargGibModel = PrecacheModel(GARG_GIB_MODEL);

	PrecacheScriptSound("Garg.AttackHit");
	PrecacheScriptSound("Garg.AttackMiss");
	PrecacheScriptSound("Garg.Footstep");
	PrecacheScriptSound("Garg.Breath");
	PrecacheScriptSound("Garg.Attack");
	PrecacheScriptSound("Garg.Pain");
	PrecacheScriptSound("Garg.BeamAttackOn");
	PrecacheScriptSound("Garg.BeamAttackRun");
	PrecacheScriptSound("Garg.BeamAttackOff");
	PrecacheScriptSound("Garg.StompSound");
}

Class_T  CHL1Gargantua::Classify(void)
{
	return CLASS_ALIEN_MONSTER;
}

void CHL1Gargantua::PrescheduleThink(void)
{
	if (!HasCondition(COND_SEE_ENEMY))
	{
		m_seeTime = gpGlobals->curtime + 5;
		EyeOff();
	}
	else
	{
		EyeOn(200);
	}

	EyeUpdate();
}

float CHL1Gargantua::MaxYawSpeed(void)
{
	float ys = 60;

	switch (GetActivity())
	{
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_WALK:
	case ACT_RUN:
		ys = 60;
		break;

	default:
		ys = 60;
		break;
	}

	return ys;
}

int CHL1Gargantua::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDot >= 0.7)
	{
		if (flDist <= GARG_ATTACKDIST)
		{
			return COND_CAN_MELEE_ATTACK1;
		}
	}

	return COND_NONE;
}

int CHL1Gargantua::MeleeAttack2Conditions(float flDot, float flDist)
{
	if (gpGlobals->curtime > m_flameTime)
	{
		if (flDot >= 0.8)
		{
			if (flDist > GARG_ATTACKDIST)
			{
				if (flDist <= GARG_FLAME_LENGTH)
					return COND_CAN_MELEE_ATTACK2;
			}
		}
	}

	return COND_NONE;
}

int CHL1Gargantua::RangeAttack1Conditions(float flDot, float flDist)
{
	if (gpGlobals->curtime > m_seeTime)
	{
		if (flDot >= 0.7)
		{
			if (flDist > GARG_ATTACKDIST)
			{
				return COND_CAN_RANGE_ATTACK1;
			}
		}
	}

	return COND_NONE;
}

CBaseEntity* CHL1Gargantua::GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	trace_t tr;

	Vector vForward, vUp;
	AngleVectors(GetAbsAngles(), &vForward, NULL, &vUp);

	Vector vecStart = GetAbsOrigin();
	vecStart.z += 64;
	Vector vecEnd = vecStart + (vForward * flDist) - (vUp * flDist * 0.3);

	UTIL_TraceEntity(this, GetAbsOrigin(), vecEnd, MASK_SOLID, &tr);

	if (tr.m_pEnt)
	{
		CBaseEntity* pEntity = tr.m_pEnt;

		if (iDamage > 0)
		{
			CTakeDamageInfo info(this, this, iDamage, iDmgType);
			CalculateMeleeDamageForce(&info, vForward, tr.endpos);
			pEntity->TakeDamage(info);
		}

		return pEntity;
	}

	return NULL;
}

void CHL1Gargantua::HandleAnimEvent(animevent_t* pEvent)
{
	CPASAttenuationFilter filter(this);

	switch (pEvent->event)
	{
	case GARG_AE_SLASH_LEFT:
	{
		CBaseEntity* pHurt = GargantuaCheckTraceHullAttack(GARG_ATTACKDIST + 10.0, sk_gargantua_dmg_slash.GetFloat(), DMG_SLASH);

		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(-30, -30, 30));

				Vector vRight;
				AngleVectors(GetAbsAngles(), NULL, &vRight, NULL);
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - vRight * 100);
			}

			EmitSound(filter, entindex(), "Garg.AttackHit");
		}
		else
		{
			EmitSound(filter, entindex(), "Garg.AttackMiss");
		}
	}
	break;

	case GARG_AE_RIGHT_FOOT:
	case GARG_AE_LEFT_FOOT:

		UTIL_ScreenShake(GetAbsOrigin(), 4.0, 3.0, 1.0, 1500, SHAKE_START);
		EmitSound(filter, entindex(), "Garg.Footstep");
		break;

	case GARG_AE_STOMP:
		StompAttack();
		m_seeTime = gpGlobals->curtime + 12;
		break;

	case GARG_AE_BREATHE:
		EmitSound(filter, entindex(), "Garg.Breath");
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

int CHL1Gargantua::TranslateSchedule(int scheduleType)
{
	if (FlameIsOn())
		FlameDestroy();

	switch (scheduleType)
	{
	case SCHED_MELEE_ATTACK2:
		return SCHED_GARG_FLAME;
	case SCHED_MELEE_ATTACK1:
		return SCHED_GARG_SWIPE;
	
	case SCHED_CHASE_ENEMY:
		return SCHED_GARG_CHASE_ENEMY;

	case SCHED_CHASE_ENEMY_FAILED:
		return SCHED_GARG_CHASE_ENEMY_FAILED;

	case SCHED_ALERT_STAND:
		return SCHED_CHASE_ENEMY;
		
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

void CHL1Gargantua::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_FLAME_SWEEP:
		FlameCreate();
		m_flWaitFinished = gpGlobals->curtime + pTask->flTaskData;
		m_flameTime = gpGlobals->curtime + 6;
		m_flameX = 0;
		m_flameY = 0;
		break;

	case TASK_SOUND_ATTACK:

		if (random->RandomInt(0, 100) < 30)
		{
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Garg.Attack");
		}

		TaskComplete();
		break;

	case TASK_DIE:
		m_flWaitFinished = gpGlobals->curtime + 1.6;
		DeathEffect();

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

bool CHL1Gargantua::ShouldGib(const CTakeDamageInfo& info)
{
	return false;
}

void CHL1Gargantua::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_DIE:

		if (gpGlobals->curtime > m_flWaitFinished)
		{
			m_nRenderFX = kRenderFxExplode;
			SetRenderColor(255, 0, 0, 255);
			StopAnimation();
			SetNextThink(gpGlobals->curtime + 0.15);
			SetThink(&CBaseEntity::SUB_Remove);

			int i;

			int parts = modelinfo->GetModelFrameCount(modelinfo->GetModel(gGargGibModel));

			for (i = 0; i < 10; i++)
			{
				CGib* pGib = CREATE_ENTITY(CGib, "gib");

				pGib->Spawn(GARG_GIB_MODEL);

				int bodyPart = 0;

				if (parts > 1)
					bodyPart = random->RandomInt(0, parts - 1);

				pGib->SetBodygroup(0, bodyPart);
				pGib->SetBloodColor(BLOOD_COLOR_YELLOW);
				pGib->m_material = matNone;
				pGib->SetAbsOrigin(GetAbsOrigin());
				pGib->SetAbsVelocity(UTIL_RandomBloodVector() * random->RandomFloat(300, 500));

				pGib->SetNextThink(gpGlobals->curtime + 1.25);
				pGib->SetThink(&CBaseEntity::SUB_FadeOut);
			}

			Vector vecSize = Vector(200, 200, 128);
			CPVSFilter filter(GetAbsOrigin());
			te->BreakModel(filter, 0.0, GetAbsOrigin(), vec3_angle, vecSize, vec3_origin,
				gGargGibModel, 200, 50, 3.0, BREAK_FLESH);

			return;
		}
		else
			BaseClass::RunTask(pTask);
		break;

	case TASK_FLAME_SWEEP:
		if (gpGlobals->curtime > m_flWaitFinished)
		{
			FlameDestroy();
			TaskComplete();
			FlameControls(0, 0);
			SetBoneController(0, 0);
			SetBoneController(1, 0);
		}
		else
		{
			bool cancel = false;

			QAngle angles = QAngle(0, 0, 0);

			FlameUpdate();
			CBaseEntity* pEnemy = GetEnemy();

			if (pEnemy)
			{
				Vector org = GetAbsOrigin();
				org.z += 64;
				Vector dir = pEnemy->BodyTarget(org) - org;

				VectorAngles(dir, angles);
				angles.x = -angles.x;
				angles.y -= GetAbsAngles().y;

				if (dir.Length() > 400)
					cancel = true;
			}
			if (fabs(angles.y) > 60)
				cancel = true;

			if (cancel)
			{
				m_flWaitFinished -= 0.5;
				m_flameTime -= 0.5;
			}

			FlameControls(angles.x, angles.y);
		}
		break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

void CHL1Gargantua::FlameCreate(void)
{
	int			i;
	Vector		posGun;
	QAngle angleGun;

	trace_t trace;

	Vector	vForward;

	AngleVectors(GetAbsAngles(), &vForward);

	for (i = 0; i < 4; i++)
	{
		if (i < 2)
			m_pFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE_NAME, 24.0);
		else
			m_pFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE2, 14.0);
		if (m_pFlame[i])
		{
			int attach = i % 2;
			GetAttachment(attach + 1, posGun, angleGun);

			Vector vecEnd = (vForward * GARG_FLAME_LENGTH) + posGun;

			UTIL_TraceLine(posGun, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace);

			m_pFlame[i]->PointEntInit(trace.endpos, this);
			if (i < 2)
				m_pFlame[i]->SetColor(255, 130, 90);
			else
				m_pFlame[i]->SetColor(0, 120, 255);
			m_pFlame[i]->SetBrightness(190);
			m_pFlame[i]->SetBeamFlags(FBEAM_SHADEIN);
			m_pFlame[i]->SetScrollRate(20);
			m_pFlame[i]->SetEndAttachment(attach + 2);
			CSoundEnt::InsertSound(SOUND_COMBAT, posGun, 384, 0.3);
		}
	}

	CPASAttenuationFilter filter4(this);
	EmitSound(filter4, entindex(), "Garg.BeamAttackOn");
	EmitSound(filter4, entindex(), "Garg.BeamAttackRun");
}

void CHL1Gargantua::FlameControls(float angleX, float angleY)
{
	if (angleY < -180)
		angleY += 360;
	else if (angleY > 180)
		angleY -= 360;

	if (angleY < -45)
		angleY = -45;
	else if (angleY > 45)
		angleY = 45;

	m_flameX = UTIL_ApproachAngle(angleX, m_flameX, 4);
	m_flameY = UTIL_ApproachAngle(angleY, m_flameY, 8);
	SetBoneController(0, m_flameY);
	SetBoneController(1, m_flameX);
}


void CHL1Gargantua::FlameUpdate(void)
{
	int				i;
	static float	offset[2] = { 60, -60 };
	trace_t			trace;
	Vector			vecStart;
	QAngle			angleGun;
	BOOL			streaks = FALSE;

	Vector			vForward;

	for (i = 0; i < 2; i++)
	{
		if (m_pFlame[i])
		{
			QAngle vecAim = GetAbsAngles();
			vecAim.x += -m_flameX;
			vecAim.y += m_flameY;

			AngleVectors(vecAim, &vForward);

			GetAttachment(i + 2, vecStart, angleGun);
			Vector vecEnd = vecStart + (vForward * GARG_FLAME_LENGTH);

			UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace);

			m_pFlame[i]->SetStartPos(trace.endpos);
			m_pFlame[i + 2]->SetStartPos((vecStart * 0.6) + (trace.endpos * 0.4));

			if (trace.fraction != 1.0 && gpGlobals->curtime > m_streakTime)
			{
				g_pEffects->Sparks(trace.endpos, 1, 1, &trace.plane.normal);
				streaks = TRUE;
				UTIL_DecalTrace(&trace, "SmallScorch");
			}
			FlameDamage(vecStart, trace.endpos, this, this, sk_gargantua_dmg_fire.GetFloat(), CLASS_ALIEN_MONSTER, DMG_BURN);

			CBroadcastRecipientFilter filter;
			GetAttachment(i + 2, vecStart, angleGun);
			te->DynamicLight(filter, 0.0, &vecStart, 255, 0, 0, 0, 48, 0.2, 150);
		}
	}
	if (streaks)
		m_streakTime = gpGlobals->curtime;
}

void CHL1Gargantua::FlameDamage(Vector vecStart, Vector vecEnd, CBaseEntity* pevInflictor, CBaseEntity* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	CBaseEntity* pEntity = NULL;
	trace_t		tr;
	float		flAdjustedDamage;
	Vector		vecSpot;

	Vector vecMid = (vecStart + vecEnd) * 0.5;

	float searchRadius = GARG_FLAME_LENGTH;
	float maxDamageRadius = searchRadius / 2.0;

	Vector vecAim = (vecEnd - vecStart);

	VectorNormalize(vecAim);

	while ((pEntity = gEntList.FindEntityInSphere(pEntity, GetAbsOrigin(), searchRadius)) != NULL)
	{

		if (pEntity->m_takedamage != DAMAGE_NO)
		{
			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			{
				continue;
			}

			vecSpot = pEntity->BodyTarget(vecMid);

			float dist = DotProduct(vecAim, vecSpot - vecMid);
			if (dist > searchRadius)
				dist = searchRadius;
			else if (dist < -searchRadius)
				dist = searchRadius;

			Vector vecSrc = vecMid + dist * vecAim;

			UTIL_TraceLine(vecStart, vecSpot, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction == 1.0 || tr.m_pEnt == pEntity)
			{
				dist = (vecSrc - tr.endpos).Length();

				if (dist > maxDamageRadius)
				{
					flAdjustedDamage = flDamage - (dist - maxDamageRadius) * 0.4;
					if (flAdjustedDamage <= 0)
						continue;
				}
				else
				{
					flAdjustedDamage = flDamage;
				}

				if (tr.fraction != 1.0)
				{
					ClearMultiDamage();

					Vector vDir = (tr.endpos - vecSrc);
					VectorNormalize(vDir);

					CTakeDamageInfo info(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
					CalculateMeleeDamageForce(&info, vDir, tr.endpos);
					pEntity->DispatchTraceAttack(info, vDir, &tr);
					ApplyMultiDamage();
				}
				else
				{
					pEntity->TakeDamage(CTakeDamageInfo(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType));
				}
			}
		}
	}
}

void CHL1Gargantua::FlameDestroy(void)
{
	int i;

	CPASAttenuationFilter filter4(this);
	EmitSound(filter4, entindex(), "Garg.BeamAttackOff");

	for (i = 0; i < 4; i++)
	{
		if (m_pFlame[i])
		{
			UTIL_Remove(m_pFlame[i]);
			m_pFlame[i] = NULL;
		}
	}
}


void CHL1Gargantua::EyeOn(int level)
{
	m_eyeBrightness = level;
}

void CHL1Gargantua::EyeOff(void)
{
	m_eyeBrightness = 0;
}

void CHL1Gargantua::EyeUpdate(void)
{
	if (m_pEyeGlow)
	{
		m_pEyeGlow->SetBrightness(UTIL_Approach(m_eyeBrightness, m_pEyeGlow->GetBrightness(), 26), 0.5f);
		if (m_pEyeGlow->GetBrightness() == 0)
		{
			m_pEyeGlow->AddEffects(EF_NODRAW);
		}
		else
		{
			m_pEyeGlow->RemoveEffects(EF_NODRAW);
		}
	}
}


void CHL1Gargantua::StompAttack(void)
{
	trace_t trace;

	Vector vecForward;
	AngleVectors(GetAbsAngles(), &vecForward);
	Vector vecStart = GetAbsOrigin() + Vector(0, 0, 60) + 35 * vecForward;

	CBaseEntity* pPlayer = GetEnemy();
	if (!pPlayer)
		return;

	Vector vecAim = pPlayer->GetAbsOrigin() - GetAbsOrigin();
	VectorNormalize(vecAim);
	Vector vecEnd = (vecAim * 1024) + vecStart;

	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace);

	CGargantuaStomp::StompCreate(vecStart, trace.endpos, 0, this);
	UTIL_ScreenShake(GetAbsOrigin(), 12.0, 100.0, 2.0, 1000, SHAKE_START);
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Garg.StompSound");

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 20), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace);
	if (trace.fraction < 1.0)
	{
		UTIL_DecalTrace(&trace, "SmallScorch");
	}
}

void CHL1Gargantua::DeathEffect(void)
{
	int i;

	Vector vForward;
	AngleVectors(GetAbsAngles(), &vForward);
	Vector deathPos = GetAbsOrigin() + vForward * 100;

	Vector position = GetAbsOrigin();
	position.z += 32;

	CPASFilter filter(GetAbsOrigin());
	for (i = 0; i < 7; i++)
	{
		te->Explosion(filter, i * 0.2, &GetAbsOrigin(), g_sModelIndexFireball, 10, 15, TE_EXPLFLAG_NONE, 100, 0);
		position.z += 15;
	}

	UTIL_Smoke(GetAbsOrigin(), random->RandomInt(10, 15), 10);
	UTIL_ScreenShake(GetAbsOrigin(), 25.0, 100.0, 5.0, 1000, SHAKE_START);
}

void CHL1Gargantua::Event_Killed(const CTakeDamageInfo& info)
{
	EyeOff();
	UTIL_Remove(m_pEyeGlow);
	m_pEyeGlow = NULL;
	BaseClass::Event_Killed(info);
	m_takedamage = DAMAGE_NO;
}

void CHL1Gargantua::TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr)
{
	CTakeDamageInfo subInfo = info;
	if (!IsAlive())
	{
		BaseClass::TraceAttack(subInfo, vecDir, ptr);
		return;
	}

	if (subInfo.GetDamageType() & (GARG_DAMAGE))
	{
		if (m_painSoundTime < gpGlobals->curtime)
		{
			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Garg.Pain");
			m_painSoundTime = gpGlobals->curtime + random->RandomFloat(2.5, 4);
		}
	}
	int bitsDamageType = subInfo.GetDamageType();

	bitsDamageType &= GARG_DAMAGE;

	subInfo.SetDamageType(bitsDamageType);

	if (subInfo.GetDamageType() == 0)
	{
		if (m_flDmgTime != gpGlobals->curtime || (random->RandomInt(0, 100) < 20))
		{
			g_pEffects->Ricochet(ptr->endpos, ptr->plane.normal);
			m_flDmgTime = gpGlobals->curtime;
		}
		subInfo.SetDamage(0);
	}

	BaseClass::TraceAttack(subInfo, vecDir, ptr);
}

int CHL1Gargantua::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	if (GetState() == NPC_STATE_SCRIPT)
	{
		return 0;
	}

	CTakeDamageInfo subInfo = info;

	float flDamage = subInfo.GetDamage();

	if (IsAlive())
	{
		if (!(subInfo.GetDamageType() & GARG_DAMAGE))
		{
			flDamage *= 0.01;
			subInfo.SetDamage(flDamage);
		}
		if (subInfo.GetDamageType() & DMG_BLAST)
		{
			SetCondition(COND_LIGHT_DAMAGE);
		}
	}
	return BaseClass::OnTakeDamage_Alive(subInfo);
}

AI_BEGIN_CUSTOM_NPC(monster_gargantua, CHL1Gargantua)
DECLARE_TASK(TASK_SOUND_ATTACK)
DECLARE_TASK(TASK_FLAME_SWEEP)
DEFINE_SCHEDULE
(
	SCHED_GARG_FLAME,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_SOUND_ATTACK			0"
	"		TASK_SET_ACTIVITY			ACTIVITY:ACT_MELEE_ATTACK2"
	"		TASK_FLAME_SWEEP			4.5"
	"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
)

DEFINE_SCHEDULE
(
	SCHED_GARG_SWIPE,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_MELEE_ATTACK1			0"
	"	"
	"	Interrupts"
	"   COND_CAN_MELEE_ATTACK2"
)

DEFINE_SCHEDULE
(
	SCHED_GARG_CHASE_ENEMY,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_GARG_CHASE_ENEMY_FAILED"
	"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_FACE_ENEMY					0"
	""
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_ENEMY_UNREACHABLE"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK2"
	"		COND_TOO_CLOSE_TO_ATTACK"
	"		COND_LOST_ENEMY"
);

DEFINE_SCHEDULE
(
	SCHED_GARG_CHASE_ENEMY_FAILED,

	"	Tasks"
	"		TASK_SET_ROUTE_SEARCH_TIME		2"
	"		TASK_GET_PATH_TO_RANDOM_NODE	180"
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	""
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK2"
);
AI_END_CUSTOM_NPC()