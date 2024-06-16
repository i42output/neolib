// parser.hpp
/*
 *  Copyright (c) 2024 Leigh Johnston.
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
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <string>
#include <istream>

#include <boost/functional/hash.hpp>

#include <neolib/core/i_enum.hpp>
#include <neolib/core/scoped.hpp>

namespace neolib
{
    #define declare_symbols begin_declare_enum
    #define declare_symbol declare_enum_string
    #define end_declare_symbols end_declare_enum

    enum class parser_component_type
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
        Rule,
        Concept
    };

    inline std::string to_string(parser_component_type aType)
    {
        switch (aType)
        {
        case parser_component_type::Terminal:
            return "Terminal";
        case parser_component_type::Undefined:
            return "Undefined";
        case parser_component_type::Choice:
            return "Choice";
        case parser_component_type::Sequence:
            return "Sequence";
        case parser_component_type::Repeat:
            return "Repeat";
        case parser_component_type::Range:
            return "Range";
        case parser_component_type::Optional:
            return "Optional";
        case parser_component_type::Discard:
            return "Discard";
        case parser_component_type::Primitive:
            return "Primitive";
        case parser_component_type::Atom:
            return "Atom";
        case parser_component_type::Rule:
            return "Rule";
        case parser_component_type::Concept:
            return "Concept";
        default:
            throw std::logic_error("neolib::to_string(parser_component_type)");
        }
    }

    struct parser_component_base {};

    template <parser_component_type Type>
    struct parser_component : parser_component_base
    {
        static constexpr parser_component_type type = Type;
    };

    enum class concept_association
    {
        None,
        Left,
        Right
    };

    template <typename Symbol>
    class parser
    {
    public:
        using symbol = Symbol;

        using terminal_character = std::optional<char>;

        struct terminal : terminal_character, std::string_view, parser_component<parser_component_type::Terminal>
        {
            using symbol_type = symbol;

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

        struct no_params {};

        template <typename Derived, typename Params = no_params>
        struct tuple : Params
        {
            using symbol_type = symbol;
            using derived_type = Derived;
            using params_type = Params;
            using value_type = std::vector<primitive_atom>;

            value_type value;

            tuple(primitive_atom const& primitive) :
                value{ primitive }
            {
            }

            tuple(value_type const& primitive) :
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

            tuple(value_type const& lhs, primitive_atom const& rhs) :
                value{ lhs }
            {
                value.push_back(rhs);
            }

            tuple(primitive_atom const& lhs, value_type const& rhs) :
                value{ lhs }
            {
                value.insert(value.end(), rhs.begin(), rhs.end());
            }
        };

        struct undefined : tuple<undefined>, parser_component<parser_component_type::Undefined>
        {
            using base_type = tuple<undefined>;
            using base_type::base_type;
        };

        struct choice : tuple<choice>, parser_component<parser_component_type::Choice>
        {
            using base_type = tuple<choice>;
            using base_type::base_type;
        };

        struct sequence : tuple<sequence>, parser_component<parser_component_type::Sequence>
        {
            using base_type = tuple<sequence>;
            using base_type::base_type;
        };

        struct repeat_params
        {
            bool atLeastOne = false;
        };

        struct repeat : tuple<repeat, repeat_params>, parser_component<parser_component_type::Repeat>
        {
            using base_type = tuple<repeat, repeat_params>;
            using base_type::base_type;

            repeat operator+() const
            {
                repeat result = *this;
                result.atLeastOne = true;
                return result;
            }
        };

        struct range : tuple<range>, parser_component<parser_component_type::Range>
        {
            using base_type = tuple<range>;
            using base_type::base_type;
        };

        struct optional : tuple<optional>, parser_component<parser_component_type::Optional>
        {
            using base_type = tuple<optional>;
            using base_type::base_type;
        };

        struct discard_params
        {
            bool trim = true;
        };

        struct discard : tuple<discard, discard_params>, parser_component<parser_component_type::Discard>
        {
            using base_type = tuple<discard, discard_params>;
            using base_type::base_type;

            discard operator~() const
            {
                discard result = *this;
                result.trim = false;
                return result;
            }
        };

        struct _concept : std::string, parser_component<parser_component_type::Concept>
        {
            using symbol_type = symbol;

            concept_association association = concept_association::None;

            using base_type = std::string;
            using base_type::base_type;

            _concept without_association() const
            {
                _concept result = *this;
                result.association = concept_association::None;
                return result;
            }
        };

        std::optional<_concept> without_association(std::optional<_concept> const& c)
        {
            if (!c.has_value())
                return c;
            return c.value().without_association();
        }

        struct primitive_atom : std::variant<symbol, terminal, undefined, choice, sequence, repeat, range, optional, discard>, parser_component<parser_component_type::Primitive>
        {
            using symbol_type = symbol;

            using base_type = std::variant<symbol, terminal, undefined, choice, sequence, repeat, range, optional, discard>;
            using base_type::base_type;

            std::optional<_concept> c;

            primitive_atom(primitive_atom const& other) :
                base_type{ other }, c{ other.c }
            {
            }

            primitive_atom(primitive_atom&& other) :
                base_type{ std::move(other) }, c{std::move(other.c)}
            {
            }

            primitive_atom& operator=(primitive_atom const& other)
            {
                base_type::operator=(other);
                c = other.c;
                return *this;
            }

            primitive_atom& operator=(primitive_atom&& other)
            {
                base_type::operator=(std::move(other));
                c = std::move(other.c);
                return *this;
            }

            bool is_tuple() const
            {
                return 
                    std::holds_alternative<choice>(*this) ||
                    std::holds_alternative<sequence>(*this) ||
                    std::holds_alternative<repeat>(*this);
            }
        };

        struct atom : std::vector<primitive_atom>, parser_component<parser_component_type::Atom>
        {
            using symbol_type = symbol;

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

        struct rule : parser_component<parser_component_type::Rule>
        {
            using symbol_type = symbol;

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

        struct parse_result
        {
            std::string_view value;
            char const* sourceNext;

            parse_result(std::string_view const& value) :
                value{ value },
                sourceNext{ std::to_address(value.end()) }
            {
            }
            parse_result(char const* begin, char const* end) :
                value{ begin, end },
                sourceNext{ end }
            {
            }
        };

        // Why aren't we using std::unique_ptr? Because nodes are shared between the cache
        // and the partial CST currently being built via backtracking.
        struct cst_node : std::enable_shared_from_this<cst_node>
        {
            using child_list = std::vector<std::shared_ptr<cst_node>>;

            cst_node* parent;
            parser::rule const* rule;
            std::optional<_concept> c;
            primitive_atom const* atom;
            std::string_view value;
            child_list children;

            cst_node() = default;
            cst_node(cst_node const&) = delete;

            cst_node(
                cst_node* parent,
                parser::rule const* rule,
                primitive_atom const* atom,
                std::string_view value) :
                parent{ parent },
                rule{ rule },
                atom{ atom },
                value{ value }
            {
            }

            cst_node(cst_node&& other) :
                parent{ other.parent },
                rule{ other.rule },
                c{ other.c },
                atom{ other.atom },
                value{ other.value },
                children{ std::move(other.children) }
            {
                for (auto& child : children)
                    child.parent = this;
            }

            ~cst_node()
            {
            }

            cst_node& operator=(cst_node&& other)
            {
                parent = other.parent;
                rule = other.rule;
                c = other.c;
                atom = other.atom;
                value = other.value;
                children = std::move(other.children);

                for (auto& child : children)
                    child->parent = this;

                return *this;
            }
        };

        using ast_node = cst_node;

    public:
        template <std::size_t RuleCount>
        parser(rule const (&aRules)[RuleCount]) :
            iRules{ &aRules[0], &aRules[0] + RuleCount }
        {
        }

    public:
        bool parse(symbol aRoot, std::string_view const& aSource)
        {
            iSource = aSource;
            iCst = {};
            iAst = {};
            iStack = {};
            iDeepestParse = {};
            iError = {};
            iCache = {};

            auto rootNode = std::make_shared<cst_node>(nullptr, nullptr, nullptr, aSource);

            auto const startTime = std::chrono::high_resolution_clock::now();
            auto const result = parse(aRoot, *rootNode, aSource);
            fixup_cst(*rootNode);
            simplify_cst(*rootNode);
            auto const endTime = std::chrono::high_resolution_clock::now();
            std::uint32_t linePos;
            std::uint32_t columnPos;

            auto error_print = [&](std::string const& errorPrefix, char const* pos) -> std::string
            {
                linePos = static_cast<std::uint32_t>(std::count(aSource.data(), pos, '\n') + 1);
                columnPos = static_cast<std::uint32_t>(std::distance(std::reverse_iterator(pos),
                    std::find(std::reverse_iterator(pos), std::reverse_iterator(aSource.data()), '\n')) + 1);
                std::string error = errorPrefix + "(" + std::to_string(linePos) + "," + std::to_string(columnPos) + ") ";
                error += "'" + debug_print(std::string_view{ pos, std::next(pos) }) + "' was unexpected here.";
                return error;
            };

            if (!iError && iDeepestParse < aSource.data() + aSource.size())
                iError = error_print("syntax error: ", iDeepestParse);

            if (iDebugOutput)
            {
                std::vector<std::string> lines;
                std::istringstream iss{ std::string{ aSource } };
                std::string line;
                while (std::getline(iss, line))
                    lines.push_back(line);
                std::size_t numberWidth = std::to_string(lines.size()).size();
                std::uint32_t lineNumber = 1;
                for (auto const& outputLine : lines)
                {
                    (*iDebugOutput) << std::setw(numberWidth) << lineNumber << (iError && lineNumber == linePos ? ">" : "|") << outputLine << std::endl;
                    ++lineNumber;
                }
                if (iError)
                    (*iDebugOutput) << std::string(columnPos + numberWidth, '-') << "^" << std::endl;
                if (iError)
                    (*iDebugOutput) << "Error: " << iError.value() << std::endl;
                else
                {
                    auto const time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
                    (*iDebugOutput) << "Parse time" << (iDebugScan ? " (debug)" : "") << ": " << std::setprecision(3) <<
                        time << " seconds" << " (" << static_cast<std::uint32_t>(iSource.size() / time) << " characters/second, " << 
                        static_cast<std::uint32_t>(std::count(iSource.begin(), iSource.end(), '\n') / time) << " lines/second)" << std::endl;
                }
                if (iDebugCst)
                    (*iDebugOutput) << debug_print_cst(*rootNode) << std::endl;
            }
            
            if (iError)
                return false;

            iCst = std::move(*rootNode);

            return true;
        }

        void create_ast()
        {
            iAst = std::move(iCst);
            create_ast(&iAst);

            if (iDebugOutput)
            {
                std::vector<std::string> lines;
                std::istringstream iss{ std::string{ iSource } };
                std::string line;
                while (std::getline(iss, line))
                    lines.push_back(line);
                std::size_t numberWidth = std::to_string(lines.size()).size();
                std::uint32_t lineNumber = 1;
                for (auto const& outputLine : lines)
                {
                    (*iDebugOutput) << std::setw(numberWidth) << lineNumber << "|" << outputLine << std::endl;
                    ++lineNumber;
                }
                if (iDebugCst)
                    (*iDebugOutput) << debug_print_ast(iAst) << std::endl;
            }
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
        void fixup_cst(cst_node& aNode)
        {
            fixup_cst(&aNode);
        }

        void fixup_cst(cst_node* aNode)
        {
            for (auto child = aNode->children.begin(); child != aNode->children.end(); ++child)
            {
                (**child).parent = aNode;
                fixup_cst(&**child);
            }
        }
                
        void simplify_cst(cst_node& aNode)
        {
            simplify_cst(&aNode);
        }

        std::optional<typename cst_node::child_list::iterator> simplify_cst(cst_node* aNode)
        {
            for (auto child = aNode->children.begin(); child != aNode->children.end(); )
            {
                auto result = simplify_cst(&**child);
                if (result)
                {
                    if (!aNode->c.has_value())
                        aNode->c = (***result).c;
                    child = aNode->children.erase(result.value());
                }
                else
                    ++child;
            }

            if (aNode->parent && aNode->parent->rule)
            {
                auto existing = std::find_if(aNode->parent->children.begin(), aNode->parent->children.end(), [&](auto const& e)
                {
                    return &*e == aNode;
                });

                auto const ourSymbol = std::get<symbol>(aNode->rule->lhs[0]);
                auto const parentSymbol = std::get<symbol>(aNode->parent->rule->lhs[0]);

                if (std::holds_alternative<range>(*aNode->atom))
                {
                    if (std::holds_alternative<sequence>(aNode->parent->rule->rhs[0]) || std::holds_alternative<repeat>(aNode->parent->rule->rhs[0]))
                    {
                        aNode->parent->value = std::string_view{ aNode->parent->value.data(), std::to_address(aNode->value.end()) };
                        return existing;
                    }
                }
                else if ((ourSymbol == parentSymbol || (aNode->c.has_value() && aNode->c == aNode->parent->c)) && aNode->value == aNode->parent->value)
                {
                    for (auto& child2 : aNode->children)
                        child2->parent = aNode->parent;
                    auto pos = aNode->parent->children.insert(std::next(existing),
                        std::make_move_iterator(aNode->children.begin()), std::make_move_iterator(aNode->children.end()));
                    return std::prev(pos);
                }
            }

            return {};
        }

        std::optional<typename ast_node::child_list::iterator> create_ast(ast_node* aNode)
        {
            for (auto child = aNode->children.begin(); child != aNode->children.end(); )
            {
                auto result = create_ast(&**child);
                if (result)
                    child = aNode->children.erase(result.value());
                else
                    ++child;
            }

            if (aNode->parent)
            {
                auto existing = std::find_if(aNode->parent->children.begin(), aNode->parent->children.end(), [&](auto const& e)
                    {
                        return &*e == aNode;
                    });
    
                if (!aNode->c.has_value())
                {
                    for (auto& child2 : aNode->children)
                        child2->parent = aNode->parent;
                    auto pos = aNode->parent->children.insert(std::next(existing),
                        std::make_move_iterator(aNode->children.begin()), std::make_move_iterator(aNode->children.end()));
                    return std::prev(pos);
                }
                else if (aNode->c.value().association == concept_association::Left)
                {
                    aNode->c = without_association(aNode->c);
                    auto lhs = std::prev(existing);
                    auto rhs = std::next(existing);
                    (**lhs).parent = aNode;
                    (**rhs).parent = aNode;
                    aNode->children.push_back(*lhs);
                    aNode->children.push_back(*rhs);
                    existing = aNode->parent->children.erase(lhs);
                    rhs = std::next(existing);
                    return rhs;
                }
                else if (aNode->c.value().association == concept_association::Right)
                {
                    aNode->c = without_association(aNode->c);
                    // todo
                }
            }

            return {};
        }

        std::optional<symbol> parent_symbol(cst_node& aNode) const
        {
            if (aNode.parent && aNode.parent->atom && std::holds_alternative<symbol>(*aNode.parent->atom))
                return std::get<symbol>(*aNode.parent->atom);

            return {};
        }

        bool left_recursion(cst_node const& aNode, rule const& aRule) const
        {
            if (iStack.empty())
                return false;
            for (auto r = std::next(iStack.rbegin()); r != iStack.rend(); ++r)
                if (*r == iStack.back())
                    return true;
            return false;
        }

        std::optional<parse_result> parse(symbol aSymbol, cst_node& aNode, std::string_view const& aSource)
        {
            if (iError)
                return {};

            char const* sourceNext = std::to_address(aSource.begin());
            char const* sourceEnd = std::to_address(aSource.end());

            iDeepestParse = std::max(iDeepestParse, sourceNext);

            scoped_counter sc{ iLevel };

            if (iLevel > iMaxLevel)
            {
                iError = "internal compiler error (parse too deep): ";
                bool first = true;
                auto n = &aNode;
                while (n)
                {
                    if (n->atom)
                    {
                        if (!first)
                            iError.value() += ":";
                        first = false;
                        if (std::holds_alternative<symbol>(*n->atom))
                            iError.value() += enum_to_string(std::get<symbol>(*n->atom));
                        iError.value() += "(" + std::to_string(std::distance<const rule*>(&iRules[0], n->rule)) + ")";
                    }
                    n = n->parent;
                }
                return {};
            }

            std::optional<scoped_debug_print> sdp;
            if (iDebugScan)
                sdp.emplace(*this, aSymbol, aSource);

            // todo: if more than one rule matches take the deepest parse and/or resolve ambiguity via semantic analyis through IoC.

            std::optional<parse_result> result;
            rule* resultRule = nullptr;
            typename cst_node::child_list resultChildren;

            for (auto& rule : iRules)
            {
                if (!std::holds_alternative<symbol>(rule.lhs[0]))
                    continue;
                symbol const ruleSymbol = std::get<symbol>(rule.lhs[0]);
                scoped_stack_entry sse{ *this, rule, aSource };
                if (ruleSymbol == aSymbol && !left_recursion(aNode, rule))
                {
                    aNode.rule = &rule;
                    auto const& ruleAtom = rule.rhs[0];
                    typename cst_node::child_list children;
                    std::swap(aNode.children, children);
                    auto const match = parse(ruleAtom, aNode, std::string_view{ sourceNext, sourceEnd });
                    std::swap(aNode.children, children);
                    if (match)
                    {
                        if (!result || match.value().value.size() > result.value().value.size())
                        {
                            result = match;
                            resultRule = &rule;
                            resultChildren = children;
                        }
                    }
                }
            }

            if (result)
            {
                aNode.rule = resultRule;
                if (!aNode.c.has_value())
                    aNode.c = resultRule->rhs[0].c;
                aNode.children.insert(aNode.children.end(), std::make_move_iterator(resultChildren.begin()), std::make_move_iterator(resultChildren.end()));
                return ((sdp ? sdp->ok = true : true), result);
            }

            aNode.rule = nullptr;
            return {};
        }

        std::optional<parse_result> parse(primitive_atom const& aAtom, cst_node& aNode, std::string_view const& aSource)
        {
            if (iError)
                return {};

            auto cacheEntry = iCache.find(cache_key{ &aAtom, aSource.data() });
            if (cacheEntry != iCache.end())
            {
                auto const cachedResult = cacheEntry->second.result;
                aNode.children = cacheEntry->second.nodes;
                return cachedResult;
            }

            std::optional<parse_result> result;
            char const* sourceNext = std::to_address(aSource.begin());
            char const* sourceEnd = std::to_address(aSource.end());

            iDeepestParse = std::max(iDeepestParse, sourceNext);

            scoped_counter sc{ iLevel };
            std::optional<scoped_debug_print> sdp;
            if (!std::holds_alternative<symbol>(aAtom) && iDebugScan)
                sdp.emplace(*this, aAtom, aSource);

            // todo: visitor?

            if (std::holds_alternative<symbol>(aAtom))
            {
                symbol const atomSymbol = std::get<symbol>(aAtom);
                auto newChild = std::make_shared<cst_node>(&aNode, aNode.rule, &aAtom, aSource);
                aNode.children.push_back(newChild);
                auto const partialResult = parse(atomSymbol, *newChild, aSource);
                if (partialResult)
                {
                    if (!aNode.c.has_value())
                        aNode.c = without_association(aAtom.c);
                    if (!newChild->c.has_value())
                        newChild->c = aAtom.c;
                    newChild->value = partialResult.value().value;
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, apply_partial_result(result, partialResult)};
                    return iCache[cache_key{ &aAtom, aSource.data() }].result;
                }
                aNode.children.pop_back();
            }
            else if (std::holds_alternative<terminal>(aAtom))
            {
                auto const& t = std::get<terminal>(aAtom);
                if ((!t.empty() && aSource.find(t) == 0) || (t.empty() && sourceNext == sourceEnd ))
                {
                    auto const partialResult = aSource.substr(0, t.size());
                    auto newChild = std::make_shared<cst_node>(&aNode, aNode.rule, &aAtom, partialResult);
                    newChild->c = aAtom.c;
                    aNode.children.push_back(newChild);
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, apply_partial_result(result, partialResult) };
                    return ((sdp ? sdp->ok = true : true), iCache[cache_key{ &aAtom, aSource.data() }].result);
                }
            }
            else if (std::holds_alternative<range>(aAtom))
            {
                auto const min = std::get<terminal>(std::get<range>(aAtom).value[0]).value();
                auto const max = std::get<terminal>(std::get<range>(aAtom).value[1]).value();
                if (!aSource.empty() && aSource[0] >= min && aSource[0] <= max)
                {
                    auto const partialResult = aSource.substr(0, 1);
                    auto newChild = std::make_shared<cst_node>(&aNode, aNode.rule, &aAtom, partialResult);
                    newChild->c = aAtom.c;
                    aNode.children.push_back(newChild);
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, apply_partial_result(result, partialResult) };
                    return ((sdp ? sdp->ok = true : true), iCache[cache_key{ &aAtom, aSource.data() }].result);
                }
            }
            else if (std::holds_alternative<sequence>(aAtom))
            {
                char const* spanStart = nullptr;
                char const* spanEnd = nullptr;
                typename cst_node::child_list children;
                std::swap(aNode.children, children);
                for (auto const& a : std::get<sequence>(aAtom).value)
                {
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });
                    if (!partialResult)
                    {
                        std::swap(aNode.children, children);
                        return {};
                    }
                    if (std::holds_alternative<discard>(a) && std::get<discard>(a).trim)
                    {
                        if (spanEnd == nullptr)
                            spanStart = std::to_address(partialResult->value.end());
                    }
                    else
                    {
                        if (spanStart == nullptr)
                            spanStart = std::to_address(partialResult->value.begin());
                        spanEnd = std::to_address(partialResult->value.end());
                    }
                    sourceNext = std::to_address(partialResult->sourceNext);
                }
                std::swap(aNode.children, children);
                aNode.children.insert(aNode.children.end(), std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
                if (aAtom.c.has_value())
                    aNode.c = aAtom.c;
                if (spanEnd == nullptr)
                    spanEnd = spanStart;
                result = std::string_view{ spanStart, spanEnd };
                result.value().sourceNext = sourceNext;
                iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
                return ((sdp ? sdp->ok = true : true), result);
            }
            else if (std::holds_alternative<optional>(aAtom))
            {
                for (auto const& a : std::get<optional>(aAtom).value)
                {
                    auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                    if (partialResult)
                    {
                        if (!aNode.c.has_value())
                            aNode.c = aAtom.c;
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::to_address(partialResult->sourceNext);
                    }
                }
                if (!result)
                    result.emplace(sourceNext, sourceNext);
                iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
                return ((sdp ? sdp->ok = true : true), result);
            }
            else if (std::holds_alternative<repeat>(aAtom))
            {
                bool foundAtLeastOne = false;
                bool found = false;
                char const* spanStart = nullptr;
                char const* spanEnd = nullptr;
                do
                {
                    found = false;
                    for (auto const& a : std::get<repeat>(aAtom).value)
                    {
                        auto const partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });;
                        if (partialResult)
                        {
                            if (!aNode.c.has_value())
                                aNode.c = aAtom.c;
                            foundAtLeastOne = true;
                            found = true;
                            if (std::holds_alternative<discard>(a) && std::get<discard>(a).trim)
                            {
                                if (spanEnd == nullptr)
                                    spanStart = std::to_address(partialResult->value.end());
                            }
                            else
                            {
                                if (spanStart == nullptr)
                                    spanStart = std::to_address(partialResult->value.begin());
                                spanEnd = std::to_address(partialResult->value.end());
                            }
                            sourceNext = std::to_address(partialResult->sourceNext);
                        }
                    }
                } while (found);
                if (spanEnd == nullptr)
                    spanEnd = spanStart;
                result = std::string_view{ spanStart, spanEnd };
                result.value().sourceNext = sourceNext;
                if (foundAtLeastOne)
                {
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
                    return ((sdp ? sdp->ok = true : true), result);
                }
                else if (!std::get<repeat>(aAtom).atLeastOne)
                {
                    result.emplace(sourceNext, sourceNext);
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
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
                        if (!aNode.c.has_value())
                            aNode.c = aAtom.c;
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::to_address(partialResult->sourceNext);
                        found = true;
                        break;
                    }
                }
                if (found)
                {
                    iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
                    return ((sdp ? sdp->ok = true : true), result);
                }
                return {};
            }
            else if (std::holds_alternative<discard>(aAtom))
            {
                for (auto const& a : std::get<discard>(aAtom).value)
                {
                    typename cst_node::child_list children;
                    std::swap(aNode.children, children);
                    auto partialResult = parse(a, aNode, std::string_view{ sourceNext, sourceEnd });
                    std::swap(aNode.children, children);
                    if (partialResult)
                    {
                        if (!aNode.c.has_value())
                            aNode.c = aAtom.c;
                        result = apply_partial_result(result, partialResult);
                        sourceNext = std::to_address(partialResult->sourceNext);
                    }
                }
                iCache[cache_key{ &aAtom, aSource.data() }] = cache_result{ aNode.children, result };
                return ((sdp ? sdp->ok = true : true), result);
            }

            return {};
        }

        static std::optional<parse_result> apply_partial_result(std::optional<parse_result> const& aResult, std::optional<parse_result> const& aPartialResult)
        {
            if (!aResult)
                return aPartialResult.value();
            char const* resultFirst = std::to_address(aResult->value.begin());
            char const* resultLast = std::to_address(aResult->value.end());
            char const* partialResultFirst = std::to_address(aPartialResult->value.begin());
            char const* partialResultLast = std::to_address(aPartialResult->value.end());
            std::string_view result{ std::min(resultFirst, partialResultFirst), std::max(resultLast, partialResultLast) };
            return result;
        }

    private:
        struct scoped_stack_entry
        {
            parser& owner;

            scoped_stack_entry(parser& owner, rule const& rule, std::string_view const& source) :
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
            parser& owner;
            std::string value;
            std::string_view const& source;
            bool ok = false;

            template <typename T>
            scoped_debug_print(parser& aOwner, T const& aValue, std::string_view const& aSource) : 
                owner{ aOwner },
                source{ aSource }
            {
                std::ostringstream oss;
                if constexpr (std::is_same_v<T, symbol>)
                    oss << "t(" << enum_to_string(aValue) << ")";
                else if constexpr (std::is_same_v<std::decay_t<T>, primitive_atom>)
                {
                    std::visit([&](auto const& pa)
                    {
                        if constexpr (std::is_same_v<symbol, std::decay_t<decltype(pa)>>)
                            oss << "symbol(" << enum_to_string(pa) << ")";
                        else if constexpr (std::is_same_v<terminal, std::decay_t<decltype(pa)>>)
                            oss << "teminal(" << to_string(pa.type) << ":[" << debug_print(pa) << "])";
                        else
                            oss << "atom(" << to_string(pa.type) << ")";
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
                if (charsAdded == aMaxChars)
                {
                    result << "...";
                    break;
                }
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
            }
            return result.str();
        }

        static std::string debug_print_cst(cst_node const& aNode, std::uint32_t aLevel = 0)
        {
            std::ostringstream oss;
            oss << std::string(static_cast<std::size_t>(aLevel), ' ');
            if (aNode.atom)
            {
                std::visit([&](auto const& pa)
                    {
                        if constexpr (std::is_same_v<symbol, std::decay_t<decltype(pa)>>)
                            oss << enum_to_string(pa);
                        else
                            oss << to_string(pa.type);
                        if (aNode.c.has_value())
                            oss << " : " << aNode.c.value();
                        oss << " = [" << debug_print(aNode.value, 64) << "]";
                    }, *aNode.atom);
            }
            oss << std::endl;
            for (auto const& childNode : aNode.children)
                oss << debug_print_cst(*childNode, aLevel + 1);
            return oss.str();
        }

        static std::string debug_print_ast(ast_node const& aNode, std::uint32_t aLevel = 0)
        {
            std::ostringstream oss;
            oss << std::string(static_cast<std::size_t>(aLevel), ' ');
            if (aNode.atom)
            {
                std::visit([&](auto const& pa)
                    {
                        oss << aNode.c.value();
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
        std::string_view iSource;
        cst_node iCst = {};
        cst_node iAst = {};
        std::vector<std::pair<rule const*, std::string_view>> iStack;
        std::uint32_t iMaxLevel = 256;
        std::uint32_t iLevel = 0;
        char const* iDeepestParse;
        std::optional<std::string> iError;
        struct cache_result
        {
            typename cst_node::child_list nodes;
            std::optional<parse_result> result;
        };
        using cache_key = std::pair<primitive_atom const*, char const*>;
        std::unordered_map<cache_key, cache_result, boost::hash<cache_key>> iCache;
        std::ostream* iDebugOutput = nullptr;
        bool iDebugScan = false;
        bool iDebugSource = true;
        bool iDebugCst = true;
    };

    template <typename Symbol>
    using parser_terminal = typename parser<Symbol>::terminal;

    template <typename Symbol>
    using parser_primitive = typename parser<Symbol>::primitive_atom;

    template <typename Symbol>
    using parser_atom = typename parser<Symbol>::atom;

    template <typename Symbol>
    using parser_undefined = typename parser<Symbol>::undefined;

    template <typename Symbol>
    using parser_choice = typename parser<Symbol>::choice;

    template <typename Symbol>
    using parser_sequence = typename parser<Symbol>::sequence;

    template <typename Symbol>
    using parser_repeat = typename parser<Symbol>::repeat;

    template <typename Symbol>
    using parser_range = typename parser<Symbol>::range;

    template <typename Symbol>
    using parser_optional = typename parser<Symbol>::optional;

    template <typename Symbol>
    using parser_discard = typename parser<Symbol>::discard;

    template <typename Symbol>
    using parser_rule = typename parser<Symbol>::rule;

    template <typename Symbol>
    using parser_concept = typename parser<Symbol>::_concept;

    template <typename T>
    concept ParserComponent = std::is_base_of_v<parser_component_base, T> && 
        !std::is_base_of_v<parser_component<parser_component_type::Rule>, T>;

    template <typename T>
    concept ParserTerminal = std::is_base_of_v<parser_component<parser_component_type::Terminal>, T>;

    template <typename T>
    concept ParserPrimitive = std::is_base_of_v<parser_component<parser_component_type::Primitive>, T>;

    template <typename T>
    concept ParserAtom = std::is_base_of_v<parser_component<parser_component_type::Atom>, T>;

    template <typename T>
    concept ParserUndefined = std::is_base_of_v<parser_component<parser_component_type::Undefined>, T>;

    template <typename T>
    concept ParserChoice = std::is_base_of_v<parser_component<parser_component_type::Choice>, T>;

    template <typename T>
    concept ParserSequence = std::is_base_of_v<parser_component<parser_component_type::Sequence>, T>;

    template <typename T>
    concept ParserRepeat = std::is_base_of_v<parser_component<parser_component_type::Repeat>, T>;

    template <typename T>
    concept ParserRange = std::is_base_of_v<parser_component<parser_component_type::Range>, T>;

    template <typename T>
    concept ParserOptional = std::is_base_of_v<parser_component<parser_component_type::Optional>, T>;

    template <typename T>
    concept ParserDiscard = std::is_base_of_v<parser_component<parser_component_type::Discard>, T>;

    template <typename T>
    concept ParserRule = std::is_base_of_v<parser_component<parser_component_type::Rule>, T>;

    template <typename T>
    concept ParserConcept = std::is_base_of_v<parser_component<parser_component_type::Concept>, T>;

    template <typename T>
    concept SymbolEnum = std::is_enum_v<T> && std::is_convertible_v<T, decltype(is_parser_symbol(T{}))>;

    namespace parser_operators
    {
        template <SymbolEnum Symbol>
        inline parser_rule<Symbol> operator>>(Symbol lhs, parser_primitive<Symbol> const& rhs)
        {
            return parser_rule<Symbol>{ lhs, rhs };
        }

        template <ParserRule Rule>
        inline Rule operator|(Rule const& lhs, parser_primitive<typename Rule::symbol_type> const& rhs)
        {
            return Rule{ lhs.lhs, parser_atom<typename Rule::symbol_type>{ lhs.rhs, rhs } };
        }

        template <ParserConcept Concept>
        inline parser_primitive<typename Concept::symbol_type> operator<=>(typename Concept::symbol_type lhs, Concept const& rhs)
        {
            parser_primitive<typename Concept::symbol_type> result = lhs;
            result.c = rhs;
            return result;
        }

        template <ParserConcept Concept>
        inline parser_primitive<typename Concept::symbol_type> operator<=>(parser_primitive<typename Concept::symbol_type> const& lhs, Concept const& rhs)
        {
            parser_primitive<typename Concept::symbol_type> result = lhs;
            result.c = rhs;
            return result;
        }

        template <ParserRepeat Repeat>
        inline Repeat operator|(Repeat const& lhs, typename Repeat::symbol_type rhs)
        {
            return Repeat{ lhs, rhs };
        }

        template <SymbolEnum Symbol>
        inline parser_repeat<Symbol> operator|(Symbol lhs, Symbol rhs)
        {
            return parser_repeat<Symbol>{ lhs, rhs };
        }

        template <SymbolEnum Symbol>
        inline parser_repeat<Symbol> operator|(parser_primitive<Symbol> const& lhs, Symbol rhs)
        {
            return parser_repeat<Symbol>{ lhs, rhs };
        }

        template <SymbolEnum Symbol>
        inline parser_repeat<Symbol> operator|(Symbol lhs, parser_primitive<Symbol> const& rhs)
        {
            return parser_repeat<Symbol>{ lhs, rhs };
        }

        template <ParserComponent Component1, ParserComponent Component2>
        inline parser_repeat<typename Component1::symbol_type> operator|(Component1 const& lhs, Component2 const& rhs)
        {
            return parser_repeat<typename Component1::symbol_type>{ lhs, rhs };
        }

        template <ParserComponent Component>
        inline parser_repeat<typename Component::symbol_type> operator|(Component const& lhs, char rhs)
        {
            return parser_repeat<typename Component::symbol_type>{ lhs, parser_terminal<typename Component::symbol_type>{ rhs } };
        }

        template <ParserComponent Component>
        inline parser_repeat<typename Component::symbol_type> operator|(char lhs, Component const& rhs)
        {
            return parser_repeat<typename Component::symbol_type>{ parser_terminal<typename Component::symbol_type>{ lhs },  rhs };
        }

        template <ParserRule Rule>
        inline Rule operator,(Rule const& lhs, parser_primitive<typename Rule::symbol_type> const& rhs)
        {
            return Rule{ lhs.lhs, parser_sequence<typename Rule::symbol_type>{ lhs.rhs, rhs } };
        }

        template <SymbolEnum Symbol>
        inline parser_sequence<Symbol> operator,(Symbol lhs, Symbol rhs)
        {
            return parser_sequence<Symbol>{ lhs, rhs };
        }

        template <ParserComponent Component1, ParserComponent Component2>
        inline parser_sequence<typename Component1::symbol_type> operator,(Component1 const& lhs, Component2 const& rhs)
        {
            return parser_sequence<typename Component1::symbol_type>{ lhs, rhs };
        }

        template <ParserComponent Component, SymbolEnum Symbol>
        inline parser_sequence<Symbol> operator,(Component const& lhs, Symbol rhs)
        {
            return parser_sequence<Symbol>{ lhs, rhs };
        }

        template <SymbolEnum Symbol, ParserComponent Component>
        inline parser_sequence<Symbol> operator,(Symbol lhs, Component const& rhs)
        {
            return parser_sequence<Symbol>{ lhs, rhs };
        }

        template <ParserTerminal Terminal>
        inline parser_sequence<typename Terminal::symbol_type> operator,(Terminal const& lhs, char rhs)
        {
            return parser_sequence<typename Terminal::symbol_type>{ lhs, Terminal{ rhs } };
        }

        template <ParserTerminal Terminal>
        inline parser_sequence<typename Terminal::symbol_type> operator,(char lhs, Terminal const& rhs)
        {
            return parser_sequence<typename Terminal::symbol_type>{ Terminal{ lhs }, rhs };
        }

        template <typename Symbol>
        inline parser_choice<Symbol> choice(parser_repeat<Symbol> const& lhs)
        {
            return { lhs.value };
        }

        template <typename Symbol>
        inline parser_repeat<Symbol> repeat(parser_primitive<Symbol> const& lhs)
        {
            return { lhs };
        }

        template <typename Symbol, typename... Args>
        inline parser_sequence<Symbol> sequence(Args&&... lhs)
        {
            return typename parser_sequence<Symbol>::value_type{ std::forward<Args>(lhs)... };
        }

        template <typename Symbol>
        inline parser_range<Symbol> range(parser_primitive<Symbol> const& lhs, parser_primitive<Symbol> const& rhs)
        {
            return { lhs, rhs };
        }

        template <typename Symbol>
        inline parser_optional<Symbol> optional(parser_primitive<Symbol> const& lhs)
        {
            return { lhs };
        }

        template <typename Symbol>
        inline parser_discard<Symbol> discard(parser_primitive<Symbol> const& lhs)
        {
            return { lhs };
        }
    }

#define enable_neolib_parser(symbol)\
    inline symbol is_parser_symbol(symbol) { return {}; }\
    \
    using neolib::parser_operators::operator>>;\
    using neolib::parser_operators::operator<=>;\
    using neolib::parser_operators::operator|;\
    using neolib::parser_operators::operator,;\
    \
    inline neolib::parser_terminal<symbol> operator"" _(const char* str, std::size_t len)\
    {\
        return neolib::parser_terminal<symbol>{ str, len };\
    }\
    inline neolib::parser_terminal<symbol> operator"" _(char character)\
    {\
        return neolib::parser_terminal<symbol>{ character };\
    }\
    inline neolib::parser_concept<symbol> operator"" _concept(const char* str, std::size_t len)\
    {\
        return neolib::parser_concept<symbol>{ str, len };\
    }\
    inline neolib::parser_concept<symbol> operator"" _concept_associate_left(const char* str, std::size_t len)\
    {\
        auto result = neolib::parser_concept<symbol>{ str, len };\
        result.association = neolib::concept_association::Left;\
        return result;\
    }\
    inline neolib::parser_concept<symbol> operator"" _concept_associate_right(const char* str, std::size_t len)\
    {\
        auto result = neolib::parser_concept<symbol>{ str, len };\
        result.association = neolib::concept_association::Right;\
        return result;\
    }\
    template <typename T>\
    inline neolib::parser_choice<symbol> choice(T&& lhs)\
    {\
        return neolib::parser_operators::choice<symbol>(std::forward<T>(lhs));\
    }\
    template <typename T>\
    inline neolib::parser_repeat<symbol> repeat(T&& lhs)\
    {\
        return neolib::parser_operators::repeat<symbol>(std::forward<T>(lhs));\
    }\
    template <typename... T>\
    inline neolib::parser_sequence<symbol> sequence(T&&... lhs)\
    {\
        return neolib::parser_operators::sequence<symbol>(std::forward<T>(lhs)...); \
    }\
    template <typename T>\
    inline neolib::parser_range<symbol> range(T&& lhs, T&& rhs)\
    {\
        return neolib::parser_operators::range<symbol>(std::forward<T>(lhs), std::forward<T>(rhs)); \
    }\
    template <typename T>\
    inline neolib::parser_optional<symbol> optional(T&& lhs)\
    {\
        return neolib::parser_operators::optional<symbol>(std::forward<T>(lhs)); \
    }\
    template <typename T>\
        inline neolib::parser_discard<symbol> discard(T&& lhs)\
    {\
        return neolib::parser_operators::discard<symbol>(std::forward<T>(lhs)); \
    }\
    template <typename T>\
        inline neolib::parser_discard<symbol> fold(T&& lhs)\
    {\
        return ~neolib::parser_operators::discard<symbol>(std::forward<T>(lhs)); \
    }
}
