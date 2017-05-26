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

#define MOVE_PIXEL 35

#define MAX_PLAYER 10

#define MAX_MAP_TILE 20

#define MAX_MAP_X 400
#define MAX_MAP_Y 400

#define MAX_NPC_NUM 1000

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2);


#define ID_LEN 10  
#define PWD_LEN 10  

// From Client To Server
enum EMOVE { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
enum { eCS_LOGIN = 50};
// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT, eSC_PUT_NPC, eSC_MOVE_NPC, eSC_REMOVE_NPC};
enum { eID_IP_EDIT = 101 };
enum { eIDC_CONNECT = 1001 };


#pragma pack (push, 1)

struct ST_CS_MOVE
{
	BYTE m_bytSize;
	BYTE m_bytType;
};

struct ST_CS_LOGIN
{
	BYTE m_bytSize;
	BYTE m_bytType;
	BYTE m_bytIDLen;
	BYTE m_bytPWDLen;
	char m_ID[ID_LEN];
	char m_PWD[PWD_LEN];
};

struct ST_SC_PUT_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	WORD m_wX, m_wY;
};

struct ST_SC_MOVE_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	WORD m_wX, m_wY;
};

struct ST_SC_REMOVE_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
};

#pragma pack (pop)