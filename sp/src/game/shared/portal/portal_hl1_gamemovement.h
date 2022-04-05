#ifndef C_HL1_GAMEMOVEMENT_H
#define C_HL1_GAMEMOVEMENT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#include "c_hl1_player.h"
#else
	#include "hl1_player.h"
#endif

#if defined( CLIENT_DLL )
	class CHL1_Player;
	#define CHL1_Player C_HL1_Player
#endif

#define PLAYER_LONGJUMP_SPEED 350

class CHL1GameMovement : public CGameMovement
{
public:
	DECLARE_CLASS( CHL1GameMovement, CGameMovement );

	virtual bool CheckJumpButton( void );
	virtual void HandleDuckingSpeedCrop();
	virtual void Duck(void);
	virtual void FinishUnDuck(void);
	virtual void FinishDuck(void);
	virtual bool CanUnduck();
	virtual void CheckParameters( void );

protected:
	CHL1_Player *m_pHL1Player;
};

#endif