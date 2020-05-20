// resolver.hpp
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
#include <stdexcept>
#include <vector>
#include <memory>
#include <boost/bind.hpp>
#include <neolib/task/async_task.hpp>

namespace neolib
{
    typedef boost::asio::ip::tcp tcp_protocol;
    typedef boost::asio::ip::udp udp_protocol;

    enum protocol_family
    {
        IPv4 = 0x01,
        IPv6 = 0x02,
        IPv4orIPv6 = IPv4 | IPv6
    };

    template <typename Protocol>
    inline protocol_family to_protocol_family(Protocol aProtocol)
    {
        if (aProtocol == aProtocol.v4())
            return IPv4;
        else
            return IPv6;
    }

    template <typename Protocol>
    inline Protocol to_protocol(protocol_family aProtocolFamily)
    {
        if (aProtocolFamily & IPv4)
            return Protocol::v4();
        else
            return Protocol::v6();
    }

    template <typename Protocol>
    class basic_resolver
    {
        // types
    public:
        typedef basic_resolver<Protocol> our_type;
        typedef Protocol protocol_type;
    private:
        typedef typename protocol_type::endpoint endpoint_type;
        typedef typename protocol_type::resolver resolver_type;
    public:
        typedef typename resolver_type::iterator iterator;
    public:
        class requester
        {
        public:
            virtual void host_resolved(const std::string& aHostName, iterator aHost) = 0;
            virtual void host_not_resolved(const std::string& aHostName, const boost::system::error_code& aError) = 0;
        };
        class request
        {
        public:
            struct no_requester : std::logic_error { no_requester() : std::logic_error("neolib::basic_resolver::request::no_requester") {} };
        private:
            typedef typename basic_resolver<Protocol>::requester requester_type;
        public:
            request(basic_resolver<Protocol>& aParent, requester_type& aRequester, const std::string& aHostName, neolib::protocol_family aProtocolFamily) :
                iParent(aParent), iOrphaned(false), iRequester(&aRequester), iHostName(aHostName), iProtocolFamily(aProtocolFamily)
            {
            }
        public:
            void orphan()
            {
                iOrphaned = true;
            }
            const std::string& host_name() const
            {
                return iHostName;
            }
            neolib::protocol_family protocol_family() const
            {
                return iProtocolFamily;
            }
            bool has_requester() const
            {
                return iRequester != nullptr;
            }
            requester_type& requester() const
            {
                if (iRequester == nullptr)
                    throw no_requester();
                return *iRequester;
            }
            void reset()
            {
                iRequester = nullptr;
            }
            void handle_resolve(const boost::system::error_code& aError, iterator aEndPointIterator)
            {
                if (!iOrphaned)
                    iParent.handle_resolve(*this, aError, aEndPointIterator);
            }
        private:
            basic_resolver<Protocol>& iParent;
            bool iOrphaned;
            requester_type* iRequester;
            std::string iHostName;
            neolib::protocol_family iProtocolFamily;
        };
        typedef std::shared_ptr<request> request_pointer;
        typedef std::vector<request_pointer> request_list;

        // exceptions
    public:
    
        // construction
    public:
        basic_resolver(async_task& aIoTask) :
            iIoTask(aIoTask),
            iResolver(aIoTask.io_service().native_object())
        {
        }
        ~basic_resolver()
        {
            for (auto& r : iRequests)
                r->orphan();
            iRequests.clear();
            iResolver.cancel();
        }
        
        // operations
    public:
        void resolve(requester& aRequester, const std::string& aHostName, protocol_family aProtocolFamily = IPv4orIPv6)
        {
            request_pointer newRequest(new request(*this, aRequester, aHostName, aProtocolFamily));
            iRequests.push_back(newRequest);
            iResolver.async_resolve(typename resolver_type::query{ aHostName, "0" },
                boost::bind(&request::handle_resolve, *newRequest, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
        }
        void remove_requester(requester& aRequester)
        {
            for (auto& request : iRequests)
                if (request.has_requester() && request.requester() == &aRequester)
                    request.reset();
        }
        
        // implementation
    private:
        void handle_resolve(request& aRequest, const boost::system::error_code& aError, iterator aEndPointIterator)
        {
            if (aRequest.has_requester())
            {
                if (!aError)
                {
                    bool foundGoodMatch = false;
                    for (iterator i = aEndPointIterator; !foundGoodMatch && i != iterator(); ++i)
                    {
                        endpoint_type endpoint = *i;
                        if (to_protocol_family(endpoint.protocol()) & aRequest.protocol_family())
                        {
                            foundGoodMatch = true;
                            aRequest.requester().host_resolved(aRequest.host_name(), i);
                        }
                    }
                    if (!foundGoodMatch)
                        aRequest.requester().host_resolved(aRequest.host_name(), aEndPointIterator);
                }
                else
                    aRequest.requester().host_not_resolved(aRequest.host_name(), aError);
            }
            for (auto i = iRequests.begin(); i != iRequests.end(); ++i)
                if (&**i == &aRequest)
                {
                    iRequests.erase(i);
                    break;
                }
        }

        // attibutes
    private:
        async_task& iIoTask;
        resolver_type iResolver;
        request_list iRequests;
    };

    typedef basic_resolver<tcp_protocol> tcp_resolver;
    typedef basic_resolver<udp_protocol> udp_resolver;
}
