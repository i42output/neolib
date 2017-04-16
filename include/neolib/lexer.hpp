// lexer.hpp
/*
 *  Copyright (c) 2017 Leigh Johnston.
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
#include <boost/functional/hash.hpp>
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
		Push,
		Pop,
		Eat,
		Keep,
		Not
	};
		
	template <typename Token, typename CharT = char>
	class lexer_atom
	{
	public:
		typedef Token token_type;
		typedef CharT char_type;
		typedef std::pair<char_type, char_type> range_type;
		typedef std::basic_string<char_type> string_type;
		typedef std::vector<lexer_atom_function> function_list;
		typedef std::pair<token_type, function_list> function_type;
		typedef variant<char_type, range_type, string_type, lexer_atom_match_any, token_type, function_type> value_type;
		typedef string_type token_value_type;
	public:
		struct not_token : std::logic_error { not_token() : std::logic_error("neolib::lexer_atom::not_token") {} };
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
		template <typename T>
		bool is() const
		{
			return iValue.is<T>();
		}
		const value_type& value() const
		{
			return iValue;
		}
		bool is_token() const
		{
			return iValue.is<token_type>() || iValue.is<function_type>();
		}
		token_type token() const
		{
			if (iValue.is<token_type>())
				return static_variant_cast<token_type>(iValue);
			else if (iValue.is<function_type>())
				return static_variant_cast<const function_type&>(iValue).first;
			throw not_token();
		}
		bool has_functions() const
		{
			return iValue.is<function_type>();
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

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_push(Token aToken)
	{
		return lexer_atom<Token, CharT>{ lexer_atom_function::Push, aToken };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_pop(Token aToken)
	{
		return lexer_atom<Token, CharT>{ lexer_atom_function::Pop, aToken };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_eat(Token aToken)
	{
		return lexer_atom<Token, CharT>{ lexer_atom_function::Eat, aToken };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_eat(lexer_atom<Token, CharT> aAtom)
	{
		if (aAtom.has_functions())
		{
			aAtom.functions().push_back(lexer_atom_function::Eat);
			return aAtom;
		}
		return typename lexer_atom<Token, CharT>::function_type{ aAtom.token(), { lexer_atom_function::Eat } };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_keep(Token aToken)
	{
		return lexer_atom<Token, CharT>{ lexer_atom_function::Keep, aToken };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_keep(lexer_atom<Token, CharT> aAtom)
	{
		if (aAtom.has_functions())
		{
			aAtom.functions().push_back(lexer_atom_function::Keep);
			return aAtom;
		}
		return typename lexer_atom<Token, CharT>::function_type{ aAtom.token(),{ lexer_atom_function::Keep } };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_make(Token aToken, CharT aChar)
	{
		return lexer_atom<Token, CharT>{aToken, typename lexer_atom<Token, CharT>::token_value_type{ 1, aChar } };
	}

	template <typename Token, typename CharT = char>
	inline lexer_atom<Token, CharT> token_not(Token aToken)
	{
		return lexer_atom<Token, CharT>{ lexer_atom_function::Not, aToken };
	}

	template <typename CharT = char>
	inline std::pair<CharT, CharT> token_range(CharT aFrom, CharT aTo)
	{
		return std::pair<CharT, CharT>{ aFrom, aTo };
	}

	inline lexer_atom_match_any token_any()
	{
		return lexer_atom_match_any{};
	}

	template <typename Atom>
	class lexer_rule
	{
	public:
		typedef Atom atom_type;
		typedef typename atom_type::token_type token_type;
	public:
		atom_type symbol;
		std::vector<atom_type> expression;
	};

	template <typename Atom>
	class lexer
	{
	public:
		typedef Atom atom_type;
		typedef typename atom_type::token_type token_type;
		typedef typename atom_type::char_type char_type;
		typedef typename atom_type::range_type range_type;
		typedef typename atom_type::string_type string_type;
		typedef typename atom_type::function_type function_type;
		typedef lexer_rule<atom_type> rule_type;
	private:
		typedef std::vector<rule_type> rule_list;
		class node
		{
		public:
			typedef neolib::variant<token_type, function_type> value_type;
			typedef std::pair<node*, value_type> next_type;
		public:
			struct bad_terminal_atom : std::logic_error { bad_terminal_atom() : std::logic_error("neolib::lexer::node::bad_terminal_atom") {} };
			struct unsupported_atom_type : std::logic_error { unsupported_atom_type() : std::logic_error("neolib::lexer::node::unsupported_atom_type") {} };
			struct node_exists : std::logic_error { node_exists() : std::logic_error("neolib::lexer::node::node_exists") {} };
		public:
			node() :
				iTokenMap{}, iCharMap{}
			{
			}
		public:
			void map(const atom_type& aAtom, node& aNextNode)
			{
				if (aAtom.is<char_type>())
					map(static_variant_cast<char_type>(aAtom.value()), aNextNode);
				else if (aAtom.is<token_type>())
					map(static_variant_cast<token_type>(aAtom.value()), aNextNode);
			}
			void map(const atom_type& aAtom, const atom_type& aTerminalAtom)
			{
				if (aTerminalAtom.is<token_type>())
				{
					if (aAtom.is<char_type>())
						map(static_variant_cast<char_type>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
					else if (aAtom.is<token_type>())
						map(static_variant_cast<token_type>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
					else if (aAtom.is<function_type>())
						map(static_variant_cast<const function_type&>(aAtom.value()), static_variant_cast<token_type>(aTerminalAtom.value()));
				}
				else if (aTerminalAtom.is<function_type>())
				{
					if (aAtom.is<char_type>())
						map(static_variant_cast<char_type>(aAtom.value()), static_variant_cast<const function_type&>(aTerminalAtom.value()));
					else if (aAtom.is<token_type>())
						map(static_variant_cast<token_type>(aAtom.value()), static_variant_cast<const function_type&>(aTerminalAtom.value()));
					else if (aAtom.is<function_type>())
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
				if (aAtom.is<char_type>())
					return lookup(static_variant_cast<char_type>(aAtom.value()));
				else if (aAtom.is<token_type>())
					return lookup(static_variant_cast<token_type>(aAtom.value()));
				else if (aAtom.is<function_type>())
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
		private:
			mutable std::unordered_map<char_type, next_type> iCharMap;
			mutable std::unordered_map<token_type, next_type> iTokenMap;
			mutable std::unordered_map<function_type, next_type, boost::hash<function_type>> iFunctionMap;
		};
		typedef std::list<node> node_list;
	public:
		struct bad_lex_tree : std::logic_error { bad_lex_tree() : std::logic_error("neolib::lexer_atom::bad_lex_tree") {} };
	public:
		template <typename Iter>
		lexer(std::istream& aInput, Iter aFirstRule, Iter aLastRule) : 
			iInput { aInput }
		{
			for (auto r = aFirstRule; r != aLastRule; ++r)
				build(*r);
		}
	public:
		lexer& operator>>(atom_type& aAtom)
		{
			if (iQueue.empty())
				next();
			if (!iQueue.empty())
			{
				aAtom = iQueue.front();
				iQueue.pop_front();
			}
			return *this;
		}
		explicit operator bool() const
		{
			return !iQueue.empty() || !iBuffer.empty() || iInput;
		}
	private:
		void build(const rule_type& aRule)
		{
			if (iNodes.empty())
				iNodes.push_back(node{});
			build(aRule, 0u, iNodes.front());
		}
		node& build(const rule_type& aRule, std::size_t aExpressionIndex, node& aNode)
		{
			return build(aRule, aRule.expression[aExpressionIndex], aExpressionIndex, aNode);
		}
		node& build(const rule_type& aRule, const atom_type& aAtom, std::size_t aExpressionIndex, node& aNode, bool aHalt = false)
		{
			if (aAtom.is<range_type>())
			{
				auto& r = static_variant_cast<const range_type&>(aAtom.value());
				for (char_type ch = r.first; ch < r.second; ++ch)
					build(aRule, ch, aExpressionIndex, aNode);
				return build(aRule, r.second, aExpressionIndex, aNode);
			}
			else if (aAtom.is<string_type>())
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
					iNodes.push_back(node{});
					auto& newNode = iNodes.back();
					aNode.map(aAtom, newNode);
					if (!aHalt)
						return build(aRule, aExpressionIndex + 1, newNode);
					else
						return newNode;
				}
			}
		}
		void next()
		{
			// todo
			char ch;
			while (iInput >> ch)
			{
				iBuffer.push_back(ch);
			}
		}
	private:
		node_list iNodes;
		std::istream& iInput;
		std::deque<char_type> iBuffer;
		std::deque<atom_type> iQueue;
	};
}
