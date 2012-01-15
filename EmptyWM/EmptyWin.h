#ifndef __H_EMPTY_WIN
#define __H_EMPTY_WIN

#include <Windows.h>

//Captured window
struct capt_window
{
	HWND hWnd; //pointer to the window
	bool with_borders; //if the window has borders
};

enum TrayIcon {
	ID = 13, 
	APPWM_TRAYICON = WM_APP,
	APPWM_NOP = WM_APP+1
};

//enum {
//	//  Tray icon crap
//	ID_TRAYICON         = 1,
//
//	APPWM_TRAYICON      = WM_APP,
//	APPWM_NOP           = WM_APP + 1,
//
//	//  Our commands
//	ID_ABOUT            = 2000,
//	ID_EXIT,
//};

//Changes the state of borders in the window
void flip_borders(capt_window* window);

//Set the border style for the window, if flag is true borders are active, if it's false they are inactive
void set_borders(capt_window* window, bool flag);

//Initializes the container of a window with its default data
void initialize_captured_window(capt_window *window, HWND handler);

//Hooks the application to low-level keyboard/mouse events
LRESULT CALLBACK	Keyboard_Hook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	Mouse_Hook(int nCode, WPARAM wParam, LPARAM lParam);

#endif