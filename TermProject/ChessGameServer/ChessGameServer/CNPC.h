#pragma once

#include <Windows.h>
#include <mutex>
class CNPC
{
public:
	void Init();

public:
	void Move();

public:
	const POINT& GetPos();

public:
	std::mutex lock;

private:
	POINT m_pos;

};