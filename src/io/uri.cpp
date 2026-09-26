// uri.cpp
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
#include <iterator>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/io/uri.hpp>

namespace neolib
{
    namespace
    {
        std::string to_file_url(std::filesystem::path const& aPath)
        {
            auto const u8 = std::filesystem::absolute(aPath).generic_u8string();
            std::string_view path{ reinterpret_cast<char const*>(u8.data()), u8.size() };

            auto const encode = [](std::string_view aSource, std::string& aOut)
                {
                    static constexpr char hex[] = "0123456789ABCDEF";
                    for (unsigned char ch : aSource)
                    {
                        bool const unreserved =
                            (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
                            ch == '-' || ch == '.' || ch == '_' || ch == '~' || ch == '/';
                        if (unreserved)
                            aOut += static_cast<char>(ch);
                        else
                        {
                            aOut += '%';
                            aOut += hex[ch >> 4];
                            aOut += hex[ch & 0x0F];
                        }
                    }
                };

            std::string result = "file://";
            if (path.size() >= 2 && path[1] == ':' &&
                ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')))
            {
                // C:/foo -> file:///C:/foo (keep the drive colon literal)
                result += '/';
                result.append(path.substr(0, 2));
                path.remove_prefix(2);
            }
            else if (path.size() >= 2 && path[0] == '/' && path[1] == '/')
            {
                // UNC //server/share/foo -> file://server/share/foo
                path.remove_prefix(2);
            }
            // POSIX /foo -> file:///foo (leading '/' is part of path)
            encode(path, result);
            return result;
        }

        // RFC 3986: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) ":"
        // A single-letter scheme is treated as a Windows drive letter, not a scheme.
        bool has_scheme(std::string const& aUri, bool aAllowMissingScheme = false)
        {
            auto const colon = aUri.find(':');
            if (colon == 0 && aAllowMissingScheme)
                return true;
            if (colon == std::string::npos || colon < 2)
                return false;
            auto const alpha = [](char ch) { return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); };
            if (!alpha(aUri[0]))
                return false;
            for (std::size_t i = 1; i < colon; ++i)
            {
                char const ch = aUri[i];
                if (!alpha(ch) && !(ch >= '0' && ch <= '9') && ch != '+' && ch != '-' && ch != '.')
                    return false;
            }
            return true;
        }
    }

    uri_authority::uri_authority()
    {
    }

    uri_authority::uri_authority(const std::string& aAuthority)
    {
        parse_port(parse_host(parse_user_information(aAuthority)));
    }
    
    const uri_authority::optional_user_information& uri_authority::user_information() const
    {
        return iUserInformation;
    }

    const uri_authority::optional_host& uri_authority::host() const
    {
        return iHost;
    }

    const uri_authority::optional_port& uri_authority::port() const
    {
        return iPort;
    }

    std::string uri_authority::parse_user_information(const std::string& aRest)
    {
        neolib::vecarray<std::pair<std::string::const_iterator, std::string::const_iterator>, 2> bits;
        std::string sep("@");
        neolib::tokens(aRest.begin(), aRest.end(), sep.begin(), sep.end(), bits, 2, false, false);
        if (bits.size() == 2)
        {
            iUserInformation = std::string{ bits[0].first, bits[0].second };
            return std::string{ bits[1].first, bits[1].second };
        }
        else if (bits.size() == 1)
            return std::string{ bits[0].first, bits[0].second };
        else
            return {};
    }

    std::string uri_authority::parse_host(const std::string& aRest)
    {
        neolib::vecarray<std::pair<std::string::const_iterator, std::string::const_iterator>, 2> bits;
        std::string sep(":");
        neolib::tokens(aRest.begin(), aRest.end(), sep.begin(), sep.end(), bits, 2, false, false);
        if (bits.size() == 2)
        {
            iHost = std::string{ bits[0].first, bits[0].second };
            return std::string(bits[1].first, bits[1].second);
        }
        else if (bits.size() == 1)
            iHost = std::string(bits[0].first, bits[0].second);
        return {};
    }

    void uri_authority::parse_port(const std::string& aRest)
    {
        if (!aRest.empty())
            iPort = static_cast<port_type>(std::stoi(aRest));
    }

