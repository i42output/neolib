// file.cpp - v1.3.1
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
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
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
#include <neolib/string_utils.hpp>
#include <neolib/file.hpp>

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
        return boost::filesystem::path(aString).generic_string();
    }

    std::wstring convert_path(const std::string& aString)
    {
        auto utf16 = utf8_to_utf16(aString);
        return boost::filesystem::path(reinterpret_cast<const wchar_t*>(utf16.c_str())).generic_wstring();
    }

    const std::wstring& create_path(const std::wstring& aPath)
    {
        boost::filesystem::create_directories(aPath);
        return aPath;
    }

    const std::string& create_path(const std::string& aPath)
    {
        boost::filesystem::create_directories(aPath);
        return aPath;
    }

    std::string create_file(const std::string& aFileName)
    {
        boost::filesystem::ofstream newFile(convert_path(aFileName), std::ios::out | std::ios_base::app | std::ios_base::binary);
        return boost::filesystem::path(convert_path(aFileName)).generic_string();
    }

    void create_file(const std::wstring& aFileName)
    {
        boost::filesystem::ofstream newFile(aFileName, std::ios::out | std::ios_base::app | std::ios_base::binary);
    }

    bool file_exists(const std::string& aPath)
    {
        return boost::filesystem::exists(convert_path(aPath));
    }

    bool file_exists(const std::wstring& aPath)
    {
        return boost::filesystem::exists(aPath);
    }

    std::time_t file_date(const std::string& aPath)
    {
        return boost::filesystem::last_write_time(convert_path(aPath));
    }

    std::time_t file_date(const std::wstring& aPath)
    {
        return boost::filesystem::last_write_time(aPath);
    }

    bool can_read_file(const std::string& aPath)
    {
        return can_read_file(convert_path(aPath));
    }

    bool can_read_file(const std::wstring& aPath)
    {
        boost::filesystem::ifstream test(aPath);
        return !!test;
    }

    unsigned long file_size(const std::string& aPath)
    {
        return static_cast<unsigned long>(boost::filesystem::file_size(convert_path(aPath)));
    }

    unsigned long file_size(const std::wstring& aPath)
    {
        return static_cast<unsigned long>(boost::filesystem::file_size(aPath));
    }

    unsigned long long large_file_size(const std::string& aPath)
    {
        return boost::filesystem::file_size(convert_path(aPath));
    }

    unsigned long long large_file_size(const std::wstring& aPath)
    {
        return boost::filesystem::file_size(aPath);
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
        return fseek64(aString, aOffset, aOrigin);
#endif
    }

    bool move_file(const std::string& aPathFrom, const std::string& aPathTo)
    {
        boost::system::error_code ec;
        boost::filesystem::create_directories(boost::filesystem::path(convert_path(aPathTo)).parent_path());
        boost::filesystem::rename(convert_path(aPathFrom), convert_path(aPathTo), ec);
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
        return std::string{ result, (count > 0) ? count : 0 };
#endif
    }

    std::string program_directory()
    {
        return boost::filesystem::path(program_file()).parent_path().string();
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

    simple_file::simple_file(const std::wstring& aPath, const std::wstring& aMode) : 
        iFile(new handle(_wfopen(aPath.c_str(), aMode.c_str()))), iError(valid() ? 0 : errno) 
    {
    }
}