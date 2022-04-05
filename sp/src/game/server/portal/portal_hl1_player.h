#ifndef HL1_PLAYER_H
#define HL1_PLAYER_H
#ifdef _WIN32
#pragma once
#endif
#include "player.h"

extern int TrainSpeed(int iSpeed, int iMax);
extern void CopyToBodyQue(CBaseAnimating* pCorpse);

enum HL1PlayerPhysFlag_e
{
	PFLAG_ONBARNACLE = (1 << 6)
};

class IPhysicsPlayerController;

class CSuitPowerDevice
{
public:
	CSuitPowerDevice(int bitsID, float flDrainRate) { m_bitsDeviceID = bitsID; m_flDrainRate = flDrainRate; }
private:
	int		m_bitsDeviceID;
	float	m_flDrainRate;

public:
	int		GetDeviceID(void) const { return m_bitsDeviceID; }
	float	GetDeviceDrainRate(void) const { return m_flDrainRate; }
};

class CHL1_Player : public CBasePlayer
{
	DECLARE_CLASS(CHL1_Player, CBasePlayer);
	DECLARE_SERVERCLASS();
public:

	DECLARE_DATADESC();

	CHL1_Player();
	~CHL1_Player(void);

	static CHL1_Player* CreatePlayer(const char* className, edict_t* ed)
	{
		CHL1_Player::s_PlayerEdict = ed;
		return (CHL1_Player*)CreateEntityByName(className);
	}

	void		CreateCorpse(void) { CopyToBodyQue(this); };

	void		Precache(void);
	void		Spawn(void);
	void		Event_Killed(const CTakeDamageInfo& info);
	void		CheatImpulseCommands(int iImpulse);
	void		PlayerRunCommand(CUserCmd* ucmd, IMoveHelper* moveHelper);
	void		UpdateClientData(void);
	void		OnSave(IEntitySaveUtils* pUtils);

	void		CheckTimeBasedDamage(void);

	void		InitVCollision(const Vector& vecAbsOrigin, const Vector& vecAbsVelocity);

	Class_T		Classify(void);
	Class_T		m_nControlClass;

	void		SetupVisibility(CBaseEntity* pViewEntity, unsigned char* pvs, int pvssize);

	float		GetIdleTime(void) const { return (m_flIdleTime - m_flMoveTime); }
	float		GetMoveTime(void) const { return (m_flMoveTime - m_flIdleTime); }
	float		GetLastDamageTime(void) const { return m_flLastDamageTime; }
	bool		IsDucking(void) const { return !!(GetFlags() & FL_DUCKING); }

	int			OnTakeDamage(const CTakeDamageInfo& info);
	int			OnTakeDamage_Alive(const CTakeDamageInfo& info);
	void		FindMissTargets(void);
	bool		GetMissPosition(Vector* position);

	void		OnDamagedByExplosion(const CTakeDamageInfo& info) { };
	void		PlayerPickupObject(CBasePlayer* pPlayer, CBaseEntity* pObject);

	virtual void CreateViewModel(int index);

	virtual CBaseEntity* GiveNamedItem(const char* pszName, int iSubType = 0);

	virtual void OnRestore(void);

	bool		IsPullingObject() { return m_bIsPullingObject; }
	void		StartPullingObject(CBaseEntity* pObject);
	void		StopPullingObject();
	void		UpdatePullingObject();

	virtual const char* GetOverrideStepSound(const char* pszBaseStepSoundName);

protected:
	void		PreThink(void);
	bool		HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt);

private:
	Vector		m_vecMissPositions[16];
	int			m_nNumMissPositions;

	float		m_flIdleTime;
	float		m_flMoveTime;
	float		m_flLastDamageTime;
	float		m_flTargetFindTime;

	EHANDLE					m_hPullObject;
	IPhysicsConstraint* m_pPullConstraint;


public:
	int			FlashlightIsOn(void);
	void		FlashlightTurnOn(void);
	void		FlashlightTurnOff(void);
	float		m_flFlashLightTime;
	CNetworkVar(int, m_nFlashBattery);

	CNetworkVar(float, m_flStartCharge);
	CNetworkVar(float, m_flAmmoStartCharge);
	CNetworkVar(float, m_flPlayAftershock);
	CNetworkVar(float, m_flNextAmmoBurn);

	CNetworkVar(bool, m_bHasLongJump);
	CNetworkVar(bool, m_bIsPullingObject);
};

inline CHL1_Player* ToHL1Player(CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return static_cast<CHL1_Player*>(pEntity);
}

#endif // HL1_PLAYER_H