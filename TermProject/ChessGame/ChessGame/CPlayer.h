#pragma once

#include "CTransmission.h"
#include <chrono>

class CPlayer : public CTransmission
{
public:
	CImage m_chess;
	void SetMove(const WORD&);
	void SetPos();
	const int& GetPlayerNum();
	const bool& IsMoved();

public:
	void Login(char a_ID[], char a_PWD[]);
	void SignUp(char a_ID[], char a_PWD[]);
// To contrcut only one CPlayer
public:
	static CPlayer* Instance();
	static void DestroyInstance();

private:
	std::chrono::high_resolution_clock::time_point m_last_move_time;

private:
	CPlayer();

private:
	static CPlayer* m_pInstance;

private:
	WORD m_usRidder;
};