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
#include <neolib/app/logger.hpp>

namespace neolib
{
    namespace logger
    {
        template <std::size_t Instance = 0, typename CharT = char, typename Traits = std::char_traits<CharT>>
        class basic_ostream_logger : public logger<Instance>
        {
        public:
            basic_ostream_logger(std::basic_ostream<CharT, Traits>& aStream) :
                iStream{ aStream }
            {
            }
            ~basic_ostream_logger()
            {
                {
                    std::unique_lock<std::mutex> lk(commit_signal_mutex());
                    set_destroying();
                }
                commit();
            }
        public:
            using logger::commit;
        protected:
            void commit(buffer_t const& aBuffer) override
            {
                std::lock_guard<std::recursive_mutex> lg{ mutex() };
                iStream << aBuffer << std::flush;
            }
        private:
            std::basic_ostream<CharT, Traits>& iStream;
        };

        template <std::size_t Instance = 0>
        using ostream_logger = basic_ostream_logger<Instance>;
    }
}
