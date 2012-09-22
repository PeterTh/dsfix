// Dark Souls FPS fix by Clement Barnier
////////////////////////////////////////

#include "FPS.h"

#include <windows.h>

#include "Settings.h"
#include "main.h"

static DWORD originalBase = NULL;
static DWORD imageBase = NULL;

void enableGFWLCompatibility(void);

void writeToAddress(void* Data, DWORD Address, int Size) {
	DWORD oldProtect;
	VirtualProtect((LPVOID)Address, Size, PAGE_READWRITE, &oldProtect);
	memcpy((void*)Address, Data, Size);
	VirtualProtect((LPVOID)Address, Size, oldProtect, &oldProtect);
}

DWORD getAbsoluteAddress(DWORD offset) {
	if (imageBase)
		return imageBase + offset;
	else
		return NULL;
}

DWORD convertAddress(DWORD Address) {
	return getAbsoluteAddress(Address - originalBase);
}

void applyFPSPatch() {
	enableGFWLCompatibility();

	// Get imageBase
	originalBase = 0x0400000;
	HANDLE exeHandle = GetModuleHandle(NULL);

	if(exeHandle != NULL)
		imageBase = (DWORD)exeHandle;

	SDLOG(0, "FPS patch: image base %p\n", imageBase);

	// Apply patch
	DWORD address;
	BYTE data16;
	DWORD data;
	DWORD64 data64;
	
	// Desired max. FPS
	double FPS = Settings::get().getMaxFPS();

	// Gameplay FPS patch
	address = convertAddress(0x012497F0);
	float dt = 1/(float)FPS;
	data = *(DWORD*)&dt;
	writeToAddress(&data, address, sizeof(data));

	// Target FPS patch (display ?)
	address = convertAddress(0x01249958);
	data64 = *(DWORD64*)&FPS;
	writeToAddress(&data64, address, sizeof(data64));

	// Max FPS patch
	address = convertAddress(0x00644BBA);
	data16 = (BYTE)FPS;
	writeToAddress(&data16, address, sizeof(data16));

	// Override FPS Divider		
	address = convertAddress(0x010275AE); 
	data = 1;
	writeToAddress(&data, address, sizeof(data));
	
	SDLOG(0, "FPS patch: patched to %4.1lf maximum FPS\n", FPS);
}
