// tree.hpp
/*
 *  v1.3
 *
 *  Copyright (c) 2007-present, Leigh Johnston.
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
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <type_traits>

namespace neolib
{
	typedef std::true_type true_type;
	typedef std::false_type false_type;

	namespace detail
	{
		template <typename T, typename IsScalar>
		struct tree_scalar_trait_impl
		{ 
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef IsScalar type;
			typedef const T& parameter_type; 
		};
		template <typename T>
		struct tree_scalar_trait_impl<T, true_type>
		{ 
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef true_type type;
			typedef T parameter_type; 
		};
	}

	template <typename T>
	struct default_tree_element_deleter
	{
		void operator()(T& element) {}
	};

	template <typename T, typename A = std::allocator<T>, typename ElementDeleter = default_tree_element_deleter<T> >
	class tree
	{
		// types
	public:
		typedef T value_type;
		typedef typename A::reference reference;
		typedef typename A::pointer pointer;
		typedef typename A::const_reference const_reference;
		typedef typename A::const_pointer const_pointer;
		typedef typename A::difference_type difference_type;
		typedef typename A::size_type size_type;
		typedef A allocator_type;
		typedef ElementDeleter element_deleter;
	private:
		typedef tree<T, A, ElementDeleter> tree_type;
		typedef detail::tree_scalar_trait_impl<T, typename std::is_scalar<T>::type> scalar_trait;
	public:
		typedef typename scalar_trait::parameter_type parameter_type;
	private:
		class node;
		typedef node* node_pointer;
		typedef const node* const_node_pointer;
		class node
		{
			// construction
		public:
			node(node_pointer aParent, bool aIsHead) : iParent(aParent), iIsHead(aIsHead), iNext(this), iPrevious(this) {}
			node(const node& aOther) : iParent(aOther.iParent), iIsHead(aOther.iIsHead), iNext(this), iPrevious(this) { copy(aOther); }
			node& operator=(const node& aOther) { copy(aOther); return *this; }
			// operations
		public:
			const_node_pointer identity() const { return this; }
			node_pointer identity() { return this; }
			node_pointer parent() const { return iParent; }
			node_pointer& parent() { return iParent; }
			node_pointer previous() const { return iPrevious; }
			node_pointer& previous() { return iPrevious; }
			node_pointer next() const { return iNext; }
			node_pointer& next() { return iNext; }
			bool empty() const { return iNext == this && iPrevious == this; }
			bool has_parent() const { return iParent != nullptr; }
			bool is_head() const { return iIsHead; }
			bool is_root() const { return !has_parent() && is_head(); }
			size_type depth() const
			{
				size_type d = 0;
				for (const_node_pointer p = this; p->has_parent(); p = p->parent())
					++d;
				return d;
			}
			void copy(const node& aOther)
			{
				iParent = aOther.iParent;
				iIsHead = aOther.iIsHead;
				iPrevious = aOther.iPrevious;
				iNext = aOther.iNext;
				if (iPrevious == &aOther)
					iPrevious = this;
				if (iNext == &aOther)
					iNext = this;
				if (previous()->next() == &aOther)
					previous()->next() = this;
				if (next()->previous() == &aOther)
					next()->previous() = this;
			}
			void swap(node& aOther)
			{
				if (&aOther == this)
					return;
				std::swap(iParent, aOther.iParent);
				std::swap(iIsHead, aOther.iIsHead);
				std::swap(iNext, aOther.iNext);
				std::swap(iPrevious, aOther.iPrevious);
				if (iNext == &aOther)
					iNext = this;
				else if (iNext == this)
					iNext = &aOther;
				if (iPrevious == &aOther)
					iPrevious = this;
				else if (iPrevious == this)
					iPrevious = &aOther;
				if (aOther.iNext == this)
					aOther.iNext = &aOther;
				if (aOther.iPrevious == this)
					aOther.iPrevious = &aOther;
				iNext->iPrevious = this;
				iPrevious->iNext = this;
				aOther.iNext->iPrevious = &aOther;
				aOther.iPrevious->iNext = &aOther;
			}			// attributes
		protected:
			node_pointer iParent;
			bool iIsHead;
			node_pointer iPrevious;
			node_pointer iNext;
		};
		class node_with_value : public node
		{
			// construction
		public:
			node_with_value(node_pointer aParent, const value_type& aValue) : node(aParent, false), iValue(aValue), iHead(this, true) {}
			node_with_value(const node_with_value& aOther) : node(aOther), iValue(aOther.iValue), iHead(this, true) {}
			// operations
		public:
			void put_before(node& aNodeAfter)
			{
				if (!aNodeAfter.is_head() || !aNodeAfter.empty())
					node::previous() = aNodeAfter.previous();
				else // before empty head
					node::previous() = aNodeAfter.identity();
				node::next() = aNodeAfter.identity();
				if (!node::previous()->is_head() || node::previous()->next() == node::previous())
					node::previous()->next() = this;
				node::next()->previous() = this;
			}
			const value_type& value() const { return iValue; }
			value_type& value() { return iValue; }
			const_node_pointer head() const { return &iHead; }
			node_pointer head() { return &iHead; }
			// attributes
		private:
			value_type iValue;
			node iHead;
		};
		typedef typename allocator_type:: template rebind<node_with_value>::other node_allocator_type;
		typedef node_with_value* node_with_value_pointer;
		typedef const node_with_value* const_node_with_value_pointer;
		template <typename NodePointer, typename NodeWithValuePointer, typename Pointer, typename Reference, bool SiblingIterator>
		class iterator_base : public std::iterator<std::bidirectional_iterator_tag, value_type, difference_type, Pointer, Reference>
		{
			friend class tree;
			template <typename NodePointer2, typename NodeWithValuePointer2, typename Pointer2, typename Reference2, bool SiblingIterator2>
			friend class iterator_base;
		private:
			typedef NodePointer our_node_pointer_type;
			typedef NodeWithValuePointer our_node_with_value_pointer_type;
		protected:
			iterator_base() : iNode(nullptr) {}
			iterator_base(our_node_pointer_type aNode) : iNode(aNode) {}
			iterator_base(const iterator_base& x) : iNode(x.iNode) {}
			template <typename NodePointer2, typename NodeWithValuePointer2, typename Pointer2, typename Reference2, bool SiblingIterator2>
			iterator_base(const iterator_base<NodePointer2, NodeWithValuePointer2, Pointer2, Reference2, SiblingIterator2>& x) : iNode(const_cast<our_node_pointer_type>(x.iNode)) {}
		public:
			template <typename NodePointer2, typename NodeWithValuePointer2, typename Pointer2, typename Reference2, bool SiblingIterator2>
			bool operator==(const iterator_base<NodePointer2, NodeWithValuePointer2, Pointer2, Reference2, SiblingIterator2>& rhs) const { return iNode == rhs.iNode; }
			template <typename NodePointer2, typename NodeWithValuePointer2, typename Pointer2, typename Reference2, bool SiblingIterator2>
			bool operator!=(const iterator_base<NodePointer2, NodeWithValuePointer2, Pointer2, Reference2, SiblingIterator2>& rhs) const { return iNode != rhs.iNode; }
		protected:
			void increment() 
			{ 
				if (SiblingIterator)
					iNode = iNode->next();
				else
				{
					if (!head()->empty())
						iNode = head()->next();
					else if (!iNode->next()->is_head())
						iNode = iNode->next();
					else if (iNode->has_parent())
					{
						while (iNode == iNode->parent()->next())
						{
							iNode = iNode->parent();
							if (iNode->is_root())
								return;
						}
						iNode = iNode->parent();
						if (iNode->is_root())
							return;
						iNode = iNode->next();
						while (iNode->is_head() && !iNode->is_root())
							iNode = iNode->parent()->next();
					}
					else
						iNode = iNode->next();
				}
			}
			void decrement() 
			{ 
				iNode = iNode->previous();
				if (!SiblingIterator)
				{
					bool backTracked = false;
					if (iNode->is_head() && !iNode->is_root())
					{
						iNode = iNode->parent();
						backTracked = true;
					}
					if (!backTracked)
						while(!iNode->is_head() && !head()->empty())
							iNode = head()->previous();
				}
			}
		private:
			our_node_pointer_type ptr() const { return iNode; }
			our_node_pointer_type head() const { return iNode->is_head() ? iNode : static_cast<our_node_with_value_pointer_type>(iNode)->head(); }
		protected:
			our_node_pointer_type iNode;
		};		
	public:
		class const_iterator;
		class sibling_iterator;
		class const_sibling_iterator;
		class iterator : public iterator_base<node_pointer, node_with_value_pointer, pointer, reference, false>
		{
		private:
			friend class tree;
			friend class const_iterator;
			typedef iterator_base<node_pointer, node_with_value_pointer, pointer, reference, false> base;
		public:
			iterator() : base() {}
			iterator(const iterator& x) : base(x) {}
			iterator(const sibling_iterator& x);
		private:
			iterator(const_node_pointer x) : base(const_cast<node_pointer>(x)) {}
		public:
			iterator& operator++() { base::increment(); return *this; }
			iterator& operator--() { base::decrement(); return *this; }
			iterator operator++(int) { iterator ret(*this); operator++(); return ret; }
			iterator operator--(int) { iterator ret(*this); operator--(); return ret; }
			reference operator*() const { return static_cast<node_with_value&>(*base::iNode).value(); }
			pointer operator->() const { return &operator*(); }
		protected:
			iterator parent() const { return base::ptr()->parent(); }
			bool has_parent() const { return base::ptr()->has_parent(); }
			bool is_root() const { return !has_parent() && base::ptr()->is_head(); }
		};
		class const_iterator : public iterator_base<const_node_pointer, const_node_with_value_pointer, const_pointer, const_reference, false>
		{
		private:
			friend class tree;
			typedef iterator_base<const_node_pointer, const_node_with_value_pointer, const_pointer, const_reference, false> base;
		public:
			const_iterator() : base() {}
			const_iterator(const const_iterator& x) : base(x) {}
			const_iterator(const typename tree_type::iterator& x) : base(x) {}
			const_iterator(const sibling_iterator& x);
			const_iterator(const const_sibling_iterator& x);
		private:
			const_iterator(const_node_pointer x) : base(x) {}
		public:
			const_iterator& operator++() { base::increment(); return *this; }
			const_iterator& operator--() { base::decrement(); return *this; }
			const_iterator operator++(int) { const_iterator ret(*this); operator++(); return ret; }
			const_iterator operator--(int) { const_iterator ret(*this); operator--(); return ret; }
			const_reference operator*() const { return static_cast<const node_with_value&>(*base::ptr()).value(); }
			const_pointer operator->() const { return &operator*(); }
		protected:
			const_iterator parent() const { return base::ptr()->parent(); }
			bool has_parent() const { return base::ptr()->has_parent(); }
			bool is_root() const { return !has_parent() && base::ptr()->is_head(); }
		};
		class sibling_iterator : public iterator_base<node_pointer, node_with_value_pointer, pointer, reference, true>
		{
		private:
			friend class tree;
			friend class tree::const_iterator;
			typedef iterator_base<node_pointer, node_with_value_pointer, pointer, reference, true> base;
		public:
			sibling_iterator() : base() {}
			sibling_iterator(const typename tree_type::iterator& x) : base(x) {}
			sibling_iterator(const sibling_iterator& x) : base(x) {}
		private:
			sibling_iterator(const_node_pointer x) : base(const_cast<node_pointer>(x)) {}
		public:
			sibling_iterator& operator++() { base::increment(); return *this; }
			sibling_iterator& operator--() { base::decrement(); return *this; }
			sibling_iterator operator++(int) { sibling_iterator ret(*this); operator++(); return ret; }
			sibling_iterator operator--(int) { sibling_iterator ret(*this); operator--(); return ret; }
			reference operator*() const { return static_cast<node_with_value&>(*base::iNode).value(); }
			pointer operator->() const { return &operator*(); }
		protected:
			sibling_iterator parent() const { return base::ptr()->parent(); }
			bool has_parent() const { return base::ptr()->has_parent(); }
			bool is_root() const { return !has_parent() && base::ptr()->is_head(); }
		};
		class const_sibling_iterator : public iterator_base<const_node_pointer, const_node_with_value_pointer, const_pointer, const_reference, true>
		{
		private:
			friend class tree;
			typedef iterator_base<const_node_pointer, const_node_with_value_pointer, const_pointer, const_reference, true> base;
		public:
			const_sibling_iterator() : base() {}
			const_sibling_iterator(const const_iterator& x) : base(x) {}
			const_sibling_iterator(const typename tree_type::iterator& x) : base(x) {}
			const_sibling_iterator(const sibling_iterator& x) : base(x) {}
		private:
			const_sibling_iterator(const_node_pointer x) : base(x) {}
		public:
			const_sibling_iterator& operator++() { base::increment(); return *this; }
			const_sibling_iterator& operator--() { base::decrement(); return *this; }
			const_sibling_iterator operator++(int) { const_iterator ret(*this); operator++(); return ret; }
			const_sibling_iterator operator--(int) { const_iterator ret(*this); operator--(); return ret; }
			const_reference operator*() const { return static_cast<const node_with_value&>(*base::ptr()).value(); }
			const_pointer operator->() const { return &operator*(); }
		protected:
			const_sibling_iterator parent() const { return base::ptr()->parent(); }
			bool has_parent() const { return base::ptr()->has_parent(); }
			bool is_root() const { return !has_parent() && base::ptr()->is_head(); }
		};
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef std::reverse_iterator<sibling_iterator> reverse_sibling_iterator;
		typedef std::reverse_iterator<const_sibling_iterator> const_reverse_sibling_iterator;
		
		// construction
	public:
		tree(const element_deleter& elementDeleter = element_deleter()) : iElementDeleter(elementDeleter), iHead(nullptr, true), iSize(0) {}
		tree(const A& allocator, const element_deleter& elementDeleter = element_deleter()) : iAllocator(allocator), iElementDeleter(elementDeleter), iHead(nullptr, true), iSize(0) {}
		tree(const tree& rhs) : iAllocator(rhs.iAllocator), iElementDeleter(rhs.iElementDeleter), iHead(nullptr, true), iSize(0)
		{
			copy(rhs);
		}
		template <typename T2, typename A2, typename ElementDeleter2>
		tree(const tree<T2, A2, ElementDeleter2>& rhs) : iHead(nullptr, true), iSize(0) 
		{
			copy(rhs);
		}
		tree(size_type n, parameter_type value = value_type()) : iHead(nullptr, true), iSize(0)
		{
			insert(begin(), n, value);
		}
		template <typename InputIterator>
		tree(InputIterator first, InputIterator last) : iHead(nullptr, true), iSize(0)
		{
			insert(begin(), first, last);
		}
		~tree()
		{
			clear();
		}

		// traversals
	public:
		const_iterator root() const { return const_iterator(&iHead); }
		const_iterator begin() const { return const_iterator(iHead.next()); }
		const_iterator begin(const_iterator parent) const { return const_iterator(parent.head()->next()); }
		const_sibling_iterator sibling_begin() const { return begin(); }
		const_sibling_iterator sibling_begin(const_iterator parent) const { return const_sibling_iterator(parent.head()->next()); }
		const_iterator end() const { return const_iterator(&iHead); }
		const_iterator end(const_iterator parent) const { return const_iterator(parent.head()); }
		const_sibling_iterator sibling_end() const { return end(); }
		const_sibling_iterator sibling_end(const_iterator parent) const { return const_sibling_iterator(parent.head()); }
		const_iterator parent(const_iterator child) const { return const_iterator(child.ptr()->parent()); }
		iterator root() { return iterator(&iHead); }
		iterator begin() { return iterator(iHead.next()); }
		iterator begin(const_iterator parent) { return iterator(parent.head()->next()); }
		sibling_iterator sibling_begin() { return begin(); }
		sibling_iterator sibling_begin(const_iterator parent) { return sibling_iterator(parent.head()->next()); }
		iterator end() { return iterator(&iHead); }
		iterator end(const_iterator parent) { return iterator(parent.head()); }
		sibling_iterator sibling_end() { return end(); }
		sibling_iterator sibling_end(const_iterator parent) { return sibling_iterator(parent.head()); }
		iterator parent(const_iterator child) { return iterator(child.ptr()->parent()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		const_reverse_iterator rbegin(const_iterator parent) const { return const_reverse_iterator(end(parent)); }
		const_reverse_sibling_iterator sibling_rbegin() const { return rbegin(); }
		const_reverse_sibling_iterator sibling_rbegin(const_iterator parent) const { return const_reverse_sibling_iterator(sibling_end(parent)); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		const_reverse_iterator rend(const_iterator parent) const { return const_reverse_iterator(begin(parent)); }
		const_reverse_sibling_iterator sibling_rend() const { return rend(); }
		const_reverse_sibling_iterator sibling_rend(const_iterator parent) const { return const_reverse_sibling_iterator(sibling_begin(parent)); }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		reverse_iterator rbegin(const_iterator parent) { return reverse_iterator(end(parent)); }
		reverse_sibling_iterator sibling_rbegin(const_iterator parent) { return reverse_sibling_iterator(sibling_end(parent)); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		reverse_iterator rend(const_iterator parent) { return reverse_iterator(begin(parent)); }
		reverse_sibling_iterator sibling_rend(const_iterator parent) { return reverse_sibling_iterator(sibling_begin(parent)); }
		bool empty() const { return iSize == 0; }
		size_type size() const { return iSize; }
		size_type count_children(const_iterator parent) const { return std::distance(sibling_begin(parent), sibling_end(parent)); }
		size_type depth(const_iterator position) const { return position.ptr()->depth(); }
		bool has_children(const_iterator parent) const { return !parent.head()->empty(); }
		const void* to_node_ptr(const_iterator position) const { return position.ptr(); }
		void* to_node_ptr(iterator position) { return position.ptr(); }
		const_iterator to_iterator(const void* node_ptr) const { return const_iterator(static_cast<const_node_pointer>(node_ptr)); }
		iterator to_iterator(void* node_ptr) { return iterator(static_cast<node_pointer>(node_ptr)); }

		// element access
	public:
		const_reference front() const { return *begin(iHead.next()); }
		const_reference front(const_iterator parent) const { return *begin(parent); }
		const_reference back() const { return *--end(); }
		const_reference back(const_iterator parent) const { return *--end(parent); }
		reference front() { return *begin(iHead.next()); }
		reference front(const_iterator parent) { return *begin(parent); }
		reference back() { return *--end(); }
		reference back(const_iterator parent) { return *--end(parent); }

		// modifiers
	public:
		tree& operator=(const tree& rhs)
		{
			tree(rhs).swap(*this);
			return *this;
		}
		template<typename T2, typename A2, typename ElementDeleter2>
		tree& operator=(const tree<T2, A2, ElementDeleter2>& rhs)
		{
			tree(rhs).swap(*this);
			return *this;
		}
		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last)
		{
			clear();
			insert(begin(), first, last);
		}
		void assign(size_type n, parameter_type value)
		{
			clear();
			insert(begin(), n, value);
		}
		iterator insert(const_iterator position, parameter_type value)
		{
			return do_insert(position, value);
		}
		void insert(const_iterator position, size_type n, parameter_type value)
		{
			const_sibling_iterator at(position);
			// TODO : strong exception gaurantee
			while(n > 0)
			{
				at = insert(at, value);
				++at;
				--n;
			}
		}
		template <typename InputIterator>
		void insert(const_iterator position, InputIterator first, InputIterator last)
		{
			const_sibling_iterator at(position);
			insert_dispatch(at, first, last, typename std::is_integral<InputIterator>::type());
		}
		iterator append(parameter_type value)
		{
			return insert(end(), value);
		}
		iterator append(const_iterator parent, parameter_type value)
		{
			return insert(end(parent), value);
		}
		void push_front(parameter_type value)
		{
			insert(begin(iHead.next()), value);
		}
		void push_front(const_iterator parent, parameter_type value)
		{
			insert(begin(parent), value);
		}
		void pop_front()
		{
			erase(begin(iHead.next()));
		}
		void pop_front(const_iterator parent)
		{
			erase(begin(parent));
		}
		void push_back(parameter_type value)
		{
			insert(end(), value);
		}
		void push_back(const_iterator parent, parameter_type value)
		{
			insert(end(parent), value);
		}
		void pop_back()
		{
			erase(--end());
		}
		void pop_back(const_iterator parent)
		{
			erase(--end(parent));
		}
		void swap(tree& x)
		{
			std::swap(iAllocator, x.iAllocator);
			std::swap(iElementDeleter, x.iElementDeleter);
			iHead.swap(x.iHead);
			std::swap(iSize, x.iSize);
		}
		void swap(iterator first, iterator second)
		{
			first.ptr()->swap(*second.ptr());
		}
		iterator erase(const_iterator position) 
		{ 
			iterator toDelete(position.ptr());
			iterator ret = toDelete;
			--ret;
			destroy_node(static_cast<node_with_value_pointer>(toDelete.ptr()));
			return ++ret;
		}
		void erase(const_iterator first, const_iterator last) 
		{ 
			if (first == last)
				return;
			while(first != last)
				first = erase(first);
		}
		void remove(parameter_type value, bool multiple = true)
		{
			for (iterator i = begin(); i != end(); )
			{
				if (*i == value)
				{
					i = erase(i);
					if (!multiple)
						return;
				}
				else
					++i;
			}
		}
		template <typename Predicate>
		void remove_if(Predicate Pred, bool multiple = true)
		{
			for (iterator i = begin(); i != end(); )
			{
				if (Pred(*i))
				{
					i = erase(i);
					if (!multiple)
						return;
				}
				else
					++i;
			}
		}
		void clear()
		{
			for (node_pointer n = iHead.next(); n != &iHead;)
			{
				node_pointer nn = n->next();
				destroy_node(static_cast<node_with_value_pointer>(n));
				n = nn;
			}
			iHead.next() = iHead.previous() = &iHead;
		}
		void sort()
		{
			sort(std::less<value_type>());
		}
		template <typename Predicate>
		void sort(Predicate pred)
		{
			iHead = merge_sort(pred, iHead);
		}
		void resort() // use if it is known in advance that the tree is already mostly sorted
		{
			resort(std::less<value_type>());
		}
		template <typename Predicate>
		void resort(Predicate pred)
		{
			iHead = merge_resort(pred, iHead);
		}
	
		// implementation
	private:
		void destroy_node(node_with_value_pointer p)
		{
			for (node_pointer n = p->head()->next(); n != p->head();)
			{
				node_pointer nn = n->next();
				destroy_node(static_cast<node_with_value_pointer>(n));
				n = nn;
			}
			p->previous()->next() = p->next();
			p->next()->previous() = p->previous();
			iElementDeleter(p->value());
			iAllocator.destroy(p);
			iAllocator.deallocate(p, 1);
			--iSize;
		}
		iterator create_node(const_iterator position, const value_type& value)
		{
			node_with_value_pointer newNode = iAllocator.allocate(1);
			if (position.ptr()->parent() != nullptr)
				iAllocator.construct(newNode, node_with_value(position.ptr()->parent(), value));
			else
				iAllocator.construct(newNode, node_with_value(&iHead, value));
			newNode->put_before(const_cast<node&>(*position.ptr()));
			++iSize;
			return newNode;
		}
		void copy(const tree& rhs)
		{
			tree tmp;
			for (const_iterator i = rhs.sibling_begin(rhs.root()); i != rhs.sibling_end(rhs.root()); ++i)
				copy(&tmp.iHead, rhs, i);
			tmp.swap(*this);
		}
		template <typename T2, typename A2, typename ElementDeleter2>
		void copy(const tree<T2, A2, ElementDeleter2>& rhs)
		{
			tree tmp;
			for (typename tree<T2, A2, ElementDeleter2>::const_iterator i = rhs.sibling_begin(); i != rhs.sibling_end(); ++i)
				copy(&iHead, rhs, i);
			tmp.swap(*this);
		}
		template <typename Tree, typename TreeIterator>
		void copy(const_iterator head, const Tree& other, TreeIterator otherIterator)
		{
			const_iterator newNode = create_node(head, *otherIterator);
			for (TreeIterator i = other.sibling_begin(otherIterator); i != other.sibling_end(otherIterator); ++i)
				copy(newNode.head(), other, i);
		}
		void insert_dispatch(const_iterator position, const_pointer first, const_pointer last, false_type)
		{
			const_sibling_iterator at(position);
			do_insert(at, first, last);
		}
		void insert_dispatch(iterator position, pointer first, pointer last, false_type)
		{
			const_sibling_iterator at(position);
			do_insert(at, first, last);
		}
		template <typename InputIterator>
		void insert_dispatch(const_iterator position, InputIterator first, InputIterator last, false_type)
		{
			const_sibling_iterator at(position);
			while(first != last)
			{
				at = insert(at, *first++);
				++at;
			}
		}
		template <typename Integer>
		void insert_dispatch(const_iterator position, Integer n1, Integer n2, true_type)
		{
			const_sibling_iterator at(position);
			size_type n = static_cast<size_type>(n1);
			value_type value = static_cast<value_type>(n2);
			while(n > 0)
			{
				at = insert(at, value);
				++at;
				--n;
			}
		}
		void do_insert(const_iterator position, const_pointer first, const_pointer last)
		{
			const_sibling_iterator at(position);
			// TODO : strong exception gaurantee
			while (first != last)
				at = create_node(at, *first++);
		}
		iterator do_insert(const_iterator position, parameter_type value)
		{
			return create_node(position, value);
		}
		template <typename Predicate>
		node merge_sort(Predicate pred, node& list)
		{
			for (node_pointer next = list.next(); next != &list; next = next->next())
			{
				node_with_value& nodeWithValue = static_cast<node_with_value&>(*next);
				*nodeWithValue.head() = merge_sort(pred, *nodeWithValue.head());
			}
			return merge_sort_2(pred, list);
		}
		template <typename Predicate>
		node merge_sort_2(Predicate pred, node& list)
		{
			if (list.empty())
				return list;
			node left(list.parent(), true);
			node right(list.parent(), true);
			size_type count = 0;
			for (node_pointer next = list.next(), nextNext = next->next(); next != &list; next = nextNext, nextNext = next->next(), ++count)
			{
				node_with_value_pointer nextValue = static_cast<node_with_value_pointer>(next);
				if (count & 1)
					nextValue->put_before(right);
				else
					nextValue->put_before(left);
			}
			if (count == 1)
				return left;
			left = merge_sort_2(pred, left);
			right = merge_sort_2(pred, right);
			return merge(pred, left, right);
		}
		template <typename Predicate>
		node merge_resort(Predicate pred, node& list)
		{
			for (node_pointer next = list.next(); next != &list; next = next->next())
			{
				node_with_value& nodeWithValue = static_cast<node_with_value&>(*next);
				*nodeWithValue.head() = merge_resort(pred, *nodeWithValue.head());
			}
			return merge_resort_2(pred, list);
		}
		template <typename Predicate>
		node merge_resort_2(Predicate pred, node& list)
		{
			if (list.empty())
				return list;
			node left(list.parent(), true);
			node right(list.parent(), true);
			size_type count = 0;
			bool leftSorted = true;
			bool rightSorted = true;
			node_with_value_pointer previousLeft = static_cast<node_with_value_pointer>(list.next());
			node_with_value_pointer previousRight = static_cast<node_with_value_pointer>(list.next()->next());
			for (node_pointer next = list.next(), nextNext = next->next(); next != &list; next = nextNext, nextNext = next->next(), ++count)
			{
				node_with_value_pointer nextValue = static_cast<node_with_value_pointer>(next);
				if (count & 1)
				{
					nextValue->put_before(right);
					if (rightSorted && pred(nextValue->value(), previousRight->value()))
						rightSorted = false;
					else
						previousRight = nextValue;
				}
				else
				{
					nextValue->put_before(left);
					if (leftSorted && pred(nextValue->value(), previousLeft->value()))
						leftSorted = false;
					else
						previousLeft = nextValue;
				}
			}
			if (count == 1)
				return left;
			if (!leftSorted)
				left = merge_resort_2(pred, left);
			if (!rightSorted)
				right = merge_resort_2(pred, right);
			return merge(pred, left, right);
		}
		template <typename Predicate>
		node merge(Predicate pred, node& left, node& right)
		{
			node ret(left.parent(), true);
			for (node_pointer nextLeft = left.next(), nextRight = right.next(); nextLeft != &left || nextRight != &right;)
			{
				if (nextLeft != &left && nextRight != &right)
				{
					node_with_value& leftValue = static_cast<node_with_value&>(*nextLeft);
					node_with_value& rightValue = static_cast<node_with_value&>(*nextRight);
					if (pred(leftValue.value(), rightValue.value()))
					{
						nextLeft = nextLeft->next();
						leftValue.put_before(ret);
					}
					else
					{
						nextRight = nextRight->next();
						rightValue.put_before(ret);
					}
				}
				else if (nextLeft != &left)
				{
					node_with_value& leftValue = static_cast<node_with_value&>(*nextLeft);
					nextLeft = nextLeft->next();
					leftValue.put_before(ret);
				}
				else // nextRight != &right
				{
					node_with_value& rightValue = static_cast<node_with_value&>(*nextRight);
					nextRight = nextRight->next();
					rightValue.put_before(ret);
				}
			}
			return ret;
		}
	
		// attributes
	private:
		node_allocator_type iAllocator;
		element_deleter iElementDeleter;
		node iHead;
		size_type iSize;
	};

	template <typename T>
	struct ptr_tree_element_deleter
	{
		void operator()(T& element) { delete element; element = 0; }
	};

	template <typename T, typename A = std::allocator<T> >
	class ptr_tree : public tree<T*, typename A:: template rebind<T*>::other, ptr_tree_element_deleter<T*> >
	{
		// types
	private:
		typedef tree<T*, typename A:: template rebind<T*>::other, ptr_tree_element_deleter<T*> > base;
	public:
		typedef typename base::value_type value_type;
		typedef typename base::size_type size_type;
		typedef typename base::allocator_type allocator_type;
		typedef typename base::const_iterator const_iterator;
		typedef typename base::iterator iterator;
		// construction
	public:
		ptr_tree() : base() {}
		ptr_tree(const allocator_type& allocator) : base(allocator) {} 
		ptr_tree(const ptr_tree& rhs) : base(rhs) {}
		template <typename T2, typename A2>
		ptr_tree(const ptr_tree<T2, A2>& rhs) : base(rhs) {} 
		ptr_tree(size_type n, typename base::parameter_type value = value_type()) : base(n, value) {}
		template <typename InputIterator>
		ptr_tree(InputIterator first, InputIterator last) : base(first, last) {}
		// modifiers
	public:
		using base::insert;
		template <typename U>
		iterator insert(const_iterator position, std::unique_ptr<U> value)
		{
			iterator result = base::insert(position, value.get());
			value.release();
			return result;
		}
		using base::append;
		template <typename U>
		iterator append(std::unique_ptr<U> value)
		{
			iterator result = base::append(value.get());
			value.release();
			return result;
		}
		template <typename U>
		iterator append(const_iterator parent, std::unique_ptr<U> value)
		{
			iterator result = base::append(parent, value.get());
			value.release();
			return result;
		}
		using base::push_front;
		template <typename U>
		void push_front(std::unique_ptr<U> value)
		{
			base::push_front(value.get());
			value.release();
		}
		template <typename U>
		void push_front(const_iterator parent, std::unique_ptr<U> value)
		{
			base::push_front(parent, value.get());
			value.release();
		}
		using base::push_back;
		template <typename U>
		void push_back(std::unique_ptr<U> value)
		{
			base::push_back(value.get());
			value.release();
		}
		template <typename U>
		void push_back(const_iterator parent, std::unique_ptr<U> value)
		{
			base::push_back(parent, value.get());
			value.release();
		}
	};

	template <typename T, typename A, typename ElementDeleter>
	inline tree<T, A, ElementDeleter>::iterator::iterator(const sibling_iterator& x) : base(x) {}
	template <typename T, typename A, typename ElementDeleter>
	inline tree<T, A, ElementDeleter>::const_iterator::const_iterator(const sibling_iterator& x) : base(x) {}
	template <typename T, typename A, typename ElementDeleter>
	inline tree<T, A, ElementDeleter>::const_iterator::const_iterator(const const_sibling_iterator& x) : base(x) {}
}
