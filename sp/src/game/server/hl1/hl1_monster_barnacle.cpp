#include "cbase.h"
#include "hl1_monster_barnacle.h"
#include "npcevent.h"
#include "gib.h"
#include "ai_default.h"
#include "activitylist.h"
#include "hl2_player.h"
#include "vstdlib/random.h"
#include "physics_saverestore.h"
#include "vcollide_parse.h"
#include "engine/IEngineSound.h"

ConVar	sk_barnacle_health("sk_barnacle_health", "25");

static int ACT_EAT = 0;

int	g_interactionBarnacleVictimDangle = 0;
int	g_interactionBarnacleVictimReleased = 0;
int	g_interactionBarnacleVictimGrab = 0;

LINK_ENTITY_TO_CLASS(monster_barnacle, CHL1Barnacle);
IMPLEMENT_CUSTOM_AI(monster_barnacle, CHL1Barnacle);

void CHL1Barnacle::InitCustomSchedules(void)
{
	INIT_CUSTOM_AI(CHL1Barnacle);

	ADD_CUSTOM_ACTIVITY(CHL1Barnacle, ACT_EAT);

	g_interactionBarnacleVictimDangle = CBaseCombatCharacter::GetInteractionID();
	g_interactionBarnacleVictimReleased = CBaseCombatCharacter::GetInteractionID();
	g_interactionBarnacleVictimGrab = CBaseCombatCharacter::GetInteractionID();
}


BEGIN_DATADESC(CHL1Barnacle)

DEFINE_FIELD(m_flAltitude, FIELD_FLOAT),
DEFINE_FIELD(m_flKillVictimTime, FIELD_TIME),
DEFINE_FIELD(m_cGibs, FIELD_INTEGER),
DEFINE_FIELD(m_fLiftingPrey, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTongueAdj, FIELD_FLOAT),
DEFINE_FIELD(m_flIgnoreTouchesUntil, FIELD_TIME),

DEFINE_THINKFUNC(BarnacleThink),
DEFINE_THINKFUNC(WaitTillDead),
END_DATADESC()

Class_T	CHL1Barnacle::Classify(void)
{
	return	CLASS_BARNACLE;
}

void CHL1Barnacle::HandleAnimEvent(animevent_t* pEvent)
{
	switch (pEvent->event)
	{
	case BARNACLE_AE_PUKEGIB:
		CGib::SpawnRandomGibs(this, 1, GIB_HUMAN);
		break;
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CHL1Barnacle::Spawn()
{
	Precache();

	SetModel("models/barnacle.mdl");
	UTIL_SetSize(this, Vector(-16, -16, -32), Vector(16, 16, 0));

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_NONE);
	SetBloodColor(BLOOD_COLOR_GREEN);
	m_iHealth = sk_barnacle_health.GetFloat();
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	m_flKillVictimTime = 0;
	m_cGibs = 0;
	m_fLiftingPrey = false;
	m_takedamage = DAMAGE_YES;
	m_flTongueAdj = -100;

	InitBoneControllers();

	SetDefaultEyeOffset();

	SetActivity(ACT_IDLE);

	SetThink(&CHL1Barnacle::BarnacleThink);
	SetNextThink(gpGlobals->curtime + 0.5f);
	AddEffects(EF_NOSHADOW);

	m_flIgnoreTouchesUntil = gpGlobals->curtime;
}

int	CHL1Barnacle::OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo)
{
	CTakeDamageInfo info = inputInfo;
	if (info.GetDamageType() & DMG_CLUB)
	{
		info.SetDamage(m_iHealth);
	}

	return BaseClass::OnTakeDamage_Alive(info);
}

