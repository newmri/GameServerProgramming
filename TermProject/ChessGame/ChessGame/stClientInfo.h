#pragma once
#include "Define.h"

#pragma pack(push, 1)
struct stClientInfo
{
	bool m_IsConnected;
	WORD m_wId;
	Point m_pos;
	Point m_DrawPos;
	CImage m_ciChess;
	stClientInfo()
	{
		m_IsConnected = false;
		m_wId = 65535;
		m_pos.m_wX = CHESS_FIRST_X, m_pos.m_wY = CHESS_FIRST_Y;
		m_pos.m_wZone = 1;
		m_DrawPos.m_wX = CHESS_FIRST_X, m_DrawPos.m_wY = CHESS_FIRST_Y;
		m_ciChess.Load("Pawn.png");
	}
};

struct stNPCInfo
{
	WORD m_wId;
	Point m_pos;
	Point m_DrawPos;
	CImage m_ciChess;
	bool m_IsAlive;

	stNPCInfo()
	{
		m_wId = 65535;
		m_pos.m_wX = CHESS_FIRST_X, m_pos.m_wY = CHESS_FIRST_Y;
		m_pos.m_wZone = 1;
		m_DrawPos.m_wX = CHESS_FIRST_X, m_DrawPos.m_wY = CHESS_FIRST_Y;
		m_ciChess.Load("Night.png");
		m_IsAlive = false;
	}
};
#pragma pack(pop)
