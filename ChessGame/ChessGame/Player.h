#pragma once

#include "Common.h"

#define FIRST_X 357
#define FIRST_Y 360
#define MOVE_PIXEL 85
enum { UP = 0, DOWN, LEFT, RIGHT };

enum { TOP_END = 20, BOTTOM_END = 615, LEFT_END = 17, RIGHT_END = 612 };




//struct Player
//{
//	POINT m_pos;
//	unsigned short m_ridder;
//	CImage m_chess;
//	Player();
//	void MoveChess();
//};

class Player
{
private:
	POINT m_pos;
	unsigned short m_ridder;
public:
	CImage m_chess;
	Player();
	void SetMove(unsigned short);
	POINT GetPos();
	void MoveChess();
};