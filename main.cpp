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
//                 CloseHandle(hFile);
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0 && all_file_handles.size() < 1000);
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

    auto hFile = all_file_handles[990];

    // Get the volume handle
//    HANDLE hVolume = CreateFile(_T("\\\\.\\C:"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    HANDLE hVolume = CreateFile(_T("\\\\.\\C:"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hVolume == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Error opening volume, error code %d\n"), GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Get the file size
    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    // Allocate memory for the retrieval pointers
//    DWORD bufferSize = sizeof(STARTING_VCN_INPUT_BUFFER) + sizeof(RETRIEVAL_POINTERS_BUFFER);
//    LPVOID buffer = malloc(bufferSize);
    DWORD clusterSize = 4096;
    DWORD bufferSize = sizeof(RETRIEVAL_POINTERS_BUFFER) + sizeof(LARGE_INTEGER) * (fileSize.QuadPart / clusterSize + 1);
    LPVOID buffer = malloc(bufferSize);


    if (!buffer)
    {
        _tprintf(_T("Memory allocation error\n"));
        CloseHandle(hFile);
        CloseHandle(hVolume);
        return 1;
    }

    // Initialize the input buffer
    STARTING_VCN_INPUT_BUFFER inputBuffer;
    inputBuffer.StartingVcn.QuadPart = 0;

    DWORD bytesReturned;
//    BOOL success = DeviceIoControl(hVolume, FSCTL_GET_RETRIEVAL_POINTERS, &inputBuffer, sizeof(inputBuffer), buffer, bufferSize, &bytesReturned, NULL);
    BOOL success = DeviceIoControl(hFile, FSCTL_GET_RETRIEVAL_POINTERS, &inputBuffer, sizeof(inputBuffer), buffer, bufferSize, &bytesReturned, NULL);


    // Issues here
    if (!success)
    {
        _tprintf(_T("Error retrieving allocation information, error code %d\n"), GetLastError());
        free(buffer);
        CloseHandle(hFile);
        CloseHandle(hVolume);
        return 1;
    }

    // Process the retrieval pointers information in the buffer

    // TODO: Process the retrieval pointers information as needed
    RETRIEVAL_POINTERS_BUFFER* retrieval_pointers = (RETRIEVAL_POINTERS_BUFFER*)buffer;

    std::cout << "VCN to LCN mapping:" << std::endl;
    for (DWORD i = 0; i < retrieval_pointers->ExtentCount; ++i) {
        std::cout << "VCN: " << retrieval_pointers->StartingVcn.QuadPart + i
                  << ", LCN: " << retrieval_pointers->Extents[i].Lcn.QuadPart
                  << ", Length: " << retrieval_pointers->Extents[i].NextVcn.QuadPart - retrieval_pointers->StartingVcn.QuadPart
                  << std::endl;
    }

//    free(extents);



    return 0;
}