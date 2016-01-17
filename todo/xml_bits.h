// xml_bits.h - v3.1
/*
 *  Copyright (c) 2010 Leigh Johnston.
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

#include <string>
#include <algorithm>
#include <cstdlib>
#include <cwchar>

namespace lib
{
	template <typename CharT>
	const typename xml_element<CharT>::string& xml_element<CharT>::attribute_value(const string& aAttributeName) const
	{
		for (typename attribute_list::const_iterator i = iAttributes.begin(); i != iAttributes.end(); ++i)
			if (i->first == aAttributeName)
				return i->second;
		static const string null;
		return null;
	}

	template <typename CharT>
	typename xml_element<CharT>::const_iterator xml_element<CharT>::begin() const
	{
		for (typename node::node_list::const_iterator i = node::content().begin(); i != node::content().end(); ++i)
			if ((**i).type() == node::Element)
				return const_iterator(*this, i);
		return end();
	}

	template <typename CharT>
	typename xml_element<CharT>::const_iterator xml_element<CharT>::end() const
	{
		return const_iterator(*this, node::content().end());
	}

	template <typename CharT>
	const typename xml_element<CharT>::string& xml_element<CharT>::text() const
	{
		iText.clear();
		for (typename node::node_list::const_iterator i = node::content().begin(); i != node::content().end(); ++i)
			if ((**i).type() == node::Text)
				iText += static_cast<xml_text<CharT>&>(**i).content();
		return iText;
	}

	template <typename CharT>
	void xml_element<CharT>::set_attribute_value(const string& aAttributeName, const string& aAttributeValue)
	{
		for (typename attribute_list::iterator i = iAttributes.begin(); i != iAttributes.end(); ++i)
			if (i->first == aAttributeName)
			{
				i->second = aAttributeValue;
				return;
			}
		iAttributes.push_back(attribute(aAttributeName, aAttributeValue));
	}

	template <typename CharT>
	void xml_element<CharT>::append_text(const string& aText)
	{
		node::content().push_back(node_ptr(new xml_text<CharT>(static_cast<node&>(*this), aText)));
	}

	template <typename CharT>
	const typename basic_xml<CharT>::predefined_entity basic_xml<CharT>::sPredefinedEntities[basic_xml<CharT>::PredefinedEntityCount] =
	{
		{ "amp", "&" },
		{ "lt", "<" },
		{ "gt", ">" },
		{ "apos", "\'" },
		{ "quot", "\"" },
	};
	template <>
	const basic_xml<wchar_t>::predefined_entity basic_xml<wchar_t>::sPredefinedEntities[basic_xml<wchar_t>::PredefinedEntityCount] =
	{
		{ L"amp", L"&" },
		{ L"lt", L"<" },
		{ L"gt", L">" },
		{ L"apos", L"\'" },
		{ L"quot", L"\"" },
	};

	template <typename CharT>
	basic_xml<CharT>::basic_xml(bool aStripWhitespace) : endl(std::endl), iError(false), iIndentChar(sTabChar[0]), iIndentCount(1), iStripWhitespace(aStripWhitespace)
	{
		for (std::size_t entityIndex = 0; entityIndex < PredefinedEntityCount; ++entityIndex)
			iEntities.push_back(std::make_pair(string(sPredefinedEntities[entityIndex].iFirst), string(sPredefinedEntities[entityIndex].iSecond)));
	}

	template <typename CharT>
	void basic_xml<CharT>::clear()
	{
		iDocument.clear();
		iError = false;
	}

	template <typename CharT>
	const typename basic_xml<CharT>::node& basic_xml<CharT>::document() const
	{
		return iDocument;
	}

	template <typename CharT>
	typename basic_xml<CharT>::node& basic_xml<CharT>::document()
	{
		return iDocument;
	}
	
	template <typename CharT>
	const typename basic_xml<CharT>::element& basic_xml<CharT>::root() const
	{
		for (typename node_list::const_iterator i = iDocument.content().begin(); i != iDocument.content().end(); ++i)
			if ((**i).type() == node::Element)
				return static_cast<const element&>(**i);
		iError = true;
		throw error_no_root();
	}

	template <typename CharT>
	typename basic_xml<CharT>::element& basic_xml<CharT>::root()
	{
		for (typename node_list::iterator i = iDocument.content().begin(); i != iDocument.content().end(); ++i)
			if ((**i).type() == node::Element)
				return static_cast<element&>(**i);
		iDocument.content().push_back(node_ptr(new element(iDocument)));
		return static_cast<element&>(*iDocument.content().back());
	}

	template <typename CharT>
	bool basic_xml<CharT>::got_root() const
	{
		for (typename node_list::const_iterator i = iDocument.content().begin(); i != iDocument.content().end(); ++i)
			if ((**i).type() == node::Element)
				return true;
		return false;
	}

	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sTabChar = "\t";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sSpaceChar = " ";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sNewLineChar = "\n";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sLessThanChar = "<";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sGreaterThanChar = ">";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sEqualsChar = "=";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sForwardSlashChar = "/";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sAmpersandChar = "&";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sSemicolonChar = ";";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sHashChar = "#";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sHexChar = "x";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sQuoteChar = "\"";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sSingleQuoteChar = "\'";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sNameDelimeter = "<>/=\"\'";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sNameBadDelimeter = "<=\"\'";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sAttributeValueDelimeter = "\"\'";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sAttributeValueInvalidOne = "<>\"";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sAttributeValueInvalidTwo = "<>\'";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sTagDelimeter = "<>";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sElementTagStart = "<";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sElementTagEnd= ">";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sWhitespace = " \t\r\n";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sCommentStart = "!--";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sCommentEnd = "-->";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sCdataStart = "![CDATA[";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sCdataEnd = "]]>";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sDtdStart = "!DOCTYPE";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sDtdEnd = ">";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sDeclarationStart = "?";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sDeclarationEnd = "?>";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sEmptyTagWithAttributes = " />";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sEmptyTag = "/>";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sNamespace = "xmlns";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sDefaultNamespace = "";
	template <typename CharT>
	const typename basic_xml<CharT>::string basic_xml<CharT>::sNamespaceDelimeter = ":";

	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sTabChar = L"\t";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sSpaceChar = L" ";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sNewLineChar = L"\n";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sLessThanChar = L"<";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sGreaterThanChar = L">";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sEqualsChar = L"=";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sForwardSlashChar = L"/";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sAmpersandChar = L"&";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sSemicolonChar = L";";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sHashChar = L"#";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sHexChar = L"x";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sQuoteChar = L"\"";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sSingleQuoteChar = L"\'";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sNameDelimeter = L"<>/=\"\'";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sNameBadDelimeter = L"<=\"\'";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sAttributeValueDelimeter = L"\"\'";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sAttributeValueInvalidOne = L"&<>\"";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sAttributeValueInvalidTwo = L"&<>\'";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sTagDelimeter = L"<>";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sElementTagStart = L"<";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sElementTagEnd= L">";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sWhitespace = L" \t\r\n";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sCommentStart = L"!--";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sCommentEnd = L"-->";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sCdataStart = L"![CDATA[";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sCdataEnd = L"]]>";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sDtdStart = L"!DOCTYPE";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sDtdEnd = L">";	
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sDeclarationStart = L"?";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sDeclarationEnd = L"?>";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sEmptyTagWithAttributes = L" />";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sEmptyTag = L"/>";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sNamespace = L"xmlns";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sDefaultNamespace = L"";
	template<> const basic_xml<wchar_t>::string basic_xml<wchar_t>::sNamespaceDelimeter = L":";

	template <typename CharT>
	typename basic_xml<CharT>::tag basic_xml<CharT>::next_tag(typename string::const_iterator aNext, typename string::const_iterator aDocumentEnd)
	{
		tag nextTag;
		nextTag.first = std::find_first_of(aNext, aDocumentEnd, sElementTagStart.begin(), sElementTagStart.end());
		if (nextTag.first != aDocumentEnd)
			++nextTag.first;
		nextTag.second = std::find_first_of(nextTag.first, aDocumentEnd, sTagDelimeter.begin(), sTagDelimeter.end());
		if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sCommentStart.size() && std::equal(nextTag.first, nextTag.first+sCommentStart.size(), sCommentStart.begin()))
		{
			nextTag.iType = node::Comment;
			nextTag.first += sCommentStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sCommentEnd.begin(), sCommentEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sDeclarationStart.size() && std::equal(nextTag.first, nextTag.first+sDeclarationStart.size(), sDeclarationStart.begin()))
		{
			nextTag.iType = node::Declaration;
			nextTag.first += sDeclarationStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sDeclarationEnd.begin(), sDeclarationEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sCdataStart.size() && std::equal(nextTag.first, nextTag.first+sCdataStart.size(), sCdataStart.begin()))
		{
			nextTag.iType = node::Cdata;
			nextTag.first += sCdataStart.size();
			nextTag.second = std::search(nextTag.first, aDocumentEnd, sCdataEnd.begin(), sCdataEnd.end());
			if (nextTag.second == aDocumentEnd)
				nextTag.first = aDocumentEnd;
		}
		if (static_cast<typename string::size_type>(nextTag.second - nextTag.first) >= sDtdStart.size() + 1 && std::equal(nextTag.first, nextTag.first+sDtdStart.size(), sDtdStart.begin()) &&
			sWhitespace.find(*(nextTag.first + sDtdStart.size()) != string::npos))
		{
			nextTag.iType = node::Dtd;
			nextTag.first += sDtdStart.size();
			nextTag.second = nextTag.first;
			std::size_t nest = 1;
			while(nextTag.second != aDocumentEnd)
			{
				if (*nextTag.second == sLessThanChar[0])
					++nest;
				if (*nextTag.second == sGreaterThanChar[0])
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

	template <typename CharT>
	typename basic_xml<CharT>::string::const_iterator basic_xml<CharT>::parse(node& aNode, const tag& aStartTag, typename string::const_iterator aDocumentEnd)
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
							sNameBadDelimeter.find(*attributeName.first) != string::npos)
						{
							iError = true;
							return aDocumentEnd;
						}
						next = aStartTag.second;
						break;
					}
					token attributeEquals = next_token(sAttributeValueDelimeter, false, attributeName.second, aStartTag.second);
					if (attributeEquals.second - attributeEquals.first != 1 || *attributeEquals.first != sEqualsChar[0])
					{
						iError = true;
						return aDocumentEnd;
					}
					token attributeStart = next_token(sAttributeValueDelimeter, false, attributeEquals.second, aStartTag.second);
					if (attributeStart.first != attributeStart.second ||
						attributeStart.first == aStartTag.second ||
						sAttributeValueDelimeter.find(*attributeStart.first) == string::npos)
					{
						iError = true;
						return aDocumentEnd;
					}
					token attributeValue = next_token(*attributeStart.first == sQuoteChar[0] ? sAttributeValueInvalidOne : sAttributeValueInvalidTwo, true, attributeStart.second + 1, aStartTag.second);
					if (attributeValue.first == aStartTag.second ||
						attributeValue.second == aStartTag.second ||
						sAttributeValueDelimeter.find(*attributeValue.second) == string::npos)
					{
						iError = true;
						return aDocumentEnd;
					}
					next = attributeValue.second + 1;
					theElement.attributes().push_back(std::make_pair(string(attributeName.first, attributeName.second), parse_entities(string(attributeValue.first, attributeValue.second))));
					element::attribute& theAttribute =  theElement.attributes().back();
					strip_if(theAttribute.second);
					typename string::size_type delimPos = theAttribute.first.find_first_of(sNamespaceDelimeter);
					if (delimPos != string::npos && (delimPos == theAttribute.first.size() - 1 ||
						theAttribute.first.find_last_of(sNamespaceDelimeter) != delimPos))
					{
						iError = true;
						return aDocumentEnd;
					}
					if (theAttribute.first.find(sNamespace) == 0)
					{
						if (theAttribute.first == sNamespace)
						{
							theElement.namespaces()[sDefaultNamespace] = theAttribute.second;
							strip(theElement.namespaces()[sDefaultNamespace]);
							theElement.name().set_namespace(theElement.namespaces().find(sDefaultNamespace));
							theElement.attributes().pop_back();
						}
						if (delimPos == sNamespace.size())
						{
							theElement.namespaces()[theAttribute.first.substr(delimPos+1)] = theAttribute.second;
							theElement.attributes().pop_back();
						}
					}
				}

				if (!parse_namespaces(theElement))
				{
					iError = true;
					return aDocumentEnd;
				}

				if (*(aStartTag.second-1) == sForwardSlashChar[0]) // empty tag
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
					if (content.find_first_not_of(sWhitespace) == string::npos)
						content = string();
					if (!content.empty())
					{
						content = parse_entities(content);
						theElement.content().push_back(node_ptr(new text(static_cast<node&>(theElement), content)));
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
						if (*nextTag.first == sForwardSlashChar[0])
						{
							if (theElement.name() != string(nextTag.first+1, nextTag.second))
							{
								iError = true;
								return aDocumentEnd;
							}
							theElement.set_use_empty_element_tag(false);
							return nextTag.second+1;
						}
						theElement.content().push_back(node_ptr(new element(static_cast<node&>(theElement))));
						break;
					case node::Comment:
						theElement.content().push_back(node_ptr(new comment(static_cast<node&>(theElement))));
						break;
					case node::Declaration:
						theElement.content().push_back(node_ptr(new declaration(static_cast<node&>(theElement))));
						break;
					case node::Cdata:
						theElement.content().push_back(node_ptr(new cdata(static_cast<node&>(theElement))));
						break;
					case node::Dtd:
						theElement.content().push_back(node_ptr(new dtd(static_cast<node&>(theElement))));
						break;
					}
					next = parse(*theElement.content().back(), nextTag, aDocumentEnd);
				}
				return next;
			}
			break;
		case node::Comment:
			if (aNode.type() == node::Comment)
				static_cast<comment&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.content().push_back(node_ptr(new comment(aNode, string(aStartTag.first, aStartTag.second))));
			return aStartTag.second + aStartTag.end_skip();
		case node::Declaration:
			if (aNode.type() == node::Declaration)
				static_cast<declaration&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.content().push_back(node_ptr(new declaration(aNode, string(aStartTag.first, aStartTag.second))));
			return aStartTag.second + aStartTag.end_skip();
		case node::Cdata:
			if (aNode.type() == node::Cdata)
				static_cast<cdata&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.content().push_back(node_ptr(new cdata(aNode, string(aStartTag.first, aStartTag.second))));
			return aStartTag.second + aStartTag.end_skip();
		case node::Dtd:
			if (aNode.type() == node::Dtd)
				static_cast<dtd&>(aNode).content() = string(aStartTag.first, aStartTag.second);
			else
				aNode.content().push_back(node_ptr(new dtd(aNode, string(aStartTag.first, aStartTag.second))));
			return aStartTag.second + aStartTag.end_skip();
		default:
			iError = true;
			return aDocumentEnd;
		}
	}

	template <typename CharT>
	bool basic_xml<CharT>::parse_namespaces(element& aElement)
	{
		// todo
		return true;
	}

	template <typename CharT>
	bool basic_xml<CharT>::read(std::basic_istream<CharT>& aStream)
	{
		iError = false;

		string document;
		string line;
		bool first = true;
		while(std::getline(aStream, line))
		{
			if (document.size() != 0)
			{
				CharT chEnd = document[document.size()-1];
				if (line.size() != 0)
				{
					CharT chStart = line[0];
					if (chEnd != sGreaterThanChar[0] && chEnd != sSpaceChar[0] && chStart != sSpaceChar[0] && chStart != sLessThanChar[0])
						document += sSpaceChar;
				}
			}
			if (!first)
				document += sNewLineChar;
			first = false;
			document += line;
		}

		tag nextTag = next_tag(document.begin(), document.end());		
		while (nextTag.first != document.end())
		{
			while(nextTag.first != document.end() && nextTag.first == nextTag.second)
				nextTag = next_tag(nextTag.first, document.end());
			nextTag = next_tag(parse(iDocument, nextTag, document.end()), document.end());
		};

		return got_root();
	}

	template <typename CharT>
	bool basic_xml<CharT>::write(std::basic_ostream<CharT>& aStream)
	{
		iError = false;
		node_writer theWriter(aStream);
		write_node(theWriter, iDocument, 0);
		return true;
	}

	template <typename CharT>
	void basic_xml<CharT>::set_indent(CharT aIndentChar, std::size_t aIndentCount)
	{
		iIndentChar = aIndentChar;
		iIndentCount = aIndentCount;
	}

	template <typename CharT>
	void basic_xml<CharT>::set_strip_whitespace(bool aStripWhitespace)
	{
		iStripWhitespace = aStripWhitespace;
	}

	template <typename CharT>
	void basic_xml<CharT>::write_node(node_writer& aStream, node& aNode, std::size_t aIndent)
	{
		switch(aNode.type())
		{
		case node::Document:
			for (typename node_list::iterator i = aNode.content().begin(); i != aNode.content().end(); ++i)
			{
				write_node(aStream, **i, aIndent);
				aStream << endl;
			}
			break;
		case node::Element:
			{
				element& theElement = static_cast<element&>(aNode);

				if (&theElement != &root())
					aStream << endl;
				aStream << string(aIndent*iIndentCount, iIndentChar);
				aStream << sLessThanChar;
				aStream << theElement.name();

				if (!theElement.attributes().empty())
					for (typename attribute_list::iterator i = theElement.attributes().begin(); i != theElement.attributes().end(); ++i)
						aStream << sSpaceChar << i->first << sEqualsChar << sQuoteChar << generate_entities(i->second) << sQuoteChar;

				if (!theElement.content().empty())
				{
					aStream << sGreaterThanChar;
					for (typename node_list::iterator i = theElement.content().begin(); i != theElement.content().end(); ++i)
					{
						switch((*i)->type())
						{
						case node::Text:
							if (i != theElement.content().begin())
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
						}
						write_node(aStream, **i, aIndent+1);
					}
					if (theElement.content().back()->type() != node::Text)
						aStream << endl << string(aIndent*iIndentCount, iIndentChar);
					aStream << sLessThanChar << sForwardSlashChar << theElement.name() << sGreaterThanChar;
				}
				else if (theElement.use_empty_element_tag())
					aStream << (theElement.attributes().size() ? sEmptyTagWithAttributes : sEmptyTag);
				else
					aStream << sGreaterThanChar << sLessThanChar << sForwardSlashChar << theElement.name() << sGreaterThanChar;
			}
			break;
		case node::Text:
			aStream << generate_entities(static_cast<text&>(aNode).content());
			break;
		case node::Comment:
			aStream << sLessThanChar << sCommentStart << static_cast<comment&>(aNode).content() << sCommentEnd;
			break;
		case node::Declaration:
			aStream << sLessThanChar << sDeclarationStart << static_cast<declaration&>(aNode).content() << sDeclarationEnd;
			break;
		case node::Cdata:
			aStream << sLessThanChar << sCdataStart << static_cast<cdata&>(aNode).content() << sCdataEnd;
			break;
		case node::Dtd:
			aStream << sLessThanChar << sDtdStart << static_cast<dtd&>(aNode).content() << sDtdEnd;
			break;
		}
	}

	template <typename CharT>
	typename basic_xml<CharT>::element& basic_xml<CharT>::insert(node& aParent, typename node_list::iterator aPosition, const CharT* aName)
	{
		element& newElement = static_cast<element&>(**aParent.content().insert(aPosition,node_ptr(new element(aParent))));
		newElement.name() = aName;
		return newElement;
	}

	template <typename CharT>
	typename basic_xml<CharT>::element& basic_xml<CharT>::append(node& aParent, const CharT* aName)
	{
		typename node_list::iterator endPos = aParent.content().end();
		return insert(aParent, endPos, aName);
	}
	
	template <typename CharT>
	void basic_xml<CharT>::erase(node& aParent, typename node_list::iterator aPosition)
	{
		aParent.content().erase(aPosition);
	}
	
	template <typename CharT>
	typename basic_xml<CharT>::node_list::iterator basic_xml<CharT>::find(node& aParent, const CharT* aName)
	{
		for (typename node_list::iterator i = aParent.content().begin(); i != aParent.content().end(); ++i)
			if ((*i)->type() == node::Element && static_cast<element&>(**i).name() == aName)
				return i;
		return aParent.content().end();
	}

	template <typename CharT>
	typename basic_xml<CharT>::string basic_xml<CharT>::parse_entities(const string& aString) const
	{
		string newString = aString;
		typename string::size_type pos = 0;
		while((pos = newString.find(sAmpersandChar, pos)) != string::npos)
		{
			typename string::size_type endPos = newString.find(sSemicolonChar, pos);
			if (endPos == string::npos || (pos+1 == endPos))
			{
				iError = true;
				return aString;
			}
			bool replaced = false;
			if (newString[pos+1] == sHashChar[0])
			{
				string characterValue = newString.substr(pos+2, endPos - (pos + 2));
				string character;
				struct converter
				{
					static long string_to_integer(const typename basic_xml<char>::string& aString, std::size_t aBase)
					{
						return strtol(aString.c_str(), 0, aBase);
					}
					static long string_to_integer(const typename basic_xml<wchar_t>::string& aString, std::size_t aBase)
					{
						return wcstol(aString.c_str(), 0, aBase);
					}
				};
				if (characterValue[0] != sHexChar[0])
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
					placeholder += sAmpersandChar;
					placeholder += i->first;
					placeholder += sSemicolonChar;
					if (placeholder.size() != endPos - pos + 1)
						continue;
					if (std::equal(placeholder.begin(), placeholder.end(), newString.begin() + pos))
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

	template <typename CharT>
	typename basic_xml<CharT>::string basic_xml<CharT>::generate_entities(const string& aString) const
	{
		string newString = aString;
		for(typename entity_list::const_iterator i = iEntities.begin(); i != iEntities.end(); ++i)
		{
			string placeholder;
			placeholder += sAmpersandChar;
			placeholder += i->first;
			placeholder += sSemicolonChar;
			typename string::size_type pos = 0;
			while((pos = newString.find(i->second, pos)) != string::npos)
			{
				newString.replace(pos, i->second.size(), placeholder);
				pos += placeholder.size();
			}
		}	
		return newString;
	}

	template <typename CharT>
	void basic_xml<CharT>::strip(string& aString) const
	{
		typename string::size_type start = aString.find_first_not_of(sWhitespace);
		if (start != string::npos)
			aString.erase(0, start);
		else
			aString = string();
		typename string::size_type end = aString.find_last_not_of(sWhitespace);
		if (end != string::npos)
			aString.erase(end+1, aString.size() - (end+1));
		typename string::iterator src = aString.begin();
		typename string::iterator dest = aString.begin();
		for (; src != aString.end(); ++src)
		{
			if (sWhitespace.find(*src) == string::npos)
				*dest++ = *src;
			else if (dest != aString.begin() && *(dest-1) != sSpaceChar[0])
				*dest++ = sSpaceChar[0];
		}
		aString.erase(dest, aString.end());
	}

	template <typename CharT>
	void basic_xml<CharT>::strip_if(string& aString) const
	{
		if (iStripWhitespace)
			strip(aString);
	}

	template <typename CharT>
	typename basic_xml<CharT>::token basic_xml<CharT>::next_token(const string& aDelimeters, bool aIgnoreWhitespace, typename string::const_iterator aCurrent, typename string::const_iterator aEnd) const
	{
		if (!aIgnoreWhitespace)
		{
			while (aCurrent != aEnd)
				if (sWhitespace.find(*aCurrent) != string::npos)
					++aCurrent;
				else
					break;
		}
		token ret;
		ret.first = aCurrent;
		while (aCurrent != aEnd)
		{
			if (!aIgnoreWhitespace && sWhitespace.find(*aCurrent) != string::npos)
				break;
			if (aDelimeters.find(*aCurrent) != string::npos)
				break;
			++aCurrent;
		}
		ret.second = aCurrent;
		return ret;
	}

} // namespace lib
