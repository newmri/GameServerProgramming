#pragma once

#include "stDBInfo.h"

#pragma pack(push, 1)


struct stClientInfo
{
	bool m_IsConnected;
	WORD m_wId;
	stDBInfo m_Info;
	Point m_DrawPos;
	stClientInfo()
	{
		m_IsConnected = false;
		m_wId = 65535;
		m_DrawPos.m_wX = 0, m_DrawPos.m_wY = 0;

	}
};

struct stNPCInfo
{
	WORD m_wId;
	Point m_pos;
	Point m_DrawPos;
	bool m_IsAlive;

	stNPCInfo()
	{
		m_wId = 65535;
		m_pos.m_wX = 0, m_pos.m_wY = 0;
		m_pos.m_wZone = 1;
		m_DrawPos.m_wX = 0, m_DrawPos.m_wY = 0;
		m_IsAlive = false;
	}
};
#pragma pack(pop)
