// DSfix Copyright 2012 Peter Thoman (Durante)
// thanks to Azorbix's D3D Starter Kit as well as 
// Matthew Fisher's Direct3D 9 API Interceptor project
// for providing examples of D3D interception

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>

#pragma once

#define VERSION "2.4"

#define RELEASE_VER

#define WITHOUT_GFWL_LIB

#ifndef RELEASE_VER
#define SDLOG(_level, _str, ...) if(Settings::get().getLogLevel() > _level) { sdlog(_str, __VA_ARGS__); }
#else
#define SDLOG(_level, _str, ...) {}
#endif
#define SAFERELEASE(_p) { if(_p) { (_p)->Release(); (_p) = NULL; } }
#define SAFEDELETE(_p) { if(_p) { delete (_p); (_p) = NULL; } }

#include "d3d9.h"
#include "dinput.h"
#include <string>

char *GetDirectoryFile(const char *filename);
bool fileExists(const char *filename);
void createDirectory(const char *filename);
bool writeFile(const char *filename, const char *data, size_t length);
std::string formatMessage(DWORD messageId);
std::string strError(int err);
void __cdecl sdlogtime();
void __cdecl sdlog(const char * fmt, ...);
void errorExit(LPTSTR lpszFunction);

extern bool timingIntroMode;

typedef IDirect3D9 *(APIENTRY *tDirect3DCreate9)(UINT);
extern tDirect3DCreate9 oDirect3DCreate9;

typedef HRESULT (WINAPI *tDirectInput8Create)(HINSTANCE inst_handle, DWORD version, const IID& r_iid, LPVOID* out_wrapper, LPUNKNOWN p_unk);
extern tDirectInput8Create oDirectInput8Create;
