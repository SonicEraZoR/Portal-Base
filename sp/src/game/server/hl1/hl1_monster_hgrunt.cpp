// Patch 2.1

#include	"cbase.h"
#include	"beam_shared.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_route.h"
#include	"ai_squad.h"
#include	"ai_squadslot.h"
#include	"ai_motor.h"
#include	"hl1_monster_hgrunt.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"effect_dispatch_data.h"
#include	"te_effect_dispatch.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include	"ai_interactions.h"
#include	"scripted.h"
#include	"hl1_grenade_mp5.h"

ConVar	sk_hgrunt_health("sk_hgrunt_health", "0");
ConVar  sk_hgrunt_kick("sk_hgrunt_kick", "0");
ConVar  sk_hgrunt_pellets("sk_hgrunt_pellets", "0");
ConVar  sk_hgrunt_gspeed("sk_hgrunt_gspeed", "0");

extern ConVar sk_plr_dmg_grenade;
extern ConVar sk_plr_dmg_mp5_grenade;

#define SF_GRUNT_LEADER	( 1 << 5  )

int g_fGruntQuestion;
int g_iSquadIndex = 0;

#define HGRUNT_GUN_SPREAD 0.08716f

#define	GRUNT_CLIP_SIZE					36
#define GRUNT_VOL						0.35
#define GRUNT_SNDLVL					SNDLVL_NORM
#define HGRUNT_LIMP_HEALTH				20
#define HGRUNT_DMG_HEADSHOT				( DMG_BULLET | DMG_CLUB )
#define HGRUNT_NUM_HEADS				2
#define HGRUNT_MINIMUM_HEADSHOT_DAMAGE	15
#define	HGRUNT_SENTENCE_VOLUME			(float)0.35

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)

#define HEAD_GROUP					1
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

#define		HGRUNT_AE_RELOAD		( 2 )
#define		HGRUNT_AE_KICK			( 3 )
#define		HGRUNT_AE_BURST1		( 4 )
#define		HGRUNT_AE_BURST2		( 5 ) 
#define		HGRUNT_AE_BURST3		( 6 ) 
#define		HGRUNT_AE_GREN_TOSS		( 7 )
#define		HGRUNT_AE_GREN_LAUNCH	( 8 )
#define		HGRUNT_AE_GREN_DROP		( 9 )
#define		HGRUNT_AE_CAUGHT_ENEMY	( 10)
#define		HGRUNT_AE_DROP_GUN		( 11)

const char* CHGrunt::pGruntSentences[] =
{
	"HG_GREN",
	"HG_ALERT",
	"HG_MONSTER",
	"HG_COVER",
	"HG_THROW",
	"HG_CHARGE",
	"HG_TAUNT",
};

enum
{
	HGRUNT_SENT_NONE = -1,
	HGRUNT_SENT_GREN = 0,
	HGRUNT_SENT_ALERT,
	HGRUNT_SENT_MONSTER,
	HGRUNT_SENT_COVER,
	HGRUNT_SENT_THROW,
	HGRUNT_SENT_CHARGE,
	HGRUNT_SENT_TAUNT,
} HGRUNT_SENTENCE_TYPES;

enum
{
	SCHED_GRUNT_FAIL = LAST_SHARED_SCHEDULE,
	SCHED_GRUNT_COMBAT_FAIL,
	SCHED_GRUNT_VICTORY_DANCE,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY,
	SCHED_GRUNT_FOUND_ENEMY,
	SCHED_GRUNT_COMBAT_FACE,
	SCHED_GRUNT_SIGNAL_SUPPRESS,
	SCHED_GRUNT_SUPPRESS,
	SCHED_GRUNT_WAIT_IN_COVER,
	SCHED_GRUNT_TAKE_COVER,
	SCHED_GRUNT_GRENADE_COVER,
	SCHED_GRUNT_TOSS_GRENADE_COVER,
	SCHED_GRUNT_HIDE_RELOAD,
	SCHED_GRUNT_SWEEP,
	SCHED_GRUNT_RANGE_ATTACK1A,
	SCHED_GRUNT_RANGE_ATTACK1B,
	SCHED_GRUNT_RANGE_ATTACK2,
	SCHED_GRUNT_REPEL,
	SCHED_GRUNT_REPEL_ATTACK,
	SCHED_GRUNT_REPEL_LAND,
	SCHED_GRUNT_TAKE_COVER_FAILED,
	SCHED_GRUNT_RELOAD,
	SCHED_GRUNT_TAKE_COVER_FROM_ENEMY,
	SCHED_GRUNT_BARNACLE_HIT,
	SCHED_GRUNT_BARNACLE_PULL,
	SCHED_GRUNT_BARNACLE_CHOMP,
	SCHED_GRUNT_BARNACLE_CHEW,
};

enum
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_SHARED_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
};

enum
{
	COND_GRUNT_NOFIRE = LAST_SHARED_CONDITION + 1,
};

enum SquadSlot_T
{
	SQUAD_SLOT_GRENADE1 = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOT_GRENADE2,
	SQUAD_SLOT_ENGAGE1,
	SQUAD_SLOT_ENGAGE2,
};


int ACT_GRUNT_LAUNCH_GRENADE;
int ACT_GRUNT_TOSS_GRENADE;
int ACT_GRUNT_MP5_STANDING;
int ACT_GRUNT_MP5_CROUCHING;
int ACT_GRUNT_SHOTGUN_STANDING;
int ACT_GRUNT_SHOTGUN_CROUCHING;

LINK_ENTITY_TO_CLASS(monster_human_grunt, CHGrunt);
PRECACHE_REGISTER(monster_human_grunt);

BEGIN_DATADESC(CHGrunt)
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),
DEFINE_FIELD(m_flNextPainTime, FIELD_TIME),
DEFINE_FIELD(m_flCheckAttackTime, FIELD_FLOAT),
DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_iLastGrenadeCondition, FIELD_INTEGER),
DEFINE_FIELD(m_fStanding, FIELD_BOOLEAN),
DEFINE_FIELD(m_fFirstEncounter, FIELD_BOOLEAN),
DEFINE_FIELD(m_iClipSize, FIELD_INTEGER),
DEFINE_FIELD(m_voicePitch, FIELD_INTEGER),
DEFINE_FIELD(m_iSentence, FIELD_INTEGER),
DEFINE_KEYFIELD(m_iWeapons, FIELD_INTEGER, "weapons"),
DEFINE_KEYFIELD(m_SquadName, FIELD_STRING, "netname"),
DEFINE_FIELD(m_bInBarnacleMouth, FIELD_BOOLEAN),
DEFINE_FIELD(m_flLastEnemySightTime, FIELD_TIME),
DEFINE_FIELD(m_flTalkWaitTime, FIELD_TIME),
END_DATADESC()

