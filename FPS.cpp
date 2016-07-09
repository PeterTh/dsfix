// Dark Souls FPS fix by Clement Barnier (Nwks)
///////////////////////////////////////////////

#include "FPS.h"

#include <windows.h>

#include "Settings.h"
#include "main.h"
#include "Detouring.h"
#include "RenderstateManager.h"
#include "memory.h"

#ifndef WITHOUT_GFWL_LIB
void enableGFWLCompatibility(void);
#endif

// Globals
static DWORD OriginalBase = 0x0400000;
static DWORD ImageBase = NULL;

// Hook Globals
double lastRenderTime;
static LARGE_INTEGER timerFreq;
static LARGE_INTEGER counterAtStart;

// Hook Parameters
//-----------------------------------
// Dark Souls executable timestamp
DWORD EXE_TIMESTAMP = 0x546FA3C5; // Steam Beta build 2.0

// Time-step value address
DWORD ADDR_TS = 0x011E4D70;  // 1.0.0 was 0x012497F0, 1.0.1 was 0x012498E0
LPCSTR TS_PATTERN = "0080264400009444000058420000C0428988083D0000A044";
DWORD TS_OFFSET = 0x00000010;

// Presentation interval address
DWORD ADDR_PRESINT = 0x00FFA15E; // 1.0.0 was 0x010275AE, 1.0.1 was 0x0102788E
LPCSTR PRESINT_PATTERN = "FF15xxxxxxxx83C408C78648020000020000005EC20800";
DWORD PRESINT_OFFSET = 0x0000000F;

// getDrawThreadMsgCommand address in HGCommandDispatcher loop
DWORD ADDR_GETCMD =	0x00BAC3DD; // 1.0.0 was 0x00BD601D, 1.0.1 was 0x00BD60ED
LPCSTR GETCMD_PATTERN = "6A018BCDE8xxxxxxxx8BF08BCEE8xxxxxxxx83F805";
DWORD GETCMD_OFFSET = 0x0000000D;

//----------------------------------------------------------------------------------------
// Support Functions
//----------------------------------------------------------------------------------------

// Misc
//------------------------------------
DWORD getAbsoluteAddress(DWORD offset) {
	if (ImageBase)
		return ImageBase + offset;
	else
		return NULL;
}

DWORD convertAddress(DWORD Address) {
	return getAbsoluteAddress(Address - OriginalBase);
}

// Memory
//------------------------------------
void updateAnimationStepTime(float stepTime, float minFPS, float maxFPS) {
	float FPS = 1.0f/(stepTime/1000);

	if (FPS < minFPS)
		FPS = minFPS;
	else if (FPS > maxFPS)
		FPS = maxFPS;
	
	float cappedStep = 1/(float)FPS;
	if(RSManager::get().isPaused()) cappedStep = 0.000000000000000001f;
	DWORD data = *(DWORD*)&cappedStep;

	writeToAddress(&data, convertAddress(ADDR_TS), sizeof(data));
}

// Timer
double getElapsedTime(void) {
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (double)( (c.QuadPart - counterAtStart.QuadPart) * 1000.0 / (double)timerFreq.QuadPart );
}

//----------------------------------------------------------------------------------------
// Hook functions
//----------------------------------------------------------------------------------------

void _stdcall updateFramerate(unsigned int cmd) {	
	// If rendering was performed, update animation step-time
	if((cmd == 2) || (cmd == 5)) {
		// FPS regulation based on previous render
		double maxFPS = (double)Settings::get().getCurrentFPSLimit();
		double minFPS = 10.0f;
		double currentTime = getElapsedTime();
		double deltaTime = currentTime - lastRenderTime;
		lastRenderTime = currentTime;

		// Update step-time
		updateAnimationStepTime((float)deltaTime, (float)minFPS, (float)maxFPS);
	}
}

// Hook
__declspec(naked) void getDrawThreadMsgCommand(void) {
	__asm {
		MOV EAX, [ECX+0Ch] // Put msgCmd in EAX (Return value)
		PUSHAD
		PUSH EAX
		CALL updateFramerate // Call updateFramerate(msgCmd)
		POPAD
		RETN
	}
}

//----------------------------------------------------------------------------------------
// Game Patches
//----------------------------------------------------------------------------------------
void applyFPSPatch() {

	SDLOG(0, "Starting FPS unlock...\n");
#ifndef WITHOUT_GFWL_LIB
	SDLOG(0, "Applying GFWL compatibility\n");
	enableGFWLCompatibility();
#endif

	// Get image info
	MODULEINFO moduleInfo;
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS ntHeader;
    IMAGE_FILE_HEADER header;

	if(GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(moduleInfo)))
	{
		ImageBase = (DWORD)moduleInfo.lpBaseOfDll;
		SDLOG(0, "ImageBase at 0x%08X\n", ImageBase);

		dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
		ntHeader = (PIMAGE_NT_HEADERS)((DWORD)(dosHeader) + (dosHeader->e_lfanew));
		header = ntHeader->FileHeader;
		DWORD TimeStamp = header.TimeDateStamp;
				SDLOG(0, "Executable timestamp: 0x%08X, config: 0x%08X\n", TimeStamp, EXE_TIMESTAMP);

		// Perform pattern matching if timestamp differs
		if (TimeStamp != EXE_TIMESTAMP) {
			SDLOG(0, "Trying pattern matching...\n");

			DWORD address;
			address = GetMemoryAddressFromPattern(NULL, TS_PATTERN, TS_OFFSET);
			if(address != NULL) {
				SDLOG(0, "ADDR_TS found at 0x%08X\n", address);
				ADDR_TS = address;
			}
			else {
				SDLOG(0, "Could not match ADDR_TS pattern, FPS not unlocked\n");
				return;
			}
			address = GetMemoryAddressFromPattern(NULL, PRESINT_PATTERN, PRESINT_OFFSET);
			if(address != NULL) {
				SDLOG(0, "ADDR_PRESINT found at 0x%08X\n", address);
				ADDR_PRESINT = address;
			}
			else {
				SDLOG(0, "Could not match ADDR_PRESINT pattern, FPS not unlocked\n");
				return;
			}
			address = GetMemoryAddressFromPattern(NULL, GETCMD_PATTERN, GETCMD_OFFSET);
			if(address != NULL) {
				SDLOG(0, "ADDR_GETCMD found at 0x%08X\n", address);
				ADDR_GETCMD = address;
			}
			else {
				SDLOG(0, "Could not match ADDR_GETCMD pattern, FPS not unlocked\n");
				return;
			}
			SDLOG(0, "Pattern matching successful\n");
		}
		else
			SDLOG(0, "Using configured addresses\n");
	}
	else
	{
		SDLOG(0, "GetModuleInformation failed, FPS not unlocked\n");
		return;
	}

	// Binary patches
	//--------------------------------------------------------------
	DWORD address;
	DWORD data;

	// Override D3D Presentation Interval
	address = convertAddress(ADDR_PRESINT);
	data = 5; //Set to immediate
	writeToAddress(&data, address, sizeof(data));

	// Detour call to getDrawThreadMsgCommand
	address = convertAddress(ADDR_GETCMD);
	DetourApply((BYTE*)address, (BYTE*)getDrawThreadMsgCommand, 5, CALLOP);
		
	SDLOG(0, "FPS unlocked\n");
}

void initFPSTimer() {
	// Init counter for frame-rate calculations
	lastRenderTime = 0.0f;
	QueryPerformanceFrequency(&timerFreq);
	QueryPerformanceCounter(&counterAtStart);
}
