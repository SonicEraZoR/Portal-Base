//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the base world item. Most of this was moved from items.cpp.
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "engine/IEngineSound.h"
#include "iservervehicle.h"
#include "physics_saverestore.h"
#include "world.h"

#ifdef HL2MP
#include "hl2mp_gamerules.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24

extern ConVar sv_portalbase_item_touch_area;

class CWorldItem : public CBaseAnimating
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWorldItem, CBaseAnimating );

	bool	KeyValue( const char *szKeyName, const char *szValue ); 
	void	Spawn( void );

	int		m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

BEGIN_DATADESC( CWorldItem )

DEFINE_FIELD( m_iType, FIELD_INTEGER ),

END_DATADESC()


bool CWorldItem::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "type"))
	{
		m_iType = atoi(szValue);
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}

void CWorldItem::Spawn( void )
{
	CBaseEntity *pEntity = NULL;

	switch (m_iType) 
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create( "item_battery", GetLocalOrigin(), GetLocalAngles() );
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create( "item_suit", GetLocalOrigin(), GetLocalAngles() );
		break;
	}

	if (!pEntity)
	{
		Warning("unable to create world_item %d\n", m_iType );
	}
	else
	{
		pEntity->m_target = m_target;
		pEntity->SetName( GetEntityName() );
		pEntity->ClearSpawnFlags();
		pEntity->AddSpawnFlags( m_spawnflags );
	}

	UTIL_RemoveImmediate( this );
}


BEGIN_DATADESC( CItem )

	DEFINE_FIELD( m_bActivateWhenAtRest,	 FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vOriginalSpawnOrigin, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vOriginalSpawnAngles, FIELD_VECTOR ),
	DEFINE_PHYSPTR( m_pConstraint ),
	DEFINE_FIELD(m_hTouchArea, FIELD_EHANDLE),

	// Function Pointers
	DEFINE_ENTITYFUNC( ItemTouch ),
	DEFINE_THINKFUNC( Materialize ),
	DEFINE_THINKFUNC( ComeToRest ),

#if defined( HL2MP ) || defined( TF_DLL )
	DEFINE_FIELD( m_flNextResetCheckTime, FIELD_TIME ),
	DEFINE_THINKFUNC( FallThink ),
