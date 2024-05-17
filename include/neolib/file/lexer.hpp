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

#include <neolib/neolib.hpp>

#include <concepts>
#include <vector>
#include <variant>
#include <optional>
#include <string>
#include <istream>

namespace neolib
{
    enum class lexer_component_type
    {
        Terminal,
        Tuple,
        Choice,
        Sequence,
        Repeat,
        Range,
        Optional,
        Discard,
        Primitive,
        Atom,
        Rule
    };

    template <lexer_component_type Type>
    struct lexer_component {};

    template <typename Token>
    class lexer
    {
    public:
        using token = Token;

        using terminal_character = std::optional<char>;

        struct terminal : terminal_character, std::string_view, lexer_component<lexer_component_type::Terminal>
        {
            using token_type = token;

            using base_type = std::string_view;
            using base_type::base_type;

            terminal(char character) :
                terminal_character{ character }, base_type{ &terminal_character::value(), &terminal_character::value() + 1 }
            {}
        };

        struct primitive_atom;

        struct tuple : lexer_component<lexer_component_type::Tuple>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            tuple(value_type const& value) :
                value{ value }
            {}

            tuple(primitive_atom const& primitive) :
                value{ primitive }
            {}

            tuple(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            tuple(tuple const& tuple, primitive_atom const& primitive) :
                value{ tuple.value }
            {
                value.push_back(primitive);
            }
        };

        struct choice : lexer_component<lexer_component_type::Choice>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            choice(value_type const& value) :
                value{ value }
            {}

            choice(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value = std::get<tuple>(primitive).value;
                else
                    value.push_back(primitive);
            }

            choice(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            choice(choice const& choice, primitive_atom const& primitive) :
                value{ choice.value }
            {
                value.push_back(primitive);
            }
        };

        struct sequence : lexer_component<lexer_component_type::Sequence>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            sequence(value_type const& value) :
                value{ value }
            {}

            sequence(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value = std::get<tuple>(primitive).value;
                else
                    value.push_back(primitive);
            }

            sequence(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            sequence(sequence const& sequence, primitive_atom const& primitive) :
                value{ sequence.value }
            {
                value.push_back(primitive);
            }
        };

        struct repeat : lexer_component<lexer_component_type::Repeat>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            repeat(value_type const& value) :
                value{ value }
            {}

            repeat(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value = std::get<tuple>(primitive).value;
                else
                    value.push_back(primitive);
            }

            repeat(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            repeat(repeat const& repeat, primitive_atom const& primitive) :
                value{ repeat.value }
            {
                value.push_back(primitive);
            }
        };

        struct range : lexer_component<lexer_component_type::Range>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            range(value_type const& value) :
                value{ value }
            {}

            range(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value = std::get<tuple>(primitive).value;
                else
                    value.push_back(primitive);
            }

            range(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            range(range const& range, primitive_atom const& primitive) :
                value{ range.value }
            {
                value.push_back(primitive);
            }
        };

        struct optional : lexer_component<lexer_component_type::Optional>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            optional(value_type const& value) :
                value{ value }
            {}

            optional(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value.push_back(sequence{ primitive });
                else
                    value.push_back(primitive);
            }

            optional(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            optional(optional const& optional, primitive_atom const& primitive) :
                value{ optional.value }
            {
                value.push_back(primitive);
            }
        };

        struct discard : lexer_component<lexer_component_type::Discard>
        {
            using token_type = token;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            discard(value_type const& value) :
                value{ value }
            {}

            discard(primitive_atom const& primitive)
            {
                if (std::holds_alternative<tuple>(primitive))
                    value.push_back(sequence{ primitive });
                else
                    value.push_back(primitive);
            }

            discard(primitive_atom const& lhs, primitive_atom const& rhs) :
                value{ lhs, rhs }
            {}

            discard(discard const& discard, primitive_atom const& primitive) :
                value{ discard.value }
            {
                value.push_back(primitive);
            }
        };

        struct primitive_atom : std::variant<token, terminal, tuple, choice, sequence, repeat, range, optional, discard>, lexer_component<lexer_component_type::Primitive>
        {
            using token_type = token;

            using base_type = std::variant<token, terminal, tuple, choice, sequence, repeat, range, optional, discard>;
            using base_type::base_type;

