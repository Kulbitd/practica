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
	virtual void FindTextInFile(const std::wstring&, T) = 0;
	virtual void SearchFilesFrom(const std::wstring&, T) = 0;
	virtual void WriteResult() = 0;
};

template<typename T>
class SearcherCommon : public ISearcher
{
private:
	std::vector<std::wstring> result = {};
	void FindTextInFile(const std::wstring& file_path, void* T) {
		DWORD bytes_read;
		const DWORD buffer_size = 1024;
		char* buffer = new char[buffer_size];

		HANDLE hFiler = CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFiler) {
			delete[] buffer;
			return;
		}

		const char* search_bytes = reinterpret_cast<const char*>(&search_value);
		size_t search_size = sizeof(T);  

		std::vector<char> file_content;

		while (ReadFile(hFiler, buffer, buffer_size, &bytes_read, NULL) && bytes_read > 0) {
			file_content.insert(file_content.end(), buffer, buffer + bytes_read);
				
			if (file_content.size() >= search_size) {
				for (size_t i = 0; i <= file_content.size() - search_size; ++i) {
					if (memcmp(&file_content[i], search_bytes, search_size) == 0) {
						result.push_back(file_path);
						CloseHandle(hFiler);
						delete[] buffer;
						return;
					}
				}
			}

			if (file_content.size() > search_size) {
				file_content.erase(file_content.begin(), file_content.end() - search_size + 1);
			}
		}

		delete[] buffer;  
		CloseHandle(hFiler);
	}

	void FindTextInFile(const std::wstring& file_path, const char* search_text) {
		DWORD bytes_read;
		const DWORD buffer_size = 1024;
		char* buffer = new char[buffer_size];

		HANDLE hFiler = CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFiler) {
			delete[] buffer;
			return;
		}

		std::string file_content;

		while (ReadFile(hFiler, buffer, buffer_size, &bytes_read, NULL) && bytes_read > 0) {
			file_content.append(buffer, bytes_read);

			if (file_content.find(search_text) != std::string::npos) {
				result.push_back(file_path);
				break;
			}
		}

		delete[] buffer;
		CloseHandle(hFiler);
	}


	void FindTextInFile(const std::wstring& file_path, const wchar_t* T) {
		DWORD bytes_read;
		const DWORD buffer_size = 1024;
		char* buffer = new char[buffer_size];

		HANDLE hFiler = CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFiler) {
			delete[] buffer;
			return;
		}

		std::wstring wide_buffer;

		while (ReadFile(hFiler, buffer, buffer_size, &bytes_read, NULL) && bytes_read > 0) {
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, NULL, 0);
			std::wstring temp_wide_buffer(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, &temp_wide_buffer[0], size_needed);

			wide_buffer.append(temp_wide_buffer);

			if (wide_buffer.find(search_text) != std::wstring::npos) {
				result.push_back(file_path); 
				break;
			}
		}

		delete[] buffer;
		CloseHandle(hFiler);
	}



	void SearchFilesFrom(const std::wstring& directory, T search_text) {
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
				SearchFilesFrom((directory + L"\\" + find_file_data.cFileName), search_text);
			else
				FindTextInFile((directory + L"\\" + find_file_data.cFileName).c_str(), search_text);
		} while (FindNextFileW(h_find, &find_file_data) != 0);
		FindClose(h_find);
	}
public:
	void WriteResult() {
		for (const auto& wstr : result) {
			std::wcout << wstr << std::endl;
		}
	}
};

template<typename T>
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
		if (compName.size() != 0) {
			EnumerateNetworkResources();
		}
		for (const auto& wstr : compName) {
			std::wcout << wstr << std::endl;
		}

	}
	void SearchFiles(const std::string& search_text) {
		if (compName.size() != 0) {
			EnumerateNetworkResources();
		}
		for (const auto& wstr : compName) {
			SearchFilesFrom(wstr, search_text);
		}
	}
};

template<typename T>
class LocalSearcher : public SearcherCommon {
public:
	void SearchFiles(const std::wstring& directory, const std::string& search_text) {
		SearchFilesFrom(directory, search_text);
	}
};

template<typename T>
int main() {
	LocalSearcher local;
	SambaSearcher samba;

	local.SearchFilesLocal(L"C:\\Users\\un\\Desktop\\deb", "hello");
	local.WriteResult();

	samba.EnumerateNetworkResources(NULL);
	samba.SearchFilesSamba("hello");
	samba.WriteResult();
}