// services.cpp
/*
 *  Copyright (c) 2020 Leigh Johnston.
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
#include <neolib/app/services.hpp>

namespace neolib::services
{
    struct service_provider : public i_service_provider
    {
        std::unordered_map<uuid, i_service*> services;

        bool service_registered(uuid aServiceIid) const override
        {
            return services.find(aServiceIid) != services.end();
        }

        i_service& service(uuid aServiceIid) override
        {
            auto existing = services.find(aServiceIid);
            if (existing != services.end())
                return *existing->second;
            throw service_not_found();
        }

        void register_service(i_service& aService, uuid aServiceIid) override
        {
            services[aServiceIid] = &aService;
        }

        void unregister_service(uuid aServiceIid) override
        {
            auto existing = services.find(aServiceIid);
            if (existing != services.end())
            {
                services.erase(existing);
                return;
            }
            throw service_not_found();
        }
    };

    std::unique_ptr<service_provider> sServiceProvider;
    i_service_provider* sServiceProviderAlias;

    i_service_provider& allocate_service_provider()
    {
        sServiceProvider = std::make_unique<service_provider>();
        sServiceProviderAlias = &*sServiceProvider;
        return *sServiceProviderAlias;
    }

    i_service_provider& get_service_provider()
    {
        return *sServiceProviderAlias;
    }

    void set_service_provider(i_service_provider& aServiceProvider)
    {
        sServiceProviderAlias = &aServiceProvider;
    }
}