            bool is_tuple() const
            {
                return 
                    std::holds_alternative<tuple>(*this) ||
                    std::holds_alternative<choice>(*this) ||
                    std::holds_alternative<sequence>(*this) ||
                    std::holds_alternative<repeat>(*this);
            }
        };

        struct atom : std::vector<primitive_atom>, lexer_component<lexer_component_type::Atom>
        {
            using token_type = token;

            using base_type = std::vector<primitive_atom>;
            using base_type::base_type;

            atom(atom const& lhs) :
                base_type{ lhs }
            {
            }

            atom(primitive_atom const& lhs) :
                base_type{ 1, lhs }
            {
            }

            template <typename T>
            atom(atom const& lhs, T&& rhs) :
                base_type{ lhs}
            {
                base_type::push_back(std::forward<T>(rhs));
            }

            atom(atom const& lhs, primitive_atom const& rhs) :
                base_type{ lhs }
            {
                if (!rhs.is_tuple())
                {
                    if (!base_type::empty() && !base_type::back().is_tuple())
                        base_type::back() = tuple{ base_type::back() };
                    if (!base_type::empty() && std::holds_alternative<tuple>(base_type::back()))
                        std::get<tuple>(base_type::back()).value.push_back(rhs);
                    else
                        base_type::push_back(rhs);
                }
                else
                    base_type::push_back(rhs);
            }
        };

        struct rule : lexer_component<lexer_component_type::Rule>
        {
            using token_type = token;

            atom lhs;
            atom rhs;

            template <typename Lhs, typename Rhs>
            rule(Lhs&& lhs, Rhs&& rhs) :
                lhs{ std::forward<Lhs>(lhs) },
                rhs{ std::forward<Rhs>(rhs) }
            {}

            template <typename Lhs>
            rule(Lhs&& lhs, primitive_atom const& rhs) :
                lhs{ std::forward<Lhs>(lhs) },
                rhs{ rhs }
            {}
        };

        struct ast_node
        {
            token token;
            std::string_view value;
            std::vector<ast_node> children;
        };

    public:
        template <std::size_t RuleCount>
        lexer(rule const (&aRules)[RuleCount]) :
            iRules{ &aRules[0], &aRules[0] + RuleCount }
        {
        }

    public:
        bool parse(token aRoot, std::string_view const& aSource)
        {
            ast_node rootNode{ aRoot, { aSource } };
            bool ok = parse(rootNode, aSource);
            iAst = std::move(rootNode);
            return ok;
        }

    private:
        bool parse(ast_node& aNode, std::string_view const& aSource)
        {
            return true;
        }

    private:
        std::vector<rule> iRules;
        ast_node iAst;
    };

    template <typename Token>
    using lexer_terminal = typename lexer<Token>::terminal;

    template <typename Token>
    using lexer_primitive = typename lexer<Token>::primitive_atom;

    template <typename Token>
    using lexer_atom = typename lexer<Token>::atom;

    template <typename Token>
    using lexer_tuple = typename lexer<Token>::tuple;

    template <typename Token>
    using lexer_choice = typename lexer<Token>::choice;

    template <typename Token>
    using lexer_sequence = typename lexer<Token>::sequence;

    template <typename Token>
    using lexer_repeat = typename lexer<Token>::repeat;

    template <typename Token>
    using lexer_range = typename lexer<Token>::range;

    template <typename Token>
    using lexer_optional = typename lexer<Token>::optional;

    template <typename Token>
    using lexer_discard = typename lexer<Token>::discard;

    template <typename Token>
    using lexer_rule = typename lexer<Token>::rule;

    template <typename T>
    concept LexerTerminal = std::is_base_of_v<lexer_component<lexer_component_type::Terminal>, T>;

    template <typename T>
    concept LexerPrimitive = std::is_base_of_v<lexer_component<lexer_component_type::Primitive>, T>;

    template <typename T>
    concept LexerAtom = std::is_base_of_v<lexer_component<lexer_component_type::Atom>, T>;

    template <typename T>
    concept LexerTuple = std::is_base_of_v<lexer_component<lexer_component_type::Tuple>, T>;

