#pragma once

#include <Windows.h>
#include <mutex>

#include<chrono>
using namespace std;
using namespace chrono;

enum Event_Type { E_MOVE };

struct Timer_Event
{
	int object_id;
	high_resolution_clock::time_point exec_time;
	Event_Type event;
};

class comparison
{
	bool reverse;
public:
	bool operator()(const Timer_Event lhs, const Timer_Event rhs) const
	{
		return (lhs.exec_time > rhs.exec_time);
	}
};

class CNPC
{
public:
	void Init();

public:
	void Move();

public:
	const POINT& GetPos();
public:
	const bool& GetIsActivated() { return m_IsActivated; }
	void SetActive() { m_IsActivated = true; }
	void SetPassive() { m_IsActivated = false; }
public:
	std::mutex lock;

private:
	POINT m_pos;
	bool m_IsActivated;
};