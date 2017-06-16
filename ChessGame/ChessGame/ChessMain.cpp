#include "ChessMain.h"
#include "resource.h"
// Draw
RECT g_Clntrt;

// Global Button Handler
HWND g_hConnectBtn;

// Global Edit Handler
HWND g_hIpEdit;

// Global Static Handler
HWND g_hStatic;
CPlayer* pPlayer = CPlayer::Instance();

BOOL CALLBACK Dlg_LoginProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

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
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPED | WS_SYSMENU, 0, 0, MAX_WIN_SIZE_X, MAX_WIN_SIZE_Y,
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

	HDC hdc, hMemDC;
	HBITMAP hBit, OldBit;
	PAINTSTRUCT ps;
	static POINT mapos, chesspos;
	static CImage cChessmap, cChessImg;
	static bool bInit = false;
	static STMap stMap;
	CString cStr;
	static POINT pos{ 0,0 };
	static bool bSwapMap = false;
	HFONT hFont, hOldFont;

	switch (uMsg) {
	case WM_SOCKET: {
		pPlayer->ProcessPacket(hWnd, uMsg, wParam, lParam);
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
		GetClientRect(hWndMain, &g_Clntrt);
		break;
	}
	case WM_TIMER:
		InvalidateRect(hWnd, &g_Clntrt, FALSE);
		break;
	case WM_COMMAND: {
		switch (wParam) {
		case eIDC_CONNECT: {
			pPlayer->SetServerIP(g_hIpEdit);
			pPlayer->Init(hWnd);
			if (pPlayer->Connect(hWnd)) {
				ShowWindow(g_hIpEdit, SW_HIDE);
				ShowWindow(g_hConnectBtn, SW_HIDE);
				ShowWindow(g_hStatic, SW_HIDE);
				InvalidateRect(hWnd, &g_Clntrt, TRUE);
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_LOGIN), hWnd, Dlg_LoginProc);
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
			pPlayer->SetMove(eCS_UP);
			pPlayer->SetPos();
			break;
		case VK_DOWN:
			pPlayer->SetMove(eCS_DOWN);
			pPlayer->SetPos();
			break;
		case VK_LEFT:
			pPlayer->SetMove(eCS_LEFT);
			pPlayer->SetPos();
			break;
		case VK_RIGHT:
			pPlayer->SetMove(eCS_RIGHT);
			pPlayer->SetPos();
			break;
		default:
			break;
		}
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		hBit = CreateCompatibleBitmap(hdc, g_Clntrt.right, g_Clntrt.bottom);
		hMemDC = CreateCompatibleDC(hdc);
		OldBit = (HBITMAP)SelectObject(hMemDC, hBit);

		hFont = CreateFont(10, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
			VARIABLE_PITCH | FF_ROMAN, TEXT("±¼¸²"));
		hOldFont = (HFONT)SelectObject(hMemDC, hFont);
		SetBkColor(hMemDC, TRANSPARENT);
		SetTextColor(hMemDC, RGB(255, 0, 0));

		// Draw the player's the image of character and cChessGameMap
		// If player is in one of the game-rooms.
		if (pPlayer->GetPlayerNum() != 0) {
			if (!cChessmap.IsNull()) cChessmap.Destroy(); 

			pos = pPlayer->GetPos();

			if (pos.x >= MAX_MAP_TILE) if (pos.x % MAX_MAP_TILE == 0) { bSwapMap = !bSwapMap; }
			if (pos.y >= MAX_MAP_TILE) if (pos.y % MAX_MAP_TILE == 0) { bSwapMap = !bSwapMap; }

			stMap.DrawMap(hMemDC, bSwapMap);


			for (int i = 0; i < MAX_PLAYER; ++i) {
				//If player is not logouted
				if (pPlayer->m_stClientInfo[i].m_IsConnected) {

					memcpy(&pPlayer->m_stClientInfo[i].m_DrawPos, &pPlayer->m_stClientInfo[i].m_pos, sizeof(POINT));

					if (pPlayer->m_stClientInfo[i].m_DrawPos.x >= MAX_MAP_TILE) 
						pPlayer->m_stClientInfo[i].m_DrawPos.x = pPlayer->m_stClientInfo[i].m_DrawPos.x % MAX_MAP_TILE;
					
					if (pPlayer->m_stClientInfo[i].m_DrawPos.y >= MAX_MAP_TILE) 
						pPlayer->m_stClientInfo[i].m_DrawPos.y = pPlayer->m_stClientInfo[i].m_DrawPos.y % MAX_MAP_TILE;
					

					pPlayer->m_PlayerImg.TransparentBlt(hMemDC, (pPlayer->m_stClientInfo[i].m_DrawPos.x * MOVE_PIXEL) + 5, pPlayer->m_stClientInfo[i].m_DrawPos.y * MOVE_PIXEL,
						pPlayer->m_PlayerImg.GetWidth(), pPlayer->m_PlayerImg.GetHeight(),
						0, 0, pPlayer->m_PlayerImg.GetWidth(), pPlayer->m_PlayerImg.GetHeight(), RGB(0, 0, 0));

					cStr.Format("(%d, %d)", pPlayer->m_stClientInfo[i].m_pos.y, pPlayer->m_stClientInfo[i].m_pos.x);
					TextOut(hMemDC, (pPlayer->m_stClientInfo[i].m_DrawPos.x * MOVE_PIXEL), (pPlayer->m_stClientInfo[i].m_DrawPos.y * MOVE_PIXEL) + 25, cStr, cStr.GetLength());
				}
			}

			for (int i = 0; i < MAX_NPC_NUM; ++i) {
				if (pPlayer->m_stNPCInfo[i].m_IsAlive) {
					memcpy(&pPlayer->m_stNPCInfo[i].m_DrawPos, &pPlayer->m_stNPCInfo[i].m_pos, sizeof(POINT));

					if (pPlayer->m_stNPCInfo[i].m_DrawPos.x >= MAX_MAP_TILE)
						pPlayer->m_stNPCInfo[i].m_DrawPos.x = pPlayer->m_stNPCInfo[i].m_DrawPos.x % MAX_MAP_TILE;

					if (pPlayer->m_stNPCInfo[i].m_DrawPos.y >= MAX_MAP_TILE)
						pPlayer->m_stNPCInfo[i].m_DrawPos.y = pPlayer->m_stNPCInfo[i].m_DrawPos.y % MAX_MAP_TILE;

					pPlayer->m_NPCImg.TransparentBlt(hMemDC, (pPlayer->m_stNPCInfo[i].m_DrawPos.x * MOVE_PIXEL) + 5, pPlayer->m_stNPCInfo[i].m_DrawPos.y * MOVE_PIXEL,
						pPlayer->m_NPCImg.GetWidth(), pPlayer->m_NPCImg.GetHeight(),
						0, 0, pPlayer->m_NPCImg.GetWidth(), pPlayer->m_NPCImg.GetHeight(), RGB(0, 0, 0));

					cStr.Format("(%d, %d)", pPlayer->m_stNPCInfo[i].m_pos.y, pPlayer->m_stNPCInfo[i].m_pos.x);
					TextOut(hMemDC, (pPlayer->m_stNPCInfo[i].m_DrawPos.x * MOVE_PIXEL), (pPlayer->m_stNPCInfo[i].m_DrawPos.y * MOVE_PIXEL) + 25, cStr, cStr.GetLength());

				}
			}
			BitBlt(hdc, 0, 0, g_Clntrt.right, g_Clntrt.bottom, hMemDC, 0, 0, SRCCOPY);
			InvalidateRect(hWnd, &g_Clntrt, FALSE);
		}

		else cChessmap.BitBlt(hdc, 0, mapos.y, cChessmap.GetWidth(), cChessmap.GetHeight() + mapos.y, 0, 0, SRCCOPY);

		SelectObject(hMemDC, OldBit);
		SelectObject(hdc, hOldFont);
		DeleteDC(hMemDC);
		DeleteObject(hFont);
		ReleaseDC(hWnd, hMemDC);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_ERASEBKGND:
		return true;
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




BOOL CALLBACK Dlg_LoginProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	char ID[ID_LEN];
	char PWD[PWD_LEN];
	switch (iMessage){
	case WM_INITDIALOG:
		pPlayer->SetDlg(hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)){
		case ID_LOGIN:
			GetDlgItemText(hWnd, IDC_ID, ID, ID_LEN);
			GetDlgItemText(hWnd, IDC_PWD, PWD, PWD_LEN);
			pPlayer->Login(ID, PWD);
			break;
		case ID_EXIT:
			pPlayer->Close(true);
			PostQuitMessage(0);
			break;
		}
		break;

	}

	return 0;
}