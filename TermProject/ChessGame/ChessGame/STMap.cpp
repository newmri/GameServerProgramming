#include "STMap.h"

STMap::STMap()
{
	m_GrayBlock.Load("Gray.bmp");
	m_BlackBlock.Load("Black.bmp");
	m_RedBlock.Load("Red.png");
}

void STMap::DrawMap(const HDC& a_hdc, const bool& a_Swap, const Point& a_pos)
{
	for (int y = 0; y < MAX_MAP_TILE; ++y) {
		for (int x = 0; x < MAX_MAP_TILE; ++x) {
			if (!a_Swap) {
				if (y & 1 == 1) {
					if (x & 1 == 1) {
						m_GrayBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(),
							0, 0, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_GrayBlock.GetWidth();
					}
					else {
						m_BlackBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(),
							0, 0, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_BlackBlock.GetWidth();
					}
				}
				else {
					if (x & 1 == 1) {
						m_BlackBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(),
							0, 0, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_BlackBlock.GetWidth();
					}
					else {
						m_GrayBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(),
							0, 0, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_GrayBlock.GetWidth();
					}

				}
			}

			else {

				if (y & 1 == 1) {
					if (x & 1 == 1) {
						m_BlackBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(),
							0, 0, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_BlackBlock.GetWidth();
					}
					else {
						m_GrayBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(),
							0, 0, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_GrayBlock.GetWidth();
					}
				}
				else {
					if (x & 1 == 1) {
						m_GrayBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(),
							0, 0, m_GrayBlock.GetWidth(), m_GrayBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_GrayBlock.GetWidth();
					}
					else {
						m_BlackBlock.TransparentBlt(a_hdc, m_Point.x, m_Point.y, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(),
							0, 0, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(), RGB(255, 255, 255));
						m_Point.x += m_BlackBlock.GetWidth();
					}

				}



			}
		}
		m_Point.x = 0;
		m_Point.y += m_BlackBlock.GetWidth();
	}
	ZeroMemory(&m_Point, sizeof(m_Point));

	for (int i=0; i<20; ++i){
		
		if ((m_MapInfo[i].m_wY - a_pos.m_wY) < 20 && (m_MapInfo[i].m_wX - a_pos.m_wX) < 20) {
			printf("%d \n", m_MapInfo[i].m_wX);
				m_RedBlock.TransparentBlt(a_hdc, m_MapInfo[i].m_wX * m_RedBlock.GetWidth(), m_MapInfo[i].m_wY * m_RedBlock.GetHeight(), m_RedBlock.GetWidth(), m_RedBlock.GetHeight(),
					0, 0, m_BlackBlock.GetWidth(), m_BlackBlock.GetHeight(), RGB(255, 255, 255));
				break;
			
		}

	}

}