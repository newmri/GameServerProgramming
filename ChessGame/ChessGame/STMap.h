#pragma once

#include "Define.h"


struct STPoint
{
	int x = 0;
	int y = 0;
};

struct STMap
{
	CImage m_GrayBlock;
	CImage m_BlackBlock;
	STPoint m_Point;
	STMap();
	void DrawMap(const HDC&);
};
