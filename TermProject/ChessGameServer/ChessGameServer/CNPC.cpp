#include "CNPC.h"
#include "Timer.h"

// Map
#define MAX_MAP_X 400
#define MAX_MAP_Y 400

enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X };

void CNPC::Init(const WORD a_Id, const Point2 pos, const WORD a_obx, const WORD a_oby)
{
	m_Id = a_Id;
	m_obx = a_obx;
	m_oby = a_oby;
	IsActivated = false;
	std::string str;
	if (a_Id < MAX_NIGHT) { str = "Night.lua"; }
	else { str = "Bishop.lua"; }

	LuaScript script(str); m_Lua = script;

	m_Lua.lua_set("set_Init");
	m_Lua.lua_set("set_pos", pos.m_wX, pos.m_wY, pos.m_wZone);
	if (m_Id < MAX_NORMAL_FIXED_NIGHT) { m_name = "NormalFixedNight"; m_Move = false; }
	else if (m_Id >= MAX_NORMAL_FIXED_NIGHT && m_Id < MAX_NORMAL_FIXED_NIGHT + MAX_NORMAL_MOVING_NIGHT) {
		m_name = "NormalMovingNight";
		m_Move = true;
	}
	else if (m_Id >= MAX_NORMAL_NIGHT && m_Id < MAX_NORMAL_NIGHT + MAX_STARVED_FIXED_NIGHT) {
		m_name = "StarvedFixedNight"; m_Move = false;
	}
	else if (m_Id >= MAX_NORMAL_NIGHT + MAX_STARVED_FIXED_NIGHT && m_Id < MAX_NIGHT) {
		m_name = "StarvedMovingNight"; m_Move = true;
	}
	else if (m_Id >= MAX_NIGHT && m_Id < MAX_NIGHT + MAX_NORMAL_FIXED_BISHOP){
		m_name = "NormalFixedBishop"; m_Move = false;
	}
	else if (m_Id >= MAX_NIGHT + MAX_NORMAL_FIXED_BISHOP && m_Id < MAX_NORMAL_BISHOP) {
		m_name = "NormalMovingBishop"; m_Move = true;
	}
	else if (m_Id >= MAX_NORMAL_BISHOP && m_Id < MAX_NORMAL_BISHOP + MAX_STARVED_FIXED_BISHOP) {
		m_name = "StarvedFixedBishop"; m_Move = false;
	}

	else if (m_Id >= MAX_NORMAL_BISHOP + MAX_STARVED_FIXED_BISHOP && m_Id < MAX_BISHOP){
		m_name = "StarvedMovingBishop"; m_Move = true;
	}
	else return;
	m_Level = m_Lua.get<WORD>(m_name + ".Level");
	m_Dmg = m_Lua.get<WORD>(m_name + ".Dmg");
	m_MAX_HP = m_Lua.get<WORD>(m_name + ".MAX_HP");
	m_HP = m_Lua.get<WORD>(m_name + ".HP");
	m_Exe = m_Lua.get<WORD>(m_name + ".Exe");

	m_pos.m_wX = m_Lua.lua_getpos("get_posx");
	m_pos.m_wY = m_Lua.lua_getpos("get_posy");
	m_pos.m_wZone = m_Lua.lua_getpos("get_zone");
	m_xmin = m_Lua.lua_getpos("get_xmin");
	m_xmax = m_Lua.lua_getpos("get_xmax");
	m_ymin = m_Lua.lua_getpos("get_ymin");
	m_ymax = m_Lua.lua_getpos("get_ymax");
	//cout << m_name<<" "<< m_pos.m_wZone << endl;
	
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
		case eCS_UP: if (m_pos.m_wY > m_ymin) {
			if (m_pos.m_wX != m_obx && m_pos.m_wY - 1 != m_oby)
				m_pos.m_wY -= 1; break;
		}
		case eCS_DOWN: if (m_pos.m_wY < m_ymax) {
			if (m_pos.m_wX != m_obx && m_pos.m_wY + 1 != m_oby)
				m_pos.m_wY += 1; break;
		}
		case eCS_LEFT: if (m_pos.m_wX > m_xmin) {
			if (m_pos.m_wY != m_oby && m_pos.m_wX - 1 != m_obx)
				m_pos.m_wX -= 1; break;
		}
		case eCS_RIGHT: if (m_pos.m_wX < m_xmax) {
			if (m_pos.m_wY != m_oby && m_pos.m_wX + 1 != m_obx)
				m_pos.m_wX += 1; break;
		}
		}
	}
}


void CNPC::SetPasive()
{

	IsActivated = false;
}
const Point2& CNPC::GetPos() { return m_pos; }