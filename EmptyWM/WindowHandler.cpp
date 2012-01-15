#include "stdafx.h"

//Set the border style for the window, if flag is true borders are active, if it's false they are inactive
void set_borders(capt_window* window, bool flag)
{
	if(window->with_borders != flag)
		flip_borders(window);
}

//Changes the state of borders in the window
void flip_borders(capt_window* window)
{
	long style = GetWindowLong(window->hWnd, GWL_STYLE);
	style ^= WS_CAPTION;
	window->with_borders = (style & WS_CAPTION);
	SetWindowLongPtr(window->hWnd, GWL_STYLE, style);
	RECT lp;
	GetWindowRect(window->hWnd, &lp);
	SetWindowPos(window->hWnd, NULL, lp.left, lp.top, lp.right - lp.left, lp.bottom - lp.top, 0x20);
	ShowWindow(window->hWnd, SW_SHOW);
}

//Initializes the container of a window with its default data
void initialize_captured_window(capt_window *window, HWND handler)
{
	window->hWnd = handler;
	window->with_borders = true;
}