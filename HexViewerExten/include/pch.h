
#ifndef PCH_H
#define PCH_H

#include "framework.h"
#include <windows.h>
#include <unknwn.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <Shobjidl.h>
#include <Shlobj.h>
#include <shellapi.h>
#include <string>
#include "Reg.h"
#include <strsafe.h>
#include <iostream>
#include <fstream>



void LogMessage(const std::wstring& message);
    



#define INITGUID
#include <guiddef.h>
const CLSID CLSID_HexViewer =
 { 0x1AC8EAF1, 0x9A6A, 0x471C, 0xB8, 0x7B, 0xB1, 0xDF, 0x2B, 0x8E, 0xA6, 0x2F } ;

#endif //PCH_H
