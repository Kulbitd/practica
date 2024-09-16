#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <string.h>
#include <lm.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "mpr.lib")


class ISearcher
{
public:
	virtual void SearchFiles(const void* search_data, size_t search_data_size) = 0;
};

class SearcherCommon : public ISearcher
{

private:
	bool isSubstr(void* src, size_t src_size, const void* search_data, size_t search_data_size)
	{
		char* src_ptr = (char*)src;

		for (size_t i = 0; i <= src_size - search_data_size; ++i)
		{
			if (memcmp(src_ptr + i, search_data, search_data_size) == 0)
				return true;
		}

		return false;
	}

protected:
	std::vector<std::wstring> result = {};

	void FindTextInFile(const std::wstring& file_path, const void* search_data, size_t search_data_size) {
		DWORD bytes_read;
		const DWORD buffer_size = 1024;
		char* buffer = new char[buffer_size];

		HANDLE hFiler = CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFiler) {
			return;
		}
		while (ReadFile(hFiler, buffer, buffer_size, &bytes_read, NULL) && bytes_read > 0)
		{
			if (isSubstr(buffer, bytes_read, search_data, search_data_size))
			{
				result.push_back(file_path);
				break;
			}
		}

		delete[] buffer;
		CloseHandle(hFiler);
	}

	void WriteResult() {
		for (const auto& wstr : result) {
			std::wcout << wstr << std::endl;
		}
	}

public:
	virtual void SearchFilesFrom(const std::wstring& directory, const void* search_data, size_t search_data_size) {
		WIN32_FIND_DATA find_file_data;
		HANDLE h_find = FindFirstFileW((directory + L"\\*").c_str(), &find_file_data);
		if (h_find == INVALID_HANDLE_VALUE)
			return;
		do
		{

			if (!wcscmp(find_file_data.cFileName, L".") || !wcscmp(find_file_data.cFileName, L".."))
				continue;

			/*switch (find_file_data.dwFileAttributes)
			{
			case FILE_ATTRIBUTE_REPARSE_POINT:
				continue;

			case FILE_ATTRIBUTE_DIRECTORY:
			case FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY:
				SearchFilesFrom((directory + L"\\" + find_file_data.cFileName), search_text);
				break;


			}*/

			if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_REPARSE_POINT)
				continue;

			std::wstring temp = (directory + L"\\" + find_file_data.cFileName);

			if (find_file_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY
				|| find_file_data.dwFileAttributes == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)) // 17 = 16+1 dir + redonly
				SearchFilesFrom(temp, search_data, search_data_size);
			else
				FindTextInFile(directory + L"\\" + find_file_data.cFileName, search_data, search_data_size);
		} while (FindNextFileW(h_find, &find_file_data) != 0);

		FindClose(h_find);
	}
};

class SambaSearcher : public SearcherCommon
{
private:
	std::vector<std::wstring> compName = {};

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

public:
	void WriteComputerName() {
		if (compName.size() == 0) {
			EnumerateNetworkResources(NULL);
		}
		for (const auto& wstr : compName) {
			std::wcout << wstr << std::endl;
		}

	}

	void SearchFiles(const void* search_data, size_t search_data_size) {
		if (compName.size() == 0) {
			EnumerateNetworkResources(NULL);
		}
		for (const auto& wstr : compName) {
			SearchFilesFrom(wstr, search_data, search_data_size);
		}
	}
};

class LocalSearcher : public SearcherCommon {
public:
	void SearchFiles(const void* search_data, size_t search_data_size) {
		SearchFilesFrom(L"C:/", search_data, search_data_size);
	}
};


void do_smth(std::vector<ISearcher*>& tasks)
{
	for (int i = 0; i < tasks.size(); ++i)
	{
		tasks[i]->SearchFiles("hello", 5);
	}
}

int main() {
	LocalSearcher local;
	SambaSearcher samba;

	std::vector<ISearcher*> tasks;

	tasks.push_back(&local);
	tasks.push_back(&samba);
	do_smth(tasks);


	local.SearchFilesFrom(L"C:\\Users\\un\\Desktop\\deb", "hello", 5);

	samba.SearchFiles("hello", 5);
}

