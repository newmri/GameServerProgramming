#include "CNPC.h"
#include "Timer.h"

// Map
#define MAX_MAP_X 400
#define MAX_MAP_Y 400

enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X };

void CNPC::Init(const WORD a_Id, const Point2 pos)
{
	m_Id = a_Id;
	IsActivated = false;
	LuaScript script("Night.lua");
	m_Lua = script;
	m_Lua.lua_set("set_Init");
	m_Lua.lua_set("set_pos", pos.m_wX, pos.m_wY, pos.m_wZone);
	if (m_Id < MAX_NORMAL_FIXED_NIGHT) { m_name = "NormalFixedNight"; m_Move = false; }
	else if (m_Id >= MAX_NORMAL_FIXED_NIGHT && m_Id < MAX_NORMAL_FIXED_NIGHT + MAX_NORMAL_MOVE_NIGHT) {
		m_name = "NormalMovingNight";
		m_Move = true;
	}
	m_Level = m_Lua.get<WORD>(m_name + ".Level");
	m_Dmg = m_Lua.get<WORD>(m_name + ".Dmg");
	m_MAX_HP = m_Lua.get<WORD>(m_name + ".MAX_HP");
	m_HP = m_Lua.get<WORD>(m_name + ".HP");
	m_Exe = m_Lua.get<WORD>(m_name + ".Exe");

	m_pos.m_wX = m_Lua.lua_getpos("get_posx");
	m_pos.m_wY = m_Lua.lua_getpos("get_posy");
	m_pos.m_wZone = m_Lua.lua_getpos("get_zone");
	cout << m_name<<" "<< m_pos.m_wZone << endl;
	
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
	if (m_Move) {
		switch (rand() % 4) {
		case eCS_UP: if (m_pos.m_wY > eTOP_END) m_pos.m_wY -= 1; break;
		case eCS_DOWN: if (m_pos.m_wY < eBOTTOM_END) m_pos.m_wY += 1; break;
		case eCS_LEFT: if (m_pos.m_wX > eLEFT_END) m_pos.m_wX -= 1; break;
		case eCS_RIGHT: if (m_pos.m_wX < eRIGHT_END) m_pos.m_wX += 1; break;
		}
	}
}

const Point2& CNPC::GetPos() { return m_pos; }