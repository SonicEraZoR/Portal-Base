#include "cbase.h"
#include "hl1_baseweapon_shared.h"

#include "hl2_player_shared.h"

LINK_ENTITY_TO_CLASS(basehl1combatweapon, CBaseHL1CombatWeapon);

IMPLEMENT_NETWORKCLASS_ALIASED(BaseHL1CombatWeapon, DT_BaseHL1CombatWeapon)

BEGIN_NETWORK_TABLE(CBaseHL1CombatWeapon, DT_BaseHL1CombatWeapon)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CBaseHL1CombatWeapon)
END_PREDICTION_DATA()

void CBaseHL1CombatWeapon::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);
	m_flNextEmptySoundTime = 0.0f;

	RemoveEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);

	m_iState = WEAPON_NOT_CARRIED;

	m_nViewModelIndex = 0;

	if (UsesClipsForAmmo1())
	{
		m_iClip1 = GetDefaultClip1();
	}
	else
	{
		SetPrimaryAmmoCount(GetDefaultClip1());
		m_iClip1 = WEAPON_NOCLIP;
	}
	if (UsesClipsForAmmo2())
	{
		m_iClip2 = GetDefaultClip2();
	}
	else
	{
		SetSecondaryAmmoCount(GetDefaultClip2());
		m_iClip2 = WEAPON_NOCLIP;
	}

	SetModel(GetWorldModel());

#if !defined( CLIENT_DLL )
	FallInit();
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetBlocksLOS(false);

	SetRemoveable(false);
#endif

	if (g_pGameRules->IsMultiplayer())
	{
		CollisionProp()->UseTriggerBounds(true, 36);
	}
	else
	{
		CollisionProp()->UseTriggerBounds(true, 24);
	}

	AddEffects(EF_BONEMERGE_FASTCULL);
	AddEffects(EF_NOSHADOW);
}

#if defined( CLIENT_DLL )

#define	HL1_BOB_CYCLE_MIN	1.0f
#define	HL1_BOB_CYCLE_MAX	0.45f
#define	HL1_BOB			0.002f
#define	HL1_BOB_UP		0.5f

float	lateralBob;
float	verticalBob;

static ConVar	cl_bobcycle("cl_bobcycle", "0.8");
static ConVar	cl_bob("cl_bob", "0.002");
static ConVar	cl_bobup("cl_bobup", "0.5");

static ConVar	v_iyaw_cycle("v_iyaw_cycle", "2");
static ConVar	v_iroll_cycle("v_iroll_cycle", "0.5");
static ConVar	v_ipitch_cycle("v_ipitch_cycle", "1");
static ConVar	v_iyaw_level("v_iyaw_level", "0.3");
static ConVar	v_iroll_level("v_iroll_level", "0.1");
static ConVar	v_ipitch_level("v_ipitch_level", "0.3");
float CBaseHL1CombatWeapon::CalcViewmodelBob(void)
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;

	CBasePlayer *player = ToBasePlayer(GetOwner());

	if ((!gpGlobals->frametime) || (player == NULL))
	{
		return 0.0f;
	}

	float speed = player->GetLocalVelocity().Length2D();

	speed = clamp(speed, -320, 320);

	float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);

	bobtime += (gpGlobals->curtime - lastbobtime) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	cycle = bobtime - (int)(bobtime / HL1_BOB_CYCLE_MAX)*HL1_BOB_CYCLE_MAX;
	cycle /= HL1_BOB_CYCLE_MAX;

	if (cycle < HL1_BOB_UP)
	{
		cycle = M_PI * cycle / HL1_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - HL1_BOB_UP) / (1.0 - HL1_BOB_UP);
	}

	verticalBob = speed*0.005f;
	verticalBob = verticalBob*0.3 + verticalBob*0.7*sin(cycle);

	verticalBob = clamp(verticalBob, -7.0f, 4.0f);

	cycle = bobtime - (int)(bobtime / HL1_BOB_CYCLE_MAX * 2)*HL1_BOB_CYCLE_MAX * 2;
	cycle /= HL1_BOB_CYCLE_MAX * 2;

	if (cycle < HL1_BOB_UP)
	{
		cycle = M_PI * cycle / HL1_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle - HL1_BOB_UP) / (1.0 - HL1_BOB_UP);
	}

	lateralBob = speed*0.005f;
	lateralBob = lateralBob*0.3 + lateralBob*0.7*sin(cycle);
	lateralBob = clamp(lateralBob, -7.0f, 4.0f);

	return 0.0f;
}

void CBaseHL1CombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
	Vector	forward, right;
	AngleVectors(angles, &forward, &right, NULL);

	CalcViewmodelBob();

	VectorMA(origin, verticalBob * 0.1f, forward, origin);

	origin[2] += verticalBob * 0.1f;

	angles[ROLL] += verticalBob * 0.5f;
	angles[PITCH] -= verticalBob * 0.4f;

	angles[YAW] -= lateralBob  * 0.3f;

	VectorMA(origin, lateralBob * 0.8f, right, origin);
}


#else

Vector CBaseHL1CombatWeapon::GetSoundEmissionOrigin() const
{
	if (gpGlobals->maxClients == 1 || !GetOwner())
		return CBaseCombatWeapon::GetSoundEmissionOrigin();
	return GetOwner()->GetSoundEmissionOrigin();
}

#endif