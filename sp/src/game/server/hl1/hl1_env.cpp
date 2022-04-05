#include "cbase.h"
#include "soundscape.h"

#define SF_RENDER_MASKFX	0x00000001
#define SF_RENDER_MASKAMT	0x00000002
#define SF_RENDER_MASKMODE	0x00000004
#define SF_RENDER_MASKCOLOR	0x00000008

class CRenderFxManager : public CPointEntity
{
	DECLARE_CLASS(CRenderFxManager, CPointEntity);
public:
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void InputActivate(inputdata_t &inputdata);
	int render;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);

BEGIN_DATADESC(CRenderFxManager)
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
END_DATADESC()

void CRenderFxManager::Spawn(void)
{
	AddSolidFlags(FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_NONE);
	AddEffects(EF_NODRAW);
	render = m_nRenderFX;
}

void CRenderFxManager::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_target != NULL_STRING)
	{
		CBaseEntity *pEntity = NULL;
		while ((pEntity = gEntList.FindEntityByName(pEntity, STRING(m_target))) != NULL)
		{
			if (!HasSpawnFlags(SF_RENDER_MASKFX))
				pEntity->m_nRenderFX = render;
			if (!HasSpawnFlags(SF_RENDER_MASKAMT))
				pEntity->SetRenderColorA(GetRenderColor().a);
			if (!HasSpawnFlags(SF_RENDER_MASKMODE))
				pEntity->m_nRenderMode = m_nRenderMode;
			if (!HasSpawnFlags(SF_RENDER_MASKCOLOR))
				pEntity->m_clrRender = m_clrRender;
		}
	}
}

void CRenderFxManager::InputActivate(inputdata_t &inputdata)
{
	Use(inputdata.pActivator, inputdata.pCaller, USE_ON, 0);
}

LINK_ENTITY_TO_CLASS(env_sound, CEnvSoundscape);
PRECACHE_REGISTER(env_sound);