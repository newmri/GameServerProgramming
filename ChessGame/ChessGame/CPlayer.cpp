#include "CPlayer.h"

CPlayer::CPlayer()
{
	// Location variables
	m_eLocation = eLOBBY;

	// Inputed key variable
	m_usRidder = 0;

	// Character Image
	m_chess.Load("Pawn.png");
}

bool CPlayer::IsPlayerGameRoom()
{
	if (m_eLocation == eGAME_ROOM) return true;
	else return false;
}

void CPlayer::SetPlayerLocation(const ELocation& location){ m_eLocation = location; }

void CPlayer::SetMove(unsigned short ridder) { m_usRidder = ridder; }

void CPlayer::MoveChess()
{
	switch (m_usRidder) {
	case eUP:
		if (m_pos.y == eTOP_END) break;
		m_pos.y -= MOVE_PIXEL;
		break;
	case eDOWN:
		if (m_pos.y == eBOTTOM_END) break;
		m_pos.y+= MOVE_PIXEL;
		break;
	case eLEFT:
		if (m_pos.x == eLEFT_END) break;
		m_pos.x -= MOVE_PIXEL;
		break;
	case eRIGHT:
		if (m_pos.x == eRIGHT_END) break;
		m_pos.x += MOVE_PIXEL;
		break;
	default:
		break;
	}

}