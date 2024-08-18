#include "FileHandlesFetcher.h"

#define MAX_FILES 2000

void FileHandlesFetcher::get_file_handles_helper(const std::wstring& directory, std::vector<HANDLE> &all_file_handles)
{
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((directory + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Error opening directory: " << directory << std::endl;
        return;
    }

    do {
        const std::wstring fileName = findFileData.cFileName;

        if (fileName != L"." && fileName != L"..")
        {
            const std::wstring filePath = directory + L"\\" + fileName;
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                get_file_handles_helper(filePath, all_file_handles);
            }
            else
            {
                HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    all_file_handles.push_back(hFile);
                }
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0 && all_file_handles.size() < MAX_FILES);
}