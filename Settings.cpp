#include "Settings.h"

#include <fstream>
#include <string>

#include "main.h"
#include "WindowManager.h"

Settings Settings::instance;

void Settings::load() {
	std::ifstream sfile;
	sfile.open(GetDirectoryFile("DSfix.ini"), std::ios::in);
	char buffer[128];
	while(!sfile.eof()) {
		sfile.getline(buffer, 128);
		if(buffer[0] == '#') continue;
		if(sfile.gcount() <= 1) continue;
		std::string bstring(buffer);

		#define SETTING(_type, _var, _inistring, _defaultval) \
		if(bstring.find(_inistring) == 0) { \
			read(buffer + strlen(_inistring) + 1, _var); \
		}
		#include "Settings.def"
		#undef SETTING
	}
	sfile.close();
	
	if(getBackupInterval() < 600) {
		BackupInterval = 600;
	}

	if(getOverrideLanguage().length() >= 2 && getOverrideLanguage().find("none") != 0) {
		performLanguageOverride();
	}

	curFPSlimit = getFPSLimit();
}

void Settings::report() {
	SDLOG(0, "= Settings read:\n");
	#define SETTING(_type, _var, _inistring, _defaultval) \
	log(_inistring, _var);
	#include "Settings.def"
	#undef SETTING
	SDLOG(0, "=============\n");
}

void Settings::init() {
	if(!inited) {
		if(getDisableCursor()) WindowManager::get().toggleCursorVisibility();
		if(getCaptureCursor()) WindowManager::get().toggleCursorCapture();
		if(getBorderlessFullscreen()) WindowManager::get().toggleBorderlessFullscreen();

		inited = true;
	}
}

void Settings::shutdown() {
	if(inited) {
		undoLanguageOverride();
		inited = false;
	}
}

unsigned Settings::getCurrentFPSLimit() {
	return curFPSlimit;
}
void Settings::setCurrentFPSLimit(unsigned limit) {
	curFPSlimit = limit;
}
void Settings::toggle30FPSLimit() {
	if(curFPSlimit == 30) curFPSlimit = getFPSLimit();
	else curFPSlimit = 30;
}


// reading --------------------------------------------------------------------

void Settings::read(char* source, bool& value) {
	std::string ss(source);
	if(ss.find("true")==0 || ss.find("1")==0 || ss.find("TRUE")==0 || ss.find("enable")==0) value = true;
	else value = false;
}

void Settings::read(char* source, int& value) {
	sscanf_s(source, "%d", &value);
}

void Settings::read(char* source, unsigned& value) {
	sscanf_s(source, "%u", &value);
}

void Settings::read(char* source, float& value) {
	sscanf_s(source, "%f", &value);
}

void Settings::read(char* source, std::string& value) {
	value.assign(source);
}


// logging --------------------------------------------------------------------

void Settings::log(const char* name, bool value) {
	SDLOG(0, " - %s : %s\n", name, value ? "true" : "false");
}

void Settings::log(const char* name, int value) {
	SDLOG(0, " - %s : %d\n", name, value);
}

void Settings::log(const char* name, unsigned value) {
	SDLOG(0, " - %s : %u\n", name, value);
}

void Settings::log(const char* name, float value) {
	SDLOG(0, " - %s : %f\n", name, value);
}

void Settings::log(const char* name, const std::string& value) {
	SDLOG(0, " - %s : %s\n", name, value.c_str());
}

// language override --------------------------------------------------------------------

void Settings::performLanguageOverride() {
	HKEY key;
	// Reading operations
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\International", 0, KEY_READ, &key) != ERROR_SUCCESS) {
		SDLOG(0, "ERROR opening language registry key for reading\n");
		return;
	}
	BYTE prevLang[16]; // previous locale registry key
	DWORD prevLangSize;
	// check if prev key already set -- if so, assume correct override and return
	if(RegQueryValueEx(key, "PrevLocaleName", 0, 0, prevLang, &prevLangSize) == ERROR_SUCCESS) {
		RegCloseKey(key);
		return;
	}
	// read current locale
	if(RegQueryValueEx(key, "LocaleName", 0, 0, prevLang, &prevLangSize) != ERROR_SUCCESS) {
		RegCloseKey(key);
		SDLOG(0, "ERROR reading from language registry key\n");
		return;
	}
	// if locale already set: no override necessary
	if(getOverrideLanguage().find((char*)prevLang) == 0) {
		RegCloseKey(key);
		SDLOG(0, "Language set to %s\n", prevLang);
		return;
	}
	RegFlushKey(key);
	RegCloseKey(key);
	
	// Writing operations
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\International", 0, KEY_WRITE, &key) != ERROR_SUCCESS) {
		SDLOG(0, "ERROR opening language registry key for writing\n");
		return;
	}
	// store previous locale
	if(RegSetValueEx(key, "PrevLocaleName", 0, REG_SZ, prevLang, prevLangSize) != ERROR_SUCCESS) {
		RegCloseKey(key);
		SDLOG(0, "ERROR setting previous language registry key\n");
		return;
	}
	// override existing locale
	if(RegSetValueEx(key, "LocaleName", 0, REG_SZ, (BYTE*)getOverrideLanguage().c_str(), getOverrideLanguage().length()+1) != ERROR_SUCCESS) {
		RegCloseKey(key);
		SDLOG(0, "ERROR setting language registry key\n");
		return;
	}
	SDLOG(0, "Set Language key to %s, stored previous value %s\n", getOverrideLanguage().c_str(), prevLang);
	RegFlushKey(key);
	RegCloseKey(key);
}

void Settings::undoLanguageOverride() {
	HKEY key;
	// reading operations
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\International", 0, KEY_READ, &key) != ERROR_SUCCESS) {
		SDLOG(0, "ERROR opening language registry key for reading (restore)\n");
		return;
	}
	BYTE prevLang[16]; // previous locale registry key
	DWORD prevLangSize;
	// load previous locale
	if(RegQueryValueEx(key, "PrevLocaleName", 0, 0, prevLang, &prevLangSize) != ERROR_SUCCESS) {
		RegCloseKey(key);
		SDLOG(0, "ERROR reading previous locale from language registry key (restore)\n");
		return;
	}
	RegFlushKey(key);
	RegCloseKey(key);

	// Writing operations
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\International", 0, KEY_WRITE, &key) != ERROR_SUCCESS) {
		SDLOG(0, "ERROR opening language registry key for restoring\n");
		return;
	}
	// restore previous locale
	if(RegSetValueEx(key, "LocaleName", 0, REG_SZ, prevLang, prevLangSize) != ERROR_SUCCESS) {
		RegCloseKey(key);
		SDLOG(0, "ERROR restoring language registry key\n");
		return;
	}
	// remove PrevLocaleName value
	if(RegDeleteValue(key, "PrevLocaleName") != ERROR_SUCCESS) {
		SDLOG(0, "ERROR deleting PrevLocaleName registry key\n");
	}
	SDLOG(0, "Restored previous language value %s\n", prevLang);
	RegFlushKey(key);
	RegCloseKey(key);
}
