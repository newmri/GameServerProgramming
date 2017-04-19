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
	int a = GetSystemMetrics(SM_CYSCREEN);
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, 0, 0, MAX_WIN_SIZE_X, MAX_WIN_SIZE_Y,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	HDC hdc;
	PAINTSTRUCT ps;
	static POINT mapos, chesspos;
	static CImage cChessmap, cChessImg;
	CPlayer* pPlayer = CPlayer::Instance();
	static bool bInit = false;
	static STMap stMap;
	USHORT wLeft{}, wTop{}, wRight{}, wBottom{};
	switch (uMsg) {
	case WM_SOCKET: {
		if(pPlayer->ProcessSocketMessage(hWnd, uMsg, wParam, lParam))
			InvalidateRect(hWnd, &g_Clntrt, NULL);
		break;
	}
	case WM_CREATE: {
		hWndMain = hWnd;
		// Load the Login image.
		cChessmap.Load("ChessLogin.bmp");
		// Display various edits.
		g_hStatic = CreateWindow(TEXT("Static"), TEXT("SIP:"), WS_CHILD | WS_VISIBLE, FIRST_X - 80, FIRST_Y - 45, 30, 30, hWnd, (HMENU)-1, g_hInst, NULL);
		g_hIpEdit = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, FIRST_X - 50, FIRST_Y - 50, 120, 30, hWnd, (HMENU)eID_IP_EDIT, g_hInst, NULL);
		g_hConnectBtn = CreateWindow((LPCSTR)"Button", (LPCSTR)"Connect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, FIRST_X - 50, FIRST_Y, 120, 30, hWnd, (HMENU)eIDC_CONNECT, g_hInst, NULL);
		GetWindowRect(hWndMain, &g_Clntrt);
		break;
	}
	case WM_COMMAND: {
		switch (wParam) {
		case eIDC_CONNECT: {
			pPlayer->SetServerIP(g_hIpEdit);
			pPlayer->Init(hWnd);
			if (pPlayer->Connect(hWnd)) {
				pPlayer->SetPlayerLocation(eGAME_ROOM);
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
	case WM_KEYDOWN: {
		switch (wParam) {
		case VK_UP:
			pPlayer->SetMove(eUP);
			pPlayer->SetPos();
			break;
		case VK_DOWN:
			pPlayer->SetMove(eDOWN);
			pPlayer->SetPos();
			break;
		case VK_LEFT:
			pPlayer->SetMove(eLEFT);
			pPlayer->SetPos();
			break;
		case VK_RIGHT:
			pPlayer->SetMove(eRIGHT);
			pPlayer->SetPos();
			break;
		default:
			break;
		}
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMain, &g_Clntrt); 

		// Draw the player's the image of character and cChessGameMap
		// If player is in one of the game-rooms.
		if (pPlayer->IsPlayerGameRoom()) {
			if (cChessmap.IsNull()) cChessmap.Destroy();

			stMap.DrawMap(hdc);
			chesspos = pPlayer->GetPos();
			pPlayer->m_chess.TransparentBlt(hdc, chesspos.x, chesspos.y, pPlayer->m_chess.GetWidth(), pPlayer->m_chess.GetHeight(),
				0, 0, pPlayer->m_chess.GetWidth(), pPlayer->m_chess.GetHeight(), RGB(0, 0, 0));	
			
		}
		else
			cChessmap.BitBlt(hdc, 0, mapos.y, cChessmap.GetWidth(), cChessmap.GetHeight() + mapos.y, 0, 0, SRCCOPY);
		
	// Draw the other Players
		if (pPlayer->GetPlayerNum() != 0) {

			for (int i = 0; i < MAX_PLAYER; ++i) {
				 //If player is not logouted
				if(pPlayer->m_stPlayerInfo[i].m_eLocation == eGAME_ROOM)
					pPlayer->m_stPlayerInfo[i].m_ciChess.TransparentBlt(hdc, pPlayer->m_stPlayerInfo[i].m_pos.x, pPlayer->m_stPlayerInfo[i].m_pos.y,
					pPlayer->m_stPlayerInfo[i].m_ciChess.GetWidth(), pPlayer->m_stPlayerInfo[i].m_ciChess.GetHeight(),
					0, 0, pPlayer->m_stPlayerInfo[i].m_ciChess.GetWidth(), pPlayer->m_stPlayerInfo[i].m_ciChess.GetHeight(), RGB(0, 0, 0));
			
			}
		}


		DeleteDC(hdc);
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
		pPlayer->DestroyInstance();
		PostQuitMessage(0);
		return 0;
	}
	default:
		break;
	}

	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}




