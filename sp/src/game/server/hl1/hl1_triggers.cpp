#include "cbase.h"
#include "globalstate.h"

#define SF_MULTIMANTHREAD	0x00000001

class CMultiManager : public CPointEntity
{
public:
	bool KeyValue(const char *szKeyName, const char *szValue);
	void Spawn(void);
	void ManagerThink(void);
	void ManagerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void ManagerReport(void);
#endif

	bool HasTarget(string_t targetname);

	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_DATADESC();

	int			m_cTargets;
	int			m_index;
	float		m_startTime;
	EHANDLE		m_hActivator;
	string_t	m_iTargetName	[MAX_MULTI_TARGETS];
	float		m_flTargetDelay [MAX_MULTI_TARGETS];

	COutputEvent m_OnTrigger;

	void InputManagerTrigger(inputdata_t &data);
};

LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);
PRECACHE_REGISTER(multi_manager);

BEGIN_DATADESC(CMultiManager)
DEFINE_FIELD(m_cTargets, FIELD_INTEGER),
DEFINE_FIELD(m_index, FIELD_INTEGER),
DEFINE_FIELD(m_startTime, FIELD_TIME),
DEFINE_ARRAY(m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
DEFINE_ARRAY( m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),

DEFINE_FUNCTION(ManagerThink),
DEFINE_FUNCTION(ManagerUse),
#if _DEBUG
DEFINE_FUNCTION(ManagerReport),
#endif

DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
DEFINE_INPUTFUNC(FIELD_VOID, "Trigger", InputManagerTrigger),

END_DATADESC()

void CMultiManager::InputManagerTrigger(inputdata_t &data)
{
	ManagerUse(NULL, NULL, USE_TOGGLE, 0);
}

bool CMultiManager::KeyValue(const char *szKeyName, const char *szValue)
{
	if (BaseClass::KeyValue(szKeyName, szValue))
	{
		return true;
	}
	else
	{
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char x[128];

			UTIL_StripToken(szKeyName, x);
			m_iTargetName[m_cTargets] = AllocPooledString(x);
			m_flTargetDelay[m_cTargets] = atof(szValue);
			m_cTargets++;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CMultiManager::Spawn(void)
{
	SetSolid(SOLID_NONE);
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	int swapped = 1;

	while (swapped)
	{
		swapped = 0;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				string_t name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = 1;
			}
		}
	}
}

bool CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return TRUE;

	return FALSE;
}

void CMultiManager::ManagerThink(void)
{
	float	m_time;

	m_time = gpGlobals->curtime - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= m_time)
	{
		FireTargets(STRING(m_iTargetName[m_index]), m_hActivator, this, USE_TOGGLE, 0);
		m_index++;
	}

	if (m_index >= m_cTargets)
	{
		SetThink(NULL);
		SetUse(&CMultiManager::ManagerUse);
	}
	else
		SetNextThink(m_startTime + m_flTargetDelay[m_index]);
}

void CMultiManager::ManagerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnTrigger.FireOutput(pActivator, this);
	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->curtime;

	SetUse(NULL);

	SetThink(&CMultiManager::ManagerThink);
	SetNextThink(gpGlobals->curtime);
}

#if _DEBUG
void CMultiManager::ManagerReport(void)
{
	int	cIndex;

	for (cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		Msg("%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex]);
	}
}
#endif

#define SF_AUTO_FIRE_ONCE 0X0001

class CTriggerAuto : public CBaseEntity
{
	DECLARE_CLASS(CTriggerAuto, CBaseEntity);
public:
	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);
	void Think(void);

	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	string_t m_globalstate;

	COutputEvent m_OnTrigger;
};

LINK_ENTITY_TO_CLASS(trigger_auto, CTriggerAuto);
PRECACHE_REGISTER(trigger_auto);

BEGIN_DATADESC(CTriggerAuto)
DEFINE_KEYFIELD(m_globalstate, FIELD_STRING, "globalstate"),
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
END_DATADESC();

void CTriggerAuto::Spawn(void)
{
	Precache();
}

void CTriggerAuto::Precache(void)
{
	SetNextThink(gpGlobals->curtime + 0.1);
}

void CTriggerAuto::Think(void)
{
	if (!m_globalstate || GlobalEntity_GetState(m_globalstate) == GLOBAL_ON)
	{
		m_OnTrigger.FireOutput(NULL, this);
		if (m_spawnflags & SF_AUTO_FIRE_ONCE)
			UTIL_Remove(this);
	}
}

#define SF_RELAY_FIRE_ONCE 0x0001

class CTriggerRelay : public CBaseEntity
{
	DECLARE_CLASS(CTriggerRelay, CBaseEntity);
public:
	DECLARE_DATADESC();

	bool KeyValue(const char* szKeyName, const char* szValue);
	void Spawn(void) {}
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	
private:
	USE_TYPE triggerType;

	COutputEvent m_OnTrigger;
};

LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);
PRECACHE_REGISTER(trigger_relay);

BEGIN_DATADESC(CTriggerRelay)
DEFINE_FIELD(triggerType, FIELD_INTEGER),
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),
END_DATADESC();

bool CTriggerRelay::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "triggerstate"))
	{
		int type = atoi(szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
	}
	else
		BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

void CTriggerRelay::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnTrigger.FireOutput(pActivator, this);
	if (m_spawnflags & SF_RELAY_FIRE_ONCE)
		UTIL_Remove(this);
}

#define SF_ENDSECTION_USEONLY		0x0001

class CTriggerEndSection : public CBaseEntity
{
public:
	DECLARE_CLASS(CTriggerEndSection, CBaseEntity);
	DECLARE_DATADESC();
	void Spawn(void);
	void InputEndSection(inputdata_t &data);
};

LINK_ENTITY_TO_CLASS(trigger_endsection, CTriggerEndSection);

BEGIN_DATADESC(CTriggerEndSection)
DEFINE_INPUTFUNC(FIELD_VOID, "EndSection", InputEndSection),
END_DATADESC()

void CTriggerEndSection::Spawn(void)
{
	if (gpGlobals->deathmatch)
	{
		UTIL_Remove(this);
		return;
	}
}

void CTriggerEndSection::InputEndSection(inputdata_t &data)
{
	CBaseEntity *pPlayer = UTIL_GetLocalPlayer();

	if (pPlayer)
	{
		engine->ClientCommand(pPlayer->edict(), "toggleconsole;disconnect\n");
	}

	UTIL_Remove(this);
}
