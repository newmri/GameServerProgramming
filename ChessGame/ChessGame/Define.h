#pragma once

#include <Windows.h>
#include <atlimage.h>

#define FIRST_X 357
#define FIRST_Y 360
#define MAX_IP_LEN 16
#define MAX_BUF_SIZE 1024

#define MOVE_PIXEL 85

#define MAX_PLAYER 10

// POS_LEN
#define POS_LEN sizeof(POINT) + (sizeof(int) *2)

enum { eUP = 0, eDOWN, eLEFT, eRIGHT };


// Distiction of data type
enum enumDataType { eMOVE = 1, eCLIENT_INFO, eANOTHER_MOVE, eNOTIFY_LOGOUT};

// Location
enum enumLocation { eLOBBY, eGAME_ROOM, eLOGOUT };

enum { eID_IP_EDIT = 101 };
enum { eIDC_CONNECT = 1001 };