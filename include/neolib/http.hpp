// http.hpp
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

#include "neolib.hpp"
#include <vector>
#include <string>
#include <deque>
#include <optional>
#include <chrono>
#include "variant.hpp"
#include "event.hpp"
#include "string_utils.hpp"
#include "packet_stream.hpp"
#include "string_packet.hpp"

namespace neolib
{
    class http_packet : public string_packet
    {
        // construction
    public:
        http_packet(const contents_type& aContents = contents_type()) : 
            string_packet(aContents) 
        {
        }
        // implementation
    private:
        virtual bool has_delimiters() const
        {
            return false;
        }
    };

    typedef packet_stream<http_packet, tcp_protocol> http_stream;
    typedef i_packet_stream_observer<http_packet, tcp_protocol> http_stream_observer;

    class http : private http_stream_observer
    {
        // events
    public:
        event<> started;
        event<> progress;
        event<> completed;
        event<> failure;

        // types
    public:
        typedef std::map<ci_string, std::string> headers_t;
        typedef std::vector<char> body_t;
        enum type_e { Get, Post };
        
        // construction
    public:
        http(async_task& aIoTask);
        http(const http& aOther);
        virtual ~http();
        http& operator=(const http& aOther);
        bool operator==(const http&) const { return false; }

        // operations
    public:
        void request(const std::string& aUrl, type_e aType = Get, const headers_t& aRequestHeaders = headers_t(), const variant<body_t, std::string>& aRequestBody = std::string());
        void request(const std::string& aHost, const std::string& aResource, type_e aType = Get, unsigned short aPort = 80, bool aSecure = false, const headers_t& aRequestHeaders = headers_t(), const variant<body_t, std::string>& aRequestBody = std::string());
        bool ok() const { return iOk; }
        uint32_t status_code() const { return iStatusCode; }
        uint64_t body_length() const { return iBodyLength ? *iBodyLength : iBody.size(); }
        const std::string& response_status() const { return iResponseStatus; }
        const headers_t& response_headers() const { return iResponseHeaders; }
        const body_t& body() const { return iBody; }
        std::string body_as_string() const { return std::string(iBody.begin(), iBody.end()); }
        double percent_done() const;

        // implementation
    private:
        void reset();
        void add_response_header(const std::string& aHeaderLine);
        bool decode();
        bool decode_chunked();
        // from http_stream_observer
        virtual void connection_established(packet_stream_type& aStream);
        virtual void connection_failure(packet_stream_type& aStream, const boost::system::error_code& aError);
        virtual void packet_sent(packet_stream_type& aStream, const http_packet& aPacket);
        virtual void packet_arrived(packet_stream_type& aStream, const http_packet& aPacket);
        virtual void transfer_failure(packet_stream_type& aStream, const boost::system::error_code& aError);
        virtual void connection_closed(packet_stream_type& aStream);

        // attributes
    private:
        async_task& iIoTask;
        http_stream iPacketStream;
        std::string iHost;
        uint16_t iPort;
        bool iSecure;
        type_e iType;
        std::string iResource;
        headers_t iRequestHeaders;
        body_t iRequestBody;
        std::string iResponseLine;
        std::string iResponseStatus;
        headers_t iResponseHeaders;
        headers_t::iterator iLastResponseHeader;
        bool iOk;
        uint32_t iStatusCode;
        std::optional<uint64_t> iBodyLength;
        body_t iBody;
        enum state { ResponseStatus, ResponseHeaders, Body, Finished } iState;
        bool iPreviousWasCRLF;
        std::optional<std::chrono::time_point<std::chrono::steady_clock>> iLastPacketReceived;
    };
}
