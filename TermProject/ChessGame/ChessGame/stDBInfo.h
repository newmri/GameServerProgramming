#pragma once

#include "DBDefin.h"
#include <Windows.h>

#pragma pack (push, 1)
struct Point
{
	WORD m_wX, m_wY;
	WORD m_wZone;
	short m_wXZone, m_wYZone;
	Point()
	{
		m_wX = 65535, m_wY = 0;
		m_wZone = 1;
		m_wXZone = 1, m_wYZone = -1;
	}
};

struct stDBInfo
{
	char ID[ID_LEN];
	WORD m_Level = 0;
	WORD m_MAX_HP = 0;
	WORD m_HP = 0;
	UINT m_MAX_EXP = 0;
	UINT m_EXP = 0;
	WORD m_Damage = 0;
	Point m_pos;
};

#pragma pack (pop)