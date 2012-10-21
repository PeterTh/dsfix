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
static DWORD HighGraphics;
static DWORD TaskFuncPtr;
static LARGE_INTEGER timerFreq;
static LARGE_INTEGER counterAtStart;

//----------------------------------------------------------------------------------------
//Functions
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
		stepTime = minFPS;
	else if (stepTime > maxFPS)
		FPS = maxFPS;
	
	float cappedStep = 1/(float)FPS;
	DWORD data = *(DWORD*)&cappedStep;
	writeToAddress(&data, convertAddress(0x012497F0), sizeof(data));
}

// Timer
float getElapsedTime(void) {
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (float)( (c.QuadPart - counterAtStart.QuadPart) * 1000.0f / (float)timerFreq.QuadPart );
}

// Detour
//------------------------------------
//Make sure to adjust length according to instructions below detoured address!
//Partially overwritten instructions will mess-up disassembly and capacity to debug
void *DetourApply(BYTE *orig, BYTE *hook, int len, int type) {
	BYTE OP, SZ;

	if (type == 0) {
		OP = JMPOP;
		SZ = JMP32_SZ;
	}
	else if(type == 1) {
		OP = CALLOP;
		SZ = CALL32_SZ;
	}

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
//Game hooks
//----------------------------------------------------------------------------------------

// Render Loop
//----------------------------------------------------------------------------------------
void renderLoop(void) {
	bool run = true;
	DWORD threadID = GetCurrentThreadId();

	int taskFunc;
	DWORD task;

	//Pointers conversion
	DWORD pGetTaskF = convertAddress(0x00577330);
	DWORD pCleanTaskF = convertAddress(0x00577450);

	QueryPerformanceFrequency(&timerFreq);
	QueryPerformanceCounter(&counterAtStart);
	float lastTime = 0.0f;

	// Render loop
	do {
		// Get Job & state
		_asm { 
			PUSH 1
			MOV ECX, HighGraphics
			CALL pGetTaskF				//Get task pointer
			MOV task, EAX				//Store task pointer (EAX)
			MOV ECX, [EAX+0x0C]			//Get task function
			MOV taskFunc, ECX			//Store function
		}
		if (task == (HighGraphics+0x08))	//No task
			break;		

		switch (taskFunc) {
			// Exit loop
		case 0:
			_asm {
				MOV EDI, TaskFuncPtr
				MOV EAX, [EDI]
				MOV EDX, [EAX+0x0C]
				MOV ECX, EDI
				CALL EDX
			}
			run = false;
			break;
			// Start Watch-dog Thread
		case 1:
			_asm {
				MOV ESI, task
				MOV EDI, TaskFuncPtr
				MOV ECX, [ESI+0x2C]
				MOV EDX, [ESI+0x28]
				MOV EAX, [EDI]
				PUSH ECX
				MOV ECX, [ESI+0x24]
				PUSH EDX
				MOV EDX, [EAX+0x08]
				PUSH ECX
				MOV ECX, EDI
				CALL EDX
			}
			break;
			// Update and/or Render
		case 2: 
			_asm {
				MOV ESI, task
				MOV EDI, TaskFuncPtr
				MOV ECX, [ESI+0x24]
				MOV EAX, [EDI]
				MOV EDX, [EAX+0x10]
				PUSH ECX
				MOV ECX, EDI
				CALL EDX
			}
			break;
			// Do nothing (was debug log)
		case 3:
			_asm {
				MOV EDI, TaskFuncPtr
				MOV EAX, [EDI]
				MOV EDX, [EAX+0x14]
				MOV ECX, TaskFuncPtr
				CALL EDX
			}
			break;
			// Do nothing (was debug log)		
		case 4:
			_asm {
				MOV EDI, TaskFuncPtr
				MOV EAX, [EDI]
				MOV EDX, [EAX+0x18]
				MOV ECX, TaskFuncPtr
				CALL EDX
			}
			break;
			// Force Render (caused by watchdog timeout)
		case 5:
			_asm {
				MOV EDI, TaskFuncPtr
				MOV EAX, [EDI]
				MOV EDX, [EAX+0x1C]
				MOV ECX, TaskFuncPtr
				CALL EDX
			}
			break;
		default:
			break;
		}

		// If rendering was performed, update animation step-time
		if((taskFunc == 2) || (taskFunc == 5)) {
			// FPS regulation
			float maxFPS = (float)Settings::get().getCurrentFPSLimit();
			float minFPS = 10.0f;
			float currentTime = getElapsedTime();
			float deltaTime = currentTime - lastTime;

			// Update step-time
			updateAnimationStepTime((float)deltaTime, minFPS, maxFPS);

			lastTime = currentTime;
		}

		// Task cleanup
		_asm {
			MOV ESI, task
			PUSH ESI
			MOV ECX, HighGraphics
			CALL pCleanTaskF
		}

	} while (run);
}

// Render Entry
//----------------------------------------------------------------------------------------
__declspec(naked) void renderLoopEntry(void) {
#define LOCALOFFSET __LOCAL_SIZE
	// Prologue
	_asm {
		// Create Stack frame
		PUSH EBX
		PUSH EBP
		MOV EBP, ESP
		SUB ESP, LOCALOFFSET			
		// Retrieve stack parameters
		MOV EBX, [EBP+0x0C]
		PUSH ESI
		PUSH EDI
		// Store parameters to globals
		MOV HighGraphics, EBX		//HighGraphics: EBP
		MOV TaskFuncPtr, ECX		//Pointer to functions structure: EDI
	}

	// Start Recorder Process
	renderLoop();

	// Epilogue
	__asm {
		POP EDI
		POP ESI
		MOV ESP, EBP
		POP EBP
		POP EBX
		RETN 4
	}
}

void applyFPSPatch() {
	enableGFWLCompatibility();

	// Get imageBase
	HANDLE exeHandle = NULL;
	originalBase = 0x0400000;
	exeHandle = GetModuleHandle(NULL);

	if(exeHandle != NULL)
		imageBase = (DWORD)exeHandle;

	// Patches
	//--------------------------------------------------------------
	DWORD address;
	DWORD data;

	// Override D3D Presentation Interval
	address = convertAddress(0x010275AE); 
	data = 5; //Set to immediate
	writeToAddress(&data, address, sizeof(data));

	// Detour Render loop entry
	address = convertAddress(0x00BD6000);
	DetourApply((BYTE*)address, (BYTE*)renderLoopEntry, 6, 0);
		
	SDLOG(0, "FPS rate unlocked\n");
}
