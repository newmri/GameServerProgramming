#pragma once

#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_BUF_SIZE 1024
#define MAX_MSG_SIZE 30
#define MAX_CLIENT_NUM 50
#define MAX_WORKER_THREAD 5

// MOVE
#define CHESS_FIRST_X 357
#define CHESS_FIRST_Y 360

enum enumOperation { eOP_RECV, eOP_SEND };
// Distiction of data type
enum { eMOVE = 1 };

enum enumLocation { eLOBBY, eGAME_ROOM, eLOGOUT };
