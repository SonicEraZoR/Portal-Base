//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A volume which allows portals to be placed. Keeps a global list loaded in from the map
//			and provides an interface with which prop_portal can get this list and successfully
//			create portals wholly or partially inside the volume.
//
// $NoKeywords: $
//======================================================================================//

#include "cbase.h"
#include "func_allowportal_volume.h"
#include "prop_portal_shared.h"
#include "portal_shareddefs.h"
#include "portal_util_shared.h"
#include "collisionutils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_START_INACTIVE			0x01

CEntityClassList<CFuncAllowPortalVolume> g_FuncAllowPortalVolumeList;
template <> CFuncAllowPortalVolume *CEntityClassList<CFuncAllowPortalVolume>::m_pClassList = NULL;

CFuncAllowPortalVolume* GetAllowPortalVolumeList()
{
	return g_FuncAllowPortalVolumeList.m_pClassList;
}


LINK_ENTITY_TO_CLASS(func_allowportal_volume, CFuncAllowPortalVolume);

BEGIN_DATADESC(CFuncAllowPortalVolume)

DEFINE_FIELD(m_bActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_iListIndex, FIELD_INTEGER),
// No need to save this, its rebuilt on construct
//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Deactivate", InputDeactivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

DEFINE_FUNCTION(GetIndex),
DEFINE_FUNCTION(IsActive),

END_DATADESC()


CFuncAllowPortalVolume::CFuncAllowPortalVolume()
{
	m_bActive = true;

	// Add me to the global list
	g_FuncAllowPortalVolumeList.Insert(this);
}

CFuncAllowPortalVolume::~CFuncAllowPortalVolume()
{
	g_FuncAllowPortalVolumeList.Remove(this);
}


void CFuncAllowPortalVolume::Spawn()
{
	BaseClass::Spawn();

	if (m_spawnflags & SF_START_INACTIVE)
	{
		m_bActive = false;
	}
	else
	{
		m_bActive = true;
	}

	// Bind to our model, cause we need the extents for bounds checking
	SetModel(STRING(GetModelName()));
	SetRenderMode(kRenderNone);	// Don't draw
	SetSolid(SOLID_VPHYSICS);	// we may want slanted walls, so we'll use OBB
	AddSolidFlags(FSOLID_NOT_SOLID);
}

void CFuncAllowPortalVolume::OnActivate(void)
{
	if (!GetCollideable())
		return;
}

void CFuncAllowPortalVolume::InputActivate(inputdata_t &inputdata)
{
	m_bActive = true;

	OnActivate();
}

void CFuncAllowPortalVolume::InputDeactivate(inputdata_t &inputdata)
{
	m_bActive = false;
}

void CFuncAllowPortalVolume::InputToggle(inputdata_t &inputdata)
{
	m_bActive = !m_bActive;

	if (m_bActive)
	{
		OnActivate();
	}
}
