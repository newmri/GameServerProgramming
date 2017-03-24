#pragma once

#include "CTransmission.h"

#define MOVE_PIXEL 85

enum { eUP = 0, eDOWN, eLEFT, eRIGHT };

enum { eTOP_END = 20, eBOTTOM_END = 615, eLEFT_END = 17, eRIGHT_END = 612 };

enum ELocation { eLOBBY = 10, eGAME_ROOM };


class CPlayer : public CTransmission
{
public:
	CImage m_chess;
	CPlayer();
	bool IsPlayerGameRoom();
	void SetPlayerLocation(const ELocation&);
	void SetMove(unsigned short);
	POINT GetPos();
	void MoveChess();

private:
	ELocation m_eLocation;
	POINT m_pos;
	unsigned short m_usRidder;
};