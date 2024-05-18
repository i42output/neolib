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

// ****** NOTE: THIS LEXER MODULE IS CURRENTLY BEING IMPLEMENTED AND IS NOT YET FUNCTIONAL ******

#pragma once

#include <neolib/neolib.hpp>

#include <concepts>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <string>
#include <istream>

#include <neolib/core/vecarray.hpp>
#include <neolib/core/i_enum.hpp>
#include <neolib/core/scoped.hpp>

namespace neolib
{
    #define declare_tokens begin_declare_enum
    #define declare_token declare_enum_string
    #define end_declare_tokens end_declare_enum

    enum class lexer_component_type
    {
        Terminal,
        Undefined,
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

    inline std::string to_string(lexer_component_type aType)
    {
        switch (aType)
        {
        case lexer_component_type::Terminal:
            return "Terminal";
        case lexer_component_type::Undefined:
            return "Undefined";
        case lexer_component_type::Choice:
            return "Choice";
        case lexer_component_type::Sequence:
            return "Sequence";
        case lexer_component_type::Repeat:
            return "Repeat";
        case lexer_component_type::Range:
            return "Range";
        case lexer_component_type::Optional:
            return "Optional";
        case lexer_component_type::Discard:
            return "Discard";
        case lexer_component_type::Primitive:
            return "Primitive";
        case lexer_component_type::Atom:
            return "Atom";
        case lexer_component_type::Rule:
            return "Rule";
        default:
            throw std::logic_error("neolib::to_string(lexer_component_type)");
        }
    }

    template <lexer_component_type Type>
    struct lexer_component
    {
        static constexpr lexer_component_type type = Type;
    };

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
                terminal_character{ character }, 
                base_type{ &terminal_character::value(), &terminal_character::value() + 1 }
            {}
            terminal(terminal const& other) :
                terminal_character{ other }, 
                base_type{ other.has_value() ? std::string_view{ &terminal_character::value(), &terminal_character::value() + 1 } : std::string_view{ other } }
            {}
        };

        struct primitive_atom;
        struct atom;

        template <typename Derived>
        struct tuple
        {
            using token_type = token;
            using derived_type = Derived;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            tuple(primitive_atom const& primitive) :
                value{ primitive }
            {
            }

            tuple(std::vector<primitive_atom> const& primitive) :
                value{ primitive }
            {
            }

            tuple(primitive_atom const& lhs, primitive_atom const& rhs)
            {
                if (std::holds_alternative<derived_type>(lhs) && std::holds_alternative<derived_type>(rhs))
                {
                    value.insert(value.end(), std::get<derived_type>(lhs).value.begin(), std::get<derived_type>(lhs).value.end());
                    value.insert(value.end(), std::get<derived_type>(rhs).value.begin(), std::get<derived_type>(rhs).value.end());
                }
                else if (std::holds_alternative<derived_type>(lhs))
                {
                    value.insert(value.end(), std::get<derived_type>(lhs).value.begin(), std::get<derived_type>(lhs).value.end());
                    value.push_back(rhs);
                }
                else if (std::holds_alternative<derived_type>(rhs))
                {
                    value.push_back(lhs);
                    value.insert(value.end(), std::get<derived_type>(rhs).value.begin(), std::get<derived_type>(rhs).value.end());
                }
                else
                {
                    value.push_back(lhs);
                    value.push_back(rhs);
                }
            }

            tuple(atom const& lhs, primitive_atom const& rhs)
            {
                for (auto const& a : lhs)
                    if (std::holds_alternative<derived_type>(a))
                        for (auto const& pa : std::get<derived_type>(a).value)
                            value.push_back(pa);
                    else
                        value.push_back(a);
                value.push_back(rhs);
            }

            tuple(primitive_atom const& lhs, atom const& rhs)
            {
                value.push_back(lhs);
                for (auto const& a : rhs)
                    if (std::holds_alternative<derived_type>(a))
                        for (auto const& pa : std::get<derived_type>(a).value)
                            value.push_back(pa);
                    else
                        value.push_back(a);
            }

            tuple(std::vector<primitive_atom> const& lhs, primitive_atom const& rhs) :
                value{ lhs }
            {
                value.push_back(rhs);
            }

            tuple(primitive_atom const& lhs, std::vector<primitive_atom> const& rhs) :
                value{ lhs }
            {
                value.insert(value.end(), rhs.begin(), rhs.end());
            }
        };

