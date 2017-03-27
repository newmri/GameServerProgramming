#pragma once

#include "CTransmission.h"


class CPlayer : public CTransmission
{
public:
	CImage m_chess;
	bool IsPlayerGameRoom();
	void SetPlayerLocation(const ELocation&);
	void SetMove(unsigned short);
	void SetPos();

// To contrcut only one CPlayer
public:
	static CPlayer* Instance();
	static void DestroyInstance();

private:
	CPlayer();

private:
	static CPlayer* m_pInstance;

private:
	ELocation m_eLocation;
	unsigned short m_usRidder;
};