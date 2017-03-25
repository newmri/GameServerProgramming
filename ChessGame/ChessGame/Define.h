#pragma once

#include <Windows.h>
#include <atlimage.h>

#define FIRST_X 357
#define FIRST_Y 360
#define MAX_IP_LEN 16
#define MAX_BUF_SIZE 1024

#define MOVE_PIXEL 85

enum { eUP = 0, eDOWN, eLEFT, eRIGHT };

enum { eTOP_END = 20, eBOTTOM_END = 615, eLEFT_END = 17, eRIGHT_END = 612 };

enum { eMOVE=1 };

enum ELocation { eLOBBY = 10, eGAME_ROOM };


enum { eID_IP_EDIT = 101 };
enum { eIDC_CONNECT = 1001 };