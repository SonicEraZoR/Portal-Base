#include "cbase.h"
#include "ivmodemanager.h"
#include "clientmode_hlnormal.h"
#include "hl1_clientmode.h"

ConVar default_fov("default_fov", "75", FCVAR_CHEAT);

ConVar fov_desired("fov_desired", "75", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.", true, 75.0, true, 90.0);

IClientMode* g_pClientMode = NULL;

class CHLModeManager : public IVModeManager
{
public:
	CHLModeManager(void);
	virtual		~CHLModeManager(void);

	virtual void	Init(void);
	virtual void	SwitchMode(bool commander, bool force);
	virtual void	OverrideView(CViewSetup* pSetup);
	virtual void	CreateMove(float flInputSampleTime, CUserCmd* cmd);
	virtual void	LevelInit(const char* newmap);
	virtual void	LevelShutdown(void);
};

CHLModeManager::CHLModeManager(void)
{
}

CHLModeManager::~CHLModeManager(void)
{
}

void CHLModeManager::Init(void)
{
	g_pClientMode = GetClientModeNormal();
}

void CHLModeManager::SwitchMode(bool commander, bool force)
{
}

void CHLModeManager::OverrideView(CViewSetup* pSetup)
{
}

void CHLModeManager::CreateMove(float flInputSampleTime, CUserCmd* cmd)
{
}

void CHLModeManager::LevelInit(const char* newmap)
{
	g_pClientMode->LevelInit(newmap);
}

void CHLModeManager::LevelShutdown(void)
{
	g_pClientMode->LevelShutdown();
}

static CHLModeManager g_HLModeManager;
IVModeManager* modemanager = &g_HLModeManager;

class CHudViewport : public CBaseViewport
{
private:
	DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		gHUD.InitColors(pScheme);

		SetPaintBackgroundEnabled(false);
	}

	virtual void CreateDefaultPanels(void)
	{
		CBaseViewport::CreateDefaultPanels();
	}
};

ClientModeHL1Normal::ClientModeHL1Normal()
{
}

ClientModeHL1Normal::~ClientModeHL1Normal()
{
}

void ClientModeHL1Normal::InitViewport()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start(gameuifuncs, gameeventmanager);
}

float ClientModeHL1Normal::GetViewModelFOV(void)
{
	return 90.0f;
}


int	ClientModeHL1Normal::GetDeathMessageStartHeight(void)
{
	return m_pViewport->GetDeathMessageStartHeight();
}


ClientModeHL1Normal g_ClientModeNormal;

IClientMode* GetClientModeNormal()
{
	return &g_ClientModeNormal;
}

ClientModeHL1Normal* GetClientModeHL1Normal()
{
	Assert(dynamic_cast<ClientModeHL1Normal*>(GetClientModeNormal()));

	return static_cast<ClientModeHL1Normal*>(GetClientModeNormal());
}