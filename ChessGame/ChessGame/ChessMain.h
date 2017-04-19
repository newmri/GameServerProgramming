#pragma once

#include "CPlayer.h"
#include "STMap.h"
#pragma comment(linker , "/entry:WinMainCRTStartup /subsystem:console")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("ChessGame");
HWND hWndMain;

#define MAX_WIN_SIZE_X 785
#define MAX_WIN_SIZE_Y 805


int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

