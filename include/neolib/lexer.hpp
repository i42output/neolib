// lexer.hpp
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
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <boost/functional/hash.hpp>
#include <optional>
#include "variant.hpp"

namespace neolib
{
    struct lexer_atom_match_any {};

    inline bool operator==(const lexer_atom_match_any&, const lexer_atom_match_any&)
    {
        return true;
    }

    inline std::size_t hash_value(const lexer_atom_match_any&)
    {
        return 0u;
    }

    enum class lexer_atom_function
    {
        Eat,
        Keep,
        Not,
        End
    };

    template <typename Token, typename CharT = char>
    class lexer_token : public std::pair<Token, std::basic_string<CharT>>
    {
    public:
        typedef Token token_type;
        typedef std::basic_string<CharT> value_type;
    private:
        typedef std::pair<Token, std::basic_string<CharT>> base_type;
    public:
        lexer_token() :
            base_type{}
        {
        }
        lexer_token(token_type aToken, const value_type& aValue) :
            base_type{aToken, aValue}
        {
        }
    public:
        token_type token() const
        {
            return first;
        }
        const value_type& value() const
        {
            return second;
        }
    };
        
    struct no_scopes {};

    template <typename Token, typename Scope = no_scopes, typename CharT = char>
    class lexer_atom
    {
    public:
        typedef Token token_type;
        typedef Scope scope_type;
        typedef CharT char_type;
        typedef std::pair<char_type, char_type> range_type;
        typedef std::basic_string<char_type> string_type;
        typedef std::vector<lexer_atom_function> function_list;
        typedef std::pair<token_type, function_list> function_type;
        typedef std::pair<scope_type, bool> scope_change_type;
        typedef variant<char_type, range_type, string_type, lexer_atom_match_any, token_type, function_type, scope_type, scope_change_type> value_type;
        typedef string_type token_value_type;
    public:
        struct not_token : std::logic_error { not_token(const std::string& aBadToken) : std::logic_error("Invalid token: '" + aBadToken + "'") {} };
        struct not_scope : std::logic_error { not_scope(const std::string& aBadScope) : std::logic_error("Invalid scope: '" + aBadScope + "'") {} };
    public:
        lexer_atom() :
            iValue{}
        {
        }
        template <typename T>
        lexer_atom(const T& aValue, const token_value_type& aTokenValue = token_value_type{}) :
            iValue{ aValue }, iTokenValue{ aTokenValue }
        {
        }
        template <typename T>
        lexer_atom(lexer_atom_function aFunction, const T& aValue, const token_value_type& aTokenValue = token_value_type{}) :
            iValue{ function_type{ aValue, { aFunction } } }, iTokenValue{ aTokenValue }
        {
        }
    public:
        bool operator==(const lexer_atom& aOther) const
        {
            return iValue == aOther.iValue && iTokenValue == aOther.iTokenValue;
        }
        bool operator!=(const lexer_atom& aOther) const
        {
            return !(*this == aOther);
        }
    public:
        template <typename T>
        bool is() const
        {
            return std::holds_alternative<T>(iValue);
        }
        const value_type& value() const
        {
            return iValue;
        }
        bool is_token() const
        {
            return std::holds_alternative<token_type>(iValue) || std::holds_alternative<function_type>(iValue);
        }
        token_type token() const
        {
            if (std::holds_alternative<token_type>(iValue))
                return static_variant_cast<token_type>(iValue);
            else if (std::holds_alternative<function_type>(iValue))
                return static_variant_cast<const function_type&>(iValue).first;
            else if (std::holds_alternative<char_type>(iValue))
                throw not_token(std::string(1, static_variant_cast<char_type>(iValue)));
            else if (std::holds_alternative<string_type>(iValue))
                throw not_token(static_variant_cast<const string_type&>(iValue));
            else
                throw not_token("???");
        }
        void set_token(token_type aToken)
        {
            iValue = aToken;
        }
        bool is_scope() const
        {
            return std::holds_alternative<scope_type>(iValue);
        }
        scope_type scope() const
        {
            if (std::holds_alternative<scope_type>(iValue))
                return static_variant_cast<scope_typ>(iValue);
            else
                throw not_scope("???");
        }
        void set_scope(scope_type aScope)
        {
            iValue = aScope;
        }
        bool has_functions() const
        {
            return std::holds_alternative<function_type>(iValue);
        }
        const function_list& functions() const
        {
            return static_variant_cast<const function_type&>(iValue).second;
        }
        function_list& functions()
        {
            return static_variant_cast<function_type&>(iValue).second;
        }
        const token_value_type& token_value() const
        {
            return iTokenValue;
        }
        token_value_type& token_value()
        {
            return iTokenValue;
        }
    private:
        value_type iValue;
        token_value_type iTokenValue;
    };

