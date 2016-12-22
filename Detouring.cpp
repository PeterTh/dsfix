#include "Detouring.h"

#include <Windows.h>

#include "Settings.h"
#include "RenderstateManager.h"

bool timingIntroMode = false;

DWORD WINAPI DetouredSleepEx(DWORD dwMilliseconds, BOOL bAlertable) {
	//SDLOG(12, "T %6lu: Detouring: Sleep for %lu ms\n", GetCurrentThreadId(), dwMilliseconds);
	//return TrueSleepEx(dwMilliseconds, bAlertable);
	return 0;
}

#include <mmsystem.h>

static DWORD timeIncrease;
DWORD (WINAPI * TrueTimeGetTime)(void) = timeGetTime;
DWORD WINAPI DetouredTimeGetTime() {
	SDLOG(13, "T %6lu: Detouring: TimeGetTime - real: %10lu, returned: %10lu\n", GetCurrentThreadId(), TrueTimeGetTime(), TrueTimeGetTime() + timeIncrease);
	//timeIncrease += 16;
	return TrueTimeGetTime() + timeIncrease;
}

static LARGE_INTEGER perfCountIncrease, countsPerSec;
BOOL (WINAPI * TrueQueryPerformanceCounter)(_Out_ LARGE_INTEGER *lpPerformanceCount) = QueryPerformanceCounter;
BOOL WINAPI DetouredQueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount) {
	void *traces[128];
	DWORD hash;
	int captured = CaptureStackBackTrace(0, 128, traces, &hash);
	SDLOG(14, "T %6lu: Detouring: QueryPerformanceCounter, stack depth %3d, hash %20ul\n", GetCurrentThreadId(), captured, hash);
	BOOL ret = TrueQueryPerformanceCounter(lpPerformanceCount);
	if(timingIntroMode && captured < 3) {
		perfCountIncrease.QuadPart += countsPerSec.QuadPart/50;
	}
	lpPerformanceCount->QuadPart += perfCountIncrease.QuadPart;
	return ret;
}

typedef HRESULT (WINAPI * D3DXCreateTexture_FNType)(_In_ LPDIRECT3DDEVICE9 pDevice, _In_ UINT Width, _In_ UINT Height, _In_ UINT MipLevels, _In_ DWORD Usage, _In_ D3DFORMAT Format, _In_ D3DPOOL Pool, _Out_ LPDIRECT3DTEXTURE9 *ppTexture);
D3DXCreateTexture_FNType TrueD3DXCreateTexture = D3DXCreateTexture;
HRESULT WINAPI DetouredD3DXCreateTexture(_In_ LPDIRECT3DDEVICE9 pDevice, _In_ UINT Width, _In_ UINT Height, _In_ UINT MipLevels, _In_ DWORD Usage, _In_ D3DFORMAT Format, _In_ D3DPOOL Pool, _Out_ LPDIRECT3DTEXTURE9 *ppTexture) {
	SDLOG(4, "DetouredD3DXCreateTexture\n");
	HRESULT res = TrueD3DXCreateTexture(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
	return res;
}

typedef HRESULT (WINAPI * D3DXCreateTextureFromFileInMemory_FNType)(_In_ LPDIRECT3DDEVICE9 pDevice, _In_ LPCVOID pSrcData, _In_ UINT SrcDataSize, _Out_ LPDIRECT3DTEXTURE9 *ppTexture);
D3DXCreateTextureFromFileInMemory_FNType TrueD3DXCreateTextureFromFileInMemory = D3DXCreateTextureFromFileInMemory;
HRESULT WINAPI DetouredD3DXCreateTextureFromFileInMemory(_In_ LPDIRECT3DDEVICE9 pDevice, _In_ LPCVOID pSrcData, _In_ UINT SrcDataSize, _Out_ LPDIRECT3DTEXTURE9 *ppTexture) {
	SDLOG(4, "DetouredD3DXCreateTextureFromFileInMemory\n");
	HRESULT res = TrueD3DXCreateTextureFromFileInMemory(pDevice, pSrcData, SrcDataSize, ppTexture);
	RSManager::get().registerD3DXCreateTextureFromFileInMemory(pSrcData, SrcDataSize, *ppTexture);
	return res;
}

typedef HRESULT (WINAPI * D3DXCreateTextureFromFileInMemoryEx_FNType)(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture);
D3DXCreateTextureFromFileInMemoryEx_FNType TrueD3DXCreateTextureFromFileInMemoryEx = D3DXCreateTextureFromFileInMemoryEx;
HRESULT WINAPI DetouredD3DXCreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, 
														   DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture) {
	SDLOG(4, "DetouredD3DXCreateTextureFromFileInMemoryEx\n");
	HRESULT res = RSManager::get().redirectD3DXCreateTextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	RSManager::get().registerD3DXCreateTextureFromFileInMemory(pSrcData, SrcDataSize, *ppTexture);
	return res;
}

