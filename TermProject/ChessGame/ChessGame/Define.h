#pragma once

#include <Windows.h>
#include <atlimage.h>
#include <atlstr.h>
#include <vector>
#include <iostream>
#include "resource.h"
#include "stDBInfo.h"
#include <string>
using namespace std;

#define FIRST_X 357
#define FIRST_Y 360

#define CHESS_FIRST_X 0
#define CHESS_FIRST_Y 0

#define MAX_IP_LEN 16
#define MAX_BUF_SIZE 1024
#define MAX_PACKET_SIZE 255

#define MOVE_PIXEL 35

#define MAX_PLAYER 1000

#define MAX_MAP_TILE 20

#define MAX_MAP_X 400
#define MAX_MAP_Y 400

#define MAX_NPC_NUM 5

#define MAX_STR_SIZE 100

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2);

// From Client To Server
enum EMOVE { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
enum { eCS_LOGIN = 50, eCS_SIGNUP };

// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT, eSC_PUT_NPC, eSC_MOVE_NPC, eSC_REMOVE_NPC,eSC_MAP_NOTIFY};
enum LOGIN { eSC_LOGIN_FAIL_INCORRECT = 50, eSC_LOGIN_FAIL_LOGINED, eSC_LOGIN_SUCCESS };
enum SIGNUP { eSC_SIGNUP_FAIL = 60, eSC_SIGNUP_SUCCESS };

enum { eID_IP_EDIT = 101 };
enum { eIDC_CONNECT = 1001 };

// CHAT
enum CHAT { eCS_CHAT = 90, eSC_CHAT};

#pragma pack (push, 1)


struct ST_CS_MOVE
{
	BYTE m_bytSize;
	BYTE m_bytType;
	char m_ID[ID_LEN];
};

struct ST_CS_CHAT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	BYTE m_bytMessageLen;
	char m_Message[MAX_STR_SIZE];
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

struct ST_SC_LOGIN_FAIL
{
	BYTE m_bytSize;
	BYTE m_bytType;
};

struct ST_SC_LOGIN_SUCCESS
{
	BYTE m_bytSize;
	BYTE m_bytType;
	stDBInfo m_DBInfo;
};

struct ST_SC_SIGNUP_RESULT
{
	BYTE m_bytSize;
	BYTE m_bytType;
};

struct ST_SC_PUT_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	Point m_pos;
};

struct ST_SC_MOVE_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
	Point m_pos;
};

struct ST_SC_REMOVE_OBJECT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	WORD m_wId;
};


struct ST_SC_NOTIFY_MAP
{
	BYTE m_bytSize;
	BYTE m_bytType;
	Point m_pos;
};

struct ST_SC_CHAT
{
	BYTE m_bytSize;
	BYTE m_bytType;
	char m_Message[MAX_STR_SIZE];
};
#pragma pack (pop)