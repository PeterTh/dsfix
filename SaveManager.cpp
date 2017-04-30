#include "SaveManager.h"

#include <algorithm>
#include <functional>

#include "Settings.h"

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

#define TIMESTAMP_LENGTH 12
#define TIMESTAMP_LENGTH_STR EXPAND_AND_QUOTE(TIMESTAMP_LENGTH)

#define SAVE_EXT ".sl2"
#define BACKUP_EXT ".bak"

string getFileNameFromPath(const string& path) {
    size_t pos = path.rfind('\\');
    if (pos != path.npos) {
        return path.substr(pos + 1);
    }
    else {
        SDLOG(0, "ERROR: SaveManager could not extract file name from path %s\n", path.c_str());
    }
    return path;
}

bool operator>(const FILETIME& lhs, const FILETIME& rhs) {
    ULARGE_INTEGER uiL, uiR;

    uiL.LowPart = lhs.dwLowDateTime;
    uiL.HighPart = lhs.dwHighDateTime;

    uiR.LowPart = rhs.dwLowDateTime;
    uiR.HighPart = rhs.dwHighDateTime;

    return uiL.QuadPart > uiR.QuadPart;
}

bool fileDataWriteTimeCompareGreater(const WIN32_FIND_DATA& lhs, const WIN32_FIND_DATA& rhs) {
    return lhs.ftLastWriteTime > rhs.ftLastWriteTime;
}

time_t fileTimeToEpoch(const FILETIME& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

string joinPath(const string& base, const string& comp) {
    char buffer[MAX_PATH];
    sprintf_s(buffer, "%s\\%s", base.c_str(), comp.c_str());
    return string(buffer);
}

SaveManager SaveManager::instance;

void SaveManager::init() {
    if (Settings::get().getEnableBackups()) {
        char documents[MAX_PATH];
        HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);

        if (hr == S_OK)
        {
            userSaveFolder = joinPath(documents, "NBGI\\DarkSouls");
            SDLOG(0, "SaveManager: Found save folder at %s\n", userSaveFolder);
            removeOldBackups();
        }
    }
}

void SaveManager::tick() {
    if (Settings::get().getEnableBackups() && !userSaveFolder.empty()) {
        time_t ls, curTime = time(NULL);
        time_t backupInterval = Settings::get().getBackupInterval();
        if (curTime - getLastBackupTime() > backupInterval &&
            getLastSaveTime() - getLastBackupSaveTime() > backupInterval) {
            SDLOG(2, "SaveManager: last backup time %ld\n", lastBackupTime);
            SDLOG(2, "SaveManager: last backup save time %ld\n", lastBackupSaveTime);
            backup();
        }
    }
}

time_t SaveManager::getLastBackupTime() {
    if (lastBackupTime == 0) {
        vector<WIN32_FIND_DATA> backupFiles = getSaveFiles(BACKUP_EXT);
        if (!backupFiles.empty()) {
            lastBackupTime = lastBackupSaveTime = fileTimeToEpoch(backupFiles[0].ftLastWriteTime);
        }
    }
    return lastBackupTime;
}

time_t SaveManager::getLastBackupSaveTime()
{
    if (lastBackupSaveTime == 0) {
        vector<WIN32_FIND_DATA> backupFiles = getSaveFiles(BACKUP_EXT);
        if (!backupFiles.empty()) {
            lastBackupSaveTime = fileTimeToEpoch(backupFiles[0].ftLastWriteTime);
        }
    }
    return lastBackupSaveTime;
}

time_t SaveManager::getLastSaveTime()
{
    vector<WIN32_FIND_DATA> saveFiles = getSaveFiles(SAVE_EXT);
    return saveFiles.empty() ? 0 : fileTimeToEpoch(saveFiles[0].ftLastWriteTime);
}

vector<WIN32_FIND_DATA> SaveManager::getSaveFiles(const char* ending) {
    vector<WIN32_FIND_DATA> ret;

    if (!userSaveFolder.empty()) {
        SDLOG(4, "SaveManager: searching for files ending with %s\n", ending);

        char buffer[MAX_PATH];
        sprintf_s(buffer, "%s\\*%s", userSaveFolder.c_str(), ending);

        WIN32_FIND_DATA saveFileData;
        HANDLE sh = FindFirstFile(buffer, &saveFileData);
        if (sh != INVALID_HANDLE_VALUE)
        {
            do
            {
                ret.push_back(saveFileData);
                SDLOG(3, "SaveManager: found save file %s\\%s\n", userSaveFolder.c_str(), saveFileData.cFileName);
            } while (FindNextFile(sh, &saveFileData));
            std::sort(ret.begin(), ret.end(), fileDataWriteTimeCompareGreater);
        }
    }
    return ret;
}

void SaveManager::backup() {
    SDLOG(3, "SaveManager: Backing up save files\n");

    vector<WIN32_FIND_DATA> saveFiles = getSaveFiles(SAVE_EXT);

    if (!saveFiles.empty()) {
        WIN32_FIND_DATA saveFile = saveFiles[0];
        string saveFullPath = joinPath(userSaveFolder, saveFile.cFileName);

        char newPath[MAX_PATH];
        time_t curTime = time(NULL);
        sprintf_s(newPath, "%s\\%0" TIMESTAMP_LENGTH_STR "llu_%s" BACKUP_EXT, userSaveFolder.c_str(), curTime, saveFile.cFileName);

        if (CopyFile(saveFullPath.c_str(), newPath, false) == 0) {
            SDLOG(0, "ERROR: SaveManager failed to back up file! (Copying %s to %s)\n", saveFullPath.c_str(), newPath);
        }
        else {
            SDLOG(1, "SaveManager: Backed up %s to %s\n", saveFullPath.c_str(), newPath);
            lastBackupTime = curTime;
            lastBackupSaveTime = fileTimeToEpoch(saveFile.ftLastWriteTime);
            removeOldBackups();
        }
    }
}

void SaveManager::removeOldBackups() {
    vector<WIN32_FIND_DATA> backupFiles = getSaveFiles(BACKUP_EXT);
    if (Settings::get().getMaxBackups() < backupFiles.size()) {
        SDLOG(1, "SaveManager: Removing %u old backups\n", backupFiles.size() - Settings::get().getMaxBackups());
        for (size_t i = Settings::get().getMaxBackups(); i < backupFiles.size(); ++i) {
            DeleteFile(joinPath(userSaveFolder, backupFiles[i].cFileName).c_str());
        }
    }
}