typedef HRESULT (WINAPI * D3DXCompileShader_FNType)(_In_ LPCSTR pSrcData, _In_ UINT srcDataLen, _In_ const D3DXMACRO *pDefines, _In_ LPD3DXINCLUDE pInclude, _In_ LPCSTR pFunctionName, _In_ LPCSTR pProfile, _In_ DWORD Flags, _Out_ LPD3DXBUFFER *ppShader, _Out_ LPD3DXBUFFER *ppErrorMsgs, _Out_ LPD3DXCONSTANTTABLE *ppConstantTable);
D3DXCompileShader_FNType TrueD3DXCompileShader = D3DXCompileShader;
HRESULT WINAPI DetouredD3DXCompileShader(_In_ LPCSTR pSrcData, _In_ UINT srcDataLen, _In_ const D3DXMACRO *pDefines, _In_ LPD3DXINCLUDE pInclude, _In_ LPCSTR pFunctionName, _In_ LPCSTR pProfile, 
										 _In_ DWORD Flags, _Out_ LPD3DXBUFFER *ppShader, _Out_ LPD3DXBUFFER *ppErrorMsgs, _Out_ LPD3DXCONSTANTTABLE *ppConstantTable) {
	HRESULT res = TrueD3DXCompileShader(pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable);
	RSManager::get().registerD3DXCompileShader(pSrcData, srcDataLen, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable);
	return res;
}

void hookFunction(const char* name, const char* dllname, void** ppTarget, void* const pDetour, void** ppOriginal) {
	HMODULE dllHandle = GetModuleHandle(dllname);
	*ppTarget = GetProcAddress(dllHandle, name);
	MH_STATUS ret = MH_CreateHook(*ppTarget, pDetour, ppOriginal);
	if (ret == MH_OK) {
		SDLOG(2, "MH_CreateHook for %s in %s succeeded\n", name, dllname);
	}
	else {
		SDLOG(0, "MH_CreateHook for %s in %s failed\n", name, dllname);
		SDLOG(0, "dllHandle = %p\n", dllHandle);
		SDLOG(0, "*ppTarget = %p\n", *ppTarget);
	}
	ret = MH_EnableHook(*ppTarget);
	if (ret == MH_OK) {
		SDLOG(2, "MH_EnableHook succeeded\n");
	}
	else {
		SDLOG(0, "MH_EnableHook failed\n");
	}
}

namespace {
	void* TargetDirect3DCreate9;
	void* TargetQueryPerformanceCounter;
	void* TargetD3DXCreateTextureFromFileInMemory;
	void* TargetD3DXCreateTextureFromFileInMemoryEx;
}

void earlyDetour() {
	QueryPerformanceFrequency(&countsPerSec);
	MH_Initialize();
	hookFunction("Direct3DCreate9",
		"d3d9.dll",
		&TargetDirect3DCreate9,
		&hkDirect3DCreate9,
		reinterpret_cast<void**>(&oDirect3DCreate9));
}

void startDetour() {
	if (Settings::get().getSkipIntro()) {
		hookFunction("QueryPerformanceCounter",
			"kernel32.dll",
			&TargetQueryPerformanceCounter,
			&DetouredQueryPerformanceCounter,
			reinterpret_cast<void**>(&TrueQueryPerformanceCounter));
	}
	hookFunction("D3DXCreateTextureFromFileInMemory",
		"d3dx9_43.dll",
		&TargetD3DXCreateTextureFromFileInMemory,
		&DetouredD3DXCreateTextureFromFileInMemory,
		reinterpret_cast<void**>(&TrueD3DXCreateTextureFromFileInMemory));
	hookFunction("D3DXCreateTextureFromFileInMemoryEx",
		"d3dx9_43.dll",
		&TargetD3DXCreateTextureFromFileInMemoryEx,
		&DetouredD3DXCreateTextureFromFileInMemoryEx,
		reinterpret_cast<void**>(&TrueD3DXCreateTextureFromFileInMemoryEx));
}

void endDetour() {
	MH_RemoveHook(TargetDirect3DCreate9);
	MH_RemoveHook(TargetQueryPerformanceCounter);
	MH_RemoveHook(TargetD3DXCreateTextureFromFileInMemory);
	MH_RemoveHook(TargetD3DXCreateTextureFromFileInMemoryEx);
	MH_Uninitialize();
}
