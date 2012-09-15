#include "SaveManager.h"

#include <algorithm>

#include "Settings.h"

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

#define TIMESTAMP_LENGTH 12
#define TIMESTAMP_LENGTH_STR EXPAND_AND_QUOTE(TIMESTAMP_LENGTH)

SaveManager SaveManager::instance;

void SaveManager::init() {
	if(Settings::get().getEnableBackups()) {
		CHAR documents[MAX_PATH];
		HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);
		char buffer[MAX_PATH];
		sprintf_s(buffer, "%s%s", documents, "\\NBGI\\DarkSouls\\*");

		// find user save folder
		WIN32_FIND_DATA userSaveFolderData;
		HANDLE searchHandle = FindFirstFile(buffer, &userSaveFolderData);
		bool found = false;
		if(searchHandle != INVALID_HANDLE_VALUE) {
			do {
				if(strlen(userSaveFolderData.cFileName)>2) {
					sprintf_s(buffer, "%s%s%s", documents, "\\NBGI\\DarkSouls\\", userSaveFolderData.cFileName);
					userSaveFolder = string(buffer);
					SDLOG(0, "SaveManager: user save folder is %s\n", userSaveFolder.c_str());
					found = true;
					break;
				}
			} while(FindNextFile(searchHandle, &userSaveFolderData));
		}
		if(!found) {
			SDLOG(0, "SaveManager: could not determine user save folder\n");
			return;
		}

		removeOldBackups();
	}
}

void SaveManager::tick() {
	if(Settings::get().getEnableBackups()) {
		time_t curTime = time(NULL);
		if(curTime - getLastBackupTime() > Settings::get().getBackupInterval()) {
			backup(curTime);
			lastBackupTime = curTime;
		}
	}
}

time_t SaveManager::getLastBackupTime() {
	if(lastBackupTime == 0) {
		vector<string> backupFiles = getSaveFiles(".bak");
		if(!backupFiles.empty()) {
			string fn = getFileNameFromPath(backupFiles.front());
			sscanf_s(fn.c_str(), "%lu", &lastBackupTime);
		}
	}
	SDLOG(3, "SaveManager: last backup time %ld\n", lastBackupTime);
	return lastBackupTime;
}

vector<string> SaveManager::getSaveFiles(const char* ending /*= ".sl2"*/) {
	SDLOG(2, "SaveManager: searching for files ending on %s\n", ending);
	vector<string> ret;
	// find saved files
	if(userSaveFolder.length() > 0) {
		char buffer[MAX_PATH];
		sprintf_s(buffer, "%s\\*%s", userSaveFolder.c_str(), ending);
		WIN32_FIND_DATA saveFileData;
		HANDLE searchHandle = FindFirstFile(buffer, &saveFileData);
		if(searchHandle != INVALID_HANDLE_VALUE) {
			do {
				char buff2[MAX_PATH];
				sprintf_s(buff2, "%s\\%s", userSaveFolder.c_str(), saveFileData.cFileName);
				ret.push_back(string(buff2));
			} while(FindNextFile(searchHandle, &saveFileData));
		}
		std::sort(ret.begin(), ret.end());
		std::reverse(ret.begin(), ret.end());
		for(size_t i=0; i<ret.size(); ++i) {
			SDLOG(4, "SaveManager: found existing file %s\n", ret[i].c_str());
		}
	}
	return ret;
}

void SaveManager::backup(const time_t curTime) {
	SDLOG(1, "SaveManager: Backing up save files\n");
	char buffer[MAX_PATH];
	vector<string> saveFiles = getSaveFiles();
	for(size_t i=0; i<saveFiles.size(); ++i) {
		string fn = getFileNameFromPath(saveFiles[i]);
		sprintf_s(buffer, "%s\\%0" TIMESTAMP_LENGTH_STR "lu_", userSaveFolder.c_str(), curTime);
		string newPath = string(buffer) + fn + ".bak";
		if(CopyFile(saveFiles[i].c_str(), newPath.c_str(), false) == 0) {
			SDLOG(0, "ERROR: SaveManager failed to back up file! (Copying %s to %s)\n", saveFiles[i].c_str(), buffer);
		} else {
			SDLOG(1, "SaveManager: Backed up %s\n", fn);
		}
	}
	removeOldBackups();
}

void SaveManager::removeOldBackups() {
	vector<string> backupFiles = getSaveFiles(".bak");
	if(Settings::get().getMaxBackups() < backupFiles.size()) {
		SDLOG(1, "SaveManager: Removing %u old backups\n", backupFiles.size() - Settings::get().getMaxBackups());
		for(size_t i=Settings::get().getMaxBackups(); i<backupFiles.size(); ++i) {
			DeleteFile(backupFiles[i].c_str());
		}
	}
}

string SaveManager::getFileNameFromPath(const string& path) {
	size_t pos = path.rfind('\\');
	if(pos != path.npos) {
		pos += 1;
		return path.substr(pos);
	} else {
		SDLOG(0, "ERROR: SaveManager could not extract file name from path %s\n", path.c_str());
	}
	return path;
}
