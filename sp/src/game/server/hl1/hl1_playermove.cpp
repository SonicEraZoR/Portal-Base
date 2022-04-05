#include "cbase.h"
#include "player_command.h"
#include "igamemovement.h"
#include "in_buttons.h"
#include "ipredictionsystem.h"
#include "hl1_player.h"


static CMoveData g_MoveData;
CMoveData* g_pMoveData = &g_MoveData;

IPredictionSystem* IPredictionSystem::g_pPredictionSystems = NULL;

class CHL1PlayerMove : public CPlayerMove
{
	DECLARE_CLASS(CHL1PlayerMove, CPlayerMove);

public:
	virtual void	StartCommand(CBasePlayer* player, CUserCmd* cmd);
	virtual void	SetupMove(CBasePlayer* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move);
	virtual void	FinishMove(CBasePlayer* player, CUserCmd* ucmd, CMoveData* move);
};

static CHL1PlayerMove g_PlayerMove;

CPlayerMove* PlayerMove()
{
	return &g_PlayerMove;
}

void CHL1PlayerMove::StartCommand(CBasePlayer* player, CUserCmd* cmd)
{
	BaseClass::StartCommand(player, cmd);
}

void CHL1PlayerMove::SetupMove(CBasePlayer* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move)
{
	BaseClass::SetupMove(player, ucmd, pHelper, move);
}

void CHL1PlayerMove::FinishMove(CBasePlayer* player, CUserCmd* ucmd, CMoveData* move)
{
	BaseClass::FinishMove(player, ucmd, move);
}