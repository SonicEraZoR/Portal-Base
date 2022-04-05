#include "cbase.h"
#include "cplane.h"
#include "hl1_monster_squad.h"

BEGIN_DATADESC(CHL1SquadMonster)
DEFINE_FIELD(m_hSquadLeader, FIELD_EHANDLE),
DEFINE_ARRAY(m_hSquadMember, FIELD_EHANDLE, HL1_MAX_SQUAD_MEMBERS - 1),
DEFINE_FIELD(m_fEnemyEluded, FIELD_BOOLEAN),
DEFINE_FIELD(m_flLastEnemySightTime, FIELD_TIME),
DEFINE_FIELD(m_iMySlot, FIELD_INTEGER),
END_DATADESC();

bool CHL1SquadMonster::OccupySlot(int iDesiredSlots)
{
	int i;
	int iMask;
	int iSquadSlots;

	if (!InSquad())
	{
		return true;
	}

	if (SquadEnemySplit())
	{
		m_iMySlot = bits_SLOT_SQUAD_SPLIT;
		return true;
	}

	CHL1SquadMonster* pSquadLeader = MySquadLeader();

	if (!(iDesiredSlots ^ pSquadLeader->m_afSquadSlots))
	{
		return false;
	}

	iSquadSlots = pSquadLeader->m_afSquadSlots;

	for (i = 0; i < NUM_SLOTS; i++)
	{
		iMask = 1 << i;
		if (iDesiredSlots & iMask)
		{
			if (!(iSquadSlots & iMask))
			{
				pSquadLeader->m_afSquadSlots |= iMask;
				m_iMySlot = iMask;
				return true;
			}
		}
	}

	return false;
}

void CHL1SquadMonster::VacateSlot()
{
	if (m_iMySlot != bits_NO_SLOT && InSquad())
	{
		MySquadLeader()->m_afSquadSlots &= ~m_iMySlot;
		m_iMySlot = bits_NO_SLOT;
	}
}

void CHL1SquadMonster::OnScheduleChange(void)
{
	VacateSlot();
}

void CHL1SquadMonster::Event_Killed(const CTakeDamageInfo& info)
{
	VacateSlot();

	if (InSquad())
	{
		MySquadLeader()->SquadRemove(this);
	}

	CHL1BaseMonster::Event_Killed(info);
}

void CHL1SquadMonster::SquadRemove(CHL1SquadMonster* pRemove)
{
	ASSERT(pRemove != NULL);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_hSquadLeader == this);

	if (pRemove == MySquadLeader())
	{
		for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS - 1; i++)
		{
			CHL1SquadMonster* pMember = MySquadMember(i);
			if (pMember)
			{
				pMember->m_hSquadLeader = NULL;
				m_hSquadMember[i] = NULL;
			}
		}
	}
	else
	{
		CHL1SquadMonster* pSquadLeader = MySquadLeader();
		if (pSquadLeader)
		{
			for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS - 1; i++)
			{
				if (pSquadLeader->m_hSquadMember[i] == this)
				{
					pSquadLeader->m_hSquadMember[i] = NULL;
					break;
				}
			}
		}
	}

	pRemove->m_hSquadLeader = NULL;
}

bool CHL1SquadMonster::SquadAdd(CHL1SquadMonster* pAdd)
{
	ASSERT(pAdd != NULL);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS - 1; i++)
	{
		if (m_hSquadMember[i] == NULL)
		{
			m_hSquadMember[i] = pAdd;
			pAdd->m_hSquadLeader = this;
			return true;
		}
	}
	return false;
}

void CHL1SquadMonster::SquadPasteEnemyInfo(void)
{
	CHL1SquadMonster* pSquadLeader = MySquadLeader();
	if (pSquadLeader)
			pSquadLeader->m_vecEnemyLKP = m_vecEnemyLKP;
		
}

void CHL1SquadMonster::SquadCopyEnemyInfo(void)
{
	CHL1SquadMonster* pSquadLeader = MySquadLeader();
	if (pSquadLeader)
			m_vecEnemyLKP = pSquadLeader->m_vecEnemyLKP;
}

void CHL1SquadMonster::SquadMakeEnemy(CBaseEntity* pEnemy)
{
	if (!InSquad())
		return;

	if (!pEnemy)
	{
		DevMsg("ERROR: SquadMakeEnemy() - pEnemy is NULL!\n");
		return;
	}

	CHL1SquadMonster* pSquadLeader = MySquadLeader();
	for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS; i++)
	{
		CHL1SquadMonster* pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			if (pMember->GetEnemy() != pEnemy && !pMember->HasCondition(COND_SEE_ENEMY))
			{
				if (pMember->GetEnemy() != NULL)
				{
					pMember->UpdateEnemyMemory(pMember->GetEnemy(), pMember->GetEnemyLKP());
				}
				pMember->SetEnemy(pEnemy);
				pMember->SetCondition(COND_NEW_ENEMY);
			}
		}
	}
}