#endif

	// Outputs
	DEFINE_OUTPUT( m_OnPlayerTouch, "OnPlayerTouch" ),
	DEFINE_OUTPUT( m_OnCacheInteraction, "OnCacheInteraction" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
CItem::CItem()
{
	m_bActivateWhenAtRest = false;
}

void CItem::UpdateOnRemove()
{
	if (m_hTouchArea.Get())
		UTIL_Remove(m_hTouchArea.Get());

	BaseClass::UpdateOnRemove();
}

bool CItem::CreateItemVPhysicsObject( void )
{
	// Create the object in the physics system
	int nSolidFlags = GetSolidFlags() | FSOLID_NOT_STANDABLE;
	if (!m_bActivateWhenAtRest)
	{
		if (!sv_portalbase_item_touch_area.GetBool())
		{
			nSolidFlags |= FSOLID_TRIGGER;
		}
		else if (m_hTouchArea.Get())
		{
			static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->EnableTouchArea();
		}
	}

	IPhysicsObject* pPhys = VPhysicsInitNormal(SOLID_VPHYSICS, nSolidFlags, false);

	if (pPhys == NULL)
	{
		SetSolid(SOLID_BBOX);
		AddSolidFlags(nSolidFlags);

		// If it's not physical, drop it to the floor
		if (UTIL_DropToFloor(this, MASK_SOLID) == 0)
		{
			Warning( "Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
			UTIL_Remove( this );
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem::Spawn( void )
{
	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}

	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolid(SOLID_BBOX);
	SetBlocksLOS(false);
	AddEFlags(EFL_NO_ROTORWASH_PUSH);
	
	if( IsX360() )
	{
		AddEffects( EF_ITEM_BLINK );
	}

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	//mygamepedia: we are using env_touch_area for items so the items are not triggers and pass portals, but also have
	//the large player pickup area
	if (!sv_portalbase_item_touch_area.GetBool())
	{
		CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);
	}
	else if (!IsMarkedForDeletion()) //don't if should be removed
	{
		CBaseEntity* pEntity = CBaseEntity::Create("env_touch_area", GetLocalOrigin(), GetLocalAngles());
		if (pEntity)
		{
			CEnvTouchArea* pArea = static_cast<CEnvTouchArea*>(pEntity);

			pArea->SetModel(GetModelName().ToCStr());
			pArea->CreateItemVPhysicsObject();
			pArea->CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);
			pArea->FollowEntity(this, false);
			pArea->SetPickupItem(this);
			pArea->SetPickupType(PUT_Item);
			pArea->SetTouch(&CEnvTouchArea::ItemTouch);
			pArea->EnableTouchArea();
			m_hTouchArea = pArea;
		}
	}

	SetTouch(&CItem::ItemTouch);

	if ( CreateItemVPhysicsObject() == false )
		return;

	m_takedamage = DAMAGE_EVENTS_ONLY;

#if !defined( CLIENT_DLL )
	// Constrained start?
	if ( HasSpawnFlags( SF_ITEM_START_CONSTRAINED ) )
	{
		//Constrain the weapon in place
		IPhysicsObject *pReferenceObject, *pAttachedObject;

		pReferenceObject = g_PhysWorldObject;
		pAttachedObject = VPhysicsGetObject();

		if ( pReferenceObject && pAttachedObject )
		{
			constraint_fixedparams_t fixed;
			fixed.Defaults();
			fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );

			fixed.constraint.forceLimit	= lbs2kg( 10000 );
			fixed.constraint.torqueLimit = lbs2kg( 10000 );

			m_pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

			m_pConstraint->SetGameData( (void *) this );
		}
	}
#endif //CLIENT_DLL

#if defined( HL2MP ) || defined( TF_DLL )
	SetThink( &CItem::FallThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
#endif
}

unsigned int CItem::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_PLAYERCLIP;
}

void CItem::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer )
	{
		pPlayer->PickupObject( this );
	}
}

extern int gEvilImpulse101;


//-----------------------------------------------------------------------------
// Activate when at rest, but don't allow pickup until then
//-----------------------------------------------------------------------------
void CItem::ActivateWhenAtRest( float flTime /* = 0.5f */ )
{
	if (!sv_portalbase_item_touch_area.GetBool())
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else if (m_hTouchArea.Get())
	{
		static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->DisableTouchArea();
	}
	m_bActivateWhenAtRest = true;
	SetThink( &CItem::ComeToRest );
	SetNextThink( gpGlobals->curtime + flTime );
}


//-----------------------------------------------------------------------------
// Become touchable when we are at rest
//-----------------------------------------------------------------------------
void CItem::OnEntityEvent( EntityEvent_t event, void *pEventData )
{
	BaseClass::OnEntityEvent( event, pEventData );

	switch( event )
	{
	case ENTITY_EVENT_WATER_TOUCH:
		{
			// Delay rest for a sec, to avoid changing collision 
			// properties inside a collision callback.
			SetThink( &CItem::ComeToRest );
			SetNextThink( gpGlobals->curtime + 0.1f );
		}
		break;
	}
}


//-----------------------------------------------------------------------------
// Become touchable when we are at rest
//-----------------------------------------------------------------------------
void CItem::ComeToRest( void )
{
	if ( m_bActivateWhenAtRest )
	{
		m_bActivateWhenAtRest = false;
		if (!sv_portalbase_item_touch_area.GetBool())
		{
			AddSolidFlags(FSOLID_TRIGGER);
		}
		else if (m_hTouchArea.Get())
		{
			static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->EnableTouchArea();
		}
		SetThink( NULL );
	}
}

#if defined( HL2MP ) || defined( TF_DLL )

//-----------------------------------------------------------------------------
// Purpose: Items that have just spawned run this think to catch them when 
//			they hit the ground. Once we're sure that the object is grounded, 
//			we change its solid type to trigger and set it in a large box that 
//			helps the player get it.
//-----------------------------------------------------------------------------
void CItem::FallThink ( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );

#if defined( HL2MP )
	bool shouldMaterialize = false;
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( pPhysics )
	{
		shouldMaterialize = pPhysics->IsAsleep();
	}
	else
	{
		shouldMaterialize = (GetFlags() & FL_ONGROUND) ? true : false;
	}

	if ( shouldMaterialize )
	{
		SetThink ( NULL );

		m_vOriginalSpawnOrigin = GetAbsOrigin();
		m_vOriginalSpawnAngles = GetAbsAngles();

		HL2MPRules()->AddLevelDesignerPlacedObject( this );
	}
#endif // HL2MP

#if defined( TF_DLL )
	// We only come here if ActivateWhenAtRest() is never called,
	// which is the case when creating currencypacks in MvM
	if ( !( GetFlags() & FL_ONGROUND ) )
	{
		if ( !GetAbsVelocity().Length() && GetMoveType() == MOVETYPE_FLYGRAVITY )
		{
			// Mr. Game, meet Mr. Hammer.  Mr. Hammer, meet the uncooperative Mr. Physics.
			// Mr. Physics really doesn't want to give our friend the FL_ONGROUND flag.
			// This means our wonderfully helpful radius currency collection code will be sad.
			// So in the name of justice, we ask that this flag be delivered unto him.

			SetMoveType( MOVETYPE_NONE );
			SetGroundEntity( GetWorldEntity() );
		}
	}
	else
	{
		SetThink( &CItem::ComeToRest );
	}
#endif // TF
}

#endif // HL2MP, TF