void CHL1Barnacle::BarnacleThink(void)
{
	CBaseEntity* pTouchEnt;
	float flLength;

	SetNextThink(gpGlobals->curtime + 0.1f);

	if (CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI)
	{

	}
	else if (GetEnemy() != NULL)
	{
		if (!GetEnemy()->IsAlive())
		{
			m_fLiftingPrey = false;
			SetEnemy(NULL);
			//I have to set him to be eating before making him idle otherwise he will keep playing his pulling animation after losing his victim for some reason.
			SetActivity((Activity)ACT_EAT);
			SetActivity(ACT_IDLE);
			return;
		}

		CBaseCombatCharacter* pVictim = GetEnemyCombatCharacterPointer();
		Assert(pVictim);

		if (m_fLiftingPrey)
		{

			if (GetEnemy() != NULL && pVictim->m_lifeState == LIFE_DEAD)
			{
				SetEnemy(NULL);
				m_fLiftingPrey = false;
				//I have to set him to be eating before making him idle otherwise he will keep playing his pulling animation after losing his victim for some reason.
				SetActivity((Activity)ACT_EAT);
				SetActivity(ACT_IDLE);
				return;
			}

			Vector vecNewEnemyOrigin = GetEnemy()->GetLocalOrigin();
			vecNewEnemyOrigin.x = GetLocalOrigin().x;
			vecNewEnemyOrigin.y = GetLocalOrigin().y;

			vecNewEnemyOrigin.x -= 6 * cos(GetEnemy()->GetLocalAngles().y * M_PI / 180.0);
			vecNewEnemyOrigin.y -= 6 * sin(GetEnemy()->GetLocalAngles().y * M_PI / 180.0);

			m_flAltitude -= BARNACLE_PULL_SPEED;
			vecNewEnemyOrigin.z += BARNACLE_PULL_SPEED;

			if (fabs(GetLocalOrigin().z - (vecNewEnemyOrigin.z + GetEnemy()->GetViewOffset().z -8 )) < BARNACLE_BODY_HEIGHT)
			{
				m_fLiftingPrey = false;

				CPASAttenuationFilter filter(this);
				EmitSound(filter, entindex(), "Barnacle.Bite");

				m_flKillVictimTime = gpGlobals->curtime + 10;

				if (pVictim)
				{
					pVictim->DispatchInteraction(g_interactionBarnacleVictimDangle, NULL, this);
					SetActivity((Activity)ACT_EAT);
				}
			}

			CBaseEntity* pEnemy = GetEnemy();

			trace_t trace;
			UTIL_TraceEntity(pEnemy, pEnemy->GetAbsOrigin(), vecNewEnemyOrigin, MASK_SOLID_BRUSHONLY, pEnemy, COLLISION_GROUP_NONE, &trace);

			if (trace.fraction != 1.0)
			{
				SetEnemy(NULL);
				m_fLiftingPrey =false;

				if (pEnemy->MyCombatCharacterPointer())
				{
					pEnemy->MyCombatCharacterPointer()->DispatchInteraction(g_interactionBarnacleVictimReleased, NULL, this);
				}

				m_flIgnoreTouchesUntil = gpGlobals->curtime + 1.5;

				//I have to set him to be eating before making him idle otherwise he will keep playing his pulling animation after losing his victim for some reason.
				SetActivity((Activity)ACT_EAT);
				SetActivity(ACT_IDLE);

				return;
			}

			UTIL_SetOrigin(GetEnemy(), vecNewEnemyOrigin);
		}
		else
		{
			if (m_flKillVictimTime != -1 && gpGlobals->curtime > m_flKillVictimTime)
			{
				if (pVictim)
				{
					pVictim->TakeDamage(CTakeDamageInfo(this, this, pVictim->m_iHealth, DMG_SLASH | DMG_ALWAYSGIB | DMG_CRUSH));
					m_cGibs = 3;
				}

				return;
			}

			if (pVictim && (random->RandomInt(0, 49) == 0))
			{
				CPASAttenuationFilter filter(this);
				EmitSound(filter, entindex(), "Barnacle.Chew");

				if (pVictim)
				{
					pVictim->DispatchInteraction(g_interactionBarnacleVictimDangle, NULL, this);
				}
			}
		}
	}
	else
	{
		if (!UTIL_FindClientInPVS(edict()))
		{
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(1);
				
			if (pPlayer)
			{
				Vector vecDist = pPlayer->GetAbsOrigin() - GetAbsOrigin();
					
				if (vecDist.Length2DSqr() >= Square(600.0f))
				{
					SetNextThink(gpGlobals->curtime + 1.5f);
				}
			}
		}

		if (IsActivityFinished())
		{
			SetActivity(ACT_IDLE);
			m_flTongueAdj = -100;
		}

		if (m_cGibs && random->RandomInt(0, 99) == 1)
		{
			CGib::SpawnRandomGibs(this, 1, GIB_HUMAN);
			m_cGibs--;

			CPASAttenuationFilter filter(this);
			EmitSound(filter, entindex(), "Barnacle.Chew");
		}

		pTouchEnt = TongueTouchEnt(&flLength);

		if (pTouchEnt != NULL)
		{
			CBaseCombatCharacter* pBCC = (CBaseCombatCharacter*)pTouchEnt;

			Vector vecGrabPos = pTouchEnt->GetAbsOrigin();

			if (pBCC && pBCC->DispatchInteraction(g_interactionBarnacleVictimGrab, &vecGrabPos, this))
			{
				CPASAttenuationFilter filter(this);
				EmitSound(filter, entindex(), "Barnacle.Alert");

				SetSequenceByName("attack1");

				SetEnemy(pTouchEnt);

				pTouchEnt->SetMoveType(MOVETYPE_FLY);
				pTouchEnt->SetAbsVelocity(vec3_origin);
				pTouchEnt->SetBaseVelocity(vec3_origin);
				Vector origin = GetAbsOrigin();
				origin.z = pTouchEnt->GetAbsOrigin().z;
				pTouchEnt->SetLocalOrigin(origin);

				m_fLiftingPrey = true;
				m_flKillVictimTime = -1;

				m_flAltitude = (GetAbsOrigin().z - vecGrabPos.z);
			}
		}
		else
		{
			if (m_flAltitude < flLength)
			{
				m_flAltitude += BARNACLE_PULL_SPEED;
			}
			else
			{
				m_flAltitude = flLength;
			}

		}

	}

	//Setting the Bone Controller back to 0 before setting it's actual length somehow fixes the issue with barnacle tongues often not rendering at the proper height.
	SetBoneController(0, 0);
	SetBoneController(0, -(m_flAltitude + m_flTongueAdj));
	StudioFrameAdvance();
}

