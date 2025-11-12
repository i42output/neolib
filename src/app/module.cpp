// module.cpp
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
#include <neolib/file/file.hpp>
#include <neolib/app/i_module_services.hpp>
#include <neolib/app/module.hpp>
#include <neolib/task/i_async_task.hpp>

#ifdef _WIN32
#include "../win32/app/win32_module.hpp"
#else
#include "../posix/app/posix_module.hpp"
#endif

namespace neolib
{
    module::module()
    {
    }

    module::module(const module& aOther) :
        iPath{ aOther.iPath },
        iOsModule{ aOther.loaded() ? std::make_unique<os_module>(iPath) : nullptr }
    {
    }

    module::module(module&& aOther) :
        iPath{ std::move(aOther.iPath) },
        iOsModule{ std::move(aOther.iOsModule) }
    {
    }

    module::module(const std::string& aPath) : 
        iPath{ aPath },
        iOsModule{ std::make_unique<os_module>(iPath) }
    {
    }

    module::~module()
    {
    }

    module& module::operator=(const module& aOther)
    {
        iPath = aOther.iPath;
        iOsModule = aOther.loaded() ? std::make_unique<os_module>(iPath) : nullptr;
        return *this;
    }

    module& module::operator=(module&& aOther)
    {
        iPath = std::move(aOther.iPath);
        iOsModule = std::move(aOther.iOsModule);
        return *this;
    }

    bool module::load()
    {
        if (loaded())
            return true;
        iOsModule.reset();
        if (iPath.empty())
            return false;
        auto osModule = std::make_unique<os_module>(iPath);
        if (!osModule->loaded())
            return false;
        iOsModule = std::move(osModule);
        return true;
    }

    void module::unload()
    {
        iOsModule.reset();
    }

    os_module* module::release()
    {
        return iOsModule.release();
    }

    bool module::loaded() const
    {
        return iOsModule != nullptr;
    }

    void* module::procedure_address(const std::string& aProcedureName)
    {
        if (!loaded())
            return 0;
        return iOsModule->procedure_address(aProcedureName);
    }

    void io_context_factory(i_async_task& aTask, bool aMultiThreaded, i_ref_ptr<i_async_service>& aResult);
        
    i_module_services& module_services()
    {
        static struct : i_module_services
        {
            void io_context_factory(i_async_task& aTask, bool aMultiThreaded, i_ref_ptr<i_async_service>& aResult) final
            {
                neolib::io_context_factory(aTask, aMultiThreaded, aResult);
            }
        } sDefaultModuleServices;
        return sDefaultModuleServices;
    }
}