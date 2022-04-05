#include	"cbase.h"
#include	"ai_default.h"
#include	"ai_task.h"
#include	"ai_schedule.h"
#include	"ai_node.h"
#include	"ai_hull.h"
#include	"ai_hint.h"
#include	"ai_memory.h"
#include	"ai_route.h"
#include	"ai_senses.h"
#include	"soundent.h"
#include	"game.h"
#include	"npcevent.h"
#include	"entitylist.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"IEffects.h"
#include	"vstdlib/random.h"
#include	"engine/IEngineSound.h"
#include	"ammodef.h"
#include    "te.h"
#include    "hl1_monster_hornet.h"

int iHornetTrail;
int iHornetPuff;

LINK_ENTITY_TO_CLASS(hornet, CHL1Hornet);

extern ConVar sk_npc_dmg_hornet;
extern ConVar sk_plr_dmg_hornet;

BEGIN_DATADESC(CHL1Hornet)
DEFINE_FIELD(m_flStopAttack, FIELD_TIME),
DEFINE_FIELD(m_iHornetType, FIELD_INTEGER),
DEFINE_FIELD(m_flFlySpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flDamage, FIELD_INTEGER),
DEFINE_FIELD(m_vecEnemyLKP, FIELD_POSITION_VECTOR),

DEFINE_ENTITYFUNC(DieTouch),
DEFINE_THINKFUNC(StartDart),
DEFINE_THINKFUNC(StartTrack),
DEFINE_ENTITYFUNC(DartTouch),
DEFINE_ENTITYFUNC(TrackTouch),
DEFINE_THINKFUNC(TrackTarget),
END_DATADESC()

void CHL1Hornet::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_FLY);
	SetSolid(SOLID_BBOX);
	m_takedamage = DAMAGE_YES;
	AddFlag(FL_NPC);
	m_iHealth = 1;
	m_bloodColor = DONT_BLEED;

	if (g_pGameRules->IsMultiplayer())
	{
		m_flStopAttack = gpGlobals->curtime + 3.5;
	}
	else
	{
		m_flStopAttack = gpGlobals->curtime + 5.0;
	}

	m_flFieldOfView = 0.9;

	if (random->RandomInt(1, 5) <= 2)
	{
		m_iHornetType = 0;
		m_flFlySpeed = 600;
	}
	else
	{
		m_iHornetType = 1;
		m_flFlySpeed = 800;
	}

	SetModel("models/hornet.mdl");
	UTIL_SetSize(this, Vector(-4, -4, -4), Vector(4, 4, 4));

	SetTouch(&CHL1Hornet::DieTouch);
	SetThink(&CHL1Hornet::StartTrack);

	if (GetOwnerEntity() && (GetOwnerEntity()->GetFlags() & FL_CLIENT))
	{
		m_flDamage = sk_plr_dmg_hornet.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_hornet.GetFloat();
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
	ResetSequenceInfo();

	m_vecEnemyLKP = vec3_origin;
}

void CHL1Hornet::Precache()
{
	PrecacheModel("models/hornet.mdl");

	iHornetPuff = PrecacheModel("sprites/muz1.vmt");
	iHornetTrail = PrecacheModel("sprites/laserbeam.vmt");

	PrecacheScriptSound("Hornet.Die");
	PrecacheScriptSound("Hornet.Buzz");
}

Disposition_t CHL1Hornet::IRelationType(CBaseEntity* pTarget)
{
	if (pTarget->GetModelIndex() == GetModelIndex())
	{
		return D_NU;
	}

	return BaseClass::IRelationType(pTarget);
}

Class_T CHL1Hornet::Classify(void)
{
	if (GetOwnerEntity() && (GetOwnerEntity()->GetFlags() & FL_CLIENT))
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

void CHL1Hornet::StartDart(void)
{
	IgniteTrail();

	SetTouch(&CHL1Hornet::DartTouch);

	SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + 4);
}


