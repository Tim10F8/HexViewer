#pragma once

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "dwmapi.lib")
typedef HWND NativeWindow;
#elif __APPLE__
typedef void* NativeWindow;
#else
typedef unsigned long NativeWindow;  
#endif

struct AppOptions {
  bool darkMode;
  int defaultBytesPerLine; 
  bool autoReload;
  bool contextMenu;  
};

inline bool g_isNative = false;
void DetectNative();

class OptionsDialog {
public:
#ifdef _WIN32
  static bool Show(HWND parent, AppOptions& options);
#else
  static bool Show(NativeWindow parent, AppOptions& options);
#endif
};