    template <typename T, typename Token, typename Scope, typename CharT>
    inline bool holds_alternative(const lexer_atom<Token, Scope, CharT>& aAtom)
    {
        return aAtom.is<T>();
    }

    template <typename Atom>
    class lexer_rule
    {
    public:
        typedef Atom atom_type;
        typedef typename atom_type::char_type char_type;
        typedef typename atom_type::token_type token_type;
        typedef typename atom_type::scope_type scope_type;
        typedef typename atom_type::range_type range_type;
    public:
        atom_type symbol;
        std::vector<atom_type> expression;
    public:
        static constexpr std::pair<scope_type, bool> enter_scope(scope_type aScope)
        {
            return std::make_pair(aScope, true);
        }
        static constexpr std::pair<scope_type, bool> leave_scope(scope_type aScope)
        {
            return std::make_pair(aScope, false);
        }
        static constexpr atom_type token_end(token_type aToken)
        {
            return atom_type{ lexer_atom_function::End, aToken };
        }
        static atom_type token_end(atom_type aAtom)
        {
            if (aAtom.has_functions())
            {
                aAtom.functions().push_back(lexer_atom_function::End);
                return aAtom;
            }
            return typename atom_type::function_type{ aAtom.token(),{ lexer_atom_function::End } };
        }
        static constexpr atom_type token_eat(token_type aToken)
        {
            return atom_type{ lexer_atom_function::Eat, aToken };
        }
        static atom_type token_eat(atom_type aAtom)
        {
            if (aAtom.has_functions())
            {
                aAtom.functions().push_back(lexer_atom_function::Eat);
                return aAtom;
            }
            return typename atom_type::function_type{ aAtom.token(),{ lexer_atom_function::Eat } };
        }
        static constexpr atom_type token_keep(token_type aToken)
        {
            return atom_type{ lexer_atom_function::Keep, aToken };
        }
        static atom_type token_keep(atom_type aAtom)
        {
            if (aAtom.has_functions())
            {
                aAtom.functions().push_back(lexer_atom_function::Keep);
                return aAtom;
            }
            return typename atom_type::function_type{ aAtom.token(),{ lexer_atom_function::Keep } };
        }
        static constexpr atom_type token_make(token_type aToken, char_type aChar)
        {
            return atom_type{aToken, typename atom_type::token_value_type{ 1, aChar } };
        }
        static constexpr atom_type token_not(token_type aToken)
        {
            return atom_type{ lexer_atom_function::Not, aToken };
        }
        static constexpr range_type token_range(char_type aFrom, char_type aTo)
        {
            return range_type{ aFrom, aTo };
        }
        static constexpr lexer_atom_match_any token_any()
        {
            return lexer_atom_match_any{};
        }
    };