//-----------------------------------------------------------------------------
// Purpose: Used to tell whether an item may be picked up by the player.  This
//			accounts for solid obstructions being in the way.
// Input  : *pItem - item in question
//			*pPlayer - player attempting the pickup
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool UTIL_ItemCanBeTouchedByPlayer( CBaseEntity *pItem, CBasePlayer *pPlayer )
{
	if (pItem == NULL || pPlayer == NULL || !pPlayer->IsPlayer())
		return false;

	// For now, always allow a vehicle riding player to pick up things they're driving over
	if ( pPlayer->IsInAVehicle() )
		return true;

	// Get our test positions
	Vector vecStartPos;
	IPhysicsObject *pPhysObj = pItem->VPhysicsGetObject();
	if ( pPhysObj != NULL )
	{
		// Use the physics hull's center
		QAngle vecAngles;
		pPhysObj->GetPosition( &vecStartPos, &vecAngles );
	}
	else
	{
		// Use the generic bbox center
		vecStartPos = pItem->CollisionProp()->WorldSpaceCenter();
	}

	Vector vecEndPos = pPlayer->EyePosition();

	// FIXME: This is the simple first try solution towards the problem.  We need to take edges and shape more into account
	//		  for this to be fully robust.

	// Trace between to see if we're occluded
	trace_t tr;
	CTraceFilterSkipTwoEntities filter( pPlayer, pItem, COLLISION_GROUP_PLAYER_MOVEMENT );
	UTIL_TraceLine( vecStartPos, vecEndPos, MASK_SOLID, &filter, &tr );

	// Occluded
	// FIXME: For now, we exclude starting in solid because there are cases where this doesn't matter
	if ( tr.fraction < 1.0f )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not the item can be touched and picked up by the player, taking
//			into account obstructions and other hinderances
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CItem::ItemCanBeTouchedByPlayer( CBasePlayer *pPlayer )
{
	return UTIL_ItemCanBeTouchedByPlayer( this, pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pOther - 
//-----------------------------------------------------------------------------
void CItem::ItemTouch( CBaseEntity *pOther )
{
	//mygamepedia: don't run touch checks that we run in touch area, unless disabled
	if (!sv_portalbase_item_touch_area.GetBool())
	{
		if (!pOther)
			return;

		// Vehicles can touch items + pick them up
		if (pOther->GetServerVehicle())
		{
			pOther = pOther->GetServerVehicle()->GetPassenger();
			if (!pOther)
				return;
		}

		// if it's not a player, ignore
		if (!pOther->IsPlayer())
			return;
	}

	CBasePlayer *pPlayer = static_cast<CBasePlayer*>(pOther);

	// Must be a valid pickup scenario (no blocking). Though this is a more expensive
	// check than some that follow, this has to be first Obecause it's the only one
	// that inhibits firing the output OnCacheInteraction.
	if ( ItemCanBeTouchedByPlayer( pPlayer ) == false )
		return;

	m_OnCacheInteraction.FireOutput(pOther, this);

	// Can I even pick stuff up?
	if ( !pPlayer->IsAllowedToPickupWeapons() )
		return;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if ( MyTouch( pPlayer ) )
	{
		m_OnPlayerTouch.FireOutput(pOther, this);

		SetTouch( NULL );
		SetThink( NULL );

		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );

#ifdef HL2MP
			HL2MPRules()->RemoveLevelDesignerPlacedObject( this );
#endif
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	AddEffects( EF_NODRAW );

	VPhysicsDestroyObject();

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_NONE );

	if (!sv_portalbase_item_touch_area.GetBool())
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}
	else if (m_hTouchArea.Get())
	{
		static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->EnableTouchArea();
	}

	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.
	SetAbsAngles( g_pGameRules->VecItemRespawnAngles( this ) );// set the angles.

#if !defined( TF_DLL )
	UTIL_DropToFloor( this, MASK_SOLID );
#endif

	RemoveAllDecals(); //remove any decals

	SetThink ( &CItem::Materialize );
	SetNextThink( gpGlobals->curtime + g_pGameRules->FlItemRespawnTime( this ) );
	return this;
}

void CItem::Materialize( void )
{
	CreateItemVPhysicsObject();

	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.

#ifdef HL2MP
		EmitSound( "AlyxEmp.Charge" );
#else
		EmitSound( "Item.Materialize" );
#endif
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	SetTouch( &CItem::ItemTouch );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Item.Materialize" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysGunUser - 
//			PICKED_UP_BY_CANNON - 
//-----------------------------------------------------------------------------
void CItem::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	m_OnCacheInteraction.FireOutput(pPhysGunUser, this);

	if ( reason == PICKED_UP_BY_CANNON )
	{
		// Expand the pickup box
		if (!sv_portalbase_item_touch_area.GetBool())
		{
			CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT * 2);
		}
		else if (m_hTouchArea.Get())
		{
			static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT * 2);
		}

		if( m_pConstraint != NULL )
		{
			physenv->DestroyConstraint( m_pConstraint );
			m_pConstraint = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysGunUser - 
//			reason - 
//-----------------------------------------------------------------------------
void CItem::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	// Restore the pickup box to the original
	if (!sv_portalbase_item_touch_area.GetBool())
	{
		CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);
	}
	else if (m_hTouchArea.Get())
	{
		static_cast<CEnvTouchArea*>(m_hTouchArea.Get())->CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);
	}
}


//-----------------------------------------------------------------------------
// Env Touch Area - MyGamepedia
//-----------------------------------------------------------------------------


LINK_ENTITY_TO_CLASS(env_touch_area, CEnvTouchArea);

BEGIN_DATADESC(CEnvTouchArea)

DEFINE_FIELD(m_hPickupItem, FIELD_EHANDLE),
DEFINE_FIELD(m_iPickupType, FIELD_SHORT),
DEFINE_FIELD(m_bIsEnabled, FIELD_BOOLEAN),

// Function Pointers
DEFINE_ENTITYFUNC(ItemTouch),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

END_DATADESC()


