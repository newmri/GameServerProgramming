#pragma once

#include <WinSock2.h>
#include <iostream>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>

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
#define MAX_MAP_X 400
#define MAX_MAP_Y 400

#define MAX_NPC_NUM 1000

#define NPC_MOVE_SEC 1000

// Check the boundary
enum { eTOP_END = 0, eBOTTOM_END = MAX_MAP_Y, eLEFT_END = 0, eRIGHT_END = MAX_MAP_X};

enum enumOperation { eOP_RECV, eOP_SEND, eMOVE };

// From Client To Server
enum { eCS_UP, eCS_DOWN, eCS_LEFT, eCS_RIGHT };
// From Server To Client
enum { eSC_PUT_CLIENT, eSC_MOVE_CLIENT, eSC_REMOVE_CLIENT, eSC_PUT_NPC, eSC_MOVE_NPC, eSC_REMOVE_NPC};



struct STTimerInfo
{
	WORD wId;
	enumOperation eOperation;
	LONGLONG lTime;
};

#pragma pack (push, 1)

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