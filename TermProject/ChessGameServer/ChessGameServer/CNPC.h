#pragma once

#include "CLuaScript.h"

enum TYPE { NORMAL = 0, WAR };
enum MOVE { FIX = 0, NOTFIX };

// NPC
#define MAX_NPC_NUM 1500
#define MAX_NORMAL_FIXED_NIGHT 300
#define MAX_NORMAL_MOVING_NIGHT 150
#define MAX_NORMAL_NIGHT (MAX_NORMAL_FIXED_NIGHT + MAX_NORMAL_MOVING_NIGHT)
#define MAX_STARVED_FIXED_NIGHT 150
#define MAX_STARVED_MOVING_NIGHT 150
#define MAX_NIGHT (MAX_NORMAL_NIGHT + MAX_STARVED_FIXED_NIGHT + MAX_STARVED_MOVING_NIGHT)

#define MAX_NORMAL_FIXED_BISHOP  300
#define MAX_NORMAL_MOVING_BISHOP 150
#define MAX_NORMAL_BISHOP (MAX_NIGHT + MAX_NORMAL_FIXED_BISHOP + MAX_NORMAL_MOVING_BISHOP)
#define MAX_STARVED_FIXED_BISHOP 150
#define MAX_STARVED_MOVING_BISHOP 150
#define MAX_BISHOP (MAX_NORMAL_BISHOP + MAX_STARVED_FIXED_BISHOP + MAX_STARVED_MOVING_BISHOP)

struct Point2
{
	WORD m_wX, m_wY;
	WORD m_wZone;
};
class CNPC
{
public:
	void Init(const WORD, const Point2, const WORD, const WORD);

public:
	void WakeUp();
public:
	void Move();

public:
	void SetPasive();
public:
	const Point2& GetPos();
	const bool& GetMove() { return m_Move; }

private:
	std::string m_name;
private:
	Point2 m_pos;
	WORD m_Level;
	WORD m_Dmg;
	WORD m_MAX_HP;
	WORD m_HP;
	WORD m_Exe;
	bool m_Move;
	bool IsActivated;
	WORD m_Id;
	WORD m_xmin, m_xmax;
	WORD m_ymin, m_ymax;
	WORD m_obx, m_oby;
	LuaScript m_Lua;
};