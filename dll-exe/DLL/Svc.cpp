#include "Svc.h"

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "mpr.lib")

const std::wstring f_file = L"\\*";
std::vector<std::wstring> result = {};
std::vector<std::wstring> compName = {};


void find_text_in_file(const std::wstring& file_path, const std::string& search_text)
{
	DWORD bytes_read;
	const DWORD buffer_size = 1024;
	char* buffer = new char[1024];

	HANDLE hFiler = CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFiler) {
		return;
	}
	while (ReadFile(hFiler, buffer, buffer_size, &bytes_read, NULL) && bytes_read > 0)
	{
		std::stringstream ss;
		ss << buffer;
		std::string st = ss.str();
		size_t pos = st.find(search_text);
		if (pos != std::string::npos) {
			result.push_back(file_path);
			break;
		}
	}
	CloseHandle(hFiler);
}

void EnumerateNetworkResources(NETRESOURCE* lpnr) {
	HANDLE hEnum;
	DWORD dwResult, dwResultEnum;
	DWORD cbBuffer = 16384;
	DWORD cEntries = -1; 
	LPNETRESOURCE lpnrLocal;
	DWORD i;

	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, lpnr, &hEnum);

	if (dwResult != NO_ERROR) {
		return;
	}

	lpnrLocal = (LPNETRESOURCE)GlobalAlloc(GPTR, cbBuffer);

	if (lpnrLocal == NULL) {
		return;
	}

	do {
		dwResultEnum = WNetEnumResource(hEnum, &cEntries, lpnrLocal, &cbBuffer);

		if (dwResultEnum == NO_ERROR) {
			for (i = 0; i < cEntries; i++) {
				if (lpnrLocal[i].dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) {
					compName.push_back(lpnrLocal[i].lpRemoteName);
				}

				if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER)) {
					EnumerateNetworkResources(&lpnrLocal[i]);
				}
			}
		}
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
			break;
		}
	} while (dwResultEnum != ERROR_NO_MORE_ITEMS);

	GlobalFree((HGLOBAL)lpnrLocal);
	WNetCloseEnum(hEnum);
}

void reverse_files(const std::wstring& directory, const std::wstring& file_name, const std::string& search_text)
{
	WIN32_FIND_DATA find_file_data;
	HANDLE h_find = FindFirstFileW((directory + file_name).c_str(), &find_file_data);
	if (h_find == INVALID_HANDLE_VALUE)
		return;
	do
	{

		if (!wcscmp(find_file_data.cFileName,L".") || !wcscmp(find_file_data.cFileName, L".."))
			continue;
		if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_REPARSE_POINT)
			continue;
		if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY or find_file_data.dwFileAttributes == 17) // 17 = 16+1 dir + redonly
			reverse_files((directory + L"\\" + find_file_data.cFileName), f_file, search_text);
		else
			find_text_in_file((directory + L"\\" + find_file_data.cFileName).c_str(), search_text);
	} while (FindNextFileW(h_find, &find_file_data) != 0);
	FindClose(h_find);
}


VOID WINAPI LalaMain(const std::string& search_text)
{
	NETRESOURCE nr;
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwType = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	nr.dwUsage = RESOURCEUSAGE_CONTAINER;
	nr.lpLocalName = NULL;
	nr.lpRemoteName = NULL;
	nr.lpComment = NULL;
	nr.lpProvider = NULL;

	EnumerateNetworkResources(&nr);

	for (const auto& wstr : compName) {
		reverse_files(wstr, f_file, search_text);
	}
		
	for (const auto& wstr : result) {
		std::wcout << wstr << std::endl;
	}
}