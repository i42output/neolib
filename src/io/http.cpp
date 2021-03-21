// http.cpp
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
#include <neolib/core/vecarray.hpp>
#include <neolib/io/http.hpp>

namespace neolib
{
    constexpr long long DEFAULT_PROGRESS_INTERVAL_ms = 250;

    http::http(async_task& aIoTask) : 
        iIoTask(aIoTask), 
        iPort(80), 
        iSecure(false), 
        iType(Get), 
        iLastResponseHeader(iResponseHeaders.end()), 
        iOk(false), 
        iStatusCode(0)
    {
        reset();
    }

    http::http(const http& aOther) : 
        iIoTask(aOther.iIoTask), 
        iHost(aOther.iHost), 
        iPort(aOther.iPort), 
        iSecure(aOther.iSecure), 
        iType(aOther.iType), 
        iResource(aOther.iResource), 
        iLastResponseHeader(iResponseHeaders.end()), 
        iOk(false), 
        iStatusCode(0)
    {
        reset();
    }

    http::~http()
    {
    }

    http& http::operator=(const http& aOther) 
    { 
        reset();
        iHost = aOther.iHost; 
        iResource = aOther.iResource; 
        return *this;
    }

    http_stream& http::stream()
    {
        return *iPacketStream;
    }

    void http::reset()
    {
        iHost.clear(); 
        iPort = 80;
        iSecure = false;
        iType = Get;
        iResource.clear(); 
        iRequestHeaders.clear();
        iRequestBody.clear();
        iResponseLine.clear();
        iResponseStatus.clear();
        iResponseHeaders.clear();
        iLastResponseHeader = iResponseHeaders.end();
        iOk = false;
        iStatusCode = 0;
        iBodyLength.reset();
        iBody.clear();
        iState = ResponseStatus;
        iPreviousWasCRLF = false;
        iLastPacketReceived = std::nullopt;

        iPacketStream.reset();
        iPacketStream.emplace(iIoTask);

        stream().ConnectionEstablished([this]() { connection_established(); });
        stream().ConnectionFailure([this](const boost::system::error_code& aError) { connection_failure(aError); });
        stream().PacketSent([this](const http_packet& aPacket) { packet_sent(aPacket); });
        stream().PacketArrived([this](const http_packet& aPacket) { packet_arrived(aPacket); });
        stream().TransferFailure([this](const boost::system::error_code& aError) { transfer_failure(aError); });
        stream().ConnectionClosed([this]() { connection_closed(); });
    }

    void http::add_response_header(const std::string& aHeaderLine)
    {
        if (aHeaderLine.empty())
            return;
        if (aHeaderLine[0] == ' ' || aHeaderLine[0] == '\t')
        {
            if (iLastResponseHeader != iResponseHeaders.end())
                iLastResponseHeader->second += aHeaderLine;
            return;
        }
        vecarray<std::pair<std::string::const_iterator, std::string::const_iterator>, 2> bits;
        tokens(aHeaderLine, std::string(":"), bits, 2);
        if (bits.size() >= 2)
        {
            std::string headerName(bits[0].first, bits[0].second);
            std::string headerValue(bits[1].first, aHeaderLine.end());
            if (iResponseHeaders.find(make_ci_string(headerName)) == iResponseHeaders.end())
                iResponseHeaders[make_ci_string(headerName)] = neolib::remove_leading(headerValue, std::string(" "));
            else
                iResponseHeaders[make_ci_string(headerName)] += ("," + neolib::remove_leading(headerValue, std::string(" ")));
            iLastResponseHeader = iResponseHeaders.find(make_ci_string(headerName));
            if (make_ci_string(headerName) == "Content-Length")
                iBodyLength = string_to_uint32(headerValue);
        }
    }

    bool http::decode()
    {
        headers_t::const_iterator encoding = iResponseHeaders.find("Transfer-Encoding");
        bool ok = false;
        if (encoding == iResponseHeaders.end())
            ok = true;
        else
        {
            if (make_ci_string(encoding->second) == "chunked")
                ok = decode_chunked();
            if (ok && iBodyLength && *iBodyLength != iBody.size())
                *iBodyLength = iBody.size();
        }
        return ok;
    }