void CHL1Barnacle::Event_Killed(const CTakeDamageInfo& info)
{
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;
	m_lifeState = LIFE_DEAD;
	if (GetEnemy() != NULL)
	{
		CBaseCombatCharacter* pVictim = GetEnemyCombatCharacterPointer();

		if (pVictim)
		{
			pVictim->DispatchInteraction(g_interactionBarnacleVictimReleased, NULL, this);
		}
	}

	CGib::SpawnRandomGibs(this, 4, GIB_HUMAN);

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Barnacle.Die");

	SetActivity(ACT_DIESIMPLE);
	SetBoneController(0, 0);

	StudioFrameAdvance();

	SetNextThink(gpGlobals->curtime + 0.1f);
	SetThink(&CHL1Barnacle::WaitTillDead);
}

void CHL1Barnacle::WaitTillDead(void)
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	StudioFrameAdvance();
	DispatchAnimEvents(this);

	if (IsActivityFinished())
	{
		StopAnimation();
		SetThink(NULL);
	}
}

void CHL1Barnacle::Precache()
{
	PrecacheModel("models/barnacle.mdl");

	PrecacheScriptSound("Barnacle.Bite");
	PrecacheScriptSound("Barnacle.Chew");
	PrecacheScriptSound("Barnacle.Alert");
	PrecacheScriptSound("Barnacle.Die");

	BaseClass::Precache();
}

#define BARNACLE_CHECK_SPACING	8
CBaseEntity* CHL1Barnacle::TongueTouchEnt(float* pflLength)
{
	trace_t		tr;
	float		length;

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 2048),
		MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	length = fabs(GetAbsOrigin().z - tr.endpos.z);
	length -= 16;
	if (pflLength)
	{
		*pflLength = length;
	}

	if (m_flIgnoreTouchesUntil > gpGlobals->curtime)
		return NULL;

	Vector delta = Vector(BARNACLE_CHECK_SPACING, BARNACLE_CHECK_SPACING, 0);
	Vector mins = GetAbsOrigin() - delta;
	Vector maxs = GetAbsOrigin() + delta;
	maxs.z = GetAbsOrigin().z;

	mins.z -= min(m_flAltitude, length);

	CBaseEntity* pList[10];
	int count = UTIL_EntitiesInBox(pList, 10, mins, maxs, (FL_CLIENT | FL_NPC));
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			CBaseCombatCharacter* pVictim = ToBaseCombatCharacter(pList[i]);

			bool bCanHurt = false;

			if (IRelationType(pList[i]) == D_HT || IRelationType(pList[i]) == D_FR)
				bCanHurt = true;

			if (pList[i] != this && bCanHurt == true && pVictim->m_lifeState == LIFE_ALIVE)
			{
				return pList[i];
			}
		}
	}

	return NULL;
}