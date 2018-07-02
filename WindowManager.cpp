#include "WindowManager.h"
#include <utility>

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
		int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
		int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
		::SetWindowPos(hwnd, NULL, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorWidth, monitorHeight, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
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
	int monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	int monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	// How much do we overlap or are smaller than the actual screen size
	int widthDiff = monitorWidth - (clientW ? clientW : prevWindowRect.right);
	int heightDiff = monitorHeight - (clientH ? clientH : prevWindowRect.bottom);

 	RECT desiredRect;
	desiredRect.left = monitorInfo.rcMonitor.left + widthDiff / 2;
	desiredRect.top = monitorInfo.rcMonitor.top + heightDiff / 2;
	desiredRect.right = monitorWidth - (widthDiff / 2);
	desiredRect.bottom = monitorHeight - (heightDiff / 2);
 	LONG lStyle = ::GetWindowLong(hwnd, GWL_STYLE);
 	::AdjustWindowRect(&desiredRect, lStyle, false);
	::SetWindowPos(hwnd, NULL, desiredRect.left, desiredRect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

BOOL CALLBACK monitorEnumProc(
	HMONITOR hMonitor,
	HDC      hdcMonitor,
	LPRECT   lprcMonitor,
	LPARAM   dwData
) {
	static int i = 0;

	const auto data = reinterpret_cast<std::pair<MONITORINFO*, int>*>(dwData);
	if (i++ == data->second)
	{
		data->first->cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, data->first);
		return FALSE;
	}
	return TRUE;
}

void WindowManager::overrideMonitor(int monitor_id) {
	// If there is a monitor given, tries to find it
	if (monitor_id != -1)
	{
		monitorInfo.cbSize = 0;
		std::pair<MONITORINFO*, int> data{ &monitorInfo, monitor_id };
		EnumDisplayMonitors(nullptr, nullptr, monitorEnumProc, reinterpret_cast<LPARAM>(&data));
		if (monitorInfo.cbSize != 0)
		{
			return;
		}
	}

	// If there is no override, uses the 'nearest' monitor to the current window
	HWND hwnd = ::GetActiveWindow();
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
}
