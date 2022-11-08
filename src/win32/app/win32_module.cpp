// win32_module.cpp
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

#include <algorithm>
#include <neolib/neolib.hpp>
#include <neolib/file/file.hpp>
#include "win32_module.hpp"

namespace neolib
{
    os_module::os_module(const std::string& aPath) : 
        iHandle(NULL) 
    { 
        load(aPath); 
    }

    os_module::os_module(const os_module& aOther) :
        iHandle{ aOther.iHandle }
    {
    }

    os_module::os_module(os_module&& aOther) :
        iHandle{ aOther.iHandle }
    {
        aOther.iHandle = NULL;
    }

    os_module::~os_module() 
    { 
        unload(); 
    }

    os_module& os_module::operator=(const os_module& aOther)
    {
        iHandle = aOther.iHandle;
        return *this;
    }

    os_module& os_module::operator=(os_module&& aOther)
    {
        iHandle = aOther.iHandle;
        aOther.iHandle = NULL;
        return *this;
    }

    namespace
    {
        std::string GetLastErrorText()
        {
            DWORD errorMessageID = ::GetLastError();
            if (errorMessageID == 0)
                return std::string{};

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            std::string message{ messageBuffer, size };
            LocalFree(messageBuffer);

            if (message.empty())
                message = "Unknown error, code: " + std::to_string(errorMessageID);

            return message;
        }
    }

    bool os_module::load(const std::string& aPath)
    { 
        try
        {
            auto widePath = neolib::convert_path(aPath);
            std::replace(widePath.begin(), widePath.end(), L'/', L'\\');
            iHandle = ::LoadLibraryW(widePath.c_str());
            if (iHandle == nullptr)
            {
                throw std::runtime_error{ GetLastErrorText() };
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "neolib::os_module: Failed to load module '" + aPath + "', reason: " + e.what() };
        }
        catch (...)
        {
            throw std::runtime_error{ "neolib::os_module: Failed to load module '" + aPath + "', unknown reason" };
        }
        return loaded(); 
    }

    void os_module::unload() 
    { 
        if (iHandle != NULL)
            ::FreeLibrary(iHandle); 
        iHandle = NULL; 
    }

    bool os_module::loaded() const 
    { 
        return iHandle != NULL; 
    }

    void* os_module::procedure_address(const std::string& aProcedureName)
    { 
        return ::GetProcAddress(iHandle, aProcedureName.c_str()); 
    }
}