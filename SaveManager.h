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
	time_t lastBackupSaveTime;

	vector<WIN32_FIND_DATA> getSaveFiles(const char* ending);

	void removeOldBackups();

	time_t getLastBackupTime();
	time_t getLastBackupSaveTime();
	time_t getLastSaveTime();

public:
	static SaveManager& get() {
		return instance;
	}

	SaveManager() : lastBackupTime(0) {}

	void init();
	void tick();
	void backup();
};

