#include "cbase.h"
#include "hl1_baseweapon_shared.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"

void CBaseHL1CombatWeapon::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("BaseCombatWeapon.WeaponDrop");
}

void CBaseHL1CombatWeapon::FallInit(void)
{
	SetModel(GetWorldModel());
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER);
	AddSolidFlags(FSOLID_NOT_SOLID);

	SetPickupTouch();

	SetThink(&CBaseHL1CombatWeapon::FallThink);

	SetNextThink(gpGlobals->curtime + 0.1f);

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 2), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		SetGroundEntity(tr.m_pEnt);
	}

	SetViewOffset(Vector(0, 0, 8));
}

void CBaseHL1CombatWeapon::FallThink(void)
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	if (GetFlags() & FL_ONGROUND)
	{
		if (GetOwnerEntity())
		{
			EmitSound("BaseCombatWeapon.WeaponDrop");
		}

		QAngle ang = GetAbsAngles();
		ang.x = 0;
		ang.z = 0;
		SetAbsAngles(ang);

		Materialize();

		SetSize(Vector(-24, -24, 0), Vector(24, 24, 16));
	}
}

void CBaseHL1CombatWeapon::EjectShell(CBaseEntity *pPlayer, int iType)
{
	QAngle angShellAngles = pPlayer->GetAbsAngles();

	Vector vecForward, vecRight, vecUp;
	AngleVectors(angShellAngles, &vecForward, &vecRight, &vecUp);

	Vector vecShellPosition = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();
	switch (iType)
	{
	case 0:
	default:
		vecShellPosition += vecRight * 4;
		vecShellPosition += vecUp * -12;
		vecShellPosition += vecForward * 20;
		break;
	case 1:
		vecShellPosition += vecRight * 6;
		vecShellPosition += vecUp * -12;
		vecShellPosition += vecForward * 32;
		break;
	}

	Vector vecShellVelocity = vec3_origin;
	vecShellVelocity += vecRight * random->RandomFloat(50, 70);
	vecShellVelocity += vecUp * random->RandomFloat(100, 150);
	vecShellVelocity += vecForward * 25;

	angShellAngles.x = 0;
	angShellAngles.z = 0;

	CEffectData	data;
	data.m_vStart = vecShellVelocity;
	data.m_vOrigin = vecShellPosition;
	data.m_vAngles = angShellAngles;
	data.m_fFlags = iType;

	DispatchEffect("HL1ShellEject", data);
}