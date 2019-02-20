// application_info.cpp - v1.0
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
#ifdef _WIN32
#include <Shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif
#include <neolib/application_info.hpp>
#include <neolib/file.hpp>

namespace neolib
{
    std::string settings_folder(const std::string& aApplicationName, const std::string& aCompanyName)
    {
        std::string settingsFolder;
#ifdef _WIN32
        char szPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
        {
            settingsFolder =  tidy_path(std::string(szPath)) + "/" + aCompanyName + "/" + aApplicationName;
        }
        else
        {
            throw std::runtime_error("neolib::settings_folder: Cannot get CSIDL_APPDATA folder path");
        }
#else
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        settingsFolder = std::string(homedir) + "/." + aApplicationName;
#endif
        create_path(settingsFolder);
        return settingsFolder;
    }
}
