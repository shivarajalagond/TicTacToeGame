// TicTacToeGame.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TicTacToeGame.h"
#include <windowsx.h>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOEGAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOEGAME));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOEGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOEGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// Global variables for game
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
int playerTurn = 1;
HICON hIcon1,hIcon2;
int gameBoard[9] = { 0,0,0,0,0,0,0,0,0 };
int winner = 0;
int wins[3];

//Function to find the center to draw rectangle
BOOL GetGameBoardeRect(HWND hWnd, RECT * pRect) {
	RECT rc;
	if (GetClientRect(hWnd, &rc)) {

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;


		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (height - CELL_SIZE * 3) / 2;
		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;
		return TRUE;
	}
	SetRectEmpty(pRect);
	return FALSE;

}

//To draw the lines inside the layout
void DrawLine(HDC hwnd, int x1, int y1, int x2, int y2) {
	MoveToEx(hwnd, x1, y1, NULL );
	LineTo(hwnd, x2, y2);
}

//To get the mouseclick position
int GetCellIndexFromPoint(HWND hdc, int x, int y) {
	POINT pt = {x,y};
	RECT rc;
	if (GetGameBoardeRect(hdc, &rc)) {
		if (PtInRect(&rc, pt)) {
		
			//Normalize x & y
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			//convert to index  ( 0 to 8)
			return column + row * 3;
		}
	}
	return -1; //Outise the layout
}

BOOL GetCellRect(HWND hWnd, int index, RECT * rccell) {

	RECT rect;
	SetRectEmpty(rccell);
	if (index < 0 || index >8) {
		return FALSE;
	}

	if (GetGameBoardeRect(hWnd, &rect)) {

		int x = index % 3;
		int y = index / 3;
		rccell->left = rect.left + x* CELL_SIZE + 1;
		rccell->top = rect.top + y* CELL_SIZE + 1;
		rccell->right = rccell->left + CELL_SIZE - 1;
		rccell->bottom = rccell->top + CELL_SIZE - 1;

		return TRUE;

	}
}

int GetWinner(int wins[3]) {

	int cells[] = { 0,1,2, 3,4,5, 6,7,8,  0,3,6,  1,4,7, 2,5,8,  0,4,8, 2,4,6 };

	for (int i = 0; i < ARRAYSIZE(cells); i += 3) {

		if ((0 != gameBoard[cells[i]]) && gameBoard[cells[i]] == gameBoard[cells[i + 1]]
			&& gameBoard[cells[i]] == gameBoard[cells[i + 2]])
		{
			// winner is present
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}
		//if all the spaces are not over
		
		for (int i = 0; i < ARRAYSIZE(cells); i++) 
			if (0 == gameBoard[i]) 
				return 0;
			
		return 3;

	}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

	case WM_CREATE: {
		hbr1 = CreateSolidBrush(RGB(255, 0, 0));
		hbr2 = CreateSolidBrush(RGB(0, 0, 255));
	}
	break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_NEW:
			{
				int ret = MessageBox(hWnd, L"Are you sure want to start a new game ?", L"New Game", MB_YESNO | MB_ICONQUESTION);
				if (IDYES == ret)
					playerTurn = 1;
				    winner = 0;
					ZeroMemory(gameBoard, sizeof(gameBoard));
					InvalidateRect(hWnd, NULL, TRUE);
					UpdateWindow(hWnd);
			}
			break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_GETMINMAXINFO: {
		MINMAXINFO * pminmax = (MINMAXINFO *) lParam;
		pminmax->ptMinTrackSize.x = CELL_SIZE * 5;
		pminmax->ptMinTrackSize.y = CELL_SIZE * 5;
	}
		break;

	case WM_LBUTTONDOWN: {

		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (playerTurn == 0)
			break;
		int index = GetCellIndexFromPoint(hWnd, xPos, yPos);


		HDC hdc = GetDC(hWnd); 

		//if (NULL != hdc) {
		//	WCHAR temp[100];
		//	wsprintf(temp, L"Index = %d", index);
		//	//TextOut(hdc, xPos, yPos, temp, lstrlen(temp));

		//}

		if (index != -1) {
			RECT rccell;
			if(gameBoard[index] ==0 && GetCellRect(hWnd, index, &rccell)) {
				gameBoard[index] = playerTurn;
				FillRect(hdc, &rccell, playerTurn ==2 ? hbr1 : hbr2);
				
				//To check for the winner
			  winner = GetWinner(wins);
			  if (winner == 1 || winner == 2) {

				  //To notify the winner
				  MessageBox(hWnd, (winner == 1) ? L"Player 1 Won!" : L"Player 2 Won!",
					  L"You Win",
					  MB_OK | MB_ICONINFORMATION);
				  playerTurn = 0;

			  }
			  else if (winner == 0) {
				  playerTurn = (playerTurn == 1) ? 2 : 1;
			  }
			  else if (winner == 3) {
				  MessageBox(hWnd, L"Its a draw",
					  L"Well played!!",
					  MB_OK | MB_ICONINFORMATION);
				  playerTurn = 0;
			  }
				
			}
			
		}

	}
	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			RECT rc;
			if (GetGameBoardeRect(hWnd, &rc)) {
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
			}

			for (int i = 0; i < 4; i++) {

            //TO Draw Vertical Lines
			DrawLine(hdc, rc.left + i*CELL_SIZE, rc.top , rc.left + i * CELL_SIZE, rc.bottom);
		    //To Draw Horizontal Lines
			DrawLine(hdc, rc.left, rc.top + i* CELL_SIZE, rc.right, rc.top + i* CELL_SIZE);
			}
			
			// To draw all occupied Cells
			RECT rect;
			for (int i = 0; i < 9; i++) {

				if (0 != gameBoard[i] && GetCellRect(hWnd, i, &rect)) {

					FillRect(hdc, &rect, (gameBoard[i] == 1) ? hbr2 : hbr1);
				}
			}
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteObject(hbr1);
		DeleteObject(hbr2);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
