#include "Player.h"

Player::Player()
{

	m_pos.x = FIRST_X;
	m_pos.y = FIRST_Y;
	m_ridder = 0;
	m_chess.Load("Pawn.png");
	
}

void Player::SetMove(unsigned short ridder) { m_ridder = ridder; }

POINT Player::GetPos() { return m_pos; }

void Player::MoveChess()
{


	switch (m_ridder) {
	case UP:
		if (m_pos.y == TOP_END) break;
		m_pos.y -= MOVE_PIXEL;
		break;
	case DOWN:
		if (m_pos.y == BOTTOM_END) break;
		m_pos.y+= MOVE_PIXEL;
		break;
	case LEFT:
		if (m_pos.x == LEFT_END) break;
		m_pos.x -= MOVE_PIXEL;
		break;
	case RIGHT:
		if (m_pos.x == RIGHT_END) break;
		m_pos.x += MOVE_PIXEL;
		break;
	default:
		break;

	}

}