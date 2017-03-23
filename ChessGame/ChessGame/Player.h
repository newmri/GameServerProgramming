#pragma once

#include "Transmission.h"

#define MOVE_PIXEL 85

enum { UP = 0, DOWN, LEFT, RIGHT };

enum { TOP_END = 20, BOTTOM_END = 615, LEFT_END = 17, RIGHT_END = 612 };

enum Location { LOBBY = 10, GAMEROOM };


class Player : public Transmission
{
private:
	Location m_location;
	POINT m_pos;
	unsigned short m_ridder;
public:
	CImage m_chess;
	Player();
	bool IsPlayerGameRoom();
	void SetPlayerLocation(const Location&);
	void SetMove(unsigned short);
	POINT GetPos();
	void MoveChess();
};