int CHL1SquadMonster::SquadCount(void)
{
	if (!InSquad())
		return 0;

	CHL1SquadMonster* pSquadLeader = MySquadLeader();
	int squadCount = 0;
	for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS; i++)
	{
		if (pSquadLeader->MySquadMember(i) != NULL)
			squadCount++;
	}

	return squadCount;
}

int CHL1SquadMonster::SquadRecruit(int searchRadius, int maxMembers)
{
	int squadCount;
	int iMyClass = Classify();
	
	if (InSquad())
		return 0;

	if (maxMembers < 2)
		return 0;

	m_hSquadLeader = this;
	squadCount = 1;

	CBaseEntity* pEntity = NULL;

	if (m_SquadName != NULL_STRING)
	{
		pEntity = gEntList.FindEntityByClassname(pEntity, GetClassname());
		while (pEntity)
		{
			if (squadCount >= maxMembers)
				break;

			CHL1SquadMonster* pRecruit = (CHL1SquadMonster*)pEntity->MyNPCPointer();

			if (pRecruit)
			{
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass && pRecruit != this && pRecruit->m_SquadName == m_SquadName)
				{
					if (!SquadAdd(pRecruit))
						break;
					squadCount++;
				}
			}

			pEntity = gEntList.FindEntityByClassname(pEntity, GetClassname());
		}
	}
	else
	{
		while ((pEntity = gEntList.FindEntityInSphere(pEntity, GetAbsOrigin(), searchRadius)) != NULL)
		{
			if (squadCount >= maxMembers)
				break;

			CHL1SquadMonster* pRecruit = (CHL1SquadMonster*)pEntity->MyNPCPointer();

			if (pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_hCine)
			{
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass &&
					((iMyClass != CLASS_ALIEN_MONSTER) || FClassnameIs(this, pRecruit->GetClassname())) &&
					!pRecruit->m_SquadName)
				{
					trace_t tr;
					UTIL_TraceLine(GetAbsOrigin() + GetViewOffset(), pRecruit->GetAbsOrigin() + GetViewOffset(), MASK_NPCSOLID_BRUSHONLY, pRecruit, COLLISION_GROUP_NONE, &tr);
					if (tr.fraction == 1.0)
					{
						if (!SquadAdd(pRecruit))
							break;

						squadCount++;
					}
				}
			}
		}
	}

	if (squadCount == 1)
	{
		m_hSquadLeader = NULL;
	}

	return squadCount;
}

void CHL1SquadMonster::GatherEnemyConditions(CBaseEntity* pEnemy)
{
	BaseClass::GatherEnemyConditions(pEnemy);

	if (InSquad() && (CBaseEntity*)GetEnemy() == MySquadLeader()->GetEnemy())
	{
		if (GetEnemyLKP() != m_vecEnemyLKP)
		{
			SquadPasteEnemyInfo();
		}
		else
		{
			SquadCopyEnemyInfo();
		}
	}
}

void CHL1SquadMonster::StartNPC(void)
{
	BaseClass::StartNPC();
	
	if ((CapabilitiesGet() & bits_CAP_SQUAD) && !InSquad())
	{
		if (m_SquadName != NULL_STRING)
		{
			if (!(GetSpawnFlags() & SF_SQUADMONSTER_LEADER))
			{
				return;
			}
		}
		
		SquadRecruit(1024, 4);

		if (IsLeader() && FClassnameIs(this, "monster_human_grunt"))
		{
			SetBodygroup(1, 1);
			m_nSkin = 0;
		}

	}
}

NPC_STATE CHL1SquadMonster::GetIdealState(void)
{
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
	case NPC_STATE_ALERT:
		if (HasCondition(COND_NEW_ENEMY) && InSquad())
		{
			SquadMakeEnemy(GetEnemy());
		}
		break;
	}

	return BaseClass::GetIdealState();
}

bool CHL1SquadMonster::IsValidCover(const Vector& vecCoverLocation, CAI_Hint const* pHint)
{
	if (!InSquad())
	{
		return false;
	}

	if (SquadMemberInRange(vecCoverLocation, 128))
	{
		return false;
	}

	return BaseClass::IsValidCover(vecCoverLocation, pHint);
}

bool CHL1SquadMonster::SquadEnemySplit(void)
{
	if (!InSquad())
		return false;

	CHL1SquadMonster* pSquadLeader = MySquadLeader();
	CBaseEntity* pEnemy = pSquadLeader->GetEnemy();

	for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS; i++)
	{
		CHL1SquadMonster* pMember = pSquadLeader->MySquadMember(i);
		if (pMember != NULL && pMember->GetEnemy() != NULL && pMember->GetEnemy() != pEnemy)
		{
			return true;
		}
	}
	return false;
}

bool CHL1SquadMonster::SquadMemberInRange(const Vector& vecLocation, float flDist)
{
	if (!InSquad())
		return false;

	CHL1SquadMonster* pSquadLeader = MySquadLeader();

	for (int i = 0; i < HL1_MAX_SQUAD_MEMBERS; i++)
	{
		CHL1SquadMonster* pSquadMember = pSquadLeader->MySquadMember(i);
		if (pSquadMember && (vecLocation - pSquadMember->GetAbsOrigin()).Length2D() <= flDist)
			return true;
	}
	return false;
}