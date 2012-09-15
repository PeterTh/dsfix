#pragma once

#include <Shlobj.h>
#include <ctime>
#include <cstdio>

#include <string>
#include <vector>

using std::vector;
using std::string;

#include "main.h"

class SaveManager {
	static SaveManager instance;
	
	string userSaveFolder;
	time_t lastBackupTime; 

	string getFileNameFromPath(const string& path);
	vector<string> getSaveFiles(const char* ending = ".sl2");
	
	void backup(const time_t curTime);
	void removeOldBackups();

	time_t getLastBackupTime();

public:
	static SaveManager& get() {
		return instance;
	}

	SaveManager() : lastBackupTime(0) {}

	void init();
	void tick();
};

