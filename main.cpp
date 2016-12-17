
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <fstream>
#include <ostream>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <strsafe.h>

#include "main.h"
#include "d3d9.h"
#include "d3dutil.h"
#include "Settings.h"
#include "KeyActions.h"
#include "Detouring.h"
#include "SaveManager.h"
#include "FPS.h"

// globals
tDirect3DCreate9 oDirect3DCreate9 = Direct3DCreate9;
tDirectInput8Create oDirectInput8Create;
std::ofstream ofile;	
char dlldir[320];

bool WINAPI DllMain(HMODULE hDll, DWORD dwReason, PVOID pvReserved) {
	TCHAR fileName[512];
	GetModuleFileName(NULL, fileName, 512);

	if(dwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hDll);
		GetModuleFileName(hDll, dlldir, 512);
		for(int i = strlen(dlldir); i > 0; i--) { if(dlldir[i] == '\\') { dlldir[i+1] = 0; break; } }
		ofile.open(GetDirectoryFile("DSfix.log"), std::ios::out);
		sdlogtime();
		SDLOG(0, "===== start DSfix %s = fn: %s\n", VERSION, fileName);
		
		// load settings
		Settings::get().load();
		Settings::get().report();
		
		KeyActions::get().load();
		KeyActions::get().report();

		// load original dinput8.dll
		HMODULE hMod;
		if(Settings::get().getDinput8dllWrapper().empty() || (Settings::get().getDinput8dllWrapper().find("none") == 0)) {
			char syspath[320];
			GetSystemDirectory(syspath, 320);
			strcat_s(syspath, "\\dinput8.dll");
			hMod = LoadLibrary(syspath);
		} else {
			sdlog(0, "Loading dinput wrapper %s\n", Settings::get().getDinput8dllWrapper().c_str());
			hMod = LoadLibrary(Settings::get().getDinput8dllWrapper().c_str());
		}
		if(!hMod) {
			sdlog("Could not load original dinput8.dll\nABORTING.\n");
			errorExit("Loading of specified dinput wrapper");
		}
		oDirectInput8Create = (tDirectInput8Create)GetProcAddress(hMod, "DirectInput8Create");
		
		SaveManager::get().init();

		earlyDetour();

		initFPSTimer();
		if(Settings::get().getUnlockFPS()) applyFPSPatch();

		return true;
	} else if(dwReason == DLL_PROCESS_DETACH) {
		Settings::get().shutdown();
		endDetour();
		SDLOG(0, "===== end = fn: %s\n", fileName);
		if(ofile) { ofile.close(); }
	}

    return false;
}

char *GetDirectoryFile(char *filename) {
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void __cdecl sdlogtime() {
	char timebuf[26];
    time_t ltime;
    struct tm gmt;
	time(&ltime);
    _gmtime64_s(&gmt, &ltime);
    asctime_s(timebuf, 26, &gmt);
	timebuf[24] = '\0'; // remove newline
	SDLOG(0, "===== %s =====\n", timebuf);
}

void __cdecl sdlog(const char *fmt, ...) {
	if(ofile.good()) {
		if(!fmt) { return; }

		va_list va_alist;
		char logbuf[9999] = {0};

		va_start (va_alist, fmt);
		_vsnprintf_s(logbuf+strlen(logbuf), sizeof(logbuf) - strlen(logbuf), _TRUNCATE, fmt, va_alist);
		va_end (va_alist);

		ofile << logbuf;
		ofile.flush();
	}
}

void errorExit(LPTSTR lpszFunction) { 
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

bool fileExists(const char *filename) {
  return std::ifstream(filename).good();
}
