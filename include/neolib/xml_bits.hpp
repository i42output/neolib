// xml_bits.hpp
/*
 *  NoFussXML v5.3.3
 *
 *  Copyright (c) 2012-present, Leigh Johnston.
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

#include "neolib.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cwchar>
#include "quick_string.hpp"

#ifdef _MSC_VER
#define SELECTANY __declspec( selectany ) 
#else
#define SELECTANY
#endif

namespace neolib
{
	namespace
	{
		template <typename> 
		struct characters {};
		template <>
		struct characters<char>
		{
			static const char sTabChar = '\t';
			static const char sSpaceChar = ' ';
			static const char sNewLineChar = '\n';
			static const char sCarriageReturnChar = '\r';
			static const char sLessThanChar = '<';
			static const char sGreaterThanChar = '>';
			static const char sEqualsChar = '=';
			static const char sForwardSlashChar = '/';
			static const char sAmpersandChar = '&';
			static const char sSemicolonChar = ';';
			static const char sHashChar = '#';
			static const char sHexChar = 'x';
			static const char sQuoteChar = '\"';
			static const char sSingleQuoteChar = '\'';
			static const char sElementTagStart = '<';
			static const char sElementTagEnd= '>';
		};
		const SELECTANY char characters<char>::sTabChar;
		const SELECTANY char characters<char>::sSpaceChar;
		const SELECTANY char characters<char>::sNewLineChar;
		const SELECTANY char characters<char>::sCarriageReturnChar;
		const SELECTANY char characters<char>::sLessThanChar;
		const SELECTANY char characters<char>::sGreaterThanChar;
		const SELECTANY char characters<char>::sEqualsChar;
		const SELECTANY char characters<char>::sForwardSlashChar;
		const SELECTANY char characters<char>::sAmpersandChar;
		const SELECTANY char characters<char>::sSemicolonChar;
		const SELECTANY char characters<char>::sHashChar;
		const SELECTANY char characters<char>::sHexChar;
		const SELECTANY char characters<char>::sQuoteChar;
		const SELECTANY char characters<char>::sSingleQuoteChar;
		const SELECTANY char characters<char>::sElementTagStart;
		const SELECTANY char characters<char>::sElementTagEnd;
		template <>
		struct characters<wchar_t>
		{
			static const wchar_t sTabChar = L'\t';
			static const wchar_t sSpaceChar = L' ';
			static const wchar_t sNewLineChar = L'\n';
			static const wchar_t sCarriageReturnChar = L'\r';
			static const wchar_t sLessThanChar = L'<';
			static const wchar_t sGreaterThanChar = L'>';
			static const wchar_t sEqualsChar = L'=';
			static const wchar_t sForwardSlashChar = L'/';
			static const wchar_t sAmpersandChar = L'&';
			static const wchar_t sSemicolonChar = L';';
			static const wchar_t sHashChar = L'#';
			static const wchar_t sHexChar = L'x';
			static const wchar_t sQuoteChar = L'\"';
			static const wchar_t sSingleQuoteChar = L'\'';
			static const wchar_t sElementTagStart = L'<';
			static const wchar_t sElementTagEnd= L'>';
		};
		const SELECTANY wchar_t characters<wchar_t>::sTabChar;
		const SELECTANY wchar_t characters<wchar_t>::sSpaceChar;
		const SELECTANY wchar_t characters<wchar_t>::sNewLineChar;
		const SELECTANY wchar_t characters<wchar_t>::sCarriageReturnChar;
		const SELECTANY wchar_t characters<wchar_t>::sLessThanChar;
		const SELECTANY wchar_t characters<wchar_t>::sGreaterThanChar;
		const SELECTANY wchar_t characters<wchar_t>::sEqualsChar;
		const SELECTANY wchar_t characters<wchar_t>::sForwardSlashChar;
		const SELECTANY wchar_t characters<wchar_t>::sAmpersandChar;
		const SELECTANY wchar_t characters<wchar_t>::sSemicolonChar;
		const SELECTANY wchar_t characters<wchar_t>::sHashChar;
		const SELECTANY wchar_t characters<wchar_t>::sHexChar;
		const SELECTANY wchar_t characters<wchar_t>::sQuoteChar;
		const SELECTANY wchar_t characters<wchar_t>::sSingleQuoteChar;
		const SELECTANY wchar_t characters<wchar_t>::sElementTagStart;
		const SELECTANY wchar_t characters<wchar_t>::sElementTagEnd;

		template <typename CharT>
		struct predefined_entities;
		template <>
		struct predefined_entities<char>
		{
			enum { PredefinedEntityCount = 5 };
			typedef std::pair<const char*, const char*> entity;
			static const entity sPredefinedEntities[PredefinedEntityCount];
		};
		const SELECTANY predefined_entities<char>::entity predefined_entities<char>::sPredefinedEntities[predefined_entities<char>::PredefinedEntityCount] = 
		{
			std::make_pair("amp", "&"),
			std::make_pair("lt", "<"),
			std::make_pair("gt", ">"),
			std::make_pair("apos", "\'"),
			std::make_pair("quot", "\"")
		};
		template <>
		struct predefined_entities<wchar_t>
		{
			enum { PredefinedEntityCount = 5 };
			typedef std::pair<const wchar_t*, const wchar_t*> entity;
			static const entity sPredefinedEntities[PredefinedEntityCount];
		};
		const SELECTANY predefined_entities<wchar_t>::entity predefined_entities<wchar_t>::sPredefinedEntities[predefined_entities<wchar_t>::PredefinedEntityCount] = 
		{
			std::make_pair(L"amp", L"&"),
			std::make_pair(L"lt", L"<"),
			std::make_pair(L"gt", L">"),
			std::make_pair(L"apos", L"\'"),
			std::make_pair(L"quot", L"\"")
		};

		template <typename CharT>
		struct parsing_bits {};
		template <>
		struct parsing_bits<char>
		{
			typedef std::string string;
			typedef basic_character_map<char> character_map;
			static const character_map sNameDelimeter;
			static const character_map sNameBadDelimeter;
			static const character_map sAttributeValueDelimeter;
			static const character_map sAttributeValueInvalidOne;
			static const character_map sAttributeValueInvalidTwo;
			static const character_map sTagDelimeter;
			static const character_map sWhitespace;
			static const string sCommentStart;
			static const string sCommentEnd;
			static const string sCdataStart;
			static const string sCdataEnd;
			static const string sDtdStart;
			static const string sDtdEnd;
			static const string sDeclarationStart;
			static const string sDeclarationEnd;
			static const string sEmptyTagWithAttributes;
			static const string sEmptyTag;
		};
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sNameDelimeter = std::string("<>/=\"\'");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sNameBadDelimeter = std::string("<=\"\'");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sAttributeValueDelimeter = std::string("\"\'");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sAttributeValueInvalidOne = std::string("<>\"");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sAttributeValueInvalidTwo = std::string("<>\'");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sTagDelimeter = std::string("<>");
		const SELECTANY parsing_bits<char>::character_map parsing_bits<char>::sWhitespace = std::string(" \t\r\n");
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sCommentStart = "!--";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sCommentEnd = "-->";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sCdataStart = "![CDATA[";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sCdataEnd = "]]>";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sDtdStart = "!DOCTYPE";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sDtdEnd = ">";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sDeclarationStart = "?";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sDeclarationEnd = "?>";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sEmptyTagWithAttributes = " />";
		const SELECTANY parsing_bits<char>::string parsing_bits<char>::sEmptyTag = "/>";
		template <>
		struct parsing_bits<wchar_t>
		{
			typedef std::wstring string;
			typedef basic_character_map<wchar_t> character_map;
			static const character_map sNameDelimeter;
			static const character_map sNameBadDelimeter;
			static const character_map sAttributeValueDelimeter;
			static const character_map sAttributeValueInvalidOne;
			static const character_map sAttributeValueInvalidTwo;
			static const character_map sTagDelimeter;
			static const character_map sWhitespace;
			static const string sCommentStart;
			static const string sCommentEnd;
			static const string sCdataStart;
			static const string sCdataEnd;
			static const string sDtdStart;
			static const string sDtdEnd;
			static const string sDeclarationStart;
			static const string sDeclarationEnd;
			static const string sEmptyTagWithAttributes;
			static const string sEmptyTag;
		};
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sNameDelimeter = std::wstring(L"<>/=\"\'");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sNameBadDelimeter = std::wstring(L"<=\"\'");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sAttributeValueDelimeter = std::wstring(L"\"\'");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sAttributeValueInvalidOne = std::wstring(L"<>\"");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sAttributeValueInvalidTwo = std::wstring(L"<>\'");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sTagDelimeter = std::wstring(L"<>");
		const SELECTANY parsing_bits<wchar_t>::character_map parsing_bits<wchar_t>::sWhitespace = std::wstring(L" \t\r\n");
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sCommentStart = L"!--";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sCommentEnd = L"-->";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sCdataStart = L"![CDATA[";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sCdataEnd = L"]]>";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sDtdStart = L"!DOCTYPE";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sDtdEnd = L">";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sDeclarationStart = L"?";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sDeclarationEnd = L"?>";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sEmptyTagWithAttributes = L" />";
		const SELECTANY parsing_bits<wchar_t>::string parsing_bits<wchar_t>::sEmptyTag = L"/>";
	}

	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sNameDelimeter = parsing_bits<CharT>::sNameDelimeter;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sNameBadDelimeter = parsing_bits<CharT>::sNameBadDelimeter;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sAttributeValueDelimeter = parsing_bits<CharT>::sAttributeValueDelimeter;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sAttributeValueInvalidOne = parsing_bits<CharT>::sAttributeValueInvalidOne;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sAttributeValueInvalidTwo = parsing_bits<CharT>::sAttributeValueInvalidTwo;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sTagDelimeter = parsing_bits<CharT>::sTagDelimeter;
	template <typename CharT, typename Alloc>
	const basic_character_map<CharT> basic_xml<CharT, Alloc>::sWhitespace = parsing_bits<CharT>::sWhitespace;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sCommentStart = parsing_bits<CharT>::sCommentStart;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sCommentEnd = parsing_bits<CharT>::sCommentEnd;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sCdataStart = parsing_bits<CharT>::sCdataStart;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sCdataEnd = parsing_bits<CharT>::sCdataEnd;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sDtdStart = parsing_bits<CharT>::sDtdStart;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sDtdEnd = parsing_bits<CharT>::sDtdEnd;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sDeclarationStart = parsing_bits<CharT>::sDeclarationStart;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sDeclarationEnd = parsing_bits<CharT>::sDeclarationEnd;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sEmptyTagWithAttributes = parsing_bits<CharT>::sEmptyTagWithAttributes;
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::sEmptyTag = parsing_bits<CharT>::sEmptyTag;

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::const_iterator xml_node<CharT, Alloc>::begin(type_e aFilter) const
	{
		for (typename node_list::const_iterator i = iContent.begin(); i != iContent.end(); ++i)
			if ((**i).type() & aFilter)
				return const_iterator(*this, i, aFilter);
		return end(aFilter);
	}

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::const_iterator xml_node<CharT, Alloc>::end(type_e aFilter) const
	{
		return const_iterator(*this, iContent.end(), aFilter);
	}

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::iterator xml_node<CharT, Alloc>::begin(type_e aFilter)
	{
		for (typename node_list::iterator i = iContent.begin(); i != iContent.end(); ++i)
			if ((**i).type() & aFilter)
				return iterator(*this, i, aFilter);
		return end(aFilter);
	}

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::iterator xml_node<CharT, Alloc>::end(type_e aFilter)
	{
		return iterator(*this, iContent.end(), aFilter);
	}

		template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::const_iterator xml_node<CharT, Alloc>::find(const string& aName) const
	{
		for (typename node::const_iterator i = begin(); i != end(); ++i)
			if (i->type() == node::Element && static_cast<const xml_element<CharT, Alloc>&>(*i).name() == aName)
				return i;
		return end();
	}

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::iterator xml_node<CharT, Alloc>::find(const string& aName)
	{
		for (typename node::iterator i = begin(); i != end(); ++i)
			if (i->type() == node::Element && static_cast<xml_element<CharT, Alloc>&>(*i).name() == aName)
				return i;
		return end();
	}

	template <typename CharT, typename Alloc>
	typename xml_node<CharT, Alloc>::iterator xml_node<CharT, Alloc>::find_or_append(const string& aName)
	{
		for (typename node::iterator i = begin(); i != end(); ++i)
			if (i->type() == node::Element && static_cast<xml_element<CharT, Alloc>&>(*i).name() == aName)
				return i;
		insert(end(), new xml_element<CharT, Alloc>(aName.c_str()));
		typename node::iterator newNode = end();
		return --newNode;
	}

	template <typename CharT, typename Alloc>
	template <typename Exception>
	typename xml_node<CharT, Alloc>::const_iterator xml_node<CharT, Alloc>::find_or_throw(const string& aName) const
	{
		typename node::const_iterator i = find(aName);
		if (i != end())
			return i;
		throw Exception();
	}

	template <typename CharT, typename Alloc>
	template <typename Exception>
	typename xml_node<CharT, Alloc>::iterator xml_node<CharT, Alloc>::find_or_throw(const string& aName)
	{
		typename node::iterator i = find(aName);
		if (i != end())
			return i;
		throw Exception();
	}

	template <typename CharT, typename Alloc>
	bool xml_element<CharT, Alloc>::has_attribute(const string& aAttributeName) const
	{
		return iAttributes.find(aAttributeName) != iAttributes.end();
	}

	template <typename CharT, typename Alloc>
	const typename xml_element<CharT, Alloc>::string& xml_element<CharT, Alloc>::attribute_value(const string& aAttributeName) const
	{
		typename attribute_list::const_iterator a = iAttributes.find(aAttributeName);
		if (a != iAttributes.end())
				return a->second;
		static const string null;
		return null;
	}

	template <typename CharT, typename Alloc>
	const typename xml_element<CharT, Alloc>::string& xml_element<CharT, Alloc>::attribute_value(const string& aNewAttributeName, const string& aOldAttributeName) const
	{
		if (has_attribute(aNewAttributeName))
			return attribute_value(aNewAttributeName);
		else
			return attribute_value(aOldAttributeName);
	}

	template <typename CharT, typename Alloc>
	const typename xml_element<CharT, Alloc>::string& xml_element<CharT, Alloc>::text() const
	{
		iText.clear();
		for (typename node::const_iterator i = node::begin(); i != node::end(); ++i)
			if (i->type() == node::Text)
				iText += static_cast<const xml_text<CharT, Alloc>&>(*i).content();
		return iText;
	}

	template <typename CharT, typename Alloc>
	void xml_element<CharT, Alloc>::set_attribute(const string& aAttributeName, const string& aAttributeValue)
	{
		iAttributes[aAttributeName] = aAttributeValue;
	}

	template <typename CharT, typename Alloc>
	void xml_element<CharT, Alloc>::append_text(const string& aText)
	{
		node::push_back(new xml_text<CharT, Alloc>(aText));
	}

	template <typename CharT, typename Alloc>
		template <typename T>
	typename basic_xml<CharT, Alloc>::node_writer& basic_xml<CharT, Alloc>::node_writer::operator<<(const T& aData)
	{
		if (!iLastWasNewLine)
			iStream << aData;
		iLastWasNewLine = false;
		return *this;
	}
	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node_writer& basic_xml<CharT, Alloc>::node_writer::operator<<(const string& aData)
	{
		iStream << static_cast<const typename string::string_type&>(aData);
		if (aData.size() && aData[aData.size() - 1] == characters<CharT>::sNewLineChar)
			iLastWasNewLine = true;
		else
			iLastWasNewLine = false;
		return *this;
	}

	template <typename CharT, typename Alloc>
	basic_xml<CharT, Alloc>::basic_xml(bool aStripWhitespace) : 
		endl(std::endl), iError(false), iIndentChar(characters<CharT>::sTabChar), iIndentCount(1), iStripWhitespace(aStripWhitespace)
	{
		for (std::size_t entityIndex = 0; entityIndex < predefined_entities<CharT>::PredefinedEntityCount; ++entityIndex)
			iEntities.push_back(predefined_entities<CharT>::sPredefinedEntities[entityIndex]);
	}

	template <typename CharT, typename Alloc>
	basic_xml<CharT, Alloc>::basic_xml(const std::string& aPath, bool aStripWhitespace) :
		endl(std::endl), iError(false), iIndentChar(characters<CharT>::sTabChar), iIndentCount(1), iStripWhitespace(aStripWhitespace)
	{
		for (std::size_t entityIndex = 0; entityIndex < predefined_entities<CharT>::PredefinedEntityCount; ++entityIndex)
			iEntities.push_back(predefined_entities<CharT>::sPredefinedEntities[entityIndex]);
		std::ifstream input(aPath);
		if (!input)
			throw failed_to_open_file();
		read(input);
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::clear()
	{
		iDocument.clear();
		iDocumentText.clear();
		iError = false;
	}

	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::node& basic_xml<CharT, Alloc>::document() const
	{
		return iDocument;
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node& basic_xml<CharT, Alloc>::document()
	{
		return iDocument;
	}
	
	template <typename CharT, typename Alloc>
	const typename basic_xml<CharT, Alloc>::element& basic_xml<CharT, Alloc>::root() const
	{
		for (typename node::const_iterator i = iDocument.begin(); i != iDocument.end(); ++i)
			if (i->type() == node::Element)
				return static_cast<const element&>(*i);
		iError = true;
		throw error_no_root();
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::element& basic_xml<CharT, Alloc>::root()
	{
		for (typename node::iterator i = iDocument.begin(); i != iDocument.end(); ++i)
			if (i->type() == node::Element)
				return static_cast<element&>(*i);
		iDocument.push_back(new element());
		return static_cast<element&>(iDocument.back());
	}

	template <typename CharT, typename Alloc>
	bool basic_xml<CharT, Alloc>::got_root() const
	{
		for (typename node::const_iterator i = iDocument.begin(); i != iDocument.end(); ++i)
			if (i->type() == node::Element)
				return true;
		return false;
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::tag basic_xml<CharT, Alloc>::next_tag(typename basic_xml<CharT, Alloc>::string::const_iterator aNext, typename basic_xml<CharT, Alloc>::string::const_iterator aDocumentEnd)
	{
		tag nextTag;
		nextTag.first = std::find(aNext, aDocumentEnd, CharT(characters<CharT>::sElementTagStart));
		if (nextTag.first != aDocumentEnd)
			++nextTag.first;
		nextTag.second = std::find(nextTag.first, aDocumentEnd, CharT(characters<CharT>::sElementTagEnd));
		if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sCommentStart.size() && std::equal(nextTag.first, nextTag.first+sCommentStart.size(), sCommentStart.begin()))
		{
			nextTag.iType = node::Comment;
			nextTag.first += sCommentStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sCommentEnd.begin(), sCommentEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		else if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sDeclarationStart.size() && std::equal(nextTag.first, nextTag.first+sDeclarationStart.size(), sDeclarationStart.begin()))
		{
			nextTag.iType = node::Declaration;
			nextTag.first += sDeclarationStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sDeclarationEnd.begin(), sDeclarationEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		else if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sCdataStart.size() && std::equal(nextTag.first, nextTag.first+sCdataStart.size(), sCdataStart.begin()))
		{
			nextTag.iType = node::Cdata;
			nextTag.first += sCdataStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sCdataEnd.begin(), sCdataEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		else if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sDtdStart.size() + 1 && std::equal(nextTag.first, nextTag.first+sDtdStart.size(), sDtdStart.begin()) &&
			sWhitespace.find(*(nextTag.first + sDtdStart.size())))
		{
			nextTag.iType = node::Dtd;
			nextTag.first += sDtdStart.size();
			nextTag.second = nextTag.first;
			std::size_t nest = 1;
			while(nextTag.second != aDocumentEnd)
			{
				if (*nextTag.second == characters<CharT>::sLessThanChar)
					++nest;
				if (*nextTag.second == characters<CharT>::sGreaterThanChar)
					--nest;
				if (nest == 0)
					break;
				++nextTag.second;
			}
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		return nextTag;
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::string::const_iterator basic_xml<CharT, Alloc>::parse(node& aNode, const tag& aStartTag, typename basic_xml<CharT, Alloc>::string::const_iterator aDocumentEnd)
	{
		if (aStartTag.first == aDocumentEnd || aStartTag.first >= aStartTag.second)
			return aDocumentEnd;

		switch(aStartTag.type())
		{
		case node::Element:
			{
				if (aNode.type() == node::Document && got_root())
				{
					iError = true;
					return aDocumentEnd;
				}
				element& theElement = aNode.type() == node::Element ? static_cast<element&>(aNode) : root();

				/* get element name */
				token elementName = next_token(sNameDelimeter, false, aStartTag.first, aStartTag.second);
				if (elementName.first == aStartTag.second)
				{
					iError = true;
					return aDocumentEnd;
				}
				theElement.name() = string(elementName.first, elementName.second);

				typename string::const_iterator next = elementName.second;

				/* get element attributes */
				while(next != aStartTag.second)
				{
					token attributeName = next_token(sNameDelimeter, false, next, aStartTag.second);
					if (attributeName.first == attributeName.second)
					{
						if (attributeName.first != aStartTag.second &&
							sNameBadDelimeter.find(*attributeName.first))
						{
							iError = true;
							return aDocumentEnd;
						}
						next = aStartTag.second;
						break;
					}
					token attributeEquals = next_token(sAttributeValueDelimeter, false, attributeName.second, aStartTag.second);
					if (attributeEquals.second - attributeEquals.first != 1 || *attributeEquals.first != characters<CharT>::sEqualsChar)
					{
						iError = true;
						return aDocumentEnd;
					}
					token attributeStart = next_token(sAttributeValueDelimeter, false, attributeEquals.second, aStartTag.second);
					if (attributeStart.first != attributeStart.second ||
						attributeStart.first == aStartTag.second ||
						!sAttributeValueDelimeter.find(*attributeStart.first))
					{
						iError = true;
						return aDocumentEnd;
					}
					token attributeValue = next_token(*attributeStart.first == characters<CharT>::sQuoteChar ? sAttributeValueInvalidOne : sAttributeValueInvalidTwo, true, attributeStart.second + 1, aStartTag.second);
					if (attributeValue.first == aStartTag.second ||
						attributeValue.second == aStartTag.second ||
						!sAttributeValueDelimeter.find(*attributeValue.second))
					{
						iError = true;
						return aDocumentEnd;
					}
					next = attributeValue.second + 1;
					typename attribute_list::iterator a = theElement.attributes().insert(std::make_pair(string(attributeName.first, attributeName.second), attributeValue.iHasEntities ? parse_entities(string(attributeValue.first, attributeValue.second)) : string(attributeValue.first, attributeValue.second))).first;
					strip_if(a->second);
				}

				if (*(aStartTag.second-1) == characters<CharT>::sForwardSlashChar) // empty tag
					return next+1;

				++next;

				/* get element content */
				while(next != aDocumentEnd)
				{
					token contentToken = next_token(sTagDelimeter, true, next, aDocumentEnd);
					next = contentToken.second;
					if (next == aDocumentEnd)
						return next;
					string content(contentToken.first, contentToken.second);
					strip_if(content);
					bool hasContent = false;
					for (typename string::const_iterator i = content.cbegin(); !hasContent && i != content.cend(); ++i)
					{
						switch(*i)
						{
						case characters<CharT>::sTabChar:
						case characters<CharT>::sSpaceChar:
						case characters<CharT>::sNewLineChar:
						case characters<CharT>::sCarriageReturnChar:
							break;
						default:
							hasContent = true;
						}
					}
					if (!hasContent)
						content = string();
					if (!content.empty())
					{
						if (contentToken.iHasEntities)
							content = parse_entities(content);
						theElement.push_back(new text(content));
					}
					tag nextTag = next_tag(next, aDocumentEnd); 
					if (nextTag.first > nextTag.second)
						return next;
					if (nextTag.first == nextTag.second)
					{
						next= nextTag.first;
						continue;
					}
					switch (nextTag.type())
					{
					case node::Element:
						if (*nextTag.first == characters<CharT>::sForwardSlashChar)
						{
							if (theElement.name() != string(nextTag.first+1, nextTag.second))
							{
								iError = true;
								return aDocumentEnd;
							}
							theElement.set_use_empty_element_tag(false);
							return nextTag.second+1;
						}
						theElement.push_back(new element());
						break;
					case node::Comment:
						theElement.push_back(new comment());
						break;
					case node::Declaration:
						theElement.push_back(new declaration());
						break;
					case node::Cdata:
						theElement.push_back(new cdata());
						break;
					case node::Dtd:
						theElement.push_back(new dtd());
						break;
					default:
						break;
					}
					next = parse(theElement.back(), nextTag, aDocumentEnd);
				}
				return next;
			}
			// no break required here due to return above (warning dampening)
		case node::Comment:
			if (aNode.type() == node::Comment)
				static_cast<comment&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.push_back(new comment(string(aStartTag.first, aStartTag.second)));
			return aStartTag.second + aStartTag.end_skip();
		case node::Declaration:
			if (aNode.type() == node::Declaration)
				static_cast<declaration&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.push_back(new declaration(string(aStartTag.first, aStartTag.second)));
			return aStartTag.second + aStartTag.end_skip();
		case node::Cdata:
			if (aNode.type() == node::Cdata)
				static_cast<cdata&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.push_back(new cdata(string(aStartTag.first, aStartTag.second)));
			return aStartTag.second + aStartTag.end_skip();
		case node::Dtd:
			if (aNode.type() == node::Dtd)
				static_cast<dtd&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.push_back(new dtd(string(aStartTag.first, aStartTag.second)));
			return aStartTag.second + aStartTag.end_skip();
		default:
			iError = true;
			return aDocumentEnd;
		}
	}

	template <typename CharT, typename Alloc>
	bool basic_xml<CharT, Alloc>::read(std::basic_istream<CharT>& aStream)
	{
		iError = false;

		clear();

		if (!aStream)
			return false;

		typename std::basic_istream<CharT>::pos_type count = 0;
		aStream.seekg(0, std::ios::end);
		if (aStream)
		{
			count = static_cast<long>(aStream.tellg());
			if (count == typename std::basic_istream<CharT>::pos_type(-1))
				count = 0;
			aStream.seekg(0, std::ios::beg);
		}
		else
			aStream.clear();

		string& document = iDocumentText;

		if (count != typename std::basic_istream<CharT>::pos_type(0))
		{
			document.resize(static_cast<typename string::size_type>(count));
			aStream.read(&document[0], count);
			document.resize(static_cast<typename string::size_type>(aStream.gcount()));
		}
		else
		{
			string line;
			CharT buffer[1024];
			while(aStream.read(buffer, 1024))
				document.append(buffer, static_cast<typename string::size_type>(aStream.gcount()));
			if (aStream.eof())
				document.append(buffer, static_cast<typename string::size_type>(aStream.gcount()));
		}

		tag nextTag = next_tag(document.cbegin(), document.cend());		
		while (nextTag.first != document.cend())
		{
			while(nextTag.first != document.cend() && nextTag.first == nextTag.second)
				nextTag = next_tag(nextTag.first, document.cend());
			nextTag = next_tag(parse(iDocument, nextTag, document.cend()), document.cend());
		};

		return got_root();
	}

	template <typename CharT, typename Alloc>
	bool basic_xml<CharT, Alloc>::write(std::basic_ostream<CharT>& aStream)
	{
		iError = false;
		std::ostringstream buffer;
		node_writer theWriter(buffer);
		write_node(theWriter, iDocument, 0);
		aStream.write(buffer.str().c_str(), buffer.str().length());
		return true;
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::set_indent(CharT aIndentChar, std::size_t aIndentCount)
	{
		iIndentChar = aIndentChar;
		iIndentCount = aIndentCount;
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::set_strip_whitespace(bool aStripWhitespace)
	{
		iStripWhitespace = aStripWhitespace;
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::write_node(node_writer& aStream, const node& aNode, std::size_t aIndent) const
	{
		switch(aNode.type())
		{
		case node::Document:
			for (typename node::const_iterator i = aNode.begin(); i != aNode.end(); ++i)
			{
				write_node(aStream, *i, aIndent);
				aStream << endl;
			}
			break;
		case node::Element:
			{
				const element& theElement = static_cast<const element&>(aNode);

				if (&theElement != &root())
					aStream << endl;
				aStream << string(aIndent*iIndentCount, iIndentChar);
				aStream << characters<CharT>::sLessThanChar;
				aStream << theElement.name();

				if (!theElement.attributes().empty())
					for (typename attribute_list::const_iterator i = theElement.attributes().begin(); i != theElement.attributes().end(); ++i)
						aStream << characters<CharT>::sSpaceChar << i->first << characters<CharT>::sEqualsChar << characters<CharT>::sQuoteChar << generate_entities(i->second) << characters<CharT>::sQuoteChar;

				if (!aNode.empty())
				{
					aStream << characters<CharT>::sGreaterThanChar;
					for (typename node::const_iterator i = aNode.begin(); i != aNode.end(); ++i)
					{
						switch(i->type())
						{
						case node::Text:
							if (i != aNode.begin())
							{
								if (iStripWhitespace)
								{
									aStream << endl;
									aStream << string((aIndent+1)*iIndentCount, iIndentChar);
								}
							}
							break;
						case node::Comment:
						case node::Declaration:
							aStream << endl;
							aStream << string((aIndent+1)*iIndentCount, iIndentChar);
							break;
						case node::Cdata:
						case node::Dtd:
							aStream << endl;
							break;
						default:
							break;
						}
						write_node(aStream, *i, aIndent+1);
					}
					if (aNode.back().type() != node::Text)
						aStream << endl << string(aIndent*iIndentCount, iIndentChar);
					aStream << characters<CharT>::sLessThanChar << characters<CharT>::sForwardSlashChar << theElement.name() << characters<CharT>::sGreaterThanChar;
				}
				else if (theElement.use_empty_element_tag())
					aStream << (theElement.attributes().size() ? sEmptyTagWithAttributes : sEmptyTag);
				else
					aStream << characters<CharT>::sGreaterThanChar << characters<CharT>::sLessThanChar << characters<CharT>::sForwardSlashChar << theElement.name() << characters<CharT>::sGreaterThanChar;
			}
			break;
		case node::Text:
			aStream << generate_entities(static_cast<const text&>(aNode).content());
			break;
		case node::Comment:
			aStream << characters<CharT>::sLessThanChar << sCommentStart << static_cast<const comment&>(aNode).content() << sCommentEnd;
			break;
		case node::Declaration:
			aStream << characters<CharT>::sLessThanChar << sDeclarationStart << static_cast<const declaration&>(aNode).content() << sDeclarationEnd;
			break;
		case node::Cdata:
			aStream << characters<CharT>::sLessThanChar << sCdataStart << static_cast<const cdata&>(aNode).content() << sCdataEnd;
			break;
		case node::Dtd:
			aStream << characters<CharT>::sLessThanChar << sDtdStart << static_cast<const dtd&>(aNode).content() << sDtdEnd;
			break;
		default:
			break;
		}
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node::iterator basic_xml<CharT, Alloc>::insert(node& aParent, typename node::iterator aPosition, const CharT* aName)
	{
		return aParent.insert(aPosition, new element(aName));
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::element& basic_xml<CharT, Alloc>::append(node& aParent, const CharT* aName)
	{
		return static_cast<typename basic_xml<CharT, Alloc>::element&>(*insert(aParent, aParent.end(), aName));
	}
	
	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::erase(node& aParent, typename node::iterator aPosition)
	{
		aParent.erase(aPosition);
	}
	
	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node::const_iterator basic_xml<CharT, Alloc>::find(const node& aParent, const CharT* aName) const
	{
		return aParent.find(aName);
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node::iterator basic_xml<CharT, Alloc>::find(node& aParent, const CharT* aName)
	{
		return aParent.find(aName);
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::node::iterator basic_xml<CharT, Alloc>::find_or_append(node& aParent, const CharT* aName)
	{
		return aParent.find_or_append(aName);
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::parse_entities(const string& aString) const
	{
		string newString = aString;
		typename string::size_type pos = 0;
		while((pos = newString.find(characters<CharT>::sAmpersandChar, pos)) != string::npos)
		{
			typename string::size_type endPos = newString.find(characters<CharT>::sSemicolonChar, pos);
			if (endPos == string::npos || (pos+1 == endPos))
			{
				iError = true;
				return aString;
			}
			bool replaced = false;
			if (const_cast<const string&>(newString)[pos+1] == characters<CharT>::sHashChar)
			{
				string characterValue = newString.substr(pos+2, endPos - (pos + 2));
				string character;
				struct converter
				{
					static long string_to_integer(const typename basic_xml<char>::string& aString, int aBase)
					{
						return strtol(aString.c_str(), 0, aBase);
					}
					static long string_to_integer(const typename basic_xml<wchar_t>::string& aString, int aBase)
					{
						return wcstol(aString.c_str(), 0, aBase);
					}
				};
				if (characterValue[0] != characters<CharT>::sHexChar)
					character += static_cast<CharT>(converter::string_to_integer(characterValue, 10));
				else
				{
					characterValue.erase(0, 1);
					character += static_cast<CharT>(converter::string_to_integer(characterValue, 16));
				}
				newString.replace(pos, (endPos - pos) + 1, character);
				++pos;
				replaced = true;
			}
			else
			{
				for(typename entity_list::const_iterator i = iEntities.begin(); i != iEntities.end(); ++i)
				{
					string placeholder;
					placeholder += characters<CharT>::sAmpersandChar;
					placeholder += i->first;
					placeholder += characters<CharT>::sSemicolonChar;
					if (placeholder.size() != endPos - pos + 1)
						continue;
					if (std::equal(placeholder.cbegin(), placeholder.cend(), newString.cbegin() + pos))
					{
						newString.replace(pos, placeholder.size(), i->second);
						pos += i->second.size();
						replaced = true;
						break;
					}
				}
			}
			if (!replaced)
				newString.erase(pos, (endPos - pos) + 1);
		}
		return newString;
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::string basic_xml<CharT, Alloc>::generate_entities(const string& aString) const
	{
		string newString = aString;
		for(typename entity_list::const_iterator i = iEntities.begin(); i != iEntities.end(); ++i)
		{
			string placeholder;
			placeholder += characters<CharT>::sAmpersandChar;
			placeholder += i->first;
			placeholder += characters<CharT>::sSemicolonChar;
			typename string::size_type pos = 0;
			while((pos = newString.find(i->second, pos)) != string::npos)
			{
				newString.replace(pos, i->second.size(), placeholder);
				pos += placeholder.size();
			}
		}	
		return newString;
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::strip(string& aString) const
	{
		typename string::size_type start = 0;
		bool foundPrintable = false;
		while (!foundPrintable && start < aString.size())
		{
			switch(aString[start])
			{
			case characters<CharT>::sTabChar:
			case characters<CharT>::sSpaceChar:
			case characters<CharT>::sNewLineChar:
			case characters<CharT>::sCarriageReturnChar:
				++start;
				break;
			default:
				foundPrintable = true;
				break;
			}
		}			
		if (foundPrintable)
		{
			if (start > 0)
				aString.erase(0, start);
		}
		else
			aString = string();
		if (aString.empty())
			return;
		typename string::size_type end = aString.size() - 1;
		foundPrintable = false;
		while (!foundPrintable && end != string::npos)
		{
			switch(aString[end])
			{
			case characters<CharT>::sTabChar:
			case characters<CharT>::sSpaceChar:
			case characters<CharT>::sNewLineChar:
			case characters<CharT>::sCarriageReturnChar:
				if (end == 0)
					end = string::npos;
				else
					--end;
				break;
			default:
				foundPrintable = true;
				break;
			}
		}		
		if (end != string::npos)
			aString.erase(end+1, aString.size() - (end+1));
		typename string::iterator src = aString.begin();
		typename string::iterator dest = aString.begin();
		bool found = false;
		while(src != aString.end())
		{
			CharT srcChar = *src++;
			switch(srcChar)
			{
			case characters<CharT>::sTabChar:
			case characters<CharT>::sSpaceChar:
			case characters<CharT>::sNewLineChar:
			case characters<CharT>::sCarriageReturnChar:
				if (dest != aString.begin() && *(dest-1) != characters<CharT>::sSpaceChar)
					*dest++ = characters<CharT>::sSpaceChar;
				break;
			default:
				if (!found)
				{
					src = static_cast<string&>(aString).begin();
					dest = static_cast<string&>(aString).begin();
					found = true;
					continue;
				}
				else
					*dest++ = srcChar;
			}
		}
		if (found && dest != aString.end())
			aString.erase(dest, aString.end());
	}

	template <typename CharT, typename Alloc>
	void basic_xml<CharT, Alloc>::strip_if(string& aString) const
	{
		if (iStripWhitespace)
			strip(aString);
	}

	template <typename CharT, typename Alloc>
	typename basic_xml<CharT, Alloc>::token basic_xml<CharT, Alloc>::next_token(const basic_character_map<CharT>& aDelimeters, bool aIgnoreWhitespace, typename basic_xml<CharT, Alloc>::string::const_iterator aCurrent, typename basic_xml<CharT, Alloc>::string::const_iterator aEnd) const
	{
		if (!aIgnoreWhitespace)
		{
			while (aCurrent != aEnd)
			{
				switch(*aCurrent)
				{
				case characters<CharT>::sTabChar:
				case characters<CharT>::sSpaceChar:
				case characters<CharT>::sNewLineChar:
				case characters<CharT>::sCarriageReturnChar:
					++aCurrent;
					break;
				default:
					goto doneStart;
				}
			}
		}
		doneStart:
		token ret;
		ret.first = aCurrent;
		while (aCurrent != aEnd)
		{
			CharT current =*aCurrent;
			if (!aIgnoreWhitespace)
			{
				switch(current)
				{
				case characters<CharT>::sTabChar:
				case characters<CharT>::sSpaceChar:
				case characters<CharT>::sNewLineChar:
				case characters<CharT>::sCarriageReturnChar:
					goto doneEnd;
				default:
					break;
				}
			}
			if (current == characters<CharT>::sAmpersandChar)
				ret.iHasEntities = true;
			if (aDelimeters.find(current))
				break;
			++aCurrent;
		}
		doneEnd:
		ret.second = aCurrent;
		return ret;
	}
}	// namespace neolib

