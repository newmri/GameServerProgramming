#pragma once

#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_BUF_SIZE 1024
#define MAX_MSG_SIZE 30
#define MAX_CLIENT_NUM 11 // Server can accommodate MAX_CLIENT_NUM - 1
#define MAX_WORKER_THREAD 6

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2)

// First pos
#define CHESS_FIRST_X 17
#define CHESS_FIRST_Y 20

#define MOVE_PIXEL 85

// Check the boundary
enum { eTOP_END = -65, eBOTTOM_END = 700, eLEFT_END = -68, eRIGHT_END = 697 };

enum enumOperation { eOP_RECV, eOP_SEND };

// Distiction of data type
enum enumDataType{ eMOVE = 1, eCLIENT_INFO, eANOTHER_MOVE, eNOTIFY_LOGOUT};

enum enumLocation { eLOBBY, eGAME_ROOM, eLOGOUT };
