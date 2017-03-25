#pragma once

#include "CTransmission.h"


class CPlayer : public CTransmission
{
public:
	CImage m_chess;
	CPlayer();
	bool IsPlayerGameRoom();
	void SetPlayerLocation(const ELocation&);
	void SetMove(unsigned short);
	void MoveChess();

private:
	ELocation m_eLocation;
	unsigned short m_usRidder;
};