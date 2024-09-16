#pragma once
#include <stdexcept>

extern "C" __declspec(dllexport) void WINAPI ServiceLalaMain(DWORD argC, LPWSTR* argV);

struct TFuncRes {
    int rs;
    int lasterror;
};