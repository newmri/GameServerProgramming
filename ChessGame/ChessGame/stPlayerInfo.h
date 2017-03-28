#pragma once
#include "Define.h"

#pragma pack(push, 1)
struct stPlayerInfo
{
	enumLocation m_eLocation;
	unsigned short m_usId;
	POINT m_pos;

	stPlayerInfo()
	{
		m_eLocation = eLOBBY;
		m_usId = 0;
		m_pos.x = 0, m_pos.y = 0;
	}
};
#pragma pack(pop)