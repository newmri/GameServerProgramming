#pragma once
#include "Define.h"

#pragma pack(push, 1)
struct stClientInfo
{
	bool m_IsConnected;
	WORD m_wId;
	char m_ID[ID_LEN];
	POINT m_pos;
	POINT m_DrawPos;
	stClientInfo()
	{
		m_IsConnected = false;
		m_wId = 65535;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;
		m_DrawPos.x = CHESS_FIRST_X, m_DrawPos.y = CHESS_FIRST_Y;
	}
};

struct stNPCInfo
{
	WORD m_wId;
	POINT m_pos;
	POINT m_DrawPos;
	bool m_IsAlive;

	stNPCInfo()
	{
		m_wId = 65535;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;
		m_DrawPos.x = CHESS_FIRST_X, m_DrawPos.y = CHESS_FIRST_Y;
		m_IsAlive = false;
	}
};
#pragma pack(pop)
