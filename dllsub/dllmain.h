#pragma once
#ifdef DLL2_EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

extern "C" DLL BOOL __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
extern "C" DLL int __stdcall USB3MON_Close(HGLOBAL hMem);
extern "C" DLL int __stdcall USB3MON_Create(HANDLE hRecipient, int, int);
extern "C" DLL int __stdcall USB3MON_DeviceChange(int, int);
extern "C" DLL int __stdcall USB3MON_ForwardMessage(int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetDeviceInfoA(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetDeviceInfoAW(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetDeviceInfoW(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetHostInfoA(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetHostInfoAW(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetHostInfoW(int, int, int, int, int);
extern "C" DLL int __stdcall USB3MON_GetInterfaceVersion(int, int);
extern "C" DLL int __stdcall USB3MON_OnDeviceChange(int, int, int);
extern "C" DLL int __stdcall USB3MON_OnQueryEndSession(int);