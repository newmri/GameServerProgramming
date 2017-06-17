#pragma once

#include "CLuaScript.h"

enum TYPE { PEACE = 0, WAR };
enum MOVE { FIX = 0, NOTFIX };

#define MAX_NORMAL_FIXED_NIGHT 300
#define MAX_NORMAL_MOVE_NIGHT 150

struct Point2
{
	WORD m_wX, m_wY;
	WORD m_wZone;
};
class CNPC
{
public:
	void Init(const WORD, const Point2);

public:
	void WakeUp();
public:
	void Move();

public:
	void SetPasive() { IsActivated = false; }
public:
	const Point2& GetPos();

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
	LuaScript m_Lua;
};