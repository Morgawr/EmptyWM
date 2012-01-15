#include "stdafx.h"

extern std::map<HWND, capt_window> capt_windows;

//definitions
#define UPDATE_INTERVAL 5 //Update time in milliseconds for the redraw window drag
#define EWM_SHIFT 0x01
#define EWM_CTRL  0x02
#define EWM_Q     0x04


//Local variables
static int alt_is_down = 0; //If the left alt key is pressed
static POINT mousecoords; //saved coordinates of the mouse
static POINT old_mousecoords; //saved mouse coordinates before the resize event (so we can get the mouse back to its original position without hijacking anything)
static int lmouse_is_down = 0; //If the left button is pressed on the mouse
static int rmouse_is_down = 0; //If the right button is pressed on the mouse
static HWND current_window; //Currently used window, usually it's the focused window
static DWORD last_drag_event_time; //Last time we updated the screen for a drag event 
static DWORD last_resize_event_time; //Last time we updated the screen for a resize event
static int keyboard_down_state = 0; //The state of the keyboard with ctrl+shift+q buttons

//Local functions
static void UpdateDraggedWindow(HWND window, POINT new_mouse_coords)
{
	RECT r;
	GetWindowRect(window,&r);

	MoveWindow(window, r.left + (new_mouse_coords.x - mousecoords.x),
				r.top + (new_mouse_coords.y - mousecoords.y),
				r.right - r.left, r.bottom - r.top, true);
}

static void UpdateResizedWindow(HWND window, POINT new_mouse_coords)
{
	RECT r;
	GetWindowRect(window,&r);

	MoveWindow(window, r.left,r.top,
				r.right - r.left + (new_mouse_coords.x - mousecoords.x),
				r.bottom - r.top + (new_mouse_coords.y - mousecoords.y), true);
}


//Hook procedure to read keyboard state.
LRESULT CALLBACK Keyboard_Hook(
  int nCode, 
  WPARAM wParam,
  LPARAM lParam
)
{
	if(nCode == HC_ACTION)
	{

		if(wParam == WM_KEYDOWN && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LSHIFT)
			keyboard_down_state |= EWM_SHIFT;
		if(wParam == WM_KEYDOWN && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == 0x51) //"Q"
			keyboard_down_state |= EWM_Q;
		if(wParam == WM_KEYDOWN && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LCONTROL)
			keyboard_down_state |= EWM_CTRL;
		if(wParam == WM_KEYUP && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LSHIFT)
			keyboard_down_state &= ~EWM_SHIFT;
		if(wParam == WM_KEYUP && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == 0x51) //"Q"
			keyboard_down_state &= ~EWM_Q;
		if(wParam == WM_KEYUP && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LCONTROL)
			keyboard_down_state &= ~EWM_CTRL;

		if(wParam == WM_SYSKEYDOWN && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LMENU)
		{
			alt_is_down = 1;
		}
		if((wParam == WM_SYSKEYUP || wParam == WM_KEYUP) && ((KBDLLHOOKSTRUCT*)lParam)->vkCode == VK_LMENU)
		{
			alt_is_down = 0;
		}

		//Activate/deactivate borders
		if(wParam == WM_KEYDOWN && ((keyboard_down_state & (EWM_CTRL | EWM_Q | EWM_SHIFT)) == (EWM_CTRL | EWM_Q | EWM_SHIFT))) //flip the state of the focused window
		{
			current_window = GetForegroundWindow();
			capt_window current = capt_windows[current_window];
			if(current.hWnd != current_window)
				initialize_captured_window(&current, current_window);
				
			flip_borders(&current);
			capt_windows[current.hWnd] = current;
		}

	}
   return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//Hook procedure to read mouse state.
LRESULT CALLBACK Mouse_Hook(
  int nCode, 
  WPARAM wParam,
  LPARAM lParam
)
{
	if(nCode == HC_ACTION)
	{ 
		current_window = GetForegroundWindow();
		if(wParam == WM_MOUSEMOVE && (lmouse_is_down || rmouse_is_down)) //If we're moving the mouse we check if the button is pressed and update the window every 5 seconds
		{
			if(last_drag_event_time != 0)
			{
				DWORD new_event_time = GetTickCount();
				if(new_event_time - last_drag_event_time >= UPDATE_INTERVAL)
				{
					POINT new_mouse_coords = ((MSLLHOOKSTRUCT*)lParam)->pt;
					UpdateDraggedWindow(current_window, new_mouse_coords);
					mousecoords = new_mouse_coords;
					last_drag_event_time = GetTickCount();
				}
			}

			if(last_resize_event_time != 0)
			{
				DWORD new_event_time = GetTickCount();
				if(new_event_time - last_resize_event_time >= UPDATE_INTERVAL)
				{
					POINT new_mouse_coords = ((MSLLHOOKSTRUCT*)lParam)->pt;
					UpdateResizedWindow(current_window, new_mouse_coords);
					mousecoords = new_mouse_coords;
					last_resize_event_time = GetTickCount();
				}
			}
		}
		if(wParam == WM_LBUTTONDOWN && alt_is_down && !lmouse_is_down) //If we're pressing the left button with alt activated we want to start dragging the window around
		{
			mousecoords = ((MSLLHOOKSTRUCT*)lParam)->pt;
			lmouse_is_down = 1;

			last_drag_event_time = GetTickCount();
		}
		if(wParam == WM_LBUTTONUP && (mousecoords.x != 0 && mousecoords.y != 0)) //If we release the left button we just want to stop and clean the data
		{
			lmouse_is_down = 0;
			POINT new_mouse_coords = ((MSLLHOOKSTRUCT*)lParam)->pt;
			UpdateDraggedWindow(current_window, new_mouse_coords);
			mousecoords.x = 0;
			mousecoords.y = 0;
			last_drag_event_time = 0;
		}

		if(wParam == WM_RBUTTONDOWN && alt_is_down && !rmouse_is_down) //If we're pressing the right button with alt activated we want to start resizing the window
		{
			rmouse_is_down = 1;
			old_mousecoords = ((MSLLHOOKSTRUCT*)lParam)->pt;
			RECT r;
			GetWindowRect(current_window,&r);
			SetCursorPos(r.right,r.bottom);
			mousecoords.x = r.right;
			mousecoords.y = r.bottom;
			last_resize_event_time = GetTickCount();
		}
		if(wParam == WM_RBUTTONUP && (mousecoords.x != 0 && mousecoords.y != 0) ) //If we release the right button we finish resizing and clean the data
		{
			rmouse_is_down = 0;
			POINT new_mouse_coords = ((MSLLHOOKSTRUCT*)lParam)->pt;
			UpdateResizedWindow(current_window, new_mouse_coords);
			mousecoords.x = 0;
			mousecoords.y = 0;
			last_resize_event_time = 0;

			SetCursorPos(old_mousecoords.x, old_mousecoords.y);
		}
	}
   return CallNextHookEx(NULL, nCode, wParam, lParam);
}