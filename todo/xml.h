// xml.h - v3.1
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

#ifndef LIB_XML
#define LIB_XML

#include <cstddef>
#include <istream>
#include <ostream>
#include <list>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <memory>
#include <exception>

#pragma warning( disable : 4355) // 'this' : used in base member initializer list

namespace lib 
{
	template <typename CharT>
	class xml_node
	{
	public:
		// types
		enum type_e { Document, Element, Text, Comment, Declaration, Cdata, Dtd };
		typedef std::basic_string<CharT> string;
		typedef std::tr1::shared_ptr<xml_node<CharT> > node_ptr;
		typedef std::vector<node_ptr> node_list;

	public:
		// construction
		xml_node(xml_node& aParent, type_e aType) : iParent(aParent), iType(aType) {}
		xml_node() : iParent(*this), iType(Document) {}

	public:
		// operations
		const xml_node& parent() const { return iParent; }
		bool has_parent() const { return &iParent != this; }
		type_e type() const { return iType; }
		const node_list& content() const { return iContent; }
		xml_node& parent() { return iParent; }
		node_list& content() { return iContent; }
		void clear() { iContent.clear(); }

	private:
		// implementation
		xml_node(const xml_node&); // cctor not allowed for this type

	private:
		// attributes
		xml_node& iParent; 
		type_e iType;
		node_list iContent;
	};

	template <typename CharT>
	class xml_element : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;
		typedef std::map<string, string> namespace_list;
		struct symbol : public string
		{
		public:
			// construction
			symbol() : iUsingNamespace(false) {}
			symbol(const string& aName) : string(aName), iUsingNamespace(false) {}
		public:
			//operations
			using string::operator=;
			bool using_namespace() const { return iUsingNamespace; }
			typename namespace_list::const_iterator get_namespace() const { return iNamespace; }
			void set_namespace(typename namespace_list::iterator aNamespace) { iNamespace = aNamespace; iUsingNamespace = true; }
			void clear_namespace() { iNamespace = typename namespace_list::iterator(); iUsingNamespace = false; }
		private:
			// attributes
			bool iUsingNamespace;
			typename namespace_list::iterator iNamespace;
		};
		typedef symbol element_name;
		typedef symbol attribute_name;
		typedef std::pair<attribute_name, string> attribute;
		typedef std::vector<attribute> attribute_list;

		struct const_iterator
		{
			const node& iNode;
			typename node::node_list::const_iterator iIterator;
			const_iterator(const node& aNode, typename node::node_list::const_iterator& aIterator) : iNode(aNode), iIterator(aIterator) {}
			const xml_element& operator*() const { return static_cast<const xml_element&>(**iIterator); }
			const xml_element* operator->() const { return static_cast<const xml_element*>(&**iIterator); }
			const_iterator& operator++() 
			{ 
				++iIterator;
				while (iIterator != iNode.content().end() && (*iIterator)->type() != node::Element)
					++iIterator;
				return *this;
			}
			const_iterator operator++(int) { const_iterator temp(this); operator++(); return temp; }
			bool operator==(const const_iterator& aOther) const { return iIterator == aOther.iIterator; }
			bool operator!=(const const_iterator& aOther) const { return !(*this == aOther); }
		};

	public:
		// construction
		xml_element(node& aParent) : node(aParent, node::Element), iNamespace(iNamespaces.end()), iUseEmptyElementTag(true) {}

	public:
		// operations
		const namespace_list& namespaces() const { return iNamespaces; }
		bool find_namespace(const string& aName, typename namespace_list::const_iterator& aNamespace) const
		{
			namespace_list::const_iterator result = iNamespaces.find(aName);
			if (result != iNamespaces.end())
			{
				aNamespace = result;
				return true;
			}
			else if (node::iParent.get_type() != node::Element)
				return false;
			else
				return static_cast<xml_element&>(iParent).find_namespace(aName, result);
		}
		const element_name& name() const { return iName; }
		const attribute_list& attributes() const { return iAttributes; }
		const string& attribute_value(const string& aAttributeName) const;
		const_iterator begin() const;
		const_iterator end() const;
		const string& text() const;
		bool use_empty_element_tag() const { return iUseEmptyElementTag; }
		namespace_list& namespaces() { return iNamespaces; }
		element_name& name() { return iName; }
		attribute_list& attributes() { return iAttributes; }
		void set_attribute_value(const string& aAttributeName, const string& aAttributeValue);
		void append_text(const string& aText);
		void set_use_empty_element_tag(bool aUseEmptyElementTag) { iUseEmptyElementTag = aUseEmptyElementTag; }

