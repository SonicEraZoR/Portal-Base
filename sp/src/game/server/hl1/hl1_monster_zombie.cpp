#include "cbase.h"
#include "game.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_route.h"
#include "npcevent.h"
#include "hl1_monster.h"
#include "gib.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"

#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03

#define ZOMBIE_FLINCH_DELAY			2

ConVar	sk_zombie_health("sk_zombie_health", "50");
ConVar  sk_zombie_dmg_one_slash("sk_zombie_dmg_one_slash", "20");
ConVar  sk_zombie_dmg_both_slash("sk_zombie_dmg_both_slash", "40");

static float DamageForce(const Vector &size, float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (size.x * size.y * size.z)) * 5;

	if (force > 1000.0)
	{
		force = 1000.0;
	}

	return force;
}

class CHL1Zombie : public CHL1BaseMonster
{
public:
	DECLARE_CLASS(CHL1Zombie, CHL1BaseMonster);

	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void) { return 120; }
	Class_T  Classify(void);
	void HandleAnimEvent(animevent_t *pEvent);
	void RemoveIgnoredConditions(void);

	void PainSound(CTakeDamageInfo &info);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);

	int RangeAttack1Conditions(float flDot, float flDist) { return FALSE; }
	int RangeAttack2Conditions(float flDot, float flDist) { return FALSE; }
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

	float m_flNextFlinch;
};

LINK_ENTITY_TO_CLASS(monster_zombie, CHL1Zombie);

Class_T	CHL1Zombie::Classify(void)
{
	return	CLASS_ALIEN_MONSTER;
}

int CHL1Zombie::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;

	if (info.GetDamageType() == DMG_BULLET)
	{
		Vector vecDir = GetAbsOrigin() - info.GetInflictor()->WorldSpaceCenter();
		VectorNormalize(vecDir);
		float flForce = DamageForce(WorldAlignSize(), info.GetDamage());
		SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
		info.ScaleDamage(0.3f);
	}

	if (IsAlive())
		PainSound(info);
	return BaseClass::OnTakeDamage_Alive(info);
}

void CHL1Zombie::PainSound(CTakeDamageInfo &info)
{
	if (random->RandomInt(0, 5) < 2)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Zombie.Pain");
	}
}

void CHL1Zombie::AlertSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Zombie.Alert");
}

void CHL1Zombie::IdleSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Zombie.Idle");
}

void CHL1Zombie::AttackSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Zombie.Attack");
}

void CHL1Zombie::HandleAnimEvent(animevent_t *pEvent)
{
	Vector v_forward, v_right;
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	CPASAttenuationFilter filter(this);

	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_zombie_dmg_one_slash.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, 18));

				GetVectors(&v_forward, &v_right, NULL);

				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - v_right * 100);
			}
			EmitSound(filter, entindex(), "Zombie.AttackHit");
		}
		else
			EmitSound(filter, entindex(), "Zombie.AttackMiss");

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_LEFT:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_zombie_dmg_one_slash.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, -18));

				GetVectors(&v_forward, &v_right, NULL);

				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - v_right * 100);
			}
			EmitSound(filter, entindex(), "Zombie.AttackHit");
		}
		else
			EmitSound(filter, entindex(), "Zombie.AttackMiss");

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_BOTH:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(70, vecMins, vecMaxs, sk_zombie_dmg_one_slash.GetFloat(), DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
			{
				pHurt->ViewPunch(QAngle(5, 0, 0));
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + v_forward * -100);
			}
			EmitSound(filter, entindex(), "Zombie.AttackHit");
		}
		else
			EmitSound(filter, entindex(), "Zombie.AttackMiss");

		if (random->RandomInt(0, 1))
			AttackSound();
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CHL1Zombie::Spawn()
{
	Precache();

	SetModel("models/zombie.mdl");
	SetRenderColor(255, 255, 255, 255);
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_GREEN;
	m_iHealth = sk_zombie_health.GetFloat();
	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;
	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP);

	NPCInit();
}

void CHL1Zombie::Precache()
{
	PrecacheModel("models/zombie.mdl");

	PrecacheScriptSound("Zombie.AttackHit");
	PrecacheScriptSound("Zombie.AttackMiss");
	PrecacheScriptSound("Zombie.Pain");
	PrecacheScriptSound("Zombie.Idle");
	PrecacheScriptSound("Zombie.Alert");
	PrecacheScriptSound("Zombie.Attack");

	BaseClass::Precache();
}

void CHL1Zombie::RemoveIgnoredConditions(void)
{
	if (GetActivity() == ACT_MELEE_ATTACK1)
	{
		ClearCondition(COND_LIGHT_DAMAGE);
		ClearCondition(COND_HEAVY_DAMAGE);
	}

	if ((GetActivity() == ACT_SMALL_FLINCH) || (GetActivity() == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->curtime)
			m_flNextFlinch = gpGlobals->curtime + ZOMBIE_FLINCH_DELAY;
	}

	BaseClass::RemoveIgnoredConditions();
}