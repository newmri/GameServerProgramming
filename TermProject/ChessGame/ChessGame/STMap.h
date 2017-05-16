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
	CImage m_RedBlock;
	STPoint m_Point;
	std::vector<Point> m_MapInfo;
	STMap();
	void DrawMap(const HDC&, const bool&, const Point&);

};
