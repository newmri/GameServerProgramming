#include "ChessMain.h"
// Draw
RECT g_Clntrt;

// Global Button Handler
HWND g_hConnectBtn;

// Global Edit Handler
HWND g_hIpEdit;

// Global Static Handler
HWND g_hStatic;

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
	static HBITMAP hBitmap;
	static POINT mapos, chesspos;
	static CImage cChessmap, cChessGameMap, cChessImg;
	static CPlayer cPlayer;
	static bool bInit = false;

	// At the first time, can't recv WM_SOCKET message IDK why it does...
	// So this code added.
	if (!bInit) {
		if (WSAGETSELECTERROR(lParam) != 0) {
			switch (WSAGETSELECTEVENT(lParam)) {
			case FD_CONNECT:
				printf("ASfasf");
				break;
			case FD_WRITE:
				printf("aa");
				cPlayer.ProcessSocketMessage(hWnd, iMessage, wParam, lParam);
				bInit = true;
				break;
			case FD_CLOSE:
				cPlayer.Close(true);
				break;
			default:
				break;
			}
		}
	}

	switch (iMessage) {
	case WM_CREATE: {
		hWndMain = hWnd;
		// Load the Login image.
		cChessmap.Load("ChessLogin.bmp");
		cChessGameMap.Load("cChessmap.bmp");
		// Display various edits.
		g_hStatic = CreateWindow(TEXT("Static"), TEXT("SIP:"), WS_CHILD | WS_VISIBLE, FIRST_X - 80, FIRST_Y - 45, 30, 30, hWnd, (HMENU)-1, g_hInst, NULL);
		g_hIpEdit = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, FIRST_X - 50, FIRST_Y - 50, 120, 30, hWnd, (HMENU)eID_IP_EDIT, g_hInst, NULL);
		g_hConnectBtn = CreateWindow((LPCSTR)"Button", (LPCSTR)"Connect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, FIRST_X - 50, FIRST_Y, 120, 30, hWnd, (HMENU)eIDC_CONNECT, g_hInst, NULL);
		break;
	}
	case WM_COMMAND: {
		switch (wParam) {
		case eIDC_CONNECT: {
			cPlayer.SetServerIP(g_hIpEdit);
			cPlayer.Init(hWnd);
			if (cPlayer.Connect()) {
				cPlayer.SetPlayerLocation(eGAME_ROOM);
				ShowWindow(g_hIpEdit, SW_HIDE);
				ShowWindow(g_hConnectBtn, SW_HIDE);
				ShowWindow(g_hStatic, SW_HIDE);
				InvalidateRect(hWnd, &g_Clntrt, NULL);
			}
			else MessageBox(NULL, _T("U should enter right IP Ex) 127.0.0.1"), _T("Connect's been failed"), 0);
			break;
		default:
			break;
		}
		break;
		}
		break;
	}
	case WM_SOCKET: {
		printf("%d", 6666);
		cPlayer.ProcessSocketMessage(hWnd, iMessage, wParam, lParam);
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam) {
		case VK_UP:
			cPlayer.SetMove(eUP);
			cPlayer.MoveChess();
			InvalidateRect(hWnd, &g_Clntrt, NULL);
			break;
		case VK_DOWN:
			cPlayer.SetMove(eDOWN);
			cPlayer.MoveChess();
			InvalidateRect(hWnd, &g_Clntrt, NULL);
			break;
		case VK_LEFT:
			cPlayer.SetMove(eLEFT);
			cPlayer.MoveChess();
			InvalidateRect(hWnd, &g_Clntrt, NULL);
			break;
		case VK_RIGHT:
			cPlayer.SetMove(eRIGHT);
			cPlayer.MoveChess();
			InvalidateRect(hWnd, &g_Clntrt, NULL);
			break;
		default:
			break;
		}
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMain, &g_Clntrt); 

		hMemDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, g_Clntrt.right, g_Clntrt.bottom);

		SelectObject(hMemDC, hBitmap);

		cChessmap.BitBlt(hMemDC, 0, mapos.y, cChessmap.GetWidth(), cChessmap.GetHeight() + mapos.y, 0, 0, SRCCOPY);

		// Draw the player's the image of character and cChessGameMap
		// If player is in one of the game-rooms.
		if (cPlayer.IsPlayerGameRoom()) {
			cChessGameMap.BitBlt(hMemDC, 0, mapos.y, cChessmap.GetWidth(), cChessmap.GetHeight() + mapos.y, 0, 0, SRCCOPY);
			chesspos = cPlayer.GetPos();
			cPlayer.m_chess.TransparentBlt(hMemDC, chesspos.x, chesspos.y, cPlayer.m_chess.GetWidth(), cPlayer.m_chess.GetHeight(),
				0, 0, cPlayer.m_chess.GetWidth(), cPlayer.m_chess.GetHeight(), RGB(0, 0, 0));	
		}
		else 
			cChessmap.BitBlt(hMemDC, 0, mapos.y, cChessmap.GetWidth(), cChessmap.GetHeight() + mapos.y, 0, 0, SRCCOPY);
	
		
		BitBlt(hdc, 0, 0, g_Clntrt.right, g_Clntrt.bottom, hMemDC, 0, 0, SRCCOPY);

		DeleteObject(hBitmap);
		DeleteDC(hMemDC);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_GETMINMAXINFO: {
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = MAX_WIN_SIZE_X;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = MAX_WIN_SIZE_Y;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = MAX_WIN_SIZE_X;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = MAX_WIN_SIZE_Y;
		return FALSE;
	}

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	default:
		break;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}




