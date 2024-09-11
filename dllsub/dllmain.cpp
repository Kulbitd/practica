// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "dllmain.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

BOOL __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}


int __stdcall USB3MON_Close(HGLOBAL hMem) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}

 
int __stdcall USB3MON_Create(HANDLE hRecipient, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_DeviceChange(int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_ForwardMessage(int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetDeviceInfoA(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetDeviceInfoAW(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetDeviceInfoW(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetHostInfoA(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetHostInfoAW(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetHostInfoW(int, int, int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_GetInterfaceVersion(int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_OnDeviceChange(int, int, int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}
int __stdcall USB3MON_OnQueryEndSession(int) {
    MessageBox(NULL, L"Ваш DLL был подменен", L"HAHHAH", MB_ABORTRETRYIGNORE);
    while (1) {

    }
    return 0;
}