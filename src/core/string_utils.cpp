// string_utils.cpp
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
#include <neolib/core/string_utils.hpp>

namespace neolib
{
    string_search_fsa::string_search_fsa() :
        iRoot{}
    {
    }

    void string_search_fsa::add_pattern(std::string const& aPattern, action_t aAction)
    {
        iPatterns[aPattern] = aAction;
        rebuild();
    }

    void string_search_fsa::search(std::string const& aText) const
    {
        auto const begin = aText.data();
        auto const end = aText.data() + aText.size();
        results_t results;
        for (auto i = begin; i != end; ++i)
            search(iRoot, nullptr, i, end, false, results);
        if (results.empty())
            return;
        for (auto i = results.begin(); i != std::prev(results.end());)
        {
            auto j = std::next(i);
            if (std::get<0>(*i) != std::get<0>(*j))
                ++i;
            else if (std::get<1>(*i) <= std::get<1>(*j) && std::get<2>(*i) >= std::get<2>(*j))
                results.erase(j);
            else if (std::get<1>(*i) >= std::get<1>(*j) && std::get<2>(*i) <= std::get<2>(*j))
                i = results.erase(i);
            else
                ++i;
        }
        for (auto& result : results)
            (*std::get<0>(result))(std::get<1>(result), std::get<2>(result));
    }

    void string_search_fsa::search(state const& aState, char const* aStart, char const* aNext, char const* aEnd, bool aSearchingWildcard, results_t& aResults) const
    {
        if (aNext == aEnd)
        {
            auto wildcard = aState.match.find('*');
            if (wildcard != aState.match.end() && wildcard->second.match.empty())
                for (auto const& action : wildcard->second.actions)
                    aResults.insert(std::make_tuple(&action, aStart, aEnd));
            return;
        }
        for (auto existing : !aSearchingWildcard ?
            std::initializer_list<decltype(aState.match)::const_iterator>{ aState.match.find(*aNext), aState.match.find('?'), aState.match.find('*') } :
            std::initializer_list<decltype(aState.match)::const_iterator>{ aState.match.find('*') })
            if (existing != aState.match.end())
            {
                bool const wildcard = (existing->first == '*');
                if (!wildcard && existing->second.match.empty())
                    for (auto const& action : existing->second.actions)
                        aResults.insert(std::make_tuple(&action, aStart, std::next(aNext)));
                search(existing->second, aStart == nullptr ? aNext : aStart, std::next(aNext), aEnd, false, aResults);
                if (wildcard)
                {
                    if (!aSearchingWildcard)
                        search(existing->second, aStart, aNext, aEnd, false, aResults);
                    search(aState, aStart, std::next(aNext), aEnd, true, aResults);
                }
            }
    }

    void string_search_fsa::rebuild()
    {
        iRoot = {};
        for (auto const& pattern : iPatterns)
        {
            state* s = &iRoot;
            for (auto const& ch : pattern.first)
            {
                s = &s->match[ch];
                if (&ch == &pattern.first.back())
                    s->actions.push_back(pattern.second);
            }
        }
    }
}
