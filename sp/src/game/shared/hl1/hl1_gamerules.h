#ifndef HL1_GAMERULES_H
#define HL1_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "gamerules.h"
#include "singleplay_gamerules.h"

#ifdef CLIENT_DLL
	#define CHalfLife1 C_HalfLife1
#endif

class CHalfLife1 : public CSingleplayRules
{
public:
	virtual bool AllowThirdPersonCamera( void ) { return true; }

	DECLARE_CLASS( CHalfLife1, CSingleplayRules );

	virtual int		Damage_GetShowOnHud( void );

	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );


#ifndef CLIENT_DLL
	
	CHalfLife1();
	virtual ~CHalfLife1() {}

	virtual bool			ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void			PlayerSpawn( CBasePlayer *pPlayer );

	virtual void			InitDefaultAIRelationships( void );
	virtual const char *	AIClassText(int classType);
	virtual const char *	GetGameDescription( void );
	virtual float			FlPlayerFallDamage( CBasePlayer *pPlayer );

	virtual void			PlayerThink( CBasePlayer *pPlayer );
	bool					CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );

	void					RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore );

	int						DefaultFOV( void ) { return 90; }

#endif

};

#endif
