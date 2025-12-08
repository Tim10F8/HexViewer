
#include "pch.h"


void LogMessage(const std::wstring& message) {
#ifdef _DEBUG
    wchar_t desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        std::wstring logFilePath = std::wstring(desktopPath) + L"\\ServiceLog.txt";
        std::wofstream logFile(logFilePath, std::ios::app);
        if (logFile.is_open()) {
            logFile << message << std::endl;
            logFile.close();
        }
        else {
            std::cerr << "Failed to open log file." << std::endl;
        }
    }
    else {
        std::cerr << "Failed to get desktop directory." << std::endl;
    }
#endif
}