void CHL1Hornet::DieTouch(CBaseEntity* pOther)
{
	if (!pOther || !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
	{
		return;
	}

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Hornet.Die");

	CTakeDamageInfo info(this, GetOwnerEntity(), m_flDamage, DMG_BULLET);
	CalculateBulletDamageForce(&info, GetAmmoDef()->Index("Hornet"), GetAbsVelocity(), GetAbsOrigin());
	pOther->TakeDamage(info);

	m_takedamage = DAMAGE_NO;

	AddEffects(EF_NODRAW);

	AddSolidFlags(FSOLID_NOT_SOLID);

	UTIL_Remove(this);
	SetTouch(NULL);
}

void CHL1Hornet::StartTrack(void)
{
	IgniteTrail();

	SetTouch(&CHL1Hornet::TrackTouch);
	SetThink(&CHL1Hornet::TrackTarget);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

void TE_BeamFollow(IRecipientFilter& filter, float delay,
	int iEntIndex, int modelIndex, int haloIndex, float life, float width, float endWidth,
	float fadeLength, float r, float g, float b, float a);

void CHL1Hornet::IgniteTrail(void)
{
	Vector vColor;

	if (m_iHornetType == 0)
		vColor = Vector(179, 39, 14);
	else
		vColor = Vector(255, 128, 0);

	CBroadcastRecipientFilter filter;
	TE_BeamFollow(filter, 0.0,
		entindex(),
		iHornetTrail,
		0,
		1,
		2,
		0.5,
		0.5,
		vColor.x,
		vColor.y,
		vColor.z,
		128);
}


unsigned int CHL1Hornet::PhysicsSolidMaskForEntity(void) const
{
	unsigned int iMask = BaseClass::PhysicsSolidMaskForEntity();

	iMask &= ~CONTENTS_MONSTERCLIP;

	return iMask;
}

void CHL1Hornet::TrackTouch(CBaseEntity* pOther)
{
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
	{
		return;
	}

	if (pOther == GetOwnerEntity() || pOther->GetModelIndex() == GetModelIndex())
	{
		return;
	}

	int nRelationship = IRelationType(pOther);
	if ((nRelationship == D_FR || nRelationship == D_NU || nRelationship == D_LI))
	{
		Vector vecVel = GetAbsVelocity();

		VectorNormalize(vecVel);

		vecVel.x *= -1;
		vecVel.y *= -1;

		SetAbsOrigin(GetAbsOrigin() + vecVel * 4);
		SetAbsVelocity(vecVel * m_flFlySpeed);

		return;
	}

	DieTouch(pOther);
}

void CHL1Hornet::DartTouch(CBaseEntity* pOther)
{
	DieTouch(pOther);
}

void CHL1Hornet::TrackTarget(void)
{
	Vector	vecFlightDir;
	Vector	vecDirToEnemy;
	float	flDelta;

	StudioFrameAdvance();

	if (gpGlobals->curtime > m_flStopAttack)
	{
		SetTouch(NULL);
		SetThink(&CBaseEntity::SUB_Remove);
		SetNextThink(gpGlobals->curtime + 0.1f);
		return;
	}

	if (GetEnemy() == NULL)
	{
		GetSenses()->Look(1024);
		SetEnemy(BestEnemy());
	}

	if (GetEnemy() != NULL && FVisible(GetEnemy()))
	{
		m_vecEnemyLKP = GetEnemy()->BodyTarget(GetAbsOrigin());
	}
	else
	{
		m_vecEnemyLKP = m_vecEnemyLKP + GetAbsVelocity() * m_flFlySpeed * 0.1;
	}

	vecDirToEnemy = m_vecEnemyLKP - GetAbsOrigin();
	VectorNormalize(vecDirToEnemy);

	if (GetAbsVelocity().Length() < 0.1)
		vecFlightDir = vecDirToEnemy;
	else
	{
		vecFlightDir = GetAbsVelocity();
		VectorNormalize(vecFlightDir);
	}

	SetAbsVelocity(vecFlightDir + vecDirToEnemy);

	flDelta = DotProduct(vecFlightDir, vecDirToEnemy);

	if (flDelta < 0.5)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "Hornet.Buzz");
	}

	if (flDelta <= 0 && m_iHornetType == 0)
	{
		flDelta = 0.25;
	}

	Vector vecVel = vecFlightDir + vecDirToEnemy;
	VectorNormalize(vecVel);

	if (GetOwnerEntity() && (GetOwnerEntity()->GetFlags() & FL_NPC))
	{
		vecVel.x += random->RandomFloat(-0.10, 0.10);
		vecVel.y += random->RandomFloat(-0.10, 0.10);
		vecVel.z += random->RandomFloat(-0.10, 0.10);
	}

	switch (m_iHornetType)
	{
	case 0:
		SetAbsVelocity(vecVel * (m_flFlySpeed * flDelta));
		SetNextThink(gpGlobals->curtime + random->RandomFloat(0.1, 0.3));
		break;
	default:
		Assert(false);
	case 1:
		SetAbsVelocity(vecVel * m_flFlySpeed);
		SetNextThink(gpGlobals->curtime + 0.1f);
		break;
	}

	QAngle angNewAngles;
	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetSolid(SOLID_BBOX);

	if (GetEnemy() != NULL && !g_pGameRules->IsMultiplayer())
	{
		if (flDelta >= 0.4 && (GetAbsOrigin() - m_vecEnemyLKP).Length() <= 300)
		{
			CPVSFilter filter(GetAbsOrigin());
			te->Sprite(filter, 0.0,
				&GetAbsOrigin(),
				iHornetPuff,
				0.2,
				128
			);

			CPASAttenuationFilter filter2(this);
			EmitSound(filter2, entindex(), "Hornet.Buzz");
			SetAbsVelocity(GetAbsVelocity() * 2);
			SetNextThink(gpGlobals->curtime + 1.0f);

			m_flStopAttack = gpGlobals->curtime;
		}
	}
}