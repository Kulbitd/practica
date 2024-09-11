#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <lm.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "mpr.lib")
template<typename T>
class ISearcher
{
public:
	virtual void FindTextInFile(const std::wstring& file_path, T) = 0;
	virtual void SearchFiles(const std::wstring& directory, T) = 0;
	virtual void WriteResult() = 0;
};

template<typename T>
class SearcherCommon : public ISearcher
{
private:
	std::vector<std::wstring> result = {};
public:
	void WriteResult() {
		for (const auto& wstr : result) {
			std::wcout << wstr << std::endl;
		}
	}

	void FindTextInFile(const std::wstring& file_path, const std::string& search_text) {
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

	void SearchFiles(const std::wstring& directory, const std::string& search_text) {
		WIN32_FIND_DATA find_file_data;
		HANDLE h_find = FindFirstFileW((directory + L"\\*").c_str(), &find_file_data);
		if (h_find == INVALID_HANDLE_VALUE)
			return;
		do
		{

			if (!wcscmp(find_file_data.cFileName, L".") || !wcscmp(find_file_data.cFileName, L".."))
				continue;
			if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_REPARSE_POINT)
				continue;
			if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY or find_file_data.dwFileAttributes == 17) // 17 = 16+1 dir + redonly
				SearchFiles((directory + L"\\" + find_file_data.cFileName), search_text);
			else
				FindTextInFile((directory + L"\\" + find_file_data.cFileName).c_str(), search_text);
		} while (FindNextFileW(h_find, &find_file_data) != 0);
		FindClose(h_find);
	}
};

template<typename T>
class SambaSearcher : public SearcherCommon
{
private:
	std::vector<std::wstring> compName = {};
public:
	void WriteComputerName() {
		if (compName.size() != 0) {
			EnumerateNetworkResources(NULL);
		}
		for (const auto& wstr : compName) {
			std::wcout << wstr << std::endl;
		}

	}
	void EnumerateNetworkResources(NETRESOURCE* lpnr = NULL) {
		if (lpnr == NULL) {
			NETRESOURCE nr;
			nr.dwScope = RESOURCE_GLOBALNET;
			nr.dwType = RESOURCETYPE_DISK;
			nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
			nr.dwUsage = RESOURCEUSAGE_CONTAINER;
			nr.lpLocalName = NULL;
			nr.lpRemoteName = NULL;
			nr.lpComment = NULL;
			nr.lpProvider = NULL;
		}

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

	void SearchFilesSamba(const std::string& search_text) {
		for (const auto& wstr : compName) {
			SearchFiles(wstr, search_text);
		}
	}
};

template<typename T>
class LocalSearcher : public SearcherCommon {
public:
	void SearchFilesLocal(const std::wstring& directory, const std::string& search_text) {
		SearchFiles(directory, search_text);
	}
};

template<typename T>
int main() {
	LocalSearcher local;
	SambaSearcher samba;

	local.SearchFilesLocal(L"C:\\Users\\un\\Desktop\\deb","hello");
	local.WriteResult();

	samba.EnumerateNetworkResources(NULL);
	samba.SearchFilesSamba("hello");
	samba.WriteResult();
}