void CHGrunt::Spawn()
{
	Precache();

	SetModel("models/hgrunt.mdl");

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	ClearEffects();
	m_iHealth = sk_hgrunt_health.GetFloat();
	m_flFieldOfView = 0.2;
	m_NPCState = NPC_STATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->curtime + 1;
	m_flNextPainTime = gpGlobals->curtime;
	m_iSentence = HGRUNT_SENT_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_MOVE_GROUND);

	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK2);
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	// Patch 2.1 Start
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	// Patch 2.1 End

	m_fFirstEncounter = true;

	m_HackedGunPos = Vector(0, 0, 55);

	if (m_iWeapons == 0)
	{
		m_iWeapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;
	}

	if (FBitSet(m_iWeapons, HGRUNT_SHOTGUN))
	{
		SetBodygroup(GUN_GROUP, GUN_SHOTGUN);
		m_iClipSize = 8;
	}
	else
	{
		m_iClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_iClipSize;

	if (random->RandomInt(0, 99) < 80)
		m_nSkin = 0;
	else
		m_nSkin = 1;

	if (FBitSet(m_iWeapons, HGRUNT_SHOTGUN))
	{
		SetBodygroup(HEAD_GROUP, HEAD_SHOTGUN);
	}
	else if (FBitSet(m_iWeapons, HGRUNT_GRENADELAUNCHER))
	{
		SetBodygroup(HEAD_GROUP, HEAD_M203);
		m_nSkin = 1;
	}

	m_flTalkWaitTime = 0;

	g_iSquadIndex = 0;

	BaseClass::Spawn();

	NPCInit();

	CreateVPhysics();
}

//Creating VPhysics with the below settings is what enables HGrunts to collide with the Tentacle Monster.
bool CHGrunt::CreateVPhysics(void)
{
	VPhysicsDestroyObject();

	CPhysCollide* pModel = PhysCreateBbox(NAI_Hull::Mins(HULL_HUMAN), NAI_Hull::Maxs(HULL_HUMAN));
	IPhysicsObject* pPhysics = PhysModelCreateCustom(this, pModel, GetAbsOrigin(), GetAbsAngles(), "barney_hull", false);

	VPhysicsSetObject(pPhysics);
	if (pPhysics)
	{
		pPhysics->SetCallbackFlags(CALLBACK_GLOBAL_COLLISION | CALLBACK_SHADOW_COLLISION);
	}
	PhysAddShadow(this);

	return true;
}

//Sometimes the VPhysics do not create properly and the best way I could find to fix this was to rebuild the VPhysics on every think.
void CHGrunt::NPCThink(void)
{
	BaseClass::NPCThink();

	CreateVPhysics();
}

int CHGrunt::IRelationPriority(CBaseEntity* pTarget)
{
	if (pTarget->Classify() == CLASS_ALIEN_MILITARY)
	{
		if (FClassnameIs(pTarget, "monster_alien_grunt"))
		{
			return (BaseClass::IRelationPriority(pTarget) + 1);
		}
	}

	return BaseClass::IRelationPriority(pTarget);
}

void CHGrunt::Precache()
{
	m_iAmmoType = GetAmmoDef()->Index("9mmRound");

	PrecacheModel("models/hgrunt.mdl");

	if (random->RandomInt(0, 1))
		m_voicePitch = 109 + random->RandomInt(0, 7);
	else
		m_voicePitch = 100;

	PrecacheScriptSound("HGrunt.Reload");
	PrecacheScriptSound("HGrunt.GrenadeLaunch");
	PrecacheScriptSound("HGrunt.9MM");
	PrecacheScriptSound("HGrunt.Shotgun");
	PrecacheScriptSound("HGrunt.Pain");
	PrecacheScriptSound("HGrunt.Die");

	BaseClass::Precache();

	UTIL_PrecacheOther("grenade_hand");
	UTIL_PrecacheOther("grenade_mp5");
}

bool CHGrunt::FOkToSpeak(void)
{
	if (gpGlobals->curtime <= m_flTalkWaitTime)
		return FALSE;

	if (m_spawnflags & SF_NPC_GAG)
	{
		if (m_NPCState != NPC_STATE_COMBAT)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CHGrunt::SpeakSentence(void)
{
	if (m_iSentence == HGRUNT_SENT_NONE)
	{
		return;
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz(edict(), pGruntSentences[m_iSentence], HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
		JustSpoke();
	}
}

void CHGrunt::JustSpoke(void)
{
	m_flTalkWaitTime = gpGlobals->curtime + random->RandomFloat(1.5f, 2.0f);
	m_iSentence = HGRUNT_SENT_NONE;
}

void CHGrunt::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (InSquad() && GetEnemy() != NULL)
	{
		if (HasCondition(COND_SEE_ENEMY))
		{
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->curtime;
		}
		else
		{
			if (gpGlobals->curtime - MySquadLeader()->m_flLastEnemySightTime > 5)
			{
				MySquadLeader()->m_fEnemyEluded = true;
			}
		}
	}
}

Class_T	CHGrunt::Classify(void)
{
	return CLASS_HUMAN_MILITARY;
}

int CHGrunt::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > 64)
		return COND_TOO_FAR_TO_ATTACK;
	else if (flDot < 0.7)
		return COND_NOT_FACING_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}

int CHGrunt::RangeAttack1Conditions(float flDot, float flDist)
{
	if (!HasCondition(COND_ENEMY_OCCLUDED) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire())
	{
		trace_t	tr;

		if (!GetEnemy()->IsPlayer() && flDist <= 64)
		{
			return COND_NONE;
		}

		Vector vecSrc;
		QAngle angAngles;

		GetAttachment("0", vecSrc, angAngles);

		UTIL_TraceLine(GetAbsOrigin() + GetViewOffset(), GetEnemy()->BodyTarget(GetAbsOrigin() + GetViewOffset()), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction == 1.0 || tr.m_pEnt == GetEnemy())
		{
			return COND_CAN_RANGE_ATTACK1;
		}
	}


	if (!NoFriendlyFire())
		return COND_WEAPON_BLOCKED_BY_FRIEND;

	return COND_NONE;
}

