#pragma once

#include <WinSock2.h>
#include <iostream>
#include <unordered_set>
#include <mutex>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVER_PORT 9000

#define MAX_PACKET_SIZE 255
#define MAX_BUF_SIZE 1024
#define MAX_MSG_SIZE 30
#define MAX_CLIENT_NUM 11 // Server can accommodate MAX_CLIENT_NUM - 1
#define MAX_WORKER_THREAD 6

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2)

// First pos
#define CHESS_FIRST_X 0
#define CHESS_FIRST_Y 0

#define MAX_VIEW 8

// Map
#define MAX_MAP_X 100
#define MAX_MAP_Y 100

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X};

enum enumOperation { eOP_RECV, eOP_SEND };

// From Client To Server
enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT};

#pragma pack (push, 1)

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