        struct undefined : tuple<undefined>, lexer_component<lexer_component_type::Undefined>
        {
            using base_type = tuple<undefined>;
            using base_type::base_type;
        };

        struct choice : tuple<choice>, lexer_component<lexer_component_type::Choice>
        {
            using base_type = tuple<choice>;
            using base_type::base_type;
        };

        struct sequence : tuple<sequence>, lexer_component<lexer_component_type::Sequence>
        {
            using base_type = tuple<sequence>;
            using base_type::base_type;
        };

        struct repeat : tuple<repeat>, lexer_component<lexer_component_type::Repeat>
        {
            using base_type = tuple<repeat>;
            using base_type::base_type;
        };

        struct range : tuple<range>, lexer_component<lexer_component_type::Range>
        {
            using base_type = tuple<range>;
            using base_type::base_type;
        };

        struct optional : tuple<optional>, lexer_component<lexer_component_type::Optional>
        {
            using base_type = tuple<optional>;
            using base_type::base_type;
        };

        struct discard : tuple<discard>, lexer_component<lexer_component_type::Discard>
        {
            using base_type = tuple<discard>;
            using base_type::base_type;
        };

        struct primitive_atom : std::variant<token, terminal, undefined, choice, sequence, repeat, range, optional, discard>, lexer_component<lexer_component_type::Primitive>
        {
            using token_type = token;

            using base_type = std::variant<token, terminal, undefined, choice, sequence, repeat, range, optional, discard>;
            using base_type::base_type;

