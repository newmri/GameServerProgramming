#include "CNPC.h"

// Map
#define MAX_MAP_X 400
#define MAX_MAP_Y 400

enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X };

void CNPC::Init()
{
	m_pos.x = 30;
	m_pos.y = 30;
}

void CNPC::Move()
{
	switch (rand() % 4) {
	case eCS_UP: if(m_pos.y > eTOP_END) m_pos.y -= 1; break;
	case eCS_DOWN: if(m_pos.y < eBOTTOM_END) m_pos.y += 1; break;
	case eCS_LEFT: if(m_pos.x > eLEFT_END) m_pos.x -= 1; break;
	case eCS_RIGHT: if(m_pos.x < eRIGHT_END) m_pos.x += 1; break;
	}
}

const POINT& CNPC::GetPos() { return m_pos; }