    template <typename Atom>
    class lexer
    {
    public:
        typedef Atom atom_type;
        typedef typename atom_type::token_type token_type;
        typedef typename atom_type::scope_type scope_type;
        typedef typename atom_type::scope_change_type scope_change_type;
        typedef typename atom_type::char_type char_type;
        typedef lexer_token<token_type, char_type> lexer_token_type;
        typedef typename atom_type::range_type range_type;
        typedef typename atom_type::string_type string_type;
        typedef typename atom_type::function_type function_type;
        typedef lexer_rule<atom_type> rule_type;
    private:
        typedef std::shared_ptr<std::istream> stream_pointer;
    public:
        class context
        {
            friend class lexer;
        public:
            context(const lexer& aParent) :
                iParent{ aParent },
                iFinished{ false }, iError{ false }, iCharIndex{}, iLineIndex{}, iColumnIndex{}, iPreviousChar{}, iInputBufferIndex{}
            {
            }
        public:
            context& operator>>(lexer_token_type& aToken)
            {
                try
                {
                    atom_type atom;
                    iParent.get_token(*this, atom);
                    if (*this)
                        aToken = lexer_token_type{ atom.token(), atom.token_value() };
                }
                catch (std::exception& e)
                {
                    throw_with_info<std::exception>(e.what());
                }
                catch (...)
                {
                    throw_with_info<std::exception>("unknown exception");
                }
                return *this;
            }
            explicit operator bool() const
            {
                return !iError;
            }
        private:
            template <typename Exception>
            void throw_with_info(const std::string& aReason)
            {
                static std::ostringstream oss;
                oss.str("");
                oss << "Lexer error: " << aReason << std::endl;
                oss << "Line: " << iLineIndex << std::endl;
                oss << "Column: " << iColumnIndex << std::endl;
                throw Exception{ oss.str().c_str() };
            }
        private:
            const lexer& iParent;
            stream_pointer iInput;
            bool iFinished;
            bool iError;
            string_type iInputBuffer;
            std::size_t iInputBufferIndex;
            uint32_t iCharIndex;
            uint32_t iLineIndex;
            uint32_t iColumnIndex;
            char iPreviousChar;
            std::deque<atom_type> iQueue;
        };
    private:
        typedef std::vector<rule_type> rule_list;
        enum class search_type
        {
            Token,
            String
        };
        enum class match_result
        {
            None,
            Partial,
            Complete
        };
        class node
        {
        public:
            typedef neolib::variant<token_type, function_type, scope_change_type> value_type;
            typedef std::pair<node*, value_type> next_type;
            typedef std::optional<next_type> optional_next_type;
        public:
            struct bad_terminal_atom : std::logic_error { bad_terminal_atom() : std::logic_error("neolib::lexer::node::bad_terminal_atom") {} };
            struct unsupported_atom_type : std::logic_error { unsupported_atom_type() : std::logic_error("neolib::lexer::node::unsupported_atom_type") {} };
            struct node_exists : std::logic_error { node_exists() : std::logic_error("neolib::lexer::node::node_exists") {} };
            struct invalid_atom : std::invalid_argument { invalid_atom() : std::invalid_argument("neolib::lexer::node::invalid_atom") {} };
        public:
            node(lexer& aParent) :
                iParent{ aParent }, iTokenMap {}, iCharMap{}
            {
            }
        public:
            void map(const atom_type& aAtom, node& aNextNode)
            {
                if (holds_alternative<char_type>(aAtom))
                    map(static_variant_cast<char_type>(aAtom.value()), aNextNode);
                else if (holds_alternative<token_type>(aAtom))
                    map(static_variant_cast<token_type>(aAtom.value()), aNextNode);
            }
            void map(const atom_type& aAtom, const atom_type& aTerminalAtom)
            {
                if (holds_alternative<token_type>(aTerminalAtom))
                {
                    if (holds_alternative<char_type>(aAtom))
                        map(static_variant_cast<char_type>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
                    else if (holds_alternative<token_type>(aAtom))
                        map(static_variant_cast<token_type>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
                    else if (holds_alternative<function_type>(aAtom))
                        map(static_variant_cast<const function_type&>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
                }
                else if (holds_alternative<function_type>(aTerminalAtom))
                {
                    if (holds_alternative<char_type>(aAtom))
                        map(static_variant_cast<char_type>(aAtom.value()), static_variant_cast<const function_type&>(aTerminalAtom.value()));
                    else if (holds_alternative<token_type>(aAtom))
                        map(static_variant_cast<token_type>(aAtom.value()), static_variant_cast<const function_type&>(aTerminalAtom.value()));
                    else if (holds_alternative<function_type>(aAtom))
                        map(static_variant_cast<const function_type&>(aAtom.value()), static_variant_cast<const function_type&>(aTerminalAtom.value()));
                }
                else
                    throw bad_terminal_atom();
            }
            void map(char_type aChar, node& aNextNode)
            {
                if (iCharMap[aChar].first == nullptr)
                    iCharMap[aChar].first = &aNextNode;
                else
                    throw node_exists();
            }
            void map(token_type aToken, node& aNextNode)
            {
                if (iTokenMap[aToken].first == nullptr)
                    iTokenMap[aToken].first = &aNextNode;
                else
                    throw node_exists();
            }
            void map(const function_type& aFunction, node& aNextNode)
            {
                if (iFunctionMap[aFunction].first == nullptr)
                    iFunctionMap[aFunction].first = &aNextNode;
                else
                    throw node_exists();
            }
            void map(char_type aChar, token_type aTerminalToken)
            {
                iCharMap[aChar].second = aTerminalToken;
            }
            void map(token_type aToken, token_type aTerminalToken)
            {
                iTokenMap[aToken].second = aTerminalToken;
            }
            void map(const function_type& aFunction, token_type aTerminalToken)
            {
                iFunctionMap[aFunction].second = aTerminalToken;
            }
            void map(char_type aChar, const function_type& aTerminalFunction)
            {
                iCharMap[aChar].second = aTerminalFunction;
            }
            void map(token_type aToken, const function_type& aTerminalFunction)
            {
                iTokenMap[aToken].second = aTerminalFunction;
            }
            void map(const function_type& aFunction, const function_type& aTerminalFunction)
            {
                iFunctionMap[aFunction].second = aTerminalFunction;
            }
            const next_type& lookup(const atom_type& aAtom) const
            {
                if (holds_alternative<char_type>(aAtom))
                    return lookup(static_variant_cast<char_type>(aAtom.value()));
                else if (holds_alternative<token_type>(aAtom))
                    return lookup(static_variant_cast<token_type>(aAtom.value()));
                else if (holds_alternative<function_type>(aAtom))
                    return lookup(static_variant_cast<const function_type&>(aAtom.value()));
                else
                    throw unsupported_atom_type();
            }
            const next_type& lookup(char_type aChar) const
            {
                return iCharMap[aChar];
            }
            const next_type& lookup(token_type aToken) const
            {
                return iTokenMap[aToken];
            }
            const next_type& lookup(const function_type& aFunction) const
            {
                return iFunctionMap[aFunction];
            }
            template <typename Iter>
            std::pair<match_result, value_type> match(Iter aFirst, Iter aLast, search_type aSearchType) const
            {
                atom_type atom = *aFirst++;
                auto atomMatch = match_atom(atom, aSearchType);
                if (atomMatch)
                {
                    if (aFirst == aLast)
                    {
                        if (atomMatch->second != neolib::none)
                            return std::make_pair(match_result::Complete, atomMatch->second);
                        else if (atomMatch->first != nullptr)
                            return std::make_pair(match_result::Partial, value_type{});
                        else
                            return std::make_pair(match_result::None, value_type{});
                    }
                    else if (atomMatch->first != nullptr && (!holds_alternative<char_type>(atom) || aSearchType == search_type::String))
                        return atomMatch->first->match(aFirst, aLast, aSearchType);
                    else
                        return std::make_pair(match_result::None, value_type{});
                }
                else
                    return std::make_pair(match_result::None, value_type{});
            }
        private:
            optional_next_type match_atom(const atom_type& aAtom, search_type aSearchType) const
            {
                if (holds_alternative<char_type>(aAtom))
                {
                    if (this == &*iParent.iNodes.begin() || aSearchType == search_type::String)
                    {
                        auto existing = iCharMap.find(static_variant_cast<char_type>(aAtom.value()));
                        if (existing != iCharMap.end())
                            return existing->second;
                    }
                    return optional_next_type{};
                }
                else if (holds_alternative<token_type>(aAtom))
                {
                    auto token = static_variant_cast<token_type>(aAtom.value());
                    auto existingToken = iTokenMap.find(token);
                    if (existingToken != iTokenMap.end())
                        return existingToken->second;
                    for (auto iterFunction = iFunctionMap.begin(); iterFunction != iFunctionMap.end(); ++iterFunction)
                    {
                        auto functionToken = iterFunction->first.first;
                        auto functions = iterFunction->first.second;
                        bool not_ = (std::find(functions.begin(), functions.end(), lexer_atom_function::Not) != functions.end());
                        if ((functionToken == token && !not_) || (functionToken != token && not_))
                            return iterFunction->second;
                    }
                    return optional_next_type{};
                }
                throw invalid_atom();
            }
        private:
            lexer& iParent;
            mutable std::unordered_map<char_type, next_type> iCharMap;
            mutable std::unordered_map<token_type, next_type> iTokenMap;
            mutable std::unordered_map<function_type, next_type, boost::hash<function_type>> iFunctionMap;
            mutable std::unordered_map<scope_type, next_type, boost::hash<function_type>> iScopeMap;
        };
        typedef std::list<node> node_list;
        typedef std::shared_ptr<std::istream> stream_pointer;
    public:
        struct style_sheet_not_utf8 : std::runtime_error { style_sheet_not_utf8(const std::string& reason = "neolib::lexer_atom::style_sheet_not_utf8") : std::runtime_error(reason) {} };
        struct bad_lex_tree : std::logic_error { bad_lex_tree(const std::string& reason = "neolib::lexer_atom::bad_lex_tree") : std::logic_error(reason) {} };
        struct end_of_file_reached : std::runtime_error { end_of_file_reached(const std::string& reason = "neolib::lexer_atom::end_of_file_reached") : std::runtime_error(reason) {} };
        struct invalid_token : std::runtime_error { invalid_token(const std::string& reason = "neolib::lexer_atom::invalid_token") : std::runtime_error(reason) {} };
    public:
        template <typename Iter>
        lexer(Iter aFirstRule, Iter aLastRule) :
            iDefaultScope{}
        {
            for (auto r = aFirstRule; r != aLastRule; ++r)
                build(*r);
        }
        template <typename Iter>
        lexer(scope_type aDefaultScope, Iter aFirstRule, Iter aLastRule) : 
            iDefaultScope{ aDefaultScope }
        {
            for (auto r = aFirstRule; r != aLastRule; ++r)
                build(*r);
        }
    public:
        context open(const std::string& aPath) const
        {
            context newContext{ *this };
            newContext.iInput = std::make_shared<std::ifstream>(aPath, std::ios_base::in | std::ios_base::binary);
            return newContext;
        }
        context use(std::istream& aStream) const
        {
            context newContext{ *this };
            newContext.iInput = stream_pointer(stream_pointer{}, &aStream);
            return newContext;
        }
        context use(const std::string& aText) const
        {
            context newContext{ *this };
            newContext.iInput = std::make_shared<std::istringstream>(aText);
            return newContext;
        }
    private:
        const lexer& get_token(context& aContext, atom_type& aAtom) const
        {
            if (aContext.iFinished && aContext.iQueue.empty())
            {
                aContext.iError = true;
                return *this;
            }
            if (aContext.iQueue.empty())
                if (!next(aContext))
                    return *this;
            bool backup = false;
            for (auto iter = aContext.iQueue.end(); iter != aContext.iQueue.begin(); iter = (backup ? aContext.iQueue.end() : iter - 1))
            {
                backup = false;
                auto match = iNodes.front().match(iter - 1, aContext.iQueue.end(), search_type::Token);
                if (match.first == match_result::Partial)
                {
                    if (!next(aContext))
                        throw end_of_file_reached();
                    backup = true;
                    continue;
                }
                if (match.first == match_result::Complete)
                {
                    atom_type atom = atom_type{};
                    bool endToken = false;
                    if (std::holds_alternative<token_type>(match.second))
                        atom = atom_type{ static_variant_cast<token_type>(match.second) };
                    else if (std::holds_alternative<function_type>(match.second))
                    {
                        auto& functions = static_variant_cast<const function_type&>(match.second);
                        atom = atom_type{ functions.first };
                        auto iter2 = iter - 1;
                        for (auto f = functions.second.begin(); f != functions.second.end(); ++f)
                        {
                            switch (*f)
                            {
                            case lexer_atom_function::Eat:
                                if (iter2 != aContext.iQueue.end())
                                {
                                    std::ptrdiff_t diff = iter - iter2;
                                    iter2 = aContext.iQueue.erase(iter2);
                                    iter = iter2 + diff;
                                }
                                break;
                            case lexer_atom_function::Keep:
                                if (iter2 != aContext.iQueue.end())
                                    ++iter2;
                                break;
                            case lexer_atom_function::End:
                                endToken = true;
                                break;
                            }
                        }
                    }
                    for (auto iter2 = iter - 1; iter2 != aContext.iQueue.end(); ++iter2)
                        atom.token_value().append(iter2->token_value());
                    if (endToken)
                    {
                        aContext.iQueue.clear();
                        aContext.iQueue.push_back(atom);
                        break;
                    }
                    else
                    {
                        if (iter != aContext.iQueue.end() || aContext.iQueue.back() != atom)
                        {
                            aContext.iQueue.erase(iter - 1, aContext.iQueue.end());
                            aContext.iQueue.insert(aContext.iQueue.end(), atom);
                            backup = true;
                        }
                        else
                        {
                            if (iter - 1 == aContext.iQueue.begin())
                            {
                                if (next(aContext))
                                    backup = true;
                            }
                        }
                    }
                }
                else if (match.first == match_result::None)
                {
                    if (iter - 1 == aContext.iQueue.begin())
                    {
                        next(aContext);
                        break;
                    }
                }
            }
            auto match = iNodes.front().match(aContext.iQueue[0].token_value().begin(), aContext.iQueue[0].token_value().end(), search_type::String);
            if (match.first == match_result::Complete)
            {
                bool changed = false;
                if (std::holds_alternative<token_type>(match.second))
                {
                    auto token = static_variant_cast<token_type>(match.second);
                    if (aContext.iQueue[0].token() != token)
                    {
                        aContext.iQueue[0].set_token(token);
                        changed = true;
                    }
                }
                else if (std::holds_alternative<function_type>(match.second))
                {
                    auto token = static_variant_cast<const function_type&>(match.second).first;
                    if (aContext.iQueue[0].token() != token)
                    {
                        aContext.iQueue[0].set_token(token);
                        changed = true;
                    }
                }
                if (changed)
                    return get_token(aContext, aAtom); // recurse
            }
            aAtom = aContext.iQueue[0];
            aContext.iQueue.pop_front();
            return *this;
        }
        void build(const rule_type& aRule)
        {
            if (iNodes.empty())
                iNodes.push_back(node{ *this });
            build(aRule, 0u, iNodes.front());
        }
        node& build(const rule_type& aRule, std::size_t aExpressionIndex, node& aNode)
        {
            return build(aRule, aRule.expression[aExpressionIndex], aExpressionIndex, aNode);
        }
        node& build(const rule_type& aRule, const atom_type& aAtom, std::size_t aExpressionIndex, node& aNode, bool aHalt = false)
        {
            if (holds_alternative<range_type>(aAtom))
            {
                auto& r = static_variant_cast<const range_type&>(aAtom.value());
                for (char_type ch = r.first; ch < r.second; ++ch)
                    build(aRule, ch, aExpressionIndex, aNode);
                return build(aRule, r.second, aExpressionIndex, aNode);
            }
            else if (holds_alternative<string_type>(aAtom))
            {
                auto s = static_variant_cast<const std::string&>(aAtom.value());
                auto& n = build(aRule, s[0], aExpressionIndex, aNode, s.size() > 1 ? true : false);
                s = s.substr(1);
                if (!s.empty())
                    return build(aRule, s, aExpressionIndex, n);
                else
                    return n;
            }
            else if (aExpressionIndex == aRule.expression.size() - 1 && !aHalt)
            {
                aNode.map(aAtom, aRule.symbol);
                return aNode;
            }
            else
            {
                auto& next = aNode.lookup(aAtom);
                if (next.first != nullptr)
                {
                    if (!aHalt)
                        return build(aRule, aExpressionIndex + 1, *next.first);
                    else
                        return *next.first;
                }
                else
                {
                    iNodes.push_back(node{ *this });
                    auto& newNode = iNodes.back();
                    aNode.map(aAtom, newNode);
                    if (!aHalt)
                        return build(aRule, aExpressionIndex + 1, newNode);
                    else
                        return newNode;
                }
            }
        }
        bool next(context& aContext) const
        {
            const std::size_t BUF_SIZE = 32;
            if (aContext.iInputBuffer.empty())
            {
                aContext.iInputBuffer.resize(BUF_SIZE);
                aContext.iInput->read(&aContext.iInputBuffer[0], BUF_SIZE);
                std::streamsize amount = aContext.iInput->gcount();
                aContext.iInputBuffer.resize(static_cast<std::size_t>(amount));
                if (amount == 0)
                {
                    if (aContext.iCharIndex == 0)
                        aContext.iError = true;
                    aContext.iFinished = true;
                    return false;
                }
            }
            if (aContext.iCharIndex == 0)
            {
                const string_type BOM_UTF8 = "\xEF\xBB\xBF";
                const string_type BOM_UTF16LE = "\xFF\xFE";
                const string_type BOM_UTF16BE = "\xFE\xFF";
                if (aContext.iInputBuffer.find(BOM_UTF8) == 0)
                    aContext.iInputBuffer = aContext.iInputBuffer.substr(BOM_UTF8.size());
                else if (aContext.iInputBuffer.find(BOM_UTF16LE) == 0)
                    throw style_sheet_not_utf8();
                else if (aContext.iInputBuffer.find(BOM_UTF16BE) == 0)
                    throw style_sheet_not_utf8();
            }
            if (aContext.iPreviousChar == '\n' || aContext.iCharIndex == 0)
            {
                ++aContext.iLineIndex;
                aContext.iColumnIndex = 0;
            }
            ++aContext.iColumnIndex;
            ++aContext.iCharIndex;
            char_type ch = aContext.iInputBuffer[aContext.iInputBufferIndex];
            aContext.iPreviousChar = ch;
            aContext.iQueue.emplace_back(ch, string_type(1, ch));
            if (++aContext.iInputBufferIndex == aContext.iInputBuffer.size())
            {
                aContext.iInputBuffer.clear();
                aContext.iInputBufferIndex = 0;
            }
            return true;
        }
    private:
        scope_type iDefaultScope;
        node_list iNodes;
    };
}