            bool is_tuple() const
            {
                return 
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
                        base_type::back() = undefined{ base_type::back() };
                    if (!base_type::empty() && std::holds_alternative<undefined>(base_type::back()))
                        std::get<undefined>(base_type::back()).value.push_back(rhs);
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

        struct ast_node : std::enable_shared_from_this<ast_node>
        {
            using child_list = std::vector<std::shared_ptr<ast_node>>;

            ast_node* parent;
            rule const* rule;
            primitive_atom const* atom;
            std::string_view value;
            child_list children;

            ast_node() = default;
            ast_node(ast_node const&) = delete;

            ast_node(
                ast_node* parent,
                lexer::rule const* rule,
                primitive_atom const* atom,
                std::string_view value) :
                parent{ parent },
                rule{ rule },
                atom{ atom },
                value{ value }
            {
            }

            ast_node(ast_node&& other) :
                parent{ other.parent },
                rule{ other.rule },
                atom{ other.atom },
                value{ other.value },
                children{ std::move(other.children) }
            {
                for (auto& child : children)
                    child.parent = this;
            }

            ast_node& operator=(ast_node&& other)
            {
                parent = other.parent;
                rule = other.rule;
                atom = other.atom;
                value = other.value;
                children = std::move(other.children);

                for (auto& child : children)
                    child->parent = this;

                return *this;
            }
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
            iAst = {};
            iStack = {};
            iError = {};

            auto rootNode = std::make_shared<ast_node>(nullptr, nullptr, nullptr, aSource);

            auto const startTime = std::chrono::high_resolution_clock::now();
            auto const result = parse(aRoot, *rootNode, aSource);
            simplify_ast(*rootNode);
            auto const endTime = std::chrono::high_resolution_clock::now();

            if (!result.has_value() && !iError)
                iError = "Unspecified error";

            if (iDebugOutput && iError)
                (*iDebugOutput) << "Error: " << iError.value() << std::endl;
            if (iDebugOutput)
                (*iDebugOutput) << "Parse time" << (iDebugScan ? " (debug)" : "") << ": " << std::setprecision(3) <<
                std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0 <<
                " seconds" << std::endl;
            
            if (!result.has_value())
                return false;

            iAst = std::move(*rootNode);

            if (iDebugOutput)
            {
                if (iDebugSource)
                    (*iDebugOutput) << aSource << std::endl;
                if (iDebugAst)
                    (*iDebugOutput) << debug_print_ast(iAst) << std::endl;
            }
            
            return true;
        }

    public:
        void set_debug_output(std::ostream& aDebugOutput)
        {
            iDebugOutput = &aDebugOutput;
        }

        void set_debug_scan(bool aDebugScan)
        {
            iDebugScan = aDebugScan;
        }

    private:
        void simplify_ast(ast_node& aNode)
        {
            simplify_ast(&aNode);
        }

        std::optional<typename ast_node::child_list::iterator> simplify_ast(ast_node* aNode)
        {
            for (auto child = aNode->children.begin(); child != aNode->children.end(); )
            {
                auto result = simplify_ast(&**child);
                if (result)
                    child = aNode->children.erase(result.value());
                else
                    ++child;
            }

            if (aNode->parent && aNode->parent->rule &&
                std::holds_alternative<token>(aNode->rule->lhs[0]) &&
                std::holds_alternative<token>(aNode->parent->rule->lhs[0]) &&
                std::get<token>(aNode->rule->lhs[0]) == std::get<token>(aNode->parent->rule->lhs[0]) &&
                aNode->value.data() >= aNode->parent->value.data() &&
                std::next(aNode->value.data(), aNode->value.size()) <= std::next(aNode->parent->value.data(), aNode->parent->value.size()))
            {
                aNode->parent->value = aNode->value;
                auto existing = std::find_if(aNode->parent->children.begin(), aNode->parent->children.end(), [&](auto const& e)
                    {
                        return &*e == aNode;
                    });
                for (auto& child2 : aNode->children)
                    child2->parent = aNode->parent;
                auto pos = aNode->parent->children.insert(std::next(existing),
                    std::make_move_iterator(aNode->children.begin()), std::make_move_iterator(aNode->children.end()));
                return std::prev(pos);
            }

            return {};
        }

        std::optional<token> parent_token(ast_node& aNode) const
        {
            if (aNode.parent && aNode.parent->atom && std::holds_alternative<token>(*aNode.parent->atom))
                return std::get<token>(*aNode.parent->atom);

            return {};
        }

        bool left_recursion(ast_node const& aNode, rule const& aRule) const
        {
            if (iStack.empty())
                return false;
            for (auto r = std::next(iStack.rbegin()); r != iStack.rend(); ++r)
                if (*r == iStack.back())
                    return true;
            return false;
        }

        std::optional<std::string_view> parse(token aToken, ast_node& aNode, std::string_view const& aSource)
        {
            if (iError)
                return {};

            char const* sourceNext = aSource.data();
            char const* sourceEnd = aSource.data() + aSource.size();

            scoped_counter sc{ iLevel };

            if (iLevel > iMaxLevel)
            {
                iError = "Internal compiler error (parse too deep): ";
                bool first = true;
                auto n = &aNode;
                while (n)
                {
                    if (n->atom)
                    {
                        if (!first)
                            iError.value() += ":";
                        first = false;
                        if (std::holds_alternative<token>(*n->atom))
                            iError.value() += enum_to_string(std::get<token>(*n->atom));
                        iError.value() += "(" + std::to_string(std::distance<const rule*>(&iRules[0], n->rule)) + ")";
                    }
                    n = n->parent;
                }
                return {};
            }

            std::optional<scoped_debug_print> sdp;
            if (iDebugScan)
                sdp.emplace(*this, aToken, aSource);

            for (auto& rule : iRules)
            {
                if (!std::holds_alternative<token>(rule.lhs[0]))
                    continue;
                token const ruleToken = std::get<token>(rule.lhs[0]);
                scoped_stack_entry sse{ *this, rule, aSource };
                if (ruleToken == aToken && !left_recursion(aNode, rule))
                {
                    aNode.rule = &rule;
                    auto const& ruleAtom = rule.rhs[0];
                    typename ast_node::child_list children;
                    std::swap(aNode.children, children);
                    auto const result = parse(ruleAtom, aNode, std::string_view{ sourceNext, sourceEnd });
                    std::swap(aNode.children, children);
                    if (result)
                    {
                        aNode.children.insert(aNode.children.end(), std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
                        return ((sdp ? sdp->ok = true : true), result);
                    }
                }
            }

            aNode.rule = nullptr;
            return {};
        }

        std::optional<std::string_view> parse(primitive_atom const& aAtom, ast_node& aNode, std::string_view const& aSource)
        {
            if (iError)
                return {};

            auto cacheAtomEntry = iCache.find(&aAtom);
            if (cacheAtomEntry != iCache.end())
            {
                auto cacheSourceEntry = cacheAtomEntry->second.find(aSource.data());
                if (cacheSourceEntry != cacheAtomEntry->second.end())
                {
                    auto const cachedResult = cacheSourceEntry->second.result;
                    auto& cachedNode = cacheSourceEntry->second.node;
                    aNode.children = cachedNode->children;
                    return cachedResult;
                }
            }

            std::string_view result = { aSource.data(), aSource.data() };
            char const* sourceNext = aSource.data();
            char const* sourceEnd = aSource.data() + aSource.size();

            scoped_counter sc{ iLevel };
            std::optional<scoped_debug_print> sdp;
            if (!std::holds_alternative<token>(aAtom) && iDebugScan)
                sdp.emplace(*this, aAtom, aSource);

            // todo: visitor?

            if (std::holds_alternative<token>(aAtom))
            {
                token const atomToken = std::get<token>(aAtom);
                aNode.children.push_back(std::make_shared<ast_node>(&aNode, aNode.rule, &aAtom, aSource));
                auto const partialResult = parse(atomToken, *aNode.children.back(), aSource);
                if (partialResult)
                {
                    iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), apply_partial_result(result, partialResult)};
                    return iCache[&aAtom][aSource.data()].result;
                }
                aNode.children.pop_back();
            }
            else if (std::holds_alternative<terminal>(aAtom))
            {
                auto const& t = std::get<terminal>(aAtom);
                if ((!t.empty() && aSource.find(t) == 0) || (t.empty() && sourceNext == sourceEnd ))
                {
                    auto const partialResult = aSource.substr(0, t.size());
                    aNode.children.push_back(std::make_shared<ast_node>(&aNode, aNode.rule, &aAtom, partialResult));
                    iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), apply_partial_result(result, partialResult) };
                    return ((sdp ? sdp->ok = true : true), iCache[&aAtom][aSource.data()].result);
                }
            }
            else if (std::holds_alternative<range>(aAtom))
            {
                auto const min = std::get<terminal>(std::get<range>(aAtom).value[0]).value();
                auto const max = std::get<terminal>(std::get<range>(aAtom).value[1]).value();
                if (!aSource.empty() && aSource[0] >= min && aSource[0] <= max)
                {
                    auto const partialResult = aSource.substr(0, 1);
                    aNode.children.push_back(std::make_shared<ast_node>(&aNode, aNode.rule, &aAtom, partialResult));
                    iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), apply_partial_result(result, partialResult) };
                    return ((sdp ? sdp->ok = true : true), iCache[&aAtom][aSource.data()].result);
                }
            }
            else if (std::holds_alternative<sequence>(aAtom))
            {
                for (auto const& a : std::get<sequence>(aAtom).value)
                {
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                    if (!partialResult)
                        return {};
                    result = apply_partial_result(result, partialResult);
                    sourceNext = std::next(sourceNext, partialResult->size());
                }
                iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), result };
                return ((sdp ? sdp->ok = true : true), result);
            }
            else if (std::holds_alternative<repeat>(aAtom))
            {
                bool foundAtLeastOne = false;
                bool found = false;
                do
                {
                    found = false;
                    for (auto const& a : std::get<repeat>(aAtom).value)
                    {
                        auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                        if (partialResult)
                        {
                            foundAtLeastOne = true;
                            found = true;
                            result = apply_partial_result(result, partialResult);
                            sourceNext = std::next(sourceNext, partialResult->size());
                        }
                    }
                } while (found);
                if (foundAtLeastOne)
                {
                    iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), result };
                    return ((sdp ? sdp->ok = true : true), result);
                }
                return {};
            }
            else if (std::holds_alternative<choice>(aAtom))
            {
                bool found = false;
                for (auto const& a : std::get<choice>(aAtom).value)
                {
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                    if (partialResult)
                    {
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::next(sourceNext, partialResult->size());
                        found = true;
                        break;
                    }
                }
                if (found)
                {
                    iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), result };
                    return ((sdp ? sdp->ok = true : true), result);
                }
                return {};
            }
            else if (std::holds_alternative<optional>(aAtom))
            {
                for (auto const& a : std::get<optional>(aAtom).value)
                {
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                    if (partialResult)
                    {
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::next(sourceNext, partialResult->size());
                    }
                }
                iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), result };
                return ((sdp ? sdp->ok = true : true), result);
            }
            else if (std::holds_alternative<discard>(aAtom))
            {
                for (auto const& a : std::get<discard>(aAtom).value)
                {
                    typename ast_node::child_list children;
                    std::swap(aNode.children, children);
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });
                    std::swap(aNode.children, children);
                    if (partialResult)
                    {
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::next(sourceNext, partialResult->size());
                    }
                }
                iCache[&aAtom][aSource.data()] = cache_result{ aNode.shared_from_this(), result };
                return ((sdp ? sdp->ok = true : true), result);
            }

            return {};
        }

        static std::string_view apply_partial_result(std::string_view const& aResult, std::optional<std::string_view> const& aPartialResult)
        {
            char const* resultFirst = aResult.data();
            char const* resultLast = std::next(aResult.data(), aResult.size());
            char const* partialResultFirst = aPartialResult.value().data();
            char const* partialResultLast = std::next(aPartialResult.value().data(), aPartialResult.value().size());
            return { std::min(resultFirst, partialResultFirst), std::max(resultLast, partialResultLast) };
        }

    private:
        struct scoped_stack_entry
        {
            lexer& owner;

            scoped_stack_entry(lexer& owner, rule const& rule, std::string_view const& source) :
                owner{ owner }
            {
                owner.iStack.push_back(std::make_pair(&rule, source));
            }
            ~scoped_stack_entry()
            {
                owner.iStack.pop_back();
            }
        };
 
        struct scoped_debug_print
        {
            lexer& owner;
            std::string value;
            std::string_view const& source;
            bool ok = false;

            template <typename T>
            scoped_debug_print(lexer& aOwner, T const& aValue, std::string_view const& aSource) : 
                owner{ aOwner },
                source{ aSource }
            {
                std::ostringstream oss;
                if constexpr (std::is_same_v<T, token>)
                    oss << "t(" << enum_to_string(aValue) << ")";
                else if constexpr (std::is_same_v<std::decay_t<T>, primitive_atom>)
                {
                    std::visit([&](auto const& pa)
                    {
                        if constexpr (std::is_same_v<token, std::decay_t<decltype(pa)>>)
                            oss << "a(" << enum_to_string(pa) << ")";
                        else if constexpr (std::is_same_v<terminal, std::decay_t<decltype(pa)>>)
                            oss << "a(" << to_string(pa.type) << ":[" << debug_print(pa) << "])";
                        else
                            oss << "a(" << to_string(pa.type) << ")";
                    }, aValue);
                }
                else
                    oss << "(" << aValue << ")";
                value = oss.str();

                if (owner.iDebugOutput)
                    (*owner.iDebugOutput) << std::string(static_cast<std::size_t>(owner.iLevel - 1), ' ') << value << ": " << "[" << debug_print(source) << "]" << std::endl;
            }

            ~scoped_debug_print()
            {
                if (owner.iDebugOutput && ok)
                    (*owner.iDebugOutput) << std::string(static_cast<std::size_t>(owner.iLevel - 1), ' ') << value << " ok: [" << debug_print(source) << "]" << std::endl;
            }
        };

        static std::string debug_print(std::string_view const& aSource, std::size_t aMaxChars = 16)
        {
            std::ostringstream result;
            std::size_t charsAdded = 0;
            for (auto ch : aSource)
            {
                ++charsAdded;
                if (ch >= ' ')
                    result << ch;
                else if (ch == '\n')
                    result << "\\n";
                else if (ch == '\r')
                    result << "\\r";
                else if (ch == '\t')
                    result << "\\t";
                else
                    result << "\\x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch));
                if (charsAdded == aMaxChars)
                    break;
            }
            return result.str();
        }

        static std::string debug_print_ast(ast_node const& aNode, std::uint32_t aLevel = 0)
        {
            std::ostringstream oss;
            oss << std::string(static_cast<std::size_t>(aLevel), ' ');
            if (aNode.atom)
            {
                std::visit([&](auto const& pa)
                    {
                        if constexpr (std::is_same_v<token, std::decay_t<decltype(pa)>>)
                            oss << "a(" << enum_to_string(pa) << ")";
                        else if constexpr (std::is_same_v<terminal, std::decay_t<decltype(pa)>>)
                            oss << "a(" << to_string(pa.type) << ":[" << debug_print(pa) << "])";
                        else
                            oss << "a(" << to_string(pa.type) << ")";
                        oss << " = [" << debug_print(aNode.value, 64) << "]";
                    }, *aNode.atom);
            }
            oss << std::endl;
            for (auto const& childNode : aNode.children)
                oss << debug_print_ast(*childNode, aLevel + 1);
            return oss.str();
        }

    private:
        std::vector<rule> iRules;
        ast_node iAst = {};
        std::vector<std::pair<rule const*, std::string_view>> iStack;
        std::uint32_t iMaxLevel = 256;
        std::uint32_t iLevel = 0;
        std::optional<std::string> iError;
        struct cache_result
        {
            std::shared_ptr<ast_node> node;
            std::optional<std::string_view> result;
        };
        std::unordered_map<primitive_atom const*, std::unordered_map<char const*, cache_result>> iCache;
        std::ostream* iDebugOutput = nullptr;
        bool iDebugScan = false;
        bool iDebugSource = true;
        bool iDebugAst = false;
    };

    template <typename Token>
    using lexer_terminal = typename lexer<Token>::terminal;

    template <typename Token>
    using lexer_primitive = typename lexer<Token>::primitive_atom;

    template <typename Token>
    using lexer_atom = typename lexer<Token>::atom;

    template <typename Token>
    using lexer_undefined = typename lexer<Token>::undefined;

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
    concept LexerUndefined = std::is_base_of_v<lexer_component<lexer_component_type::Undefined>, T>;

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

        template <LexerRepeat Repeat>
        inline Repeat operator|(Repeat const& lhs, lexer_primitive<typename Repeat::token_type> const& rhs)
        {
            return Repeat{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_repeat<Token> operator|(Token lhs, Token rhs)
        {
            return lexer_repeat<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_repeat<Token> operator|(lexer_primitive<Token> const& lhs, Token rhs)
        {
            return lexer_repeat<Token>{ lhs, rhs };
        }

        template <TokenEnum Token>
        inline lexer_repeat<Token> operator|(Token lhs, lexer_primitive<Token> const& rhs)
        {
            return lexer_repeat<Token>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_repeat<typename Terminal::token_type> operator|(Terminal const& lhs, Terminal const& rhs)
        {
            return lexer_repeat<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_repeat<typename Terminal::token_type> operator|(Terminal const& lhs, Primitive const& rhs)
        {
            return lexer_repeat<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_repeat<typename Terminal::token_type> operator|(Primitive const& lhs, Terminal const& rhs)
        {
            return lexer_repeat<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal>
        inline lexer_repeat<typename Terminal::token_type> operator|(Terminal const& lhs, char rhs)
        {
            return lexer_repeat<typename Terminal::token_type>{ lhs, Terminal{ rhs } };
        }

        template <LexerTerminal Terminal>
        inline lexer_repeat<typename Terminal::token_type> operator|(char lhs, Terminal const& rhs)
        {
            return lexer_repeat<typename Terminal::token_type>{ Terminal{ lhs }, rhs };
        }

        template <LexerRepeat Repeat, LexerTerminal Terminal>
        inline Repeat operator|(Repeat const& lhs, Terminal const& rhs)
        {
            return Repeat{ lhs, rhs };
        }

        template <LexerRepeat Repeat, LexerTerminal Terminal>
        inline Repeat operator|(Terminal const& lhs, Repeat const& rhs)
        {
            return Repeat{ lhs, rhs };
        }

        template <LexerRange Range>
        inline lexer_repeat<typename Range::token_type> operator|(Range const& lhs, Range const& rhs)
        {
            return lexer_repeat<typename Range::token_type>{ lhs , rhs };
        }

        template <LexerRepeat Repeat, LexerRange Range>
        inline Repeat operator|(Repeat const& lhs, Range const& rhs)
        {
            return Repeat{ lhs, rhs };
        }

        template <LexerRepeat Repeat, LexerRange Range>
        inline Repeat operator|(Range const& lhs, Repeat const& rhs)
        {
            return Repeat{ lhs, rhs };
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
        inline lexer_sequence<typename Terminal::token_type> operator,(Terminal const& lhs, Primitive const& rhs)
        {
            return lexer_sequence<typename Terminal::token_type>{ lhs, rhs };
        }

        template <LexerTerminal Terminal, LexerPrimitive Primitive>
        inline lexer_sequence<typename Terminal::token_type> operator,(Primitive const& lhs, Terminal const& rhs)
        {
            return lexer_sequence<typename Terminal::token_type>{ lhs, rhs };
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
