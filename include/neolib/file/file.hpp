// file.hpp v1.3.1
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

#pragma once

#include <neolib/neolib.hpp>
#include <memory>
#include <string>
#include <cstdio>
#include <ctime>

namespace neolib
{
    NEOLIB_EXPORT std::string tidy_path(std::string aPath);
    NEOLIB_EXPORT std::wstring tidy_path(std::wstring aPath);
    NEOLIB_EXPORT std::string convert_path(const std::wstring& aString);
    NEOLIB_EXPORT std::wstring convert_path(const std::string& aString);
    NEOLIB_EXPORT const std::string& create_path(const std::string& aPath);
    NEOLIB_EXPORT const std::wstring& create_path(const std::wstring& aPath);
    NEOLIB_EXPORT std::string create_file(const std::string& aFileName);
    NEOLIB_EXPORT void create_file(const std::wstring& aFileName);
    NEOLIB_EXPORT bool file_exists(const std::string& aPath);
    NEOLIB_EXPORT bool file_exists(const std::wstring& aPath);
    NEOLIB_EXPORT std::time_t file_date(const std::string& aPath);
    NEOLIB_EXPORT std::time_t file_date(const std::wstring& aPath);
    NEOLIB_EXPORT std::string file_ext(const std::string& aPath);
    NEOLIB_EXPORT std::wstring file_ext(const std::wstring& aPath);
    NEOLIB_EXPORT bool can_read_file(const std::string& aPath);
    NEOLIB_EXPORT bool can_read_file(const std::wstring& aPath);
    NEOLIB_EXPORT unsigned long file_size(const std::string& aPath);
    NEOLIB_EXPORT unsigned long file_size(const std::wstring& aPath);
    NEOLIB_EXPORT unsigned long long large_file_size(const std::string& aPath);
    NEOLIB_EXPORT unsigned long long large_file_size(const std::wstring& aPath);
    NEOLIB_EXPORT int large_file_seek(FILE* aStream, long long aOffset, int aOrigin);
    NEOLIB_EXPORT bool move_file(const std::string& aPathFrom, const std::string& aPathTo);
    NEOLIB_EXPORT std::string program_file();
    NEOLIB_EXPORT std::string program_directory();
    NEOLIB_EXPORT std::string user_documents_directory();
    NEOLIB_EXPORT std::string user_settings_directory();

    class NEOLIB_EXPORT simple_file
    {
        // types
    private:
        struct handle
        {
            std::FILE* iHandle;
            handle(FILE* aHandle) : iHandle(aHandle) {}
            ~handle() { if (iHandle != NULL) fclose(iHandle); }
        };
        // construction
    public:
        simple_file();
        simple_file(const std::string& aPath, const std::string& aMode);
#ifdef _WIN32        
        simple_file(const std::wstring& aPath, const std::wstring& aMode);
#endif
        // operations
    public:
        bool valid() const { return iFile && iFile->iHandle != NULL; }
        operator std::FILE*() const { return iFile ? iFile->iHandle : NULL; }
        void close() { iFile.reset(); }
        int error() const { return iError; }
        // attributes
    private:
        std::shared_ptr<handle> iFile;
        int iError;
    };
}
