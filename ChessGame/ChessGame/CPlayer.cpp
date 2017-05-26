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
	// Inputed key variable
	m_usRidder = 0;

}





void CPlayer::SetMove(const WORD& ridder) { m_usRidder = ridder; }

void CPlayer::SetPos()
{
	switch (m_usRidder) {
	case eCS_UP: {
		m_nPacketType = eCS_UP;
		SendMovePacket();
		break;
	}
	case eCS_DOWN: {
		m_nPacketType = eCS_DOWN;
		SendMovePacket();
		break;
	}
	case eCS_LEFT: {
		m_nPacketType = eCS_LEFT;
		SendMovePacket();
		break;
	}
	case eCS_RIGHT: {
		m_nPacketType = eCS_RIGHT;
		SendMovePacket();
		break;
	}
	default:
		break;
	}
}

const int& CPlayer::GetPlayerNum(){ return m_nClientCnt; }

const bool& CPlayer::IsMoved() { return m_IsMoved; }


void CPlayer::Login(char a_ID[], char a_PWD[])
{
	m_nPacketType = eCS_LOGIN;
	SendLoginPacket(a_ID, a_PWD);
}