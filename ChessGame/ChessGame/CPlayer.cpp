#include "CPlayer.h"

CPlayer* CPlayer::m_pInstance = NULL;

CPlayer* CPlayer::Instance()
{
	if (!m_pInstance) m_pInstance = new CPlayer;
	return m_pInstance;
}

void CPlayer::DestroyInstance()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

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

void CPlayer::SetPos()
{
	switch (m_usRidder) {
	case eUP: {
		m_pos.y -= MOVE_PIXEL;
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(eDataType);
		break;
	}
	case eDOWN: {
		m_pos.y += MOVE_PIXEL;
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(eDataType);
		break;
	}
	case eLEFT: {
		m_pos.x -= MOVE_PIXEL;
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(eDataType);
		break;
	}
	case eRIGHT: {
		m_pos.x += MOVE_PIXEL;
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(eDataType);
		break;
	}
	default:
		break;
	}
}