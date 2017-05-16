#pragma once

#include <Windows.h>
#include <mutex>

struct Point2
{
	WORD m_wX, m_wY;
	WORD m_wZone;
};
class CNPC
{
public:
	void Init();

public:
	void Move();

public:
	const Point2& GetPos();

public:
	std::mutex lock;

private:
	Point2 m_pos;

};