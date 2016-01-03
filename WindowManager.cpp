#include "WindowManager.h"

WindowManager WindowManager::instance;

void WindowManager::applyCursorCapture() {
	if(captureCursor) {
		RECT clientrect;
		HWND hwnd = ::GetActiveWindow();
		::GetClientRect(hwnd, &clientrect);
		::ClientToScreen(hwnd, (LPPOINT)&clientrect.left);
		::ClientToScreen(hwnd, (LPPOINT)&clientrect.right);
		::ClipCursor(&clientrect);
	} else {
		::ClipCursor(NULL);
	}
}

void WindowManager::toggleCursorCapture() {
	captureCursor = !captureCursor;
}

void WindowManager::toggleCursorVisibility() {	
	cursorVisible = !cursorVisible;
	::ShowCursor(cursorVisible);
}

void WindowManager::toggleBorderlessFullscreen() {
	borderlessFullscreen = !borderlessFullscreen;
	HWND hwnd = ::GetActiveWindow();
	if(borderlessFullscreen) {
		// store previous rect
		::GetClientRect(hwnd, &prevWindowRect);
		// set styles
		LONG lStyle = ::GetWindowLong(hwnd, GWL_STYLE);
		prevStyle = lStyle;
		lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		::SetWindowLong(hwnd, GWL_STYLE, lStyle);
		LONG lExStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
		prevExStyle = lExStyle;
		lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		::SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
		// adjust size & position
		HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor, &info);
		int monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
		int monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;
		::SetWindowPos(hwnd, NULL, info.rcMonitor.left, info.rcMonitor.top, monitorWidth, monitorHeight, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
	} else {
		// restore previous window
		::SetWindowLong(hwnd, GWL_STYLE, prevStyle);
		::SetWindowLong(hwnd, GWL_EXSTYLE, prevExStyle);
		RECT desiredRect = prevWindowRect;
		::AdjustWindowRect(&desiredRect, prevStyle, false);
		int wWidth = desiredRect.right - desiredRect.left, wHeight = desiredRect.bottom - desiredRect.top;
		::SetWindowPos(hwnd, NULL, prevWindowRect.left, prevWindowRect.top, wWidth, wHeight, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}
}

void WindowManager::resize(unsigned clientW, unsigned clientH) {
	HWND hwnd = ::GetActiveWindow();
	// Store current window rect
	::GetClientRect(hwnd, &prevWindowRect);
	// Get monitor size
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
	int monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

	// How much do we overlap or are smaller than the actual screen size
	int widthDiff = monitorWidth - (clientW ? clientW : prevWindowRect.right);
	int heightDiff = monitorHeight - (clientH ? clientH : prevWindowRect.bottom);

 	RECT desiredRect;
	desiredRect.left = widthDiff / 2;
	desiredRect.top = heightDiff / 2;
	desiredRect.right = monitorWidth - (widthDiff / 2);
	desiredRect.bottom = monitorHeight - (heightDiff / 2);
 	LONG lStyle = ::GetWindowLong(hwnd, GWL_STYLE);
 	::AdjustWindowRect(&desiredRect, lStyle, false);
	::SetWindowPos(hwnd, NULL, desiredRect.left, desiredRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}
