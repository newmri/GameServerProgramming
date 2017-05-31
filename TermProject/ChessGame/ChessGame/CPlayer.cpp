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
	auto curr_time = std::chrono::high_resolution_clock::now();
	if (m_last_move_time < curr_time - 0.1s) m_last_move_time = curr_time;
	else return;

	switch (m_usRidder) {
	case eCS_UP: {
		m_nPacketType = eCS_UP;
		SendPacket();
		break;
	}
	case eCS_DOWN: {
		m_nPacketType = eCS_DOWN;
		SendPacket();
		break;
	}
	case eCS_LEFT: {
		m_nPacketType = eCS_LEFT;
		SendPacket();
		break;
	}
	case eCS_RIGHT: {
		m_nPacketType = eCS_RIGHT;
		SendPacket();
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

void CPlayer::SignUp(char a_ID[], char a_PWD[])
{
	m_nPacketType = eCS_SIGNUP;
	SendSignUpPacket(a_ID, a_PWD);
}


void CPlayer::SendChatting(char a_Message[])
{
	m_nPacketType = eCS_CHAT;
	SendChatPacket(a_Message);

}