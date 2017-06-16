#pragma once
#include <iostream>
#include <chrono>
#include <queue>
#include <mutex>

using namespace std;
using namespace chrono;

#define TIMER TimerQueue::GetInstance()
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

struct TimerQueue
{
	static TimerQueue* GetInstance()
	{
		if (m_instance == nullptr)
			m_instance = new TimerQueue;

		return m_instance;
	}

	static TimerQueue* m_instance;
	mutex tq_lock;
	priority_queue<Timer_Event, vector<Timer_Event>, comparison > timer_queue;
};