    template <typename T>
    concept LexerChoice = std::is_base_of_v<lexer_component<lexer_component_type::Choice>, T>;

    template <typename T>
    concept LexerSequence = std::is_base_of_v<lexer_component<lexer_component_type::Sequence>, T>;

    template <typename T>
    concept LexerRepeat = std::is_base_of_v<lexer_component<lexer_component_type::Repeat>, T>;

    template <typename T>
    concept LexerRange = std::is_base_of_v<lexer_component<lexer_component_type::Range>, T>;

    template <typename T>
    concept LexerOptional = std::is_base_of_v<lexer_component<lexer_component_type::Optional>, T>;

    template <typename T>
    concept LexerDiscard = std::is_base_of_v<lexer_component<lexer_component_type::Discard>, T>;

    template <typename T>
    concept LexerRule = std::is_base_of_v<lexer_component<lexer_component_type::Rule>, T>;

    template <typename T>
    concept TokenEnum = std::is_enum_v<T> && std::is_convertible_v<T, decltype(is_lexer_token(T{}))>;

    namespace lexer_operators
    {
        template <TokenEnum Token>
        inline lexer_rule<Token> operator>>(Token lhs, lexer_primitive<Token> const& rhs)
        {
            return lexer_rule<Token>{ lhs, rhs };
        }

        template <LexerRule Rule>
        inline Rule operator|(Rule const& lhs, lexer_primitive<typename Rule::token_type> const& rhs)
        {
            return Rule{ lhs.lhs, lexer_atom<typename Rule::token_type>{ lhs.rhs, rhs } };
        }

