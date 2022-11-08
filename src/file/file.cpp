// file.cpp - v1.3.1
/// @deprecated Just use std::filesystem directly.
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <neolib/neolib.hpp>
#include <filesystem>
#ifdef _WIN32
#include <WINNLS.H>
#include <shellapi.h>
#include <Shlobj_core.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif
#include <vector>
#include <string>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <neolib/core/string_utils.hpp>
#include <neolib/file/file.hpp>

namespace neolib
{
    std::string tidy_path(std::string aPath)
    {
#ifdef _WIN32
        for (std::string::size_type i = 0; i != aPath.size(); ++i)
            if (aPath[i] == '\\')
                aPath[i] = '/';
#endif
        return aPath;
    }

    std::wstring tidy_path(std::wstring aPath)
    {
#ifdef _WIN32
        for (std::wstring::size_type i = 0; i != aPath.size(); ++i)
            if (aPath[i] == L'\\')
                aPath[i] = L'/';
#endif
        return aPath;
    }

    std::string convert_path(const std::wstring& aString)
    {
        return std::filesystem::path(aString).generic_string();
    }

    std::wstring convert_path(const std::string& aString)
    {
        auto const utf16 = utf8_to_utf16(aString);
        return std::filesystem::path(utf16.c_str()).generic_wstring();
    }

    const std::wstring& create_path(const std::wstring& aPath)
    {
        std::filesystem::create_directories(aPath);
        return aPath;
    }

    const std::string& create_path(const std::string& aPath)
    {
        std::filesystem::create_directories(aPath);
        return aPath;
    }

    std::string create_file(const std::string& aFileName)
    {
#ifdef _WIN32
        std::ofstream newFile(convert_path(aFileName), std::ios::out | std::ios_base::app | std::ios_base::binary);
        return std::filesystem::path(convert_path(aFileName)).generic_string();
#else
        std::ofstream newFile(aFileName, std::ios::out | std::ios_base::app | std::ios_base::binary);
        return aFileName;
#endif
    }

    void create_file(const std::wstring& aFileName)
    {
#ifdef _WIN32
        std::ofstream newFile(aFileName, std::ios::out | std::ios_base::app | std::ios_base::binary);
#else
        std::ofstream newFile(std::filesystem::path{aFileName}.string(), std::ios::out | std::ios_base::app | std::ios_base::binary);
#endif
    }

    bool file_exists(const std::string& aPath)
    {
        return std::filesystem::exists(convert_path(aPath));
    }

    bool file_exists(const std::wstring& aPath)
    {
        return std::filesystem::exists(aPath);
    }

    std::time_t file_date(const std::string& aPath)
    {
        auto const fileDate = std::filesystem::last_write_time(convert_path(aPath));
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + (fileDate - std::chrono::file_clock::now()));
    }

    std::time_t file_date(const std::wstring& aPath)
    {
        auto const fileDate = std::filesystem::last_write_time(aPath);
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + (fileDate - std::chrono::file_clock::now()));
    }

    bool can_read_file(const std::string& aPath)
    {
#ifdef _WIN32
        return can_read_file(convert_path(aPath));
#else
        std::ifstream test(std::filesystem::path{ aPath }.string());
        return !!test;
#endif
    }

    bool can_read_file(const std::wstring& aPath)
    {
#ifdef _WIN32
        std::ifstream test(aPath);
        return !!test;
#else
        std::ifstream test(std::filesystem::path{ aPath }.string());
        return !!test;
#endif
    }

    unsigned long file_size(const std::string& aPath)
    {
        return static_cast<unsigned long>(std::filesystem::file_size(convert_path(aPath)));
    }

    unsigned long file_size(const std::wstring& aPath)
    {
        return static_cast<unsigned long>(std::filesystem::file_size(aPath));
    }

    unsigned long long large_file_size(const std::string& aPath)
    {
        return std::filesystem::file_size(convert_path(aPath));
    }

    unsigned long long large_file_size(const std::wstring& aPath)
    {
        return std::filesystem::file_size(aPath);
    }

    std::string file_ext(const std::string& aPath)
    {
        std::string::size_type pos = aPath.find_last_of('.');
        if (pos != std::string::npos && pos != aPath.size() - 1)
            return aPath.substr(pos + 1);
        return "";
    }

    std::wstring file_ext(const std::wstring& aPath)
    {
        std::wstring::size_type pos = aPath.find_last_of(L'.');
        if (pos != std::wstring::npos && pos != aPath.size() - 1)
            return aPath.substr(pos + 1);
        return L"";
    }

    int large_file_seek(FILE* aStream, long long aOffset, int aOrigin)
    {
#ifdef _WIN32
        return _fseeki64(aStream, aOffset, aOrigin);
#else
        return fseeko(aStream, aOffset, aOrigin);
#endif
    }

    bool move_file(const std::string& aPathFrom, const std::string& aPathTo)
    {
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(convert_path(aPathTo)).parent_path());
        std::filesystem::rename(convert_path(aPathFrom), convert_path(aPathTo), ec);
        return !ec;
    }

    std::string program_file()
    {
#ifdef _WIN32
        wchar_t result[MAX_PATH];
        return convert_path(std::wstring{ result, GetModuleFileNameW(NULL, result, MAX_PATH) });
#else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::string{ result, static_cast<std::string::size_type>((count > 0) ? count : 0) };
#endif
    }

    std::string program_directory()
    {
        return std::filesystem::path(program_file()).parent_path().string();
    }

    std::string user_documents_directory()
    {
#ifdef _WIN32
        PWSTR result;
        SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &result);
        auto path = convert_path(result);
        CoTaskMemFree(result);
        return path;
#else
        struct passwd* pw = getpwuid(getuid());
        const char* homedir = pw->pw_dir;
        return homedir;
#endif
    }

    std::string user_settings_directory()
    {
#ifdef _WIN32
        PWSTR result;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &result);
        auto path = convert_path(result);
        CoTaskMemFree(result);
        return path;
#else
        struct passwd* pw = getpwuid(getuid());
        const char* homedir = pw->pw_dir;
        return homedir;
#endif
    }

    simple_file::simple_file() :
        iError(0)
    {
    }

    simple_file::simple_file(const std::string& aPath, const std::string& aMode) : 
        iFile(new handle(fopen(aPath.c_str(), aMode.c_str()))), iError(valid() ? 0 : errno) 
    {
    }

#ifdef _WIN32
    simple_file::simple_file(const std::wstring& aPath, const std::wstring& aMode) : 
        iFile(new handle(_wfopen(aPath.c_str(), aMode.c_str()))), iError(valid() ? 0 : errno) 
    {
    }
#endif
}
