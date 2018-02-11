// segmented_tree.h
/*
 *  Copyright (c) 2007-present, Leigh Johnston.  All Rights Reserved.
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
#include "utility.hpp"
#include "segmented_array.hpp"
#include "memory.hpp"

namespace neolib 
{
	template <typename T, size_t N, typename A = std::allocator<T> >
	class segmented_tree
	{
	public:
		typedef T value_type;
		typedef A allocator_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		class node;
		typedef node* node_pointer;
		typedef const node* const_node_pointer;
		typedef typename A:: template rebind<node_pointer>::other node_pointer_allocator;
		typedef segmented_array<node_pointer, N, node_pointer_allocator> node_children;
		typedef typename node_children::iterator iterator;
		typedef typename node_children::const_iterator const_iterator;
	public:
		class node
		{
		public:
			value_type iValue;
			node_children iChildren;
		private: 
			node() {} 
		public:
			node(const value_type& value) : iValue(value), iChildren() {}
			node(const node& rhs) : iValue(rhs.iValue), iChildren(rhs.iChildren) {}
			value_type& value() { return iValue; }
			node_children& children() { return iChildren; }
			bool empty() const { return iChildren.empty(); }
		};
		typedef typename A:: template rebind<node>::other node_allocator;

	private:
		node_pointer iRoot;
		node_allocator iAllocator;

	public:
		// construction
		segmented_tree() : iRoot(0) {}
		~segmented_tree() 
		{
			if (iRoot != 0)
				erase(iRoot); 
		}

		// traversals
		node_pointer root() { if (iRoot == 0) new_root(0); return iRoot; }
		const_node_pointer root() const { return iRoot; }
		bool empty() const { return iRoot == 0 || iRoot->iChildren.size() == 0; }
		// modifiers
		void new_root(node_pointer node)
		{
			if (iRoot != 0)
				erase(iRoot);
			if (node == 0)
				node = buy_node(value_type());
			iRoot = node;
		}
		void push_back(node_pointer parent, const value_type& value) 
		{			
			if (parent == 0)
			{
				if (iRoot == 0)
					new_root(0);
				parent = iRoot;
			}
			node_pointer new_node = buy_node(value);
			parent->iChildren.push_back(new_node);
		}
		void push_front(node_pointer parent, const value_type& value) 
		{			
			if (parent == 0)
			{
				if (iRoot == 0)
					iRoot = buy_node(value_type());
				parent = iRoot;
			}
			node_pointer new_node = buy_node(value);
			parent->iChildren.insert(parent->iChildren.begin(), new_node);
		} 
		void erase(node_pointer position, node_pointer parent = 0) 
		{ 
			if (position == 0)
				return;

			if (position == iRoot)
				iRoot = 0;

			while(!position->iChildren.empty())
				erase(static_cast<node_pointer>(*position->iChildren.begin()), position);

			iAllocator.destroy(position);
			iAllocator.deallocate(reinterpret_cast<node_allocator::pointer>(position), 1);
			if (parent != 0)
				parent->children().remove(position, false);
		}
		void erase_children(node_pointer position)
		{
			if (position == 0)
				return;
			while(!position->children().empty())
				erase(static_cast<node_pointer>(*position->children().begin()), position);
			position->children().clear();
		}
	private:
		// implementation
		node_pointer buy_node(const value_type& value)
		{
			node_pointer new_node  = reinterpret_cast<node_pointer>(iAllocator.allocate(1));
			iAllocator.construct(new_node, node(value));
			return new_node;
		}
	};
}
