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

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, 400, 50, MAX_WIN_SIZE_X, MAX_WIN_SIZE_Y,
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
	static Player player;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		ChessMap.Load("ChessMap.bmp");
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			player.m_pos.y -= 90;
			InvalidateRect(hWnd, &ClntRt, NULL);
			return 0;
		case VK_DOWN:
			player.m_pos.y += 90;
			InvalidateRect(hWnd, &ClntRt, NULL);
			return 0;
		case VK_LEFT:
			player.m_pos.x -= 90;
			InvalidateRect(hWnd, &ClntRt, NULL);
			return 0;
		case VK_RIGHT:
			player.m_pos.x += 90;
			InvalidateRect(hWnd, &ClntRt, NULL);
			return 0;
		}
		return 0;
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMain, &ClntRt); 

		hMemDC = CreateCompatibleDC(hdc);
		bitmap = CreateCompatibleBitmap(hdc, ClntRt.right, ClntRt.bottom);

		SelectObject(hMemDC, bitmap);

		
		ChessMap.BitBlt(hMemDC, 0, MapPos.y, ChessMap.GetWidth(), ChessMap.GetHeight() + MapPos.y, 0, 0, SRCCOPY);
		
		player.m_chess.TransparentBlt(hMemDC, player.m_pos.x, player.m_pos.y, player.m_chess.GetWidth(), player.m_chess.GetHeight(),
			0, 0, player.m_chess.GetWidth(), player.m_chess.GetHeight(), RGB(0, 0, 0));

		
		BitBlt(hdc, 0, 0, ClntRt.right, ClntRt.bottom, hMemDC, 0, 0, SRCCOPY);


		DeleteObject(bitmap);
		DeleteDC(hMemDC);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_GETMINMAXINFO: {
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = MAX_WIN_SIZE_X;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = MAX_WIN_SIZE_Y;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = MAX_WIN_SIZE_X;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = MAX_WIN_SIZE_Y;
		return FALSE;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}