#pragma once

#include <WinSock2.h>
#include <iostream>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>
#include <set>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace chrono;

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

#define MAX_VIEW 20

// Map
#define MAX_MAP_X 300
#define MAX_MAP_Y 300
#define MAX_MAP_TILE 20 
#define MAX_MAP_X_ZONE (MAX_MAP_X / MAX_MAP_TILE)
#define MAX_MAP_Y_ZONE (MAX_MAP_Y / MAX_MAP_TILE)

// NPC
#define MAX_NPC_NUM 30000

#define NPC_MOVE_SEC 1000

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y - 1, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X - 1};

enum enumOperation { eOP_RECV, eOP_SEND, eMOVE };

// From Client To Server
enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT, eSC_PUT_NPC, eSC_MOVE_NPC, eSC_REMOVE_NPC,
	eSC_MAP_NOTIFY
};



struct STTimerInfo
{
	WORD wId;
	enumOperation eOperation;
	LONGLONG lTime;
};

struct Point
{
	WORD m_wX, m_wY;
	WORD m_wZone;
};

#pragma pack (push, 1)

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
#pragma pack (pop)