int CHGrunt::RangeAttack2Conditions(float flDot, float flDist)
{
	m_iLastGrenadeCondition = GetGrenadeConditions(flDot, flDist);
	return m_iLastGrenadeCondition;
}

int CHGrunt::GetGrenadeConditions(float flDot, float flDist)
{
	if (!FBitSet(m_iWeapons, (HGRUNT_HANDGRENADE | HGRUNT_GRENADELAUNCHER)))
		return COND_NONE;

	if (gpGlobals->curtime < m_flNextGrenadeCheck)
		return m_iLastGrenadeCondition;

	if (m_flGroundSpeed != 0)
		return COND_NONE;

	CBaseEntity* pEnemy = GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector flEnemyLKP = GetEnemyLKP();
	if (!(pEnemy->GetFlags() & FL_ONGROUND) && pEnemy->GetWaterLevel() == 0 && flEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z))
	{
		return COND_NONE;
	}

	Vector vecTarget;

	if (FBitSet(m_iWeapons, HGRUNT_HANDGRENADE))
	{
		if (random->RandomInt(0, 1))
		{
			pEnemy->CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &vecTarget);
		}
		else
		{
			vecTarget = flEnemyLKP;
		}
	}
	else
	{
		vecTarget = GetEnemy()->GetAbsOrigin() + (GetEnemy()->BodyTarget(GetAbsOrigin()) - GetEnemy()->GetAbsOrigin());
		if (HasCondition(COND_SEE_ENEMY))
		{
			vecTarget = vecTarget + ((vecTarget - GetAbsOrigin()).Length() / sk_hgrunt_gspeed.GetFloat()) * GetEnemy()->GetAbsVelocity();
		}
	}

	if (InSquad())
	{
		if (SquadMemberInRange(vecTarget, 256))
		{
			m_flNextGrenadeCheck = gpGlobals->curtime + 1;
			return COND_NONE;
		}
	}

	if ((vecTarget - GetAbsOrigin()).Length2D() <= 256)
	{
		m_flNextGrenadeCheck = gpGlobals->curtime + 1;
		return COND_NONE;
	}


	if (FBitSet(m_iWeapons, HGRUNT_HANDGRENADE))
	{
		Vector vGunPos;
		QAngle angGunAngles;
		GetAttachment("0", vGunPos, angGunAngles);


		Vector vecToss = VecCheckToss(this, vGunPos, vecTarget, -1, 0.5, false);

		if (vecToss != vec3_origin)
		{
			m_vecTossVelocity = vecToss;

			m_flNextGrenadeCheck = gpGlobals->curtime + 0.3;

			return COND_CAN_RANGE_ATTACK2;
		}
		else
		{
			m_flNextGrenadeCheck = gpGlobals->curtime + 1;

			return COND_NONE;
		}
	}
	else
	{
		Vector vGunPos;
		QAngle angGunAngles;
		GetAttachment("0", vGunPos, angGunAngles);

		Vector vecToss = VecCheckThrow(this, vGunPos, vecTarget, sk_hgrunt_gspeed.GetFloat(), 0.5);

		if (vecToss != vec3_origin)
		{
			m_vecTossVelocity = vecToss;

			m_flNextGrenadeCheck = gpGlobals->curtime + 0.3;

			return COND_CAN_RANGE_ATTACK2;
		}
		else
		{
			m_flNextGrenadeCheck = gpGlobals->curtime + 1;

			return COND_NONE;
		}
	}
}

bool CHGrunt::FCanCheckAttacks(void)
{
	if (!HasCondition(COND_TOO_CLOSE_TO_ATTACK))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CHGrunt::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_PLAYER |
		SOUND_BULLET_IMPACT |
		SOUND_DANGER;
}