    bool http::decode_chunked()
    {
        body_t encodedData;
        encodedData.swap(iBody);
        const body_t& encoded = encodedData;
        std::string lineTerminator("\r\n");
        std::string sizeTerminator(";\r\n");
        for (body_t::const_iterator i = encoded.begin(); i != encoded.end();)
        {
            neolib::vecarray<std::pair<body_t::const_iterator, body_t::const_iterator>, 1> dataLine;
            neolib::tokens(i, encoded.end(), lineTerminator.begin(), lineTerminator.end(), dataLine, 1, false, true);
            if (dataLine.empty())
                return false;
            i = dataLine[0].second;
            if (encoded.end() - i < 2 || *i != '\r' || *(i+1) != '\n')
                return false;
            i += 2;
            neolib::vecarray<std::pair<body_t::const_iterator, body_t::const_iterator>, 1> dataSize;
            neolib::tokens(dataLine[0].first, dataLine[0].second, sizeTerminator.begin(), sizeTerminator.end(), dataSize, 1, false);
            if (dataSize.empty())
                return false;
            unsigned int chunkSize = neolib::string_to_uint32(std::string(dataSize[0].first, dataSize[0].second), 16);
            if (chunkSize != 0)
            {
                if (static_cast<unsigned int>(encoded.end() - i) < chunkSize)
                    return false;
                iBody.insert(iBody.end(), i, i + chunkSize);
                i += chunkSize;
                if (encoded.end() - i < 2 || *i != '\r' || *(i+1) != '\n')
                    return false;
                i += 2;
            }
            else
            {
                typedef std::vector<std::string> trailer_headers;
                trailer_headers trailerHeaders;
                neolib::tokens(i, encoded.end(), lineTerminator.begin(), lineTerminator.end(), trailerHeaders, 0, true, true);
                for (trailer_headers::const_iterator j = trailerHeaders.begin(); j != trailerHeaders.end(); ++j)
                    add_response_header(*j);
                return true;
            }
        }
        return false;
    }

    void http::request(const std::string& aUrl, type_e aType, const headers_t& aRequestHeaders, const std::variant<body_t, std::string>& aRequestBody)
    {
        bool secure = false;
        if (make_ci_string(aUrl).find("http://") == 0)
            secure = false;
        else if (make_ci_string(aUrl).find("https://") == 0)
            secure = true;
        else
            return;
        typedef std::pair<std::string::const_iterator, std::string::const_iterator> string_pair;
        neolib::vecarray<string_pair, 2> parts;
        std::string delim = "//";
        neolib::tokens(aUrl.begin(),aUrl.end(), delim.begin(), delim.end(), parts, 2, true, true);
        if (parts.size() != 2)
            return;
        string_pair second = parts[1];
        parts.clear();
        delim = "/";
        neolib::tokens(second.first, second.second, delim.begin(), delim.end(), parts, 2);
        if (parts.empty())
            return;
        std::string resource = "/";
        if (parts.size() == 2)
            resource = "/" + std::string(parts[1].first, aUrl.end());
        string_pair first = parts[0];
        parts.clear();
        delim = ":";
        neolib::tokens(first.first, first.second, delim.begin(), delim.end(), parts, 2);
        if (parts.empty())
            return;
        std::optional<unsigned short> port;
        if (parts.size() == 2)
            port = static_cast<unsigned short>(neolib::string_to_uint32(std::string(parts[1].first, parts[1].second)));
        std::string address = std::string(parts[0].first, parts[0].second);
        request(address, resource, aType, port ? *port : secure ? 443 : 80, secure, aRequestHeaders, aRequestBody);
    }

