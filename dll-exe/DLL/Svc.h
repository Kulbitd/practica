#pragma once
#ifdef DLL1_EXPORTS
#define MATHLIBRARY_API __declspec(dllexport)
#else
#define MATHLIBRARY_API __declspec(dllimport)
#endif

#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <lm.h>

extern "C" MATHLIBRARY_API VOID WINAPI LalaMain(const std::string& search_text);