void CHGrunt::TraceAttack(const CTakeDamageInfo& inputInfo, const Vector& vecDir, trace_t* ptr)
{
	CTakeDamageInfo info = inputInfo;

	if (ptr->hitgroup == 11)
	{
		if (GetBodygroup(1) == HEAD_GRUNT && (info.GetDamageType() & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		{
			info.SetDamage(info.GetDamage() - 20);
			if (info.GetDamage() <= 0)
				info.SetDamage(0.01);
		}
		ptr->hitgroup = HITGROUP_HEAD;
	}
	BaseClass::TraceAttack(info, vecDir, ptr);
}

int CHGrunt::OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo)
{
	Forget(bits_MEMORY_INCOVER);

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

float CHGrunt::MaxYawSpeed(void)
{
	float flYS;

	switch (GetActivity())
	{
	case ACT_IDLE:
		flYS = 150;
		break;
	case ACT_RUN:
		flYS = 150;
		break;
	case ACT_WALK:
		flYS = 180;
		break;
	case ACT_RANGE_ATTACK1:
		flYS = 120;
		break;
	case ACT_RANGE_ATTACK2:
		flYS = 120;
		break;
	case ACT_MELEE_ATTACK1:
		flYS = 120;
		break;
	case ACT_MELEE_ATTACK2:
		flYS = 120;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		flYS = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		flYS = 30;
		break;
	default:
		flYS = 90;
		break;
	}

	return flYS * 0.5f;
}

void CHGrunt::IdleSound(void)
{
	if (FOkToSpeak() && (g_fGruntQuestion || random->RandomInt(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			switch (random->RandomInt(0, 2))
			{
			case 0:
				SENTENCEG_PlayRndSz(edict(), "HG_CHECK", HGRUNT_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1:
				SENTENCEG_PlayRndSz(edict(), "HG_QUEST", HGRUNT_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			case 2:
				SENTENCEG_PlayRndSz(edict(), "HG_IDLE", HGRUNT_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1:
				SENTENCEG_PlayRndSz(edict(), "HG_CLEAR", HGRUNT_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
				break;
			case 2:
				SENTENCEG_PlayRndSz(edict(), "HG_ANSWER", HGRUNT_SENTENCE_VOLUME, SNDLVL_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

bool CHGrunt::HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
	{
		ClearSchedule("Soldier being eaten by a barnacle");
		m_bInBarnacleMouth = true;
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimReleased)
	{
		SetState(NPC_STATE_IDLE);
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
			m_hCine->CancelScript();
			ClearSchedule("Soldier grabbed by a barnacle");
		}

		SetState(NPC_STATE_PRONE);

		CTakeDamageInfo info;
		PainSound(info);
		return true;
	}
	return false;
}

void CHGrunt::CheckAmmo(void)
{
	if (m_cAmmoLoaded <= 0)
		SetCondition(COND_NO_PRIMARY_AMMO);

}

CBaseEntity* CHGrunt::Kick(void)
{
	trace_t tr;

	Vector forward;
	AngleVectors(GetAbsAngles(), &forward);
	Vector vecStart = GetAbsOrigin();
	vecStart.z += WorldAlignSize().z * 0.5;
	Vector vecEnd = vecStart + (forward * 70);

	UTIL_TraceHull(vecStart, vecEnd, Vector(-16, -16, -18), Vector(16, 16, 18), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	if (tr.m_pEnt)
	{
		CBaseEntity* pEntity = tr.m_pEnt;
		return pEntity;
	}

	return NULL;
}

Vector CHGrunt::Weapon_ShootPosition(void)
{
	if (m_fStanding)
		return GetAbsOrigin() + Vector(0, 0, 60);
	else
		return GetAbsOrigin() + Vector(0, 0, 48);
}

void CHGrunt::Event_Killed(const CTakeDamageInfo& info)
{
	Vector	vecGunPos;
	QAngle	vecGunAngles;

	GetAttachment("0", vecGunPos, vecGunAngles);

	SetBodygroup(GUN_GROUP, GUN_NONE);

	if (UTIL_PointContents(vecGunPos) & CONTENTS_SOLID)
	{
		vecGunPos = GetAbsOrigin();
	}

	if (FBitSet(m_iWeapons, HGRUNT_SHOTGUN))
	{
		DropItem("weapon_shotgun", vecGunPos, vecGunAngles);
	}
	else
	{
		DropItem("weapon_mp5", vecGunPos, vecGunAngles);
	}

	if (FBitSet(m_iWeapons, HGRUNT_GRENADELAUNCHER))
	{
		DropItem("ammo_ARgrenades", BodyTarget(GetAbsOrigin()), vecGunAngles);
	}

	BaseClass::Event_Killed(info);
}

void CHGrunt::HandleAnimEvent(animevent_t* pEvent)
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch (pEvent->event)
	{
	case HGRUNT_AE_RELOAD:
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "HGrunt.Reload");

		m_cAmmoLoaded = m_iClipSize;
		ClearCondition(COND_NO_PRIMARY_AMMO);
	}
	break;

	case HGRUNT_AE_GREN_TOSS:
	{
		CHandGrenade* pGrenade = (CHandGrenade*)Create("grenade_hand", GetAbsOrigin() + Vector(0, 0, 60), vec3_angle);
		if (pGrenade)
		{
			pGrenade->ShootTimed(this, m_vecTossVelocity, 3.5);
		}

		m_iLastGrenadeCondition = COND_NONE;
		m_flNextGrenadeCheck = gpGlobals->curtime + 6;
	}
	break;

	case HGRUNT_AE_GREN_LAUNCH:
	{
		CPASAttenuationFilter filter2(this);
		EmitSound(filter2, entindex(), "HGrunt.GrenadeLaunch");

		Vector vecSrc;
		QAngle angAngles;

		GetAttachment("0", vecSrc, angAngles);

		CGrenadeMP5* m_pMyGrenade = (CGrenadeMP5*)Create("grenade_mp5", vecSrc, angAngles, this);
		m_pMyGrenade->SetAbsVelocity(m_vecTossVelocity);
		m_pMyGrenade->SetLocalAngularVelocity(QAngle(random->RandomFloat(-100, -500), 0, 0));
		m_pMyGrenade->SetMoveType(MOVETYPE_FLYGRAVITY);
		m_pMyGrenade->SetThrower(this);
		m_pMyGrenade->SetDamage(sk_plr_dmg_mp5_grenade.GetFloat());

		if (g_iSkillLevel == SKILL_HARD)
			m_flNextGrenadeCheck = gpGlobals->curtime + random->RandomFloat(2, 5);
		else
			m_flNextGrenadeCheck = gpGlobals->curtime + 6;

		m_iLastGrenadeCondition = COND_NONE;
	}
	break;

	case HGRUNT_AE_GREN_DROP:
	{
		CHandGrenade* pGrenade = (CHandGrenade*)Create("grenade_hand", Weapon_ShootPosition(), vec3_angle);
		if (pGrenade)
		{
			pGrenade->ShootTimed(this, m_vecTossVelocity, 3.5);
		}

		m_iLastGrenadeCondition = COND_NONE;
	}
	break;

	case HGRUNT_AE_BURST1:
	{
		if (FBitSet(m_iWeapons, HGRUNT_9MMAR))
		{
			Shoot();

			CPASAttenuationFilter filter3(this);
			EmitSound(filter3, entindex(), "HGrunt.9MM");
		}
		else
		{
			Shotgun();

			CPASAttenuationFilter filter4(this);
			EmitSound(filter4, entindex(), "HGrunt.Shotgun");
		}

		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 384, 0.3);
	}
	break;

	case HGRUNT_AE_BURST2:
	case HGRUNT_AE_BURST3:
		Shoot();
		break;

	case HGRUNT_AE_KICK:
	{
		CBaseEntity* pHurt = Kick();

		if (pHurt)
		{
			Vector forward, up;
			AngleVectors(GetAbsAngles(), &forward, NULL, &up);

			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
				pHurt->ViewPunch(QAngle(15, 0, 0));

			if (pHurt->entindex() > 0)
			{
				pHurt->ApplyAbsVelocityImpulse(forward * 100 + up * 50);

				CTakeDamageInfo info(this, this, sk_hgrunt_kick.GetFloat(), DMG_CLUB);
				CalculateMeleeDamageForce(&info, forward, pHurt->GetAbsOrigin());
				pHurt->TakeDamage(info);
			}
		}
	}
	break;

	case HGRUNT_AE_CAUGHT_ENEMY:
	{
		if (FOkToSpeak())
		{
			SENTENCEG_PlayRndSz(edict(), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
			JustSpoke();
		}

	}

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CHGrunt::SetAim(const Vector& aimDir)
{
	QAngle angDir;
	VectorAngles(aimDir, angDir);

	float curPitch = GetPoseParameter("XR");
	float newPitch = curPitch + UTIL_AngleDiff(UTIL_ApproachAngle(angDir.x, curPitch, 60), curPitch);

	SetPoseParameter("XR", -newPitch);
}

void CHGrunt::Shoot(void)
{
	if (GetEnemy() == NULL)
		return;

	Vector vecShootOrigin = Weapon_ShootPosition();
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);


	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);


	Vector	vecShellVelocity = right * random->RandomFloat(40, 90) + up * random->RandomFloat(75, 200) + forward * random->RandomFloat(-40, 40);
	EjectShell(vecShootOrigin - vecShootDir * 24, vecShellVelocity, GetAbsAngles().y, 0);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, m_iAmmoType);

	DoMuzzleFlash();

	m_cAmmoLoaded--;

	SetAim(vecShootDir);
}

void CHGrunt::Shotgun(void)
{
	if (GetEnemy() == NULL)
		return;

	Vector vecShootOrigin = Weapon_ShootPosition();
	Vector vecShootDir = GetShootEnemyDir(vecShootOrigin);

	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);

	Vector	vecShellVelocity = right * random->RandomFloat(40, 90) + up * random->RandomFloat(75, 200) + forward * random->RandomFloat(-40, 40);
	EjectShell(vecShootOrigin - vecShootDir * 24, vecShellVelocity, GetAbsAngles().y, 1);
	FireBullets(sk_hgrunt_pellets.GetFloat(), vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, m_iAmmoType, 0);

	DoMuzzleFlash();

	m_cAmmoLoaded--;

	SetAim(vecShootDir);
}

void CHGrunt::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_CHECK_FIRE:
		if (!NoFriendlyFire())
		{
			SetCondition(COND_WEAPON_BLOCKED_BY_FRIEND);
		}
		TaskComplete();
		break;

	case TASK_GRUNT_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		Forget(bits_MEMORY_INCOVER);
		BaseClass::StartTask(pTask);
		break;

	case TASK_RELOAD:
		SetIdealActivity(ACT_RELOAD);
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		BaseClass::StartTask(pTask);
		if (GetMoveType() == MOVETYPE_FLYGRAVITY)
		{
			SetIdealActivity(ACT_GLIDE);
		}
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CHGrunt::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
	{
		GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + m_vecTossVelocity * 64, AI_KEEP_YAW_SPEED);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

void CHGrunt::PainSound(const CTakeDamageInfo& info)
{
	if (gpGlobals->curtime > m_flNextPainTime)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "HGrunt.Pain");

		m_flNextPainTime = gpGlobals->curtime + 1;
	}
}

void CHGrunt::DeathSound(const CTakeDamageInfo& info)
{
	CPASAttenuationFilter filter(this, ATTN_IDLE);
	EmitSound(filter, entindex(), "HGrunt.Die");
}

Activity CHGrunt::NPC_TranslateActivity(Activity eNewActivity)
{
	switch (eNewActivity)
	{
	case ACT_RANGE_ATTACK1:
		if (FBitSet(m_iWeapons, HGRUNT_9MMAR))
		{
			if (m_fStanding)
			{
				return (Activity)ACT_GRUNT_MP5_STANDING;
			}
			else
			{
				return (Activity)ACT_GRUNT_MP5_CROUCHING;
			}
		}
		else
		{
			if (m_fStanding)
			{
				return (Activity)ACT_GRUNT_SHOTGUN_STANDING;
			}
			else
			{
				return (Activity)ACT_GRUNT_SHOTGUN_CROUCHING;
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		if (m_iWeapons & HGRUNT_HANDGRENADE)
		{
			return (Activity)ACT_GRUNT_TOSS_GRENADE;
		}
		else
		{
			return (Activity)ACT_GRUNT_LAUNCH_GRENADE;
		}
		break;
	case ACT_RUN:
		if (m_iHealth <= HGRUNT_LIMP_HEALTH)
		{
			return ACT_RUN_HURT;
		}
		else
		{
			return eNewActivity;
		}
		break;
	case ACT_WALK:
		if (m_iHealth <= HGRUNT_LIMP_HEALTH)
		{
			return ACT_WALK_HURT;
		}
		else
		{
			return eNewActivity;
		}
		break;
	case ACT_IDLE:
		if (m_NPCState == NPC_STATE_COMBAT)
		{
			eNewActivity = ACT_IDLE_ANGRY;
		}

		break;
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}

void CHGrunt::ClearAttackConditions(void)
{
	bool fCanRangeAttack2 = HasCondition(COND_CAN_RANGE_ATTACK2);

	BaseClass::ClearAttackConditions();

	if (fCanRangeAttack2)
	{
		SetCondition(COND_CAN_RANGE_ATTACK2);
	}
}

int CHGrunt::SelectSchedule(void)
{
	m_iSentence = HGRUNT_SENT_NONE;

	if (GetMoveType() == MOVETYPE_FLYGRAVITY && m_NPCState != NPC_STATE_PRONE)
	{
		if (GetFlags() & FL_ONGROUND)
		{
			SetMoveType(MOVETYPE_STEP);
			SetGravity(1.0);
			return SCHED_GRUNT_REPEL_LAND;
		}
		else
		{
			if (m_NPCState == NPC_STATE_COMBAT)
				return SCHED_GRUNT_REPEL_ATTACK;
			else
				return SCHED_GRUNT_REPEL;
		}
	}

	if (HasCondition(COND_HEAR_DANGER))
	{

		if (FOkToSpeak())
		{
			SENTENCEG_PlayRndSz(edict(), "HG_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
			JustSpoke();
		}

		return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	switch (m_NPCState)
	{

	case NPC_STATE_PRONE:
	{
		if (m_bInBarnacleMouth)
		{
			return SCHED_GRUNT_BARNACLE_CHOMP;
		}
		else
		{
			return SCHED_GRUNT_BARNACLE_HIT;
		}
	}

	case NPC_STATE_COMBAT:
	{
		if (HasCondition(COND_ENEMY_DEAD))
		{
			return BaseClass::SelectSchedule();
		}

		if (HasCondition(COND_NEW_ENEMY))
		{
			if (InSquad())
			{
				if (!IsLeader())
				{
					return SCHED_TAKE_COVER_FROM_ENEMY;
				}
				else
				{
					if (FOkToSpeak())
					{
						if ((GetEnemy() != NULL) && GetEnemy()->IsPlayer())
							SENTENCEG_PlayRndSz(edict(), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
						else if ((GetEnemy() != NULL) &&
							(GetEnemy()->Classify() != CLASS_PLAYER_ALLY) &&
							(GetEnemy()->Classify() != CLASS_HUMAN_PASSIVE) &&
							(GetEnemy()->Classify() != CLASS_MACHINE))
							SENTENCEG_PlayRndSz(edict(), "HG_MONST", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);

						JustSpoke();
					}

					if (HasCondition(COND_CAN_RANGE_ATTACK1))
					{
						return SCHED_GRUNT_SUPPRESS;
					}
					else
					{
						return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
					}
				}
			}
		}
		else if (HasCondition(COND_NO_PRIMARY_AMMO))
		{
			return SCHED_GRUNT_HIDE_RELOAD;
		}

		else if (HasCondition(COND_LIGHT_DAMAGE))
		{
			int iPercent = random->RandomInt(0, 99);

			if (iPercent <= 90 && GetEnemy() != NULL)
			{
				if (FOkToSpeak())
				{
					m_iSentence = HGRUNT_SENT_COVER;
				}
				return SCHED_TAKE_COVER_FROM_ENEMY;
			}
			else
			{
				return SCHED_SMALL_FLINCH;
			}
		}
		else if (HasCondition(COND_CAN_MELEE_ATTACK1))
		{
			return SCHED_MELEE_ATTACK1;
		}

		else if (FBitSet(m_iWeapons, HGRUNT_GRENADELAUNCHER) && HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
		{
			return SCHED_RANGE_ATTACK2;
		}
		else if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			if (InSquad())
			{
				if (MySquadLeader()->m_fEnemyEluded && !HasCondition(COND_ENEMY_FACING_ME))
				{
					MySquadLeader()->m_fEnemyEluded = false;
					return SCHED_GRUNT_FOUND_ENEMY;
				}
			}

			if (OccupyStrategySlotRange(SQUAD_SLOT_ENGAGE1, SQUAD_SLOT_ENGAGE2))
			{
				return SCHED_RANGE_ATTACK1;
			}
			else if (HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				return SCHED_RANGE_ATTACK2;
			}
			else
			{
				return SCHED_TAKE_COVER_FROM_ENEMY;
			}
		}
		else if (HasCondition(COND_ENEMY_OCCLUDED))
		{
			if (HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(edict(), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					JustSpoke();
				}
				return SCHED_RANGE_ATTACK2;
			}
			else if (OccupyStrategySlotRange(SQUAD_SLOT_ENGAGE1, SQUAD_SLOT_ENGAGE2))
			{
				if (FOkToSpeak())
				{
					m_iSentence = HGRUNT_SENT_CHARGE;
				}

				return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
			}
			else
			{
				if (FOkToSpeak() && random->RandomInt(0, 1))
				{
					SENTENCEG_PlayRndSz(edict(), "HG_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					JustSpoke();
				}
				return SCHED_STANDOFF;
			}
		}

		if (HasCondition(COND_SEE_ENEMY) && !HasCondition(COND_CAN_RANGE_ATTACK1))
		{
			return SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE;
		}
	}
	case NPC_STATE_ALERT:
		if (HasCondition(COND_ENEMY_DEAD) && SelectWeightedSequence(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		{
			return SCHED_VICTORY_DANCE;
		}
		break;
	}

	return BaseClass::SelectSchedule();
}

int CHGrunt::TranslateSchedule(int scheduleType)
{

	if (scheduleType == SCHED_CHASE_ENEMY_FAILED)
	{
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}
	switch (scheduleType)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		if (InSquad())
		{
			if (g_iSkillLevel == SKILL_HARD && HasCondition(COND_CAN_RANGE_ATTACK2) && OccupyStrategySlotRange(SQUAD_SLOT_GRENADE1, SQUAD_SLOT_GRENADE2))
			{
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(edict(), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_SNDLVL, 0, m_voicePitch);
					JustSpoke();
				}
				return SCHED_GRUNT_TOSS_GRENADE_COVER;
			}
			else
			{
				return SCHED_GRUNT_TAKE_COVER;
			}
		}
		else
		{
			if (random->RandomInt(0, 1))
			{
				return SCHED_GRUNT_TAKE_COVER;
			}
			else
			{
				return SCHED_GRUNT_GRENADE_COVER;
			}
		}
	}
	case SCHED_GRUNT_TAKE_COVER_FAILED:
	{
		if (HasCondition(COND_CAN_RANGE_ATTACK1) && OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
		{
			return SCHED_RANGE_ATTACK1;
		}

		return SCHED_FAIL;
	}
	break;

	case SCHED_RANGE_ATTACK1:
	{
		if (random->RandomInt(0, 9) == 0)
		{
			m_fStanding = random->RandomInt(0, 1) != 0;
		}

		if (m_fStanding)
			return SCHED_GRUNT_RANGE_ATTACK1B;
		else
			return SCHED_GRUNT_RANGE_ATTACK1A;
	}

	case SCHED_RANGE_ATTACK2:
	{
		return SCHED_GRUNT_RANGE_ATTACK2;
	}
	case SCHED_VICTORY_DANCE:
	{
		if (InSquad())
		{
			if (!IsLeader())
			{
				return SCHED_GRUNT_FAIL;
			}
		}

		return SCHED_GRUNT_VICTORY_DANCE;
	}
	case SCHED_GRUNT_SUPPRESS:
	{
		if (GetEnemy()->IsPlayer() && m_fFirstEncounter)
		{
			m_fFirstEncounter = FALSE;
			return SCHED_GRUNT_SIGNAL_SUPPRESS;
		}
		else
		{
			return SCHED_GRUNT_SUPPRESS;
		}
	}
	case SCHED_FAIL:
	{
		if (GetEnemy() != NULL)
		{
			return SCHED_GRUNT_COMBAT_FAIL;
		}

		return SCHED_GRUNT_FAIL;
	}
	case SCHED_GRUNT_REPEL:
	{
		Vector vecVel = GetAbsVelocity();
		if (vecVel.z > -128)
		{
			vecVel.z -= 32;
			SetAbsVelocity(vecVel);
		}

		return SCHED_GRUNT_REPEL;
	}
	case SCHED_GRUNT_REPEL_ATTACK:
	{
		Vector vecVel = GetAbsVelocity();
		if (vecVel.z > -128)
		{
			vecVel.z -= 32;
			SetAbsVelocity(vecVel);
		}

		return SCHED_GRUNT_REPEL_ATTACK;
	}
	default:
	{
		return BaseClass::TranslateSchedule(scheduleType);
	}
	}
}

class CHGruntRepel :public CAI_BaseNPC
{
	DECLARE_CLASS(CHGruntRepel, CAI_BaseNPC);
public:
	void Spawn(void);
	void Precache(void);
	void RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(monster_grunt_repel, CHGruntRepel);

BEGIN_DATADESC(CHGruntRepel)
DEFINE_USEFUNC(RepelUse),
END_DATADESC()

void CHGruntRepel::Spawn(void)
{
	Precache();
	SetSolid(SOLID_NONE);

	SetUse(&CHGruntRepel::RepelUse);
}

void CHGruntRepel::Precache(void)
{
	UTIL_PrecacheOther("monster_human_grunt");
	m_iSpriteTexture = PrecacheModel("sprites/rope.vmt");
}

void CHGruntRepel::RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -4096.0), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	CBaseEntity* pEntity = Create("monster_human_grunt", GetAbsOrigin(), GetAbsAngles());
	CAI_BaseNPC* pGrunt = pEntity->MyNPCPointer();
	pGrunt->SetMoveType(MOVETYPE_FLYGRAVITY);
	pGrunt->SetGravity(0.001);
	pGrunt->SetAbsVelocity(Vector(0, 0, random->RandomFloat(-196, -128)));
	pGrunt->SetActivity(ACT_GLIDE);
	pGrunt->m_vecLastPosition = tr.endpos;

	CBeam* pBeam = CBeam::BeamCreate("sprites/rope.vmt", 10);
	pBeam->PointEntInit(GetAbsOrigin() + Vector(0, 0, 112), pGrunt);
	pBeam->SetBeamFlags(FBEAM_SOLID);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + -4096.0 * tr.fraction / pGrunt->GetAbsVelocity().z + 0.5);

	UTIL_Remove(this);
}

AI_BEGIN_CUSTOM_NPC(monster_human_grunt, CHGrunt)
DECLARE_ACTIVITY(ACT_GRUNT_LAUNCH_GRENADE)
DECLARE_ACTIVITY(ACT_GRUNT_TOSS_GRENADE)
DECLARE_ACTIVITY(ACT_GRUNT_MP5_STANDING);
DECLARE_ACTIVITY(ACT_GRUNT_MP5_CROUCHING);
DECLARE_ACTIVITY(ACT_GRUNT_SHOTGUN_STANDING);
DECLARE_ACTIVITY(ACT_GRUNT_SHOTGUN_CROUCHING);
DECLARE_CONDITION(COND_GRUNT_NOFIRE)
DECLARE_TASK(TASK_GRUNT_FACE_TOSS_DIR)
DECLARE_TASK(TASK_GRUNT_SPEAK_SENTENCE)
DECLARE_TASK(TASK_GRUNT_CHECK_FIRE)
DECLARE_SQUADSLOT(SQUAD_SLOT_GRENADE1)
DECLARE_SQUADSLOT(SQUAD_SLOT_GRENADE2)
DECLARE_SQUADSLOT(SQUAD_SLOT_ENGAGE1)
DECLARE_SQUADSLOT(SQUAD_SLOT_ENGAGE2)
DEFINE_SCHEDULE
(
	SCHED_GRUNT_FAIL,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
	"		TASK_WAIT				0"
	"		TASK_WAIT_PVS			0"
	"	"
	"	Interrupts"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_COMBAT_FAIL,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
	"		TASK_WAIT_FACE_ENEMY	2"
	"		TASK_WAIT_PVS			0"
	"	"
	"	Interrupts"
	"		COND_CAN_RANGE_ATTACK1"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_VICTORY_DANCE,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_WAIT						1.5"
	"		TASK_GET_PATH_TO_ENEMY_CORPSE	0"
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_VICTORY_DANCE"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY"
	"		TASK_GET_PATH_TO_ENEMY		0"
	"		TASK_GRUNT_SPEAK_SENTENCE	0"
	"		TASK_RUN_PATH				0"
	"		TASK_WAIT_FOR_MOVEMENT		0"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK2"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE_RETRY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_GRUNT_TAKE_COVER_FROM_ENEMY"
	"		TASK_GET_PATH_TO_ENEMY_LKP_LOS	0"
	"		TASK_GRUNT_SPEAK_SENTENCE		0"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK2"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_FOUND_ENEMY,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SIGNAL1"
	"	"
	"	Interrupts"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_COMBAT_FACE,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
	"		TASK_FACE_ENEMY			0"
	"		TASK_WAIT				1.5"
	"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_GRUNT_SWEEP"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_SIGNAL_SUPPRESS,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_FACE_IDEAL					0"
	"		TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_SIGNAL2"
	"		TASK_FACE_ENEMY					0"
	"		TASK_GRUNT_CHECK_FIRE			0"
	"		TASK_RANGE_ATTACK1				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_GRUNT_CHECK_FIRE			0"
	"		TASK_RANGE_ATTACK1				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_GRUNT_CHECK_FIRE			0"
	"		TASK_RANGE_ATTACK1				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_GRUNT_CHECK_FIRE			0"
	"		TASK_RANGE_ATTACK1				0"
	"		TASK_FACE_ENEMY					0"
	"		TASK_GRUNT_CHECK_FIRE			0"
	"		TASK_RANGE_ATTACK1				0"
	"	"
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_GRUNT_NOFIRE"
	"		COND_NO_PRIMARY_AMMO"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_SUPPRESS,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_GRUNT_CHECK_FIRE		0"
	"		TASK_RANGE_ATTACK1			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_GRUNT_CHECK_FIRE		0"
	"		TASK_RANGE_ATTACK1			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_GRUNT_CHECK_FIRE		0"
	"		TASK_RANGE_ATTACK1			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_GRUNT_CHECK_FIRE		0"
	"		TASK_RANGE_ATTACK1			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_GRUNT_CHECK_FIRE		0"
	"		TASK_RANGE_ATTACK1			0"
	"	"
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_GRUNT_NOFIRE"
	"		COND_NO_PRIMARY_AMMO"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_WAIT_IN_COVER,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
	"		TASK_WAIT_FACE_ENEMY	1"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_HEAR_DANGER"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK2"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_TAKE_COVER,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_GRUNT_TAKE_COVER_FAILED"
	"		TASK_WAIT					0.2"
	"		TASK_FIND_COVER_FROM_ENEMY	0"
	"		TASK_GRUNT_SPEAK_SENTENCE	0"
	"		TASK_RUN_PATH				0"
	"		TASK_WAIT_FOR_MOVEMENT		0"
	"		TASK_REMEMBER				MEMORY:INCOVER"
	"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER"
	"	"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_GRENADE_COVER,

	"	Tasks"
	"		TASK_STOP_MOVING						0"
	"		TASK_FIND_COVER_FROM_ENEMY				99"
	"		TASK_FIND_FAR_NODE_COVER_FROM_ENEMY		384"
	"		TASK_PLAY_SEQUENCE						ACTIVITY:ACT_SPECIAL_ATTACK1"
	"		TASK_CLEAR_MOVE_WAIT					0"
	"		TASK_RUN_PATH							0"
	"		TASK_WAIT_FOR_MOVEMENT					0"
	"		TASK_SET_SCHEDULE						SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER"
	"	"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_TOSS_GRENADE_COVER,

	"	Tasks"
	"		TASK_FACE_ENEMY				0"
	"		TASK_RANGE_ATTACK2			0"
	"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_TAKE_COVER_FROM_ENEMY"
	"	"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_HIDE_RELOAD,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_GRUNT_RELOAD"
	"		TASK_FIND_COVER_FROM_ENEMY		0"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_REMEMBER					MEMORY:INCOVER"
	"		TASK_FACE_ENEMY					0"
	"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_RELOAD"
	"	"
	"	Interrupts"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_SWEEP,

	"	Tasks"
	"		TASK_TURN_LEFT			179"
	"		TASK_WAIT				1"
	"		TASK_TURN_LEFT			179"
	"		TASK_WAIT				1"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_HEAR_WORLD"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_PLAYER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_RANGE_ATTACK1A,

	"	Tasks"
	"		TASK_STOP_MOVING					0"
	"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCH"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_HEAVY_DAMAGE"
	"		COND_ENEMY_OCCLUDED"
	"		COND_HEAR_DANGER"
	"		COND_GRUNT_NOFIRE"
	"		COND_NO_PRIMARY_AMMO"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_RANGE_ATTACK1B,

	"	Tasks"
	"		TASK_STOP_MOVING					0"
	"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_IDLE_ANGRY"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"		TASK_FACE_ENEMY						0"
	"		TASK_GRUNT_CHECK_FIRE				0"
	"		TASK_RANGE_ATTACK1					0"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_HEAVY_DAMAGE"
	"		COND_ENEMY_OCCLUDED"
	"		COND_HEAR_DANGER"
	"		COND_GRUNT_NOFIRE"
	"		COND_NO_PRIMARY_AMMO"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_RANGE_ATTACK2,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_GRUNT_FACE_TOSS_DIR	0"
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_RANGE_ATTACK2"
	"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_WAIT_IN_COVER"
	"	"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_REPEL,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_IDEAL				0"
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_GLIDE"
	"	"
	"	Interrupts"
	"		COND_SEE_ENEMY"
	"		COND_NEW_ENEMY"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_PLAYER"
	"		COND_HEAR_COMBAT"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_REPEL_ATTACK,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_FLY"
	"	"
	"	Interrupts"
	"		COND_ENEMY_OCCLUDED"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_REPEL_LAND,

	"	Tasks"
	"		TASK_STOP_MOVING					0"
	"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_LAND"
	"		TASK_GET_PATH_TO_LASTPOSITION		0"
	"		TASK_RUN_PATH						0"
	"		TASK_WAIT_FOR_MOVEMENT				0"
	"		TASK_CLEAR_LASTPOSITION				0"
	"	"
	"	Interrupts"
	"		COND_SEE_ENEMY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
"		COND_HEAR_COMBAT"
"		COND_HEAR_PLAYER"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_RELOAD,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_RELOAD"
	"	"
	"	Interrupts"
	"		COND_HEAVY_DAMAGE"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_TAKE_COVER_FROM_ENEMY,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_WAIT						0.2"
	"		TASK_FIND_COVER_FROM_ENEMY		0"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_REMEMBER					MEMORY:INCOVER"
	"		TASK_FACE_ENEMY					0"
	"		TASK_WAIT						1"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_TAKE_COVER_FAILED,
	"	Tasks"
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_BARNACLE_HIT,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_HIT"
	"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_BARNACLE_PULL"
	""
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_BARNACLE_PULL,

	"	Tasks"
	"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_PULL"
	""
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_BARNACLE_CHOMP,

	"	Tasks"
	"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHOMP"
	"		 TASK_SET_SCHEDULE			SCHEDULE:SCHED_GRUNT_BARNACLE_CHEW"
	""
	"	Interrupts"
)

DEFINE_SCHEDULE
(
	SCHED_GRUNT_BARNACLE_CHEW,

	"	Tasks"
	"		 TASK_PLAY_SEQUENCE			ACTIVITY:ACT_BARNACLE_CHEW"
)

AI_END_CUSTOM_NPC()

class CDeadHGrunt : public CAI_BaseNPC
{
	DECLARE_CLASS(CDeadHGrunt, CAI_BaseNPC);
public:
	void Spawn(void);
	Class_T	Classify(void) { return	CLASS_HUMAN_MILITARY; }
	float MaxYawSpeed(void) { return 8.0f; }

	bool KeyValue(const char* szKeyName, const char* szValue);

	int	m_iPose;
	static char* m_szPoses[3];
};

LINK_ENTITY_TO_CLASS(monster_hgrunt_dead, CDeadHGrunt);

char* CDeadHGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

bool CDeadHGrunt::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "pose"))
		m_iPose = atoi(szValue);
	else
		CAI_BaseNPC::KeyValue(szKeyName, szValue);

	return true;
}

void CDeadHGrunt::Spawn(void)
{
	PrecacheModel("models/hgrunt.mdl");
	SetModel("models/hgrunt.mdl");

	ClearEffects();
	SetSequence(0);
	m_bloodColor = BLOOD_COLOR_RED;

	SetSequence(LookupSequence(m_szPoses[m_iPose]));

	if (GetSequence() == -1)
	{
		Msg("Dead hgrunt with bad pose\n");
	}

	m_iHealth = 8;

	switch (m_nBody)
	{
	case 0:
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 1:
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 2:
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	case 3:
		m_nBody = 0;
		m_nSkin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	}

	NPCInitDead();
}