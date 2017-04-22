#pragma once

#include <Windows.h>
#include <atlimage.h>
#include <atlstr.h>

#define FIRST_X 357
#define FIRST_Y 360

#define CHESS_FIRST_X 0
#define CHESS_FIRST_Y 0

#define MAX_IP_LEN 16
#define MAX_BUF_SIZE 1024
#define MAX_PACKET_SIZE 255

#define MOVE_PIXEL 70

#define MAX_PLAYER 10

#define MAX_MAP_TILE 11

#define MAX_MAP_X 100
#define MAX_MAP_Y 100

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2);

// From Client To Server
enum EMOVE { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT };

enum { eID_IP_EDIT = 101 };
enum { eIDC_CONNECT = 1001 };


#pragma pack (push, 1)

struct ST_CS_MOVE
{
	BYTE m_bytSize;
	BYTE m_bytType;
};

struct ST_SC_PUT_CLIENT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	BYTE m_bytX, m_bytY;
};

struct ST_SC_MOVE_CLIENT {
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	BYTE m_bytX, m_bytY;
};

struct ST_SC_REMOVE_CLIENT {
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
};

#pragma pack (pop)