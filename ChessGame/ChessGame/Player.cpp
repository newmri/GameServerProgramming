#include "Player.h"

Player::Player()
{
	// Location variables
	m_location = LOBBY;
	m_pos.x = FIRST_X;
	m_pos.y = FIRST_Y;

	// Inputed key variable
	m_ridder = 0;

	// Character Image
	m_chess.Load("Pawn.png");
}

bool Player::IsPlayerGameRoom()
{
	if (m_location == GAMEROOM) return true;
	else return false;
}

void Player::SetPlayerLocation(const Location& location)
{
	m_location = location;
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