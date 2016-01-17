// module.cpp - v1.0.1
/*
 *  Copyright (c) 2012 Leigh Johnston.
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

#include "neolib.hpp"
#include "file.hpp"
#include "module.hpp"

namespace neolib
{
	class os_module
	{
		// construction
	public:
		os_module(const std::string& aPath) : iHandle(NULL) { load(aPath); }
		~os_module() { unload(); }
		// operations
	public:
		bool load(const std::string& aPath) 
		{ 
			try
			{
				iHandle = ::LoadLibrary(neolib::convert_path(aPath).c_str());
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error("neolib::os_module: Failed to load module '" + aPath + "', reason: " + e.what());
			}
			catch (...)
			{
				throw std::runtime_error("neolib::os_module: Failed to load module '" + aPath + "', unknown reason");
			}
			return loaded(); 
		}
		void unload() { ::FreeLibrary(iHandle); iHandle = NULL; }
		bool loaded() const { return iHandle != NULL; }
		void* procedure(const std::string& aProcedureName) { return ::GetProcAddress(iHandle, aProcedureName.c_str()); }
		// attributes
	private:
		HMODULE iHandle;
	};

	module::module()
	{
	}

	module::module(const module& aOther) : iPath(aOther.iPath), iOsModule(aOther.loaded() ? new os_module(iPath) : 0)
	{
	}
	
	module::module(const std::string& aPath) : iPath(aPath), iOsModule(new os_module(iPath))
	{
	}

	module::~module()
	{
	}

	bool module::load()
	{
		iOsModule.reset();
		if (iPath.empty())
			return false;
		os_module_ptr osModule(new os_module(iPath));
		if (!osModule->loaded())
			return false;
		iOsModule = osModule;
		return true;
	}

	void module::unload()
	{
		iOsModule.reset();
	}

	bool module::loaded() const
	{
		return iOsModule.get() != 0;
	}

	void* module::procedure(const std::string& aProcedureName)
	{
		if (!loaded())
			return 0;
		return iOsModule->procedure(aProcedureName);
	}
}