        template <LexerTuple Tuple>
        inline Tuple operator|(Tuple const& lhs, lexer_primitive<typename Tuple::token_type> const& rhs)
        {
            return Tuple{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_tuple<Token> operator|(Token lhs, Token rhs)
        {
            return lexer_tuple<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_tuple<Token> operator|(lexer_primitive<Token> const& lhs, Token rhs)
        {
            return lexer_tuple<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_tuple<Token> operator|(Token lhs, lexer_primitive<Token> const& rhs)
        {
            return lexer_tuple<Token>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_tuple<typename Terminal::token_type> operator|(Terminal const& lhs, Terminal const& rhs)
        {
            return lexer_tuple<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_tuple<typename Terminal::tokwn_type> operator|(Terminal const& lhs, Primitive const& rhs)
        {
            return lexer_tuple<typename Terminal::tokwn_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_tuple<typename Terminal::tokwn_type> operator|(Primitive const& lhs, Terminal const& rhs)
        {
            return lexer_tuple<typename Terminal::tokwn_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_tuple<typename Terminal::token_type> operator|(Terminal const& lhs, char rhs)
        {
            return lexer_tuple<typename Terminal::token_type>{ lhs, Terminal{ rhs } };
        }

        template <LexerTerminal Terminal>
        inline lexer_tuple<typename Terminal::token_type> operator|(char lhs, Terminal const& rhs)
        {
            return lexer_tuple<typename Terminal::token_type>{ Terminal{ lhs }, rhs };
        }

        template <LexerTuple Tuple, LexerTerminal Terminal>
        inline Tuple operator|(Tuple const& lhs, Terminal const& rhs)
        {
            return Tuple{ lhs, rhs };
        }

        template <LexerTuple Tuple, LexerTerminal Terminal>
        inline Tuple operator|(Terminal const& lhs, Tuple const& rhs)
        {
            return Tuple{ lhs, rhs };
        }

        template <LexerRule Rule>
        inline Rule operator,(Rule const& lhs, lexer_primitive<typename Rule::token_type> const& rhs)
        {
            return Rule{ lhs.lhs, lexer_sequence<typename Rule::token_type>{ lhs.rhs, rhs } };
        }

        template <LexerSequence Sequence>
        inline Sequence operator,(Sequence const& lhs, lexer_primitive<typename Sequence::token_type> const& rhs)
        {
            return Sequence{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_sequence<Token> operator,(Token lhs, Token rhs)
        {
            return lexer_sequence<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_sequence<Token> operator,(lexer_primitive<Token> const& lhs, Token rhs)
        {
            return lexer_sequence<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_sequence<Token> operator,(Token lhs, lexer_primitive<Token> const& rhs)
        {
            return lexer_sequence<Token>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_sequence<typename Terminal::token_type> operator,(Terminal const& lhs, Terminal const& rhs)
        {
            return lexer_sequence<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_sequence<typename Terminal::tokwn_type> operator,(Terminal const& lhs, Primitive const& rhs)
        {
            return lexer_sequence<typename Terminal::tokwn_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_sequence<typename Terminal::tokwn_type> operator,(Primitive const& lhs, Terminal const& rhs)
        {
            return lexer_sequence<typename Terminal::tokwn_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_sequence<typename Terminal::token_type> operator,(Terminal const& lhs, char rhs)
        {
            return lexer_sequence<typename Terminal::token_type>{ lhs, Terminal{ rhs } };
        }

        template <LexerTerminal Terminal>
        inline lexer_sequence<typename Terminal::token_type> operator,(char lhs, Terminal const& rhs)
        {
            return lexer_sequence<typename Terminal::token_type>{ Terminal{ lhs }, rhs };
        }

        template <LexerSequence Sequence, LexerTerminal Terminal>
        inline Sequence operator,(Sequence const& lhs, Terminal const& rhs)
        {
            return Sequence{ lhs, rhs };
        }

        template <LexerSequence Sequence, LexerTerminal Terminal>
        inline Sequence operator,(Terminal const& lhs, Sequence const& rhs)
        {
            return Sequence{ lhs, rhs };
        }

        template <typename Token>
        inline lexer_choice<Token> choice(lexer_primitive<Token> const& lhs)
        {
            return { lhs };
        }

        template <typename Token>
        inline lexer_repeat<Token> repeat(lexer_primitive<Token> const& lhs)
        {
            return { lhs };
        }

        template <typename Token, typename... Args>
        inline lexer_sequence<Token> sequence(Args&&... lhs)
        {
            return typename lexer_sequence<Token>::value_type{ std::forward<Args>(lhs)... };
        }

        template <typename Token>
        inline lexer_range<Token> range(lexer_primitive<Token> const& lhs, lexer_primitive<Token> const& rhs)
        {
            return { lhs, rhs };
        }

        template <typename Token>
        inline lexer_optional<Token> optional(lexer_primitive<Token> const& lhs)
        {
            return { lhs };
        }

        template <typename Token>
        inline lexer_discard<Token> discard(lexer_primitive<Token> const& lhs)
        {
            return { lhs };
        }
    }

#define enable_neolib_lexer(token)\
    inline token is_lexer_token(token) { return {}; }\
    using namespace neolib::lexer_operators;\
    inline neolib::lexer_terminal<token> operator"" _(const char* str, std::size_t len)\
    {\
        return neolib::lexer_terminal<token>{ str, len };\
    }\
    inline neolib::lexer_terminal<token> operator"" _(char character)\
    {\
        return neolib::lexer_terminal<token>{ character };\
    }\
    template <typename T>\
    inline neolib::lexer_choice<token> choice(T&& lhs)\
    {\
        return neolib::lexer_operators::choice<token>(std::forward<T>(lhs));\
    }\
    template <typename T>\
    inline neolib::lexer_repeat<token> repeat(T&& lhs)\
    {\
        return neolib::lexer_operators::repeat<token>(std::forward<T>(lhs));\
    }\
    template <typename... T>\
    inline neolib::lexer_sequence<token> sequence(T&&... lhs)\
    {\
        return neolib::lexer_operators::sequence<token>(std::forward<T>(lhs)...); \
    }\
    template <typename T>\
    inline neolib::lexer_range<token> range(T&& lhs, T&& rhs)\
    {\
        return neolib::lexer_operators::range<token>(std::forward<T>(lhs), std::forward<T>(rhs)); \
    }\
    template <typename T>\
    inline neolib::lexer_optional<token> optional(T&& lhs)\
    {\
        return neolib::lexer_operators::optional<token>(std::forward<T>(lhs)); \
    }\
    template <typename T>\
        inline neolib::lexer_discard<token> discard(T&& lhs)\
    {\
        return neolib::lexer_operators::discard<token>(std::forward<T>(lhs)); \
    }
}
