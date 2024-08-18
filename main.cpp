#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>

#include "FileHandlesFetcher.h"

int main() {
    std::wstring partitionPath = L"C:\\";

    auto all_file_handles = std::move(FileHandlesFetcher::get_all_file_handles(partitionPath));

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