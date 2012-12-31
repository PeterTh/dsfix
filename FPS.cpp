// Dark Souls FPS fix by Clement Barnier (Nwks)
///////////////////////////////////////////////

#include "FPS.h"

#include <windows.h>

#include "Settings.h"
#include "main.h"
#include "Detouring.h"

void enableGFWLCompatibility(void);

#define JMP32_SZ 5
#define CALL32_SZ 5
#define NOPOP 0x90
#define JMPOP 0xE9
#define CALLOP 0xE8

// Globals
static DWORD originalBase = NULL;
static DWORD imageBase = NULL;

// Hook Globals
float lastRenderTime;
static LARGE_INTEGER timerFreq;
static LARGE_INTEGER counterAtStart;

// Hook Addresses
//------------------------------------
// Time-step value address
#define ADDR_TS	0x012498E0  // 1.0.0 was 0x012497F0, 1.0.1 is 0x012498E0
// Presentation interval address
#define ADDR_PRESINT 0x0102788E // 1.0.0 was 0x010275AE, 1.0.1 is 0x0102788E
// getDrawThreadMsgCommand address in HGCommandDispatcher loop
#define ADDR_GETCMD	0x00BD60ED // 1.0.0 was 0x00BD601D, 1.0.1 is 0x00BD60ED

//----------------------------------------------------------------------------------------
// Support Functions
//----------------------------------------------------------------------------------------

// Misc
//------------------------------------
DWORD getAbsoluteAddress(DWORD offset) {
	if (imageBase)
		return imageBase + offset;
	else
		return NULL;
}

DWORD convertAddress(DWORD Address) {
	return getAbsoluteAddress(Address - originalBase);
}

// Memory
//------------------------------------
void writeToAddress(void* Data, DWORD Address, int Size) {
	DWORD oldProtect;
	VirtualProtect((LPVOID)Address, Size, PAGE_READWRITE, &oldProtect);
	memcpy((void*)Address, Data, Size);
	VirtualProtect((LPVOID)Address, Size, oldProtect, &oldProtect);
}

void updateAnimationStepTime(float stepTime, float minFPS, float maxFPS) {
	float FPS = 1.0f/(stepTime/1000);

	if (FPS < minFPS)
		FPS = minFPS;
	else if (FPS > maxFPS)
		FPS = maxFPS;
	
	float cappedStep = 1/(float)FPS;
	DWORD data = *(DWORD*)&cappedStep;
	writeToAddress(&data, convertAddress(ADDR_TS), sizeof(data));
}

// Timer
float getElapsedTime(void) {
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (float)( (c.QuadPart - counterAtStart.QuadPart) * 1000.0f / (float)timerFreq.QuadPart );
}

// Detour
//------------------------------------
// Make sure to adjust length according to instructions below detoured address!
// Partially overwritten instructions will mess-up disassembly and capacity to debug
void *DetourApply(BYTE *orig, BYTE *hook, int len, BYTE type)
{
	BYTE OP, SZ;

	if (type == JMPOP) {
		OP = JMPOP;
		SZ = JMP32_SZ;
	}
	else if (type == CALLOP) {
		OP = CALLOP;
		SZ = CALL32_SZ;
	}
	else return 0;

	DWORD dwProt = 0;
	BYTE *jmp = (BYTE*)malloc(len+SZ);
	VirtualProtect(orig, len, PAGE_READWRITE, &dwProt);
	memcpy(jmp, orig, len);

	jmp += len; // increment to the end of the copied bytes
	jmp[0] = OP;
	*(DWORD*)(jmp+1) = (DWORD)(orig+len - jmp) - SZ;

	memset(orig, NOPOP, len); 

	orig[0] = OP;
	*(DWORD*)(orig+1) = (DWORD)(hook - orig) - SZ;
	VirtualProtect(orig, len, dwProt, 0);

	return (jmp-len);
}

void DetourRemove(BYTE *src, BYTE *jmp, int len) {
	DWORD dwProt = 0;
	VirtualProtect(src, len, PAGE_READWRITE, &dwProt);
	memcpy(src, jmp, len);
	VirtualProtect(src, len, dwProt, 0);
}

//----------------------------------------------------------------------------------------
// Hook functions
//----------------------------------------------------------------------------------------

void _stdcall updateFramerate(unsigned int cmd) {	
	// If rendering was performed, update animation step-time
	if((cmd == 2) || (cmd == 5)) {
		// FPS regulation based on previous render
		float maxFPS = (float)Settings::get().getCurrentFPSLimit();
		float minFPS = 10.0f;
		float currentTime = getElapsedTime();
		float deltaTime = currentTime - lastRenderTime;
		lastRenderTime = currentTime;

		// Update step-time
		updateAnimationStepTime((float)deltaTime, minFPS, maxFPS);
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
	enableGFWLCompatibility();

	// Get imageBase
	HANDLE exeHandle = NULL;
	originalBase = 0x0400000;
	exeHandle = GetModuleHandle(NULL);

	if(exeHandle != NULL)
		imageBase = (DWORD)exeHandle;

	// Init counter for frame-rate calculations
	lastRenderTime = 0.0f;
	QueryPerformanceFrequency(&timerFreq);
	QueryPerformanceCounter(&counterAtStart);

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
		
	SDLOG(0, "FPS rate unlocked\n");
}
