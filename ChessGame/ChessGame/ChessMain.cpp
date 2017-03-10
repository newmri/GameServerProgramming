#include "ChessMain.h"

//FUNCTION
RECT ClntRt;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, 400, 50, MAX_WIN_SIZE, MAX_WIN_SIZE,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	static HBITMAP bitmap;
	static POINT MapPos;
	static CImage ChessMap;


	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		ChessMap.Load("ChessMap.bmp");
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMain, &ClntRt); 

		hMemDC = CreateCompatibleDC(hdc);
		bitmap = CreateCompatibleBitmap(hdc, ClntRt.right, ClntRt.bottom);

		SelectObject(hMemDC, bitmap);

		
		ChessMap.BitBlt(hMemDC, 0, MapPos.y, ChessMap.GetWidth(), ChessMap.GetHeight() + MapPos.y, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, ClntRt.right, ClntRt.bottom, hMemDC, 0, 0, SRCCOPY);
		
		DeleteObject(bitmap);
		DeleteDC(hMemDC);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_GETMINMAXINFO: {
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = MAX_WIN_SIZE;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = MAX_WIN_SIZE;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = MAX_WIN_SIZE;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = MAX_WIN_SIZE;
		return FALSE;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}