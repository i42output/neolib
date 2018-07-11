// xml.hpp
/*
 *  NoFussXML v5.3.3
 *
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
#include <cstddef>
#include <istream>
#include <ostream>
#include <list>
#include <map>
#include <utility>
#include <string>
#include <memory>
#include <exception>
#include "quick_string.hpp"
#include "memory.hpp"

#define NEOLIB_XML_USE_POOL_ALLOCATOR

namespace neolib 
{
	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_node
	{
	public:
		// types
		enum type_e { Document = 0x1, Element = 0x2, Text = 0x4, Comment = 0x8, Declaration = 0x10, Cdata = 0x20, Dtd = 0x40, All = 0xFF };
		typedef Alloc allocator_type;
		typedef basic_quick_string<CharT> string;
		typedef xml_node<CharT, allocator_type> node;
		typedef node* node_ptr;
	private:
		// types
		typedef std::list<node_ptr, typename allocator_type::template rebind<node_ptr>::other> node_list;
		/* Why std::list of pointers instead of std::vector of pointers?  std::vector is not compatible with chunk allocator
		and timings indicate performance benefit of std::list with chunk allocator compared to std::vector when parsing large (~10MB) XML files. */
	public:
		// types
		class const_iterator;
		class iterator
		{
			friend class const_iterator;
		public:
			iterator() : iNode(0), iIterator(), iFilter(node::All) {}
			iterator(node& aNode, typename node::node_list::iterator aIterator, typename node::type_e aFilter = node::All) : iNode(&aNode), iIterator(aIterator), iFilter(aFilter) {}
			iterator(const iterator& aOther) : iNode(aOther.iNode), iIterator(aOther.iIterator), iFilter(aOther.iFilter) {}
			iterator& operator=(const iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
		public:
			node& operator*() const { return static_cast<node&>(**iIterator); }
			node* operator->() const { return static_cast<node*>(&**iIterator); }
			iterator& operator++() 
			{ 
				++iIterator;
				iterator endIterator = iNode->end(iFilter);
				while (*this != endIterator && !((*iIterator)->type() & iFilter))
					++iIterator;
				return *this;
			}
			iterator& operator--()
			{
				--iIterator;
				iterator beginIterator = iNode->begin(iFilter);
				while (*this != beginIterator && !((*iIterator)->type() & iFilter))
					--iIterator;
				return *this;
			}
			iterator operator++(int) { iterator temp(*this); operator++(); return temp; }
			iterator operator--(int) { iterator temp(*this); operator--(); return temp; }
			bool operator==(const iterator& aOther) const { return iIterator == aOther.iIterator; }
			bool operator!=(const iterator& aOther) const { return !(*this == aOther); }
			typename node::node_list::iterator base() const { return iIterator; }
		private:
			node* iNode;
			typename node::node_list::iterator iIterator;
			typename node::type_e iFilter;
		};
		class const_iterator
		{
		public:
			const_iterator() : iNode(0), iIterator(), iFilter(node::All) {}
			const_iterator(const node& aNode, typename node::node_list::const_iterator aIterator, typename node::type_e aFilter = node::All) : iNode(&aNode), iIterator(aIterator), iFilter(aFilter) {}
			const_iterator(const const_iterator& aOther) : iNode(aOther.iNode), iIterator(aOther.iIterator), iFilter(aOther.iFilter) {}
			const_iterator(const iterator aIterator) : iNode(aIterator.iNode), iIterator(typename node::node_list::const_iterator(aIterator.iIterator)), iFilter(aIterator.iFilter) {}
			const_iterator& operator=(const const_iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
			const_iterator& operator=(const iterator& aOther) { iNode = aOther.iNode; iIterator = aOther.iIterator; iFilter = aOther.iFilter; return *this; }
		public:
			const node& operator*() const { return static_cast<const node&>(**iIterator); }
			const node* operator->() const { return static_cast<const node*>(&**iIterator); }
			const_iterator& operator++() 
			{ 
				++iIterator;
				const_iterator endIterator = iNode->end(iFilter);
				while (*this != endIterator && !((*iIterator)->type() & iFilter))
					++iIterator;
				return *this;
			}
			const_iterator& operator--()
			{
				--iIterator;
				const_iterator beginIterator = iNode->begin(iFilter);
				while (*this != beginIterator && !((*iIterator)->type() & iFilter))
					--iIterator;
				return *this;
			}
			const_iterator operator++(int) { const_iterator temp(*this); operator++(); return temp; }
			const_iterator operator--(int) { const_iterator temp(*this); operator--(); return temp; }
			bool operator==(const const_iterator& aOther) const { return iIterator == aOther.iIterator; }
			bool operator!=(const const_iterator& aOther) const { return !(*this == aOther); }
			typename node::node_list::const_iterator base() const { return iIterator; }
		private:
			const node* iNode;
			typename node::node_list::const_iterator iIterator;
			typename node::type_e iFilter;
		};

	public:
		// construction
		xml_node(type_e aType = Document) : iType(aType) {}
		virtual ~xml_node() { clear(); }

	public:
		// operations
		type_e type() const { return iType; }
		// access
		bool empty() const { return iContent.empty(); }
		const node& back() const { return *iContent.back(); }
		node& back() { return *iContent.back(); }
		const_iterator begin(type_e aFilter = All) const;
		const_iterator end(type_e aFilter = All) const;
		iterator begin(type_e aFilter = All);
		iterator end(type_e aFilter = All);
		const_iterator find(const string& aName) const;
		template <typename Exception>
		const_iterator find_or_throw(const string& aName) const;
		iterator find(const string& aName);
		iterator find_or_append(const string& aName);
		template <typename Exception>
		iterator find_or_throw(const string& aName);
		// modifiers
		void push_back(node_ptr aNode)
		{
			std::unique_ptr<node> newNode(aNode);
			iContent.push_back(0);
			iContent.back() = newNode.release();
		}
		iterator insert(iterator aIterator, node_ptr aNode)
		{
			std::unique_ptr<node> newNode(aNode);
			typename node_list::iterator i = iContent.insert(aIterator.base(), 0);
			*i = newNode.release();
			return iterator(*this, i);
		}
		void erase(iterator aIterator)
		{
			delete *aIterator.base();
			iContent.erase(aIterator.base());
		}
		void clear()
		{
			for (typename node_list::iterator i = iContent.begin(); i != iContent.end(); ++i)
				delete *i;
			iContent.clear();
		}

	private:
		// implementation
		const node_list& content() const { return iContent; }
		node_list& content() { return iContent; }
		xml_node(const xml_node&); // not allowed
		xml_node& operator=(const xml_node&); // not allowed

	private:
		// attributes
		type_e iType;
		node_list iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_element : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_element>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_element>::other().deallocate(static_cast<xml_element*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::allocator_type allocator_type;
		typedef typename node::string string;
		typedef std::pair<const string, string> attribute;
		typedef std::map<string, string, std::less<string>, typename allocator_type::template rebind<attribute>::other> attribute_list;
		class iterator : public node::iterator
		{
		public:
			iterator(typename node::iterator aIterator) : node::iterator(aIterator) {}
			xml_element& operator*() const { return static_cast<xml_element&>(node::iterator::operator*()); }
			xml_element* operator->() const { return static_cast<xml_element*>(node::iterator::operator->()); }
		};
		class const_iterator : public node::const_iterator
		{
		public:
			const_iterator(typename node::const_iterator aIterator) : node::const_iterator(aIterator) {}
			const_iterator(const iterator& aIterator) : node::const_iterator(aIterator) {}
			const xml_element& operator*() const { return static_cast<const xml_element&>(node::const_iterator::operator*()); }
			const xml_element* operator->() const { return static_cast<const xml_element*>(node::const_iterator::operator->()); }
		};

	public:
		// construction
		xml_element() : node(node::Element), iUseEmptyElementTag(true) {}
		xml_element(const CharT* aName) : node(node::Element), iName(aName), iUseEmptyElementTag(true) {}

	public:
		// operations
		const string& name() const { return iName; }
		using node::insert;
		typename node::iterator insert(typename node::iterator aPosition, const CharT* aName) { return node::insert(aPosition, new xml_element(aName)); }
		xml_element& append(const CharT* aName) { node::push_back(new xml_element(aName)); return static_cast<xml_element&>(node::back()); }
		const attribute_list& attributes() const { return iAttributes; }
		bool has_attribute(const string& aAttributeName) const;
		const string& attribute_value(const string& aAttributeName) const;
		const string& attribute_value(const string& aNewAttributeName, const string& aOldAttributeName) const;
		const_iterator begin() const { return const_iterator(node::begin(node::Element)); }
		const_iterator end() const { return const_iterator(node::end(node::Element)); }
		iterator begin() { return iterator(node::begin(node::Element)); }
		iterator end() { return iterator(node::end(node::Element)); }
		const string& text() const;
		bool use_empty_element_tag() const { return iUseEmptyElementTag; }
		string& name() { return iName; }
		attribute_list& attributes() { return iAttributes; }
		void set_attribute(const string& aAttributeName, const string& aAttributeValue);
		void append_text(const string& aText);
		void set_use_empty_element_tag(bool aUseEmptyElementTag) { iUseEmptyElementTag = aUseEmptyElementTag; }

	private:
		// attributes
		string iName;
		attribute_list iAttributes;
		mutable string iText;
		bool iUseEmptyElementTag;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_text : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_text>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_text>::other().deallocate(static_cast<xml_text*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::string string;

	public:
		// construction
		xml_text(const string& aContent = string()) : node(node::Text), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_comment : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_comment>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_comment>::other().deallocate(static_cast<xml_comment*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::string string;

	public:
		// construction
		xml_comment(const string& aContent = string()) : node(node::Comment), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_declaration : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_declaration>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_declaration>::other().deallocate(static_cast<xml_declaration*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::string string;

	public:
		// construction
		xml_declaration(const string& aContent = string()) : node(node::Declaration), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_cdata : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_cdata>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_cdata>::other().deallocate(static_cast<xml_cdata*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::string string;

	public:
		// construction
		xml_cdata(const string& aContent = string()) : node(node::Cdata), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class xml_dtd : public xml_node<CharT, Alloc>
	{
	public:
		// allocation
		static void* operator new(std::size_t) { return typename Alloc::template rebind<xml_dtd>::other().allocate(1); }
		static void operator delete(void* ptr) { return typename Alloc::template rebind<xml_dtd>::other().deallocate(static_cast<xml_dtd*>(ptr), 1); }

	public:
		// types
		typedef xml_node<CharT, Alloc> node;
		typedef typename node::string string;

	public:
		// construction
		xml_dtd(const string& aContent = string()) : node(node::Dtd), iContent(aContent) {}

	public:
		// operations
		const string& content() const { return iContent; }
		string& content() { return iContent; }

	private:
		// attributes
		string iContent;
	};

	template <typename CharT, typename Alloc = std::allocator<CharT> >
	class basic_xml
	{
		// types
	public:
		typedef Alloc allocator_type;
		typedef xml_node<CharT, allocator_type> node;
		typedef typename node::string string;
		typedef typename node::node_ptr node_ptr;
		typedef xml_element<CharT, allocator_type> element;
		typedef typename element::attribute attribute;
		typedef typename element::attribute_list attribute_list;
		typedef xml_text<CharT, allocator_type> text;
		typedef xml_comment<CharT, allocator_type> comment;
		typedef xml_declaration<CharT, allocator_type> declaration;
		typedef xml_cdata<CharT, allocator_type> cdata;
		typedef xml_dtd<CharT, allocator_type> dtd;
		typedef std::pair<string, string> entity;
		typedef std::list<entity, typename allocator_type::template rebind<entity>::other> entity_list;

		// exceptions
	public:
		struct error_no_root : std::runtime_error { error_no_root() : std::runtime_error("neolib::basic_xml::error_no_root") {} };
		struct failed_to_open_file : std::runtime_error { failed_to_open_file() : std::runtime_error("neolib::basic_xml::failed_to_open_file") {} };

		// construction
	public:
		basic_xml(bool aStripWhitespace = false);
		basic_xml(const std::string& aPath, bool aStripWhitespace = false);

		// operations
	public:
		void clear();
		const node& document() const;
		node& document();
		const element& root() const;
		element& root();
		bool got_root() const;
		typename node::iterator insert(node& aParent, typename node::iterator aPosition, const CharT* aName);
		element& append(node& aParent, const CharT* aName);
		void erase(node& aParent, typename node::iterator aPosition);
		typename node::const_iterator find(const node& aParent, const CharT* aName) const;
		typename node::iterator find(node& aParent, const CharT* aName);
		typename node::iterator find_or_append(node& aParent, const CharT* aName);
		bool read(std::basic_istream<CharT>& aStream);
		bool write(std::basic_ostream<CharT>& aStream);
		bool error() const { return iError; }
		void set_indent(CharT aIndentChar, std::size_t aIndentCount = 1);
		void set_strip_whitespace(bool aStripWhitespace);

		// implementation
	private:
		struct tag : std::pair<typename string::view_const_iterator, typename string::view_const_iterator>
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
					return basic_xml<CharT, Alloc>::sCommentEnd.size();
				case node::Declaration:
					return basic_xml<CharT, Alloc>::sDeclarationEnd.size();
				case node::Cdata:
					return basic_xml<CharT, Alloc>::sCdataEnd.size();
				case node::Dtd:
					return basic_xml<CharT, Alloc>::sDtdEnd.size();
				default:
					return 0;
				}
			}
		};
		struct token : std::pair<typename string::view_const_iterator, typename string::view_const_iterator>
		{
			bool iHasEntities;
			token() : iHasEntities(false) {}
		};
		tag next_tag(typename string::view_const_iterator aNext, typename string::view_const_iterator aDocumentEnd);
		typename string::view_const_iterator parse(node& aNode, const tag& aStartTag, typename string::view_const_iterator aDocumentEnd);
		struct node_writer
		{
			std::basic_ostream<CharT>& iStream;
			bool iLastWasNewLine;
			node_writer(std::basic_ostream<CharT>& aStream) : iStream(aStream), iLastWasNewLine(false) {}
			template <typename T>
			node_writer& operator<<(const T& aData);
			node_writer& operator<<(const string& aData);
		private:
			node_writer& operator=(const node_writer&); // undefined
		};
		void write_node(node_writer& aStream, const node& aNode, std::size_t aIndent) const;
		string parse_entities(const string& aString) const;
		string generate_entities(const string& aString) const;
		void strip(string& aString) const;
		void strip_if(string& aString) const;
		token next_token(const basic_character_map<CharT>& aDelimeters, bool aIgnoreWhitespace, typename string::view_const_iterator aCurrent, typename string::view_const_iterator aEnd) const;

		// attributes
	private:
		std::basic_ostream<CharT>& (&endl)(std::basic_ostream<CharT>&);
		mutable bool iError;
		node iDocument;
		entity_list iEntities;
		string iDocumentText;
		CharT iIndentChar;
		std::size_t iIndentCount;
		bool iStripWhitespace;

		static const basic_character_map<CharT> sNameDelimeter;
		static const basic_character_map<CharT> sNameBadDelimeter;
		static const basic_character_map<CharT> sAttributeValueDelimeter;
		static const basic_character_map<CharT> sAttributeValueInvalidOne;
		static const basic_character_map<CharT> sAttributeValueInvalidTwo;
		static const basic_character_map<CharT> sTagDelimeter;
		static const basic_character_map<CharT> sWhitespace;
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

	#ifndef NEOLIB_XML_USE_POOL_ALLOCATOR
	typedef basic_xml<char> xml;
	typedef basic_xml<wchar_t> wxml;
	#else // NEOLIB_XML_USE_POOL_ALLOCATOR
	typedef basic_xml<char, pool_allocator<char> > xml;
	typedef basic_xml<wchar_t, pool_allocator<char> > wxml;
	#endif
}

#include "xml.inl"
