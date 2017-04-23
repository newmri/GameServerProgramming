#pragma once

#include "CTransmission.h"


class CPlayer : public CTransmission
{
public:
	CImage m_chess;
	void SetMove(const WORD&);
	void SetPos();
	const int& GetPlayerNum();
	const bool& IsMoved();
// To contrcut only one CPlayer
public:
	static CPlayer* Instance();
	static void DestroyInstance();

private:
	CPlayer();

private:
	static CPlayer* m_pInstance;

private:
	WORD m_usRidder;
};