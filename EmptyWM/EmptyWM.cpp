// EmptyWM.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EmptyWM.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//Map holding all the data for all the windows
std::map<HWND, capt_window> capt_windows;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void AddTrayIcon( HWND hWnd, UINT uID, UINT uCallbackMsg, LPWSTR pszToolTip);
void RemoveTrayIcon( HWND hWnd, UINT uID );
void OnTrayIconShowMenu( HWND hWnd );
BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem );
void restore_borders(std::map<HWND, capt_window>* map);


int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_EMPTYWM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMPTYWM));

	try
	{
		SetWindowsHookEx(WH_KEYBOARD_LL, Keyboard_Hook, NULL, 0); 
		SetWindowsHookEx(WH_MOUSE_LL, Mouse_Hook, NULL, 0);

		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	catch(std::exception&)
	{
		;
	}
	
	restore_borders(&capt_windows);
	
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMPTYWM));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_EMPTYWM);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_EMPTYWM));

	return RegisterClassEx(&wcex);
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
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	SetWindowPos(hWnd, NULL, 150, 150, 200, 100, 0);

	if (!hWnd)
	{
		return FALSE;
	}

	
	AddTrayIcon(hWnd, TrayIcon::ID, TrayIcon::APPWM_NOP, L"EmptyWM");
	UpdateWindow(hWnd);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			RemoveTrayIcon(hWnd,TrayIcon::ID);
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	case TrayIcon::APPWM_NOP:
		switch (LOWORD(lParam))
		{
		case WM_RBUTTONUP:
			OnTrayIconShowMenu(hWnd);
			break;
		case WM_LBUTTONUP:
			OnTrayIconShowMenu(hWnd);
			break;
		}

		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//--------------------------------------------------------------------------------
//Restores the borders of all the tracked windows and removes them from the list.
void restore_borders(std::map<HWND, capt_window>* map)
{
	for (std::map<HWND, capt_window>::iterator it = map->begin(); it != map->end(); ++it)
	{
		set_borders(&(it->second),true);
	}
}

//-----------------------------------------------------------------------------
//  Add an icon to the system tray.
void AddTrayIcon( HWND hWnd, UINT uID, UINT uCallbackMsg, LPWSTR pszToolTip)
{
	NOTIFYICONDATA  nid;

	memset(&nid, 0, sizeof(nid));

	nid.cbSize              = sizeof(nid);
	nid.hWnd                = hWnd;
	nid.uID                 = uID;
	nid.uFlags              = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage    = uCallbackMsg;
	
	nid.hIcon               = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EMPTYWM));

	wsprintf(nid.szTip,pszToolTip);

	Shell_NotifyIcon(NIM_ADD, &nid);
}

//-----------------------------------------------------------------------------
//  Remove an icon from the system tray.
void RemoveTrayIcon( HWND hWnd, UINT uID )
{
	NOTIFYICONDATA  nid;

	memset( &nid, 0, sizeof( nid ) );

	nid.cbSize  = sizeof( nid );
	nid.hWnd    = hWnd;
	nid.uID     = uID;

	Shell_NotifyIcon( NIM_DELETE, &nid );
}

//-----------------------------------------------------------------------------
//  Event on tray icon displays menu.
void OnTrayIconShowMenu( HWND hWnd )
{
	SetForegroundWindow(hWnd);

	ShowPopupMenu(hWnd, NULL, -1);

	PostMessage(hWnd, TrayIcon::APPWM_NOP, 0, 0);
}

//-----------------------------------------------------------------------------
//  Create and the popupmenu when the user right-clicks on the 
//  system tray.
BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem )
{
	HMENU   hPop        = NULL;
	int     i           = 0;
	WORD    cmd;
	POINT   pt;

	hPop = CreatePopupMenu();

	if ( ! curpos ) {
		GetCursorPos( &pt );
		curpos = &pt;
	}

	InsertMenu( hPop, i++, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit" );

	SetMenuDefaultItem( hPop, IDM_EXIT, FALSE );

	SetFocus( hWnd );

	SendMessage( hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );

	cmd = TrackPopupMenu( hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON 
							| TPM_RETURNCMD | TPM_NONOTIFY,
						  curpos->x, curpos->y, 0, hWnd, NULL );

	SendMessage( hWnd, WM_COMMAND, cmd, 0 );

	DestroyMenu( hPop );

	return cmd;
}