//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
CEnvTouchArea::CEnvTouchArea()
{
	m_iPickupType = 0;
	m_bIsEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn. It doesn't contain full code for proper spawn, this entity should always be created with item.
//-----------------------------------------------------------------------------
void CEnvTouchArea::Spawn()
{
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	SetBlocksLOS(false);
	AddEFlags(EFL_NO_ROTORWASH_PUSH);
	AddEffects(EF_NODRAW);
	AddSolidFlags(FSOLID_TRIGGER);
	m_takedamage = DAMAGE_NO;
}

//-----------------------------------------------------------------------------
// Purpose: Enabled touch area by input. Can pick up item now.
//-----------------------------------------------------------------------------
void CEnvTouchArea::InputEnable(inputdata_t& inputdata)
{
	m_bIsEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: Disable touch area by input. Can't pick up item now.
//-----------------------------------------------------------------------------
void CEnvTouchArea::InputDisable(inputdata_t& inputdata)
{
	m_bIsEnabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: Toggle touch area state by input. Disable if enabled or enable if disabled.
//-----------------------------------------------------------------------------
void CEnvTouchArea::InputToggle(inputdata_t& inputdata)
{
	(m_bIsEnabled) ? m_bIsEnabled = false : m_bIsEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: A safe way to set item that will this touch area reference to. Should be a CItem and checks nullprt.
//-----------------------------------------------------------------------------
void CEnvTouchArea::SetPickupItem(CItem* pItem)
{
	if (pItem)
		m_hPickupItem = pItem;
}

//-----------------------------------------------------------------------------
// Purpose: A safe way to set weapon that will this touch area reference to. Should be a CBaseCombatWeapon and checks nullprt.
//-----------------------------------------------------------------------------
void CEnvTouchArea::SetPickupWeapon(CBaseCombatWeapon* pWeapon)
{
	if (pWeapon)
		m_hPickupItem = pWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Set pickup type that this touch area belongs to.
//-----------------------------------------------------------------------------
void CEnvTouchArea::SetPickupType(PickUpType_t PUT_Type)
{
	if (PUT_Type == PUT_Nothing)
	{
		m_iPickupType = 0;
	}
	else if (PUT_Type == PUT_Item)
	{
		m_iPickupType = 1;
	}
	else if (PUT_Type == PUT_Weapon)
	{
		m_iPickupType = 2;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Use it when you want to remove an item red for what ever reason (sets nullprt).
//-----------------------------------------------------------------------------
void CEnvTouchArea::ClearPickupObject()
{
	m_hPickupItem = NULL;
	SetPickupType(PUT_Nothing);
}

//-----------------------------------------------------------------------------
// Purpose: Touch item if I have a valid player.
//-----------------------------------------------------------------------------
void CEnvTouchArea::ItemTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	//nothing
	if (m_iPickupType == PUT_Nothing)
		return;

	//item
	if (m_iPickupType == PUT_Item && m_hPickupItem.Get() && m_hPickupItem.Get()->IsItem())
	{
		//not enabled? return
		if (m_bIsEnabled == false)
			return;

		// Vehicles can touch items + pick them up
		if (pOther->GetServerVehicle())
		{
			pOther = pOther->GetServerVehicle()->GetPassenger();
			if (!pOther)
				return;
		}

		// if it's not a player, ignore
		if (!pOther->IsPlayer())
			return;

		//call touch for my item
		static_cast<CItem*>(m_hPickupItem.Get())->ItemTouch(pOther);

		return;
	}

	//weapon
	if (m_iPickupType == PUT_Weapon && m_hPickupItem.Get() && m_hPickupItem.Get()->IsBaseCombatWeapon())
	{
		CBaseCombatWeapon* pWeapon = static_cast<CBaseCombatWeapon*>(m_hPickupItem.Get());

		// Can't pick up dissolving weapons
		if (pWeapon->IsDissolving())
			return;

		//call touch for my wpn
		pWeapon->DefaultTouch(pOther);

		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns solid mask.
//-----------------------------------------------------------------------------
unsigned int CEnvTouchArea::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_PLAYERCLIP;
}

//-----------------------------------------------------------------------------
// Purpose: Creates physics to make it work.
//-----------------------------------------------------------------------------
bool CEnvTouchArea::CreateItemVPhysicsObject(void)
{
	// Create the object in the physics system
	int nSolidFlags = GetSolidFlags() | FSOLID_NOT_SOLID;

	IPhysicsObject* pPhys = VPhysicsInitNormal(SOLID_VPHYSICS, nSolidFlags, false);

	if (pPhys == NULL)
	{
		SetSolid(SOLID_NONE);
		AddSolidFlags(nSolidFlags);
	}
	else
	{
		AddSolidFlags(nSolidFlags);
		VPhysicsGetObject()->EnableMotion(false);
	}

	return true;
}