	private:
		// attributes
		namespace_list iNamespaces;
		typename namespace_list::const_iterator iNamespace;
		element_name iName;
		attribute_list iAttributes;
		mutable string iText;
		bool iUseEmptyElementTag;
	};

	template <typename CharT>
	class xml_text : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;

	public:
		// construction
		xml_text(node& aParent, const string& aContent = string()) : node(aParent, node::Text), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT>
	class xml_comment : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;

	public:
		// construction
		xml_comment(node& aParent, const string& aContent = string()) : node(aParent, node::Comment), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT>
	class xml_declaration : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;

	public:
		// construction
		xml_declaration(node& aParent, const string& aContent = string()) : node(aParent, node::Declaration), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT>
	class xml_cdata : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;

	public:
		// construction
		xml_cdata(node& aParent, const string& aContent = string()) : node(aParent, node::Cdata), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT>
	class xml_dtd : public xml_node<CharT>
	{
	public:
		// types
		typedef xml_node<CharT> node;
		typedef typename node::string string;

	public:
		// construction
		xml_dtd(node& aParent, const string& aContent = string()) : node(aParent, node::Dtd), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT>
	class basic_xml
	{
	public:
		// types
		struct error_no_root : std::exception {};
		typedef xml_node<CharT> node;
		typedef typename node::string string;
		typedef typename node::node_list node_list;
		typedef typename node::node_ptr node_ptr;
		typedef xml_element<CharT> element;
		typedef typename element::attribute attribute;
		typedef typename element::attribute_list attribute_list;
		typedef xml_text<CharT> text;
		typedef xml_comment<CharT> comment;
		typedef xml_declaration<CharT> declaration;
		typedef xml_cdata<CharT> cdata;
		typedef xml_dtd<CharT> dtd;
		typedef std::pair<string, string> entity;
		typedef std::vector<entity> entity_list;

	public:
		// construction
		basic_xml(bool aStripWhitespace = false);

	public:
		// operations
		void clear();
		const node& document() const;
		node& document();
		const element& root() const;
		element& root();
		bool got_root() const;
		element& insert(node& aParent, typename node_list::iterator aPosition, const CharT* aName);
		element& append(node& aParent, const CharT* aName);
		void erase(node& aParent, typename node_list::iterator aPosition);
		typename node_list::iterator find(node& aParent, const CharT* aName);
		bool read(std::basic_istream<CharT>& aStream);
		bool write(std::basic_ostream<CharT>& aStream);
		bool error() const { return iError; }
		void set_indent(CharT aIndentChar, std::size_t aIndentCount = 1);
		void set_strip_whitespace(bool aStripWhitespace);

	private:
		// implementation
		struct tag : std::pair<typename string::const_iterator, typename string::const_iterator>
		{
			typename node::type_e iType;
			tag() : iType(node::Element) {}
			typename node::type_e type() const { return iType; }
			typename string::size_type end_skip() const
			{
				switch(iType)
				{
				case node::Element:
					return 1;
				case node::Comment:
					return basic_xml<CharT>::sCommentEnd.size();
				case node::Declaration:
					return basic_xml<CharT>::sDeclarationEnd.size();
				case node::Cdata:
					return basic_xml<CharT>::sCdataEnd.size();
				case node::Dtd:
					return basic_xml<CharT>::sDtdEnd.size();
				default:
					return 0;
				}
			}
		};
		typedef std::pair<typename string::const_iterator, typename string::const_iterator> token;
		tag next_tag(typename string::const_iterator aNext, typename string::const_iterator aDocumentEnd);
		typename string::const_iterator parse(node& aNode, const tag& aStartTag, typename string::const_iterator aDocumentEnd);
		bool parse_namespaces(element& aElement);
		struct node_writer
		{
			std::basic_ostream<CharT>& iStream;
			bool iLastWasNewLine;
			node_writer(std::basic_ostream<CharT>& aStream) : iStream(aStream), iLastWasNewLine(false) {}
			template <typename T>
			node_writer& operator<<(const T& aData)
			{
				if (!iLastWasNewLine)
					iStream << aData;
				iLastWasNewLine = false;
				return *this;
			}
			node_writer& operator<<(const string& aData)
			{
				iStream << aData;
				if (aData.size() && aData[aData.size() - 1] == sNewLineChar[0])
					iLastWasNewLine = true;
				else
					iLastWasNewLine = false;
				return *this;
			}
		};
		void write_node(node_writer& aStream, node& aNode, std::size_t aIndent);
		string parse_entities(const string& aString) const;
		string generate_entities(const string& aString) const;
		void strip(string& aString) const;
		void strip_if(string& aString) const;
		token next_token(const string& aDelimeters, bool aIgnoreWhitespace, typename string::const_iterator aCurrent, typename string::const_iterator aEnd) const;
	private:
		// attributes
		std::basic_ostream<CharT>& (&endl)(std::basic_ostream<CharT>&);
		mutable bool iError;
		node iDocument;
		entity_list iEntities;
		CharT iIndentChar;
		std::size_t iIndentCount;
		bool iStripWhitespace;

		enum { PredefinedEntityCount = 5 };
		struct predefined_entity { const CharT* iFirst; const CharT* iSecond; };
		static const predefined_entity sPredefinedEntities[PredefinedEntityCount];
		static const string sTabChar;
		static const string sSpaceChar;
		static const string sNewLineChar;
		static const string sLessThanChar;
		static const string sGreaterThanChar;
		static const string sEqualsChar;
		static const string sForwardSlashChar;
		static const string sAmpersandChar;
		static const string sSemicolonChar;
		static const string sHashChar;
		static const string sHexChar;
		static const string sQuoteChar;
		static const string sSingleQuoteChar;
		static const string sNameDelimeter;
		static const string sNameBadDelimeter;
		static const string sAttributeValueDelimeter;
		static const string sAttributeValueInvalidOne;
		static const string sAttributeValueInvalidTwo;
		static const string sTagDelimeter;
		static const string sElementTagStart;
		static const string sElementTagEnd;
		static const string sWhitespace;
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
		static const string sNamespace;
		static const string sDefaultNamespace;
		static const string sNamespaceDelimeter;
	};

	typedef basic_xml<char> xml;
	typedef basic_xml<wchar_t> wxml;
}

#include "xml_bits.h"

#endif // LIB_XML
