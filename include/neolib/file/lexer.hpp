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
        Optional,
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

        struct terminal : std::string_view, lexer_component<lexer_component_type::Terminal>
        {
            using token_type = token;

            using base_type = std::string_view;
            using base_type::base_type;
        };

        struct primitive_atom;

        struct tuple : lexer_component<lexer_component_type::Tuple>
        {
            using token_type = token;

            std::vector<primitive_atom> value;

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

            std::vector<primitive_atom> value;

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

            std::vector<primitive_atom> value;

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

            std::vector<primitive_atom> value;

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

        struct optional : lexer_component<lexer_component_type::Optional>
        {
            using token_type = token;

            std::vector<primitive_atom> value;

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

            optional(repeat const& repeat, primitive_atom const& primitive) :
                value{ repeat.value }
            {
                value.push_back(primitive);
            }
        };

        struct primitive_atom : std::variant<token, terminal, tuple, choice, sequence, repeat, optional>, lexer_component<lexer_component_type::Primitive>
        {
            using token_type = token;

            using base_type = std::variant<token, terminal, tuple, choice, sequence, repeat, optional>;
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

    public:
        bool parse(char const* source) const
        {
            return true;
        }
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
    using lexer_optional = typename lexer<Token>::optional;

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
    concept LexerOptional = std::is_base_of_v<lexer_component<lexer_component_type::Optional>, T>;

    template <typename T>
    concept LexerRule = std::is_base_of_v<lexer_component<lexer_component_type::Rule>, T>;

    template <typename T>
    concept TokenEnum = requires(T a)
    {
        { is_lexer_token(a) } -> std::convertible_to<T>;
    };

    namespace lexer_operators
    {
        template <TokenEnum Token>
        inline lexer_rule<Token> operator>>(Token lhs, lexer_primitive<Token> const& rhs)
        {
            return lexer_rule<Token>{ lhs, rhs };
        }

        template <LexerOptional Optional>
        inline lexer_rule<typename Optional::token_type> operator>>(Optional const& lhs, lexer_primitive<typename Optional::token_type> const& rhs)
        {
            return lexer_rule<typename Optional::token_type>{ lhs, rhs };
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

        template <LexerTerminal Terminal, TokenEnum Token>
        inline lexer_tuple<Token> operator|(Terminal const& lhs, Token rhs)
        {
            return lexer_tuple<Token>{ lhs, rhs };
        }

        template <TokenEnum Token, LexerTerminal Terminal>
        inline lexer_tuple<Token> operator|(Token lhs, Terminal const& rhs)
        {
            return lexer_tuple<Token>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_tuple<typename Terminal::token_type> operator|(Terminal const& lhs, Terminal const& rhs)
        {
            return lexer_tuple<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTuple Tuple, LexerTerminal Terminal>
        inline Tuple operator|(Tuple const& lhs, Terminal const& rhs)
        {
            return Tuple{ lhs, rhs };
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

        template <typename Token>
        inline lexer_sequence<Token> sequence(lexer_primitive<Token> const& lhs)
        {
            return { lhs };
        }

        template <typename Token>
        inline lexer_optional<Token> optional(lexer_primitive<Token> const& lhs)
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
    template <typename T>\
    inline neolib::lexer_sequence<token> sequence(T&& lhs)\
    {\
        return neolib::lexer_operators::sequence<token>(std::forward<T>(lhs)); \
    }\
    template <typename T>\
    inline neolib::lexer_optional<token> optional(T&& lhs)\
    {\
        return neolib::lexer_operators::optional<token>(std::forward<T>(lhs)); \
    }
}
