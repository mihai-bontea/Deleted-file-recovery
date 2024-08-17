#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>

void get_file_handles(const std::wstring& directory, std::vector<HANDLE> &all_file_handles)
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
                // Directory => recursively iterate over its contents()
                get_file_handles(filePath, all_file_handles);
            }
            else
            {
                HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    all_file_handles.push_back(hFile);
                }
                 CloseHandle(hFile);
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0 && all_file_handles.size() < 10000);
}

int main() {
    std::wstring partitionPath = L"C:\\";

    std::vector<HANDLE> all_file_handles;
    get_file_handles(partitionPath, all_file_handles);
    std::cout << "We have " << all_file_handles.size() << " files!\n";

    size_t nr_invalid_handles = 0;
    for (auto handle : all_file_handles)
    {
        if (handle == INVALID_HANDLE_VALUE) {
            nr_invalid_handles++;
        }
    }

    std::cout << "We have " << nr_invalid_handles << " invalid filenames out of " <<  all_file_handles.size() << " files!\n";
    return 0;
}