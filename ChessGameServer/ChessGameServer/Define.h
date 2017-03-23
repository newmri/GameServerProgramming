#pragma once

#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "user32.lib")

using namespace std;

#define MAX_BUF_SIZE 1024
#define MAX_MSG_SIZE 30
#define MAX_CLIENT_NUM 50
#define MAX_WORKER_THREAD 5

enum enumOperation { OP_RECV, OP_SEND };
// Distiction of data type
enum { MOVE = 1 };
