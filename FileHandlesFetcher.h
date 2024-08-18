#pragma once

#include <chrono>
#include <iostream>
#include <vector>

#include <tchar.h>
#include <windows.h>

class FileHandlesFetcher
{
public:
    inline static std::vector<HANDLE> get_all_file_handles(const std::wstring& directory)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<HANDLE> all_file_handles;
        get_file_handles_helper(directory, all_file_handles);

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;
        std::cout << "get_all_file_handles: " << duration.count() << " seconds" << std::endl;

        return all_file_handles;
    }
private:
    static void get_file_handles_helper(const std::wstring& directory, std::vector<HANDLE> &all_file_handles);
};
