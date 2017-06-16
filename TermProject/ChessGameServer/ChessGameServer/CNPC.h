#pragma once

#include "CLuaScript.h"

enum TYPE { PEACE = 0, WAR };
enum MOVE { FIX = 0, NOTFIX };

struct Point2
{
	WORD m_wX, m_wY;
	WORD m_wZone;
};
class CNPC
{
public:
	void Init(const WORD);

public:
	void WakeUp();
public:
	void Move();

public:
	void SetPasive() { IsActivated = false; }
public:
	const Point2& GetPos();

private:
	Point2 m_pos;
	WORD m_Level;
	bool m_Type;
	bool m_Move;
	WORD m_Exe;
	bool IsActivated;
	WORD m_Id;
	LuaScript m_Lua;
};