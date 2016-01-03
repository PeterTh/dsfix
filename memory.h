#ifndef _MEM_H
#define _MEM_H

#include <windows.h>
#include <Psapi.h>

#define JMP32_SZ 5
#define CALL32_SZ 5
#define NOPOP 0x90
#define JMPOP 0xE9
#define CALLOP 0xE8

DWORD GetMemoryAddressFromPattern(LPSTR szDllName, LPCSTR szSearchPattern, DWORD offset);
BOOL PatternEquals(LPBYTE buf, LPWORD pat, DWORD plen);
LPVOID PatternSearch(LPBYTE buf, DWORD blen, LPWORD pat, DWORD plen);
VOID MakeSearchPattern(LPCSTR pString, LPWORD pat);

void writeToAddress(void* Data, DWORD Address, int Size);
void *DetourApply(BYTE *orig, BYTE *hook, int len, BYTE type);
void DetourRemove(BYTE *src, BYTE *restore, const int len);

#endif