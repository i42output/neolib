// ostream_logger.hpp
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

#pragma once

#include <neolib/neolib.hpp>
#include <ostream>
#include <neolib/app/i_logger.hpp>

namespace neolib
{
    namespace logger
    {
        template <typename CharT, typename Traits = std::char_traits<CharT>>
        class basic_ostream_logger : public i_logger
        {
        public:
            basic_ostream_logger(std::basic_ostream<CharT, Traits>& aStream) :
                iStream{ aStream }
            {
            }
        public:
            severity filter() const override
            {
                return iFilterSeverity;
            }
            void set_filter(severity aSeverity) override
            {
                iFilterSeverity = aSeverity;
            }
        public:
            i_logger& operator<<(severity aSeverity) override
            {
                iMessageSeverity = aSeverity;
                return *this;
            }
            i_logger& operator<<(i_string const& aMessage) override
            {
                if (iMessageSeverity >= iFilterSeverity)
                    iStream << aMessage.to_std_string_view();
                return *this;
            }
            i_logger& operator<<(endl_t) override
            {
                if (iMessageSeverity >= iFilterSeverity)
                    iStream << std::endl;
                return *this;
            }
            i_logger& operator<<(flush_t) override
            {
                if (iMessageSeverity >= iFilterSeverity)
                    iStream << std::flush;
                return *this;
            }
        private:
            std::basic_ostream<CharT, Traits>& iStream;
            severity iFilterSeverity = severity::Info;
            severity iMessageSeverity = severity::Info;
        };

        using ostream_logger = basic_ostream_logger<char>;
    }
}
