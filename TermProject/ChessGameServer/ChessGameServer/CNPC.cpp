#include "CNPC.h"
#include "Timer.h"

// Map
#define MAX_MAP_X 400
#define MAX_MAP_Y 400

enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X };

void CNPC::Init(const WORD a_Id)
{
	m_Id = a_Id;
	m_pos.m_wX = rand() % MAX_MAP_X;
	m_pos.m_wY = rand() % MAX_MAP_Y;
	m_pos.m_wZone = 2;
	IsActivated = false;

	LuaScript script("Night.lua");
	m_Lua = script;

	m_Level = m_Lua.get<WORD>("Night.Level");
	m_Type = m_Lua.get<bool>("Night.Type");
	m_Exe = m_Lua.get<WORD>("Night.Exe");
	m_pos.m_wX = m_Lua.get<WORD>("Night.pos.X");
	m_pos.m_wY = m_Lua.get<WORD>("Night.pos.Y");
	m_pos.m_wZone = m_Lua.get<WORD>("Night.pos.Zone");

}

void CNPC::WakeUp()
{
	if (IsActivated) return;
	IsActivated = true;
	Timer_Event event{ m_Id, high_resolution_clock::now() + 1s, E_MOVE };
	TIMER->tq_lock.lock(); TIMER->timer_queue.push(event); TIMER->tq_lock.unlock();
}

void CNPC::Move()
{
	switch (rand() % 4) {
	case eCS_UP: if(m_pos.m_wY > eTOP_END) m_pos.m_wY -= 1; break;
	case eCS_DOWN: if(m_pos.m_wY < eBOTTOM_END) m_pos.m_wY += 1; break;
	case eCS_LEFT: if(m_pos.m_wX > eLEFT_END) m_pos.m_wX -= 1; break;
	case eCS_RIGHT: if(m_pos.m_wX < eRIGHT_END) m_pos.m_wX += 1; break;
	}
}

const Point2& CNPC::GetPos() { return m_pos; }