    uri::uri()
    {
    }

    uri::uri(const std::string& aUri)
    {
        if (aUri.empty() || has_scheme(aUri, true))
            parse(aUri);
        else
            parse(std::filesystem::path{ aUri });
    }

    uri::uri(std::filesystem::path const& aPath)
    {
        parse(aPath);
    }

    std::string uri::to_string() const
    {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    const std::string& uri::scheme() const
    {
        return iScheme;
    }

    const uri_authority& uri::authority() const
    {
        return iAuthority;
    }

    const std::string& uri::path() const
    {
        return iPath;
    }

    const std::string& uri::query() const
    {
        return iQuery;
    }

    const std::string& uri::fragment() const
    {
        return iFragment;
    }

    void uri::set_scheme(const std::string& aScheme)
    {
        iScheme = aScheme;
    }

    void uri::set_authority(const uri_authority& aAuthority)
    {
        iAuthority = aAuthority;
    }

    void uri::set_path(const std::string& aPath)
    {
        iPath = aPath;
    }

    void uri::set_query(const std::string& aQuery)
    {
        iQuery = aQuery;
    }

    void uri::set_fragment(const std::string& aFragment)
    {
        iFragment = aFragment;
    }

    void uri::parse(const std::string& aUri)
    {
        parse_authority(parse_path(parse_query(parse_fragment(parse_scheme(escaped(aUri))))));
    }

    void uri::parse(std::filesystem::path const& aPath)
    {
        parse_authority(parse_path(parse_query(parse_fragment(parse_scheme(escaped(to_file_url(aPath)))))));
    }

    void uri::parse_authority(const std::string& aRest)
    {
        iAuthority = uri_authority(aRest);
    }

    std::string uri::escaped(const std::string& aString)
    {
        static const std::unordered_set<char> sUrlChars = 
        {
            'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
            'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
            '0','1','2','3','4','5','6','7','8','9',
            '-','.','_','~',':','/','?','#','[',']','@','!','$','&','\'','(',')','*','+',',',';','=','%'
        };
        std::string escaped;
        for (auto ch : aString)
            if (sUrlChars.find(ch) != sUrlChars.end())
                escaped += ch;
            else
                escaped += "%" + uint32_to_string<char>(static_cast<uint8_t>(ch), 16, 2);
        return escaped;
    }

    std::string uri::unescaped(const std::string& aString)
    {
        std::string escaped;
        for (auto i = aString.begin(); i != aString.end(); ++i)
            if (*i != '%')
                escaped += *i;
            else if (std::distance(i, aString.end()) >= 3)
            {
                escaped += static_cast<char>(string_to_uint32(std::string{ i + 1, i + 3 }, 16));
                i += 2;
            }
        return escaped;
    }

    std::string uri::parse_path(const std::string& aRest)
    {
        neolib::vecarray<std::pair<std::string::const_iterator, std::string::const_iterator>, 2> bits;
        std::string sep{ "/" };
        neolib::tokens(aRest.begin(), aRest.end(), sep.begin(), sep.end(), bits, 2, false, false);
        if (bits.size() == 2)
            iPath = unescaped(std::string{ bits[1].first, aRest.end() });
        return std::string{ bits[0].first, bits[0].second };
    }

    std::string uri::parse_query(const std::string& aRest)
    {
        neolib::vecarray<std::string, 2> bits;
        neolib::tokens(aRest, std::string{ "?" }, bits, 2, false, false);
        if (bits.size() == 2)
            iQuery = unescaped(bits[1]);
        return bits.front();
    }

    std::string uri::parse_fragment(const std::string& aRest)
    {
        neolib::vecarray<std::string, 2> bits;
        neolib::tokens(aRest, std::string{ "#" }, bits, 2, false, false);
        if (bits.size() == 2)
            iFragment = unescaped(bits[1]);
        return bits.front();
    }

    std::string uri::parse_scheme(const std::string& aRest)
    {
        neolib::vecarray<std::string, 2> bits;
        neolib::tokens(aRest, std::string{ "://" }, bits, 2, false, true);
        if (bits.size() == 2)
            iScheme = unescaped(bits[0]);
        return bits.back();
    }
}
