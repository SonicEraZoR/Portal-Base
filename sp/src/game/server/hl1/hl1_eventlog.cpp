#include "cbase.h"
#include "../EventLog.h"

class CHL1EventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CHL1EventLog() {};

public:
	bool PrintEvent(IGameEvent* event)
	{
		if (BaseClass::PrintEvent(event))
		{
			return true;
		}

		if (Q_strcmp(event->GetName(), "hl1_") == 0)
		{
			return PrintHL1Event(event);
		}

		return false;
	}

protected:

	bool PrintHL1Event(IGameEvent* event)
	{
		return false;
	}

};

CHL1EventLog g_HL1EventLog;

IGameSystem* GameLogSystem()
{
	return &g_HL1EventLog;
}