    void http::request(const std::string& aHost, const std::string& aResource, type_e aType, unsigned short aPort, bool aSecure, const headers_t& aRequestHeaders, const std::variant<body_t, std::string>& aRequestBody)
    {
        reset();
        iHost = aHost;
        iPort = aPort;
        iSecure = aSecure;
        iType = aType;
        iResource = aResource;
        iRequestHeaders = aRequestHeaders;
        if (std::holds_alternative<body_t>(aRequestBody) && !std::get<body_t>(aRequestBody).empty())
            iRequestBody = std::get<body_t>(aRequestBody);
        else if (std::holds_alternative<std::string>(aRequestBody) && !std::get<std::string>(aRequestBody).empty())
            iRequestBody.assign(std::get<std::string>(aRequestBody).begin(), std::get<std::string>(aRequestBody).end());
        if (stream().open(aHost, aPort, aSecure))
            Started.trigger();
        else
            Failure.trigger();
    }

    double http::percent_done() const
    {
        if (!iBodyLength)
            return 0.0;
        else if (*iBodyLength == 0)
            return 100.0;
        else
            return iBody.size() * 100.0 / *iBodyLength;
    }

    void http::connection_established()
    {
        std::string theRequest = (iType == Get ? "GET " : "POST ") + iResource + " HTTP/1.1\r\n";
        theRequest += "Host: " + iHost + "\r\n";
        if (iRequestHeaders.find("Connection") == iRequestHeaders.end())
            theRequest += "Connection: close\r\n";
        for (headers_t::const_iterator i = iRequestHeaders.begin(); i != iRequestHeaders.end(); ++i)
            theRequest += neolib::make_string(i->first) + ": " + i->second + "\r\n";
        theRequest += "\r\n";
        if (!iRequestBody.empty())
            theRequest += std::string(iRequestBody.begin(), iRequestBody.end());
        stream().send_packet(http_packet(theRequest));
    }

    void http::connection_failure(const boost::system::error_code&)
    {
        iBodyLength.reset();
        iBody.clear();
        Failure.trigger();
        stream().close();
    }

    void http::packet_sent(const http_packet&)
    {
    }

    void http::packet_arrived(const http_packet& aPacket)
    {
        for (http_packet::const_iterator i = aPacket.begin(); i != aPacket.end();)
        {
            switch(iState)
            {
            case Body:
                iBody.insert(iBody.end(), i, aPacket.end());
                i = aPacket.end();
                if (iLastPacketReceived == std::nullopt ||
                    std::chrono::duration_cast<std::chrono::milliseconds>(*iLastPacketReceived - std::chrono::steady_clock::now()).count() > DEFAULT_PROGRESS_INTERVAL_ms)
                {
                    iLastPacketReceived = std::chrono::steady_clock::now();
                    Progress.trigger();
                }
                break;
            default:
                {
                    i_packet::character_type ch = *i++;
                    switch(ch)
                    {
                    case '\r':
                        break;
                    case '\n':
                        if (iPreviousWasCRLF)
                        {
                            iPreviousWasCRLF = false;
                            iState = Body;
                            break;
                        }
                        iPreviousWasCRLF = true;
                        switch(iState)
                        {
                        case ResponseStatus:
                            {
                                iState = ResponseHeaders;
                                iResponseStatus = iResponseLine;
                                iResponseLine.clear();
                                vecarray<std::string, 3> bits;
                                tokens(iResponseStatus, std::string(" "), bits, 3);
                                if (bits.size() >= 3)
                                {
                                    iStatusCode = string_to_uint32(bits[1]);
                                    iOk = (iStatusCode / 100 == 2);
                                }
                            }
                            break;
                        case ResponseHeaders:
                            add_response_header(iResponseLine);
                            iResponseLine.clear();
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        iResponseLine.push_back(ch);
                        iPreviousWasCRLF = false;
                        break;
                    }
                }
            }
        }
    }

    void http::transfer_failure(const boost::system::error_code&)
    {
        iBodyLength.reset();
        iBody.clear();
        Failure.trigger();
        stream().close();
    }

    void http::connection_closed()
    {
        iState = Finished;
        if (ok() && stream().has_error())
            iOk = false;
        if (ok() && !decode())
            iOk = false;
        if (ok() && iBodyLength && *iBodyLength != iBody.size())
            iOk = false;
        if (ok())
        {
            Progress.trigger();
            Completed.trigger();
        }
        else
        {
            iBodyLength.reset();
            iBody.clear();
            Failure.trigger();
        }
    }
}
