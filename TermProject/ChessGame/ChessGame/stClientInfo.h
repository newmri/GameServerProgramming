#pragma once
#include "Define.h"

#pragma pack(push, 1)
struct stClientInfo
{
	bool m_IsConnected;
	WORD m_wId;
	POINT m_pos;
	POINT m_DrawPos;
	CImage m_ciChess;
	stClientInfo()
	{
		m_IsConnected = false;
		m_wId = 65535;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;
		m_DrawPos.x = CHESS_FIRST_X, m_DrawPos.y = CHESS_FIRST_Y;
		m_ciChess.Load("Pawn.png");
	}
};

struct stNPCInfo
{
	WORD m_wId;
	POINT m_pos;
	POINT m_DrawPos;
	CImage m_ciChess;
	bool m_IsAlive;

	stNPCInfo()
	{
		m_wId = 65535;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;
		m_DrawPos.x = CHESS_FIRST_X, m_DrawPos.y = CHESS_FIRST_Y;
		m_ciChess.Load("Night.png");
		m_IsAlive = false;
	}
};
#pragma pack(pop)
