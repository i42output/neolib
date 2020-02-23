// segmented_tree.h
/*
 *  Copyright (c) 2020 Leigh Johnston.
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
#include <neolib/i_iterator.hpp>
#include <neolib/segmented_array.hpp>

namespace neolib 
{
    template <typename T, size_t N = 64, typename Alloc = std::allocator<T> >
    class segmented_tree
    {
        typedef segmented_tree<T, N, Alloc> self_type;
        typedef self_type tree_type;
    public:
        typedef T value_type;
        typedef Alloc allocator_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef typename std::allocator_traits<allocator_type>::pointer pointer;
        typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;
        typedef typename std::allocator_traits<allocator_type>::difference_type difference_type;
        typedef typename std::allocator_traits<allocator_type>::size_type size_type;
    private:
        class node
        {
            friend class segmented_tree<T, N, Alloc>;
        public:
            typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<node> node_allocator_type;
            typedef segmented_array<node, N, node_allocator_type> child_list;
        private:
            struct root_place_holder {};
        public:
            node() : 
                iParent{ nullptr }, 
                iSkipChildren{ false },
                iDescendentCount{ 0 },
                iSkippedDescendentCount{ 0 },
                iContents{ root_place_holder{} }
            {}
            node(const node& other) :
                iParent{ other.iParent },
                iChildren( other.iChildren ),
                iSkipChildren{ other.iSkipChildren },
                iDescendentCount{ other.iDescendentCount },
                iSkippedDescendentCount{ other.iSkippedDescendentCount },
                iContents{ root_place_holder{} }
            {
                if (!is_root())
                {
                    iContents.root.~root_place_holder();
                    new (&iContents.value) value_type{ other.iContents.value };
                }
            }
            node(node&& other) :
                iParent{},
                iSkipChildren{ false },
                iDescendentCount{ 0 },
                iSkippedDescendentCount{ 0 },
                iContents{ root_place_holder{} }
            {
                std::swap(iParent, other.iParent);
                std::swap(iChildren, other.iChildren);
                std::swap(iSkipChildren, other.iSkipChildren);
                std::swap(iDescendentCount, other.iDescendentCount);
                std::swap(iSkippedDescendentCount, other.iSkippedDescendentCount);
                if (!is_root())
                {
                    iContents.root.~root_place_holder();
                    new (&iContents.value) value_type{ std::move(other.iContents.value) };
                }
                update_parents(*this);
            }
            node(node& parent, const value_type& value) :
                iParent{ &parent }, 
                iSkipChildren{ false },
                iDescendentCount{ 0 },
                iSkippedDescendentCount{ 0 },
                iContents { value }
            {
            }
            ~node()
            {
                if (!is_root())
                {
                    iContents.value.~value_type();
                }
            }
        public:
            node& operator=(const node& other)
            {
                iParent = other.iParent;
                iChildren = other.iChildren;
                iSkipChildren = other.iSkipChildren;
                iDescendentCount = other.iDescendentCount;
                iSkippedDescendentCount = other.iSkippedDescendentCount;
                if (!other.is_root())
                    iContents.value = other.iContents.value;
                else
                    iContents.root = other.iContents.root;
                return *this;
            }
            node& operator=(node&& other)
            {
                std::swap(iParent, other.iParent);
                std::swap(iChildren, other.iChildren);
                std::swap(iSkipChildren, other.iSkipChildren);
                std::swap(iDescendentCount, other.iDescendentCount);
                std::swap(iSkippedDescendentCount, other.iSkippedDescendentCount);
                if (!is_root())
                    std::swap(iContents.value, other.iContents.value);
                update_parents(*this);
                return *this;
            }
        private:
            bool is_root() const 
            { 
                return iParent == nullptr; 
            }
            const node& parent() const
            {
                if (!is_root())
                    return *iParent;
                return *this;
            }
            node& parent()
            {
                if (!is_root())
                    return *iParent;
                return *this;
            }
            const value_type& value() const 
            { 
                return iContents.value; 
            }
            value_type& value() 
            { 
                return iContents.value; 
            }
            const child_list& children() const 
            { 
                return iChildren; 
            }
            child_list& children() 
            { 
                return iChildren; 
            }
            bool empty() const 
            { 
                return iChildren.empty(); 
            }
            std::size_t depth() const
            {
                std::size_t result = 0;
                node const* n = this;
                while (!n->parent().is_root())
                {
                    ++result;
                    n = &n->parent();
                }
                return result;
            }
            bool children_skipped() const
            {
                return iSkipChildren;
            }
            void skip_children()
            {
                if (!iSkipChildren)
                {
                    iSkipChildren = true;
                    parent().increment_skipped_descendent_count(descendent_count() - skipped_descendent_count());
                }
            }
            void unskip_children()
            {
                if (iSkipChildren)
                {
                    iSkipChildren = false;
                    parent().decrement_skipped_descendent_count(descendent_count() - skipped_descendent_count());
                }
            }
        private:
            std::size_t descendent_count() const
            {
                return iDescendentCount;
            }
            void increment_descendent_count()
            {
                ++iDescendentCount;
                if (!is_root())
                    parent().increment_descendent_count();
            }
            void decrement_descendent_count()
            {
                --iDescendentCount;
                if (!is_root())
                    parent().decrement_descendent_count();
            }
            std::size_t skipped_descendent_count() const
            {
                return iSkippedDescendentCount;
            }
            void increment_skipped_descendent_count(std::size_t aCount)
            {
                iSkippedDescendentCount += aCount;
                if (!is_root())
                    parent().increment_skipped_descendent_count(aCount);
            }
            void decrement_skipped_descendent_count(std::size_t aCount)
            {
                iSkippedDescendentCount -= aCount;
                if (!is_root())
                    parent().decrement_skipped_descendent_count(aCount);
            }
            void update_parents(node& parent)
            {
                for (auto& child : parent.children())
                {
                    child.iParent = &parent;
                    update_parents(child);
                }
            }
        private:
            node* iParent;
            child_list iChildren;
            bool iSkipChildren;
            std::size_t iDescendentCount;
            std::size_t iSkippedDescendentCount;
            union contents
            {
                value_type value;
                root_place_holder root;
                contents(const value_type& value) : value{ value } {}
                contents(const root_place_holder& root) : root{ root } {}
                ~contents() {}
            } iContents;
        };
        typedef typename node::child_list node_child_list;
        typedef typename node::child_list::const_iterator node_child_list_const_iterator;
        typedef typename node::child_list::iterator node_child_list_iterator;
    public:
        enum class iterator_type
        {
            Normal,
            Sibling,
            Skip
        };
        template <iterator_type Type>
        class basic_const_iterator;
        template <iterator_type Type>
        class basic_iterator
        {
            typedef basic_iterator<Type> self_type;
            friend class segmented_tree<T, N, Alloc>;
        public:
            typedef std::bidirectional_iterator_tag iterator_category; // todo: make iterator random access when logarithmic complexty indexing available
            typedef typename tree_type::value_type value_type;
            typedef typename tree_type::difference_type difference_type;
            typedef typename tree_type::pointer pointer;
            typedef typename tree_type::reference reference;
        public:
            basic_iterator() : iParentNode{}, iBaseIterator{} {}
            basic_iterator(node& parentNode, node_child_list_iterator childIterator) : iParentNode{ &parentNode }, iBaseIterator{ childIterator } {}
            template <iterator_type Type2>
            basic_iterator(basic_iterator<Type2> const& other) : iParentNode{ other.iParentNode }, iBaseIterator{ other.iBaseIterator } {}
        public:
            template <iterator_type Type2>
            bool operator==(basic_iterator<Type2> const& other) const
            {
                return iParentNode == other.iParentNode && iBaseIterator == other.iBaseIterator;
            }
            template <iterator_type Type2>
            bool operator!=(basic_iterator<Type2> const& other) const
            {
                return !(*this == other);
            }
        public:
            self_type& operator++()
            {
                if constexpr (Type == iterator_type::Sibling)
                    ++iBaseIterator;
                else
                {
                    if (children().empty() || (Type == iterator_type::Skip && children_skipped()))
                    {
                        ++iBaseIterator;
                        while (iBaseIterator == parent_node().children().end() && !parent_node().is_root())
                            *this = self_type{ parent_node().parent(), std::next(parent_node().parent().children().iter(parent_node())) };
                    }
                    else
                        *this = self_type{ our_node(), children().begin() };
                }
                return *this;
            }
            self_type operator++(int) const
            {
                self_type temp = *this;
                ++temp;
                return temp;
            }
            self_type& operator--()
            {
                if constexpr (Type == iterator_type::Sibling)
                    --iBaseIterator;
                else
                {
                    if (iBaseIterator == parent_node().children().begin())
                        *this = self_type{ parent_node().parent(), parent_node().parent().children().iter(parent_node()) };
                    else
                    {
                        --iBaseIterator;
                        while (!children().empty() && !(Type == iterator_type::Skip && children_skipped()))
                            *this = self_type{ our_node(), std::prev(children().end()) };
                    }
                }
                return *this;
            }
            self_type operator--(int) const
            {
                self_type temp = *this;
                --temp;
                return temp;
            }
        public:
            reference operator*() const
            {
                return our_node().value();
            }
            pointer operator->() const
            {
                return &(**this);
            }
        public:
            basic_const_iterator<iterator_type::Sibling> cparent() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ parent_node().parent(), parent_node().parent().children().iter(parent_node()) };
            }
            basic_const_iterator<iterator_type::Sibling> parent() const
            {
                return cparent();
            }
            basic_iterator<iterator_type::Sibling> parent()
            {
                return basic_iterator<iterator_type::Sibling>{ parent_node().parent(), parent_node().parent().children().iter(parent_node()) };
            }
            basic_const_iterator<iterator_type::Sibling> cbegin() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ our_node(), children().begin() };
            }
            basic_const_iterator<iterator_type::Sibling> begin() const
            {
                return cbegin();
            }
            basic_iterator<iterator_type::Sibling> begin()
            {
                return basic_iterator<iterator_type::Sibling>{ our_node(), children().begin() };
            }
            basic_const_iterator<iterator_type::Sibling> cend() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ our_node(), children().end() };
            }
            basic_const_iterator<iterator_type::Sibling> end() const
            {
                return cend();
            }
            basic_iterator<iterator_type::Sibling> end()
            {
                return basic_iterator<iterator_type::Sibling>{ our_node(), children().end() };
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> crbegin() const
            {
                return std::make_reverse_iterator(cend());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> rbegin() const
            {
                return std::make_reverse_iterator(cend());
            }
            std::reverse_iterator<basic_iterator<iterator_type::Sibling>> rbegin()
            {
                return std::make_reverse_iterator(end());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> crend() const
            {
                return std::make_reverse_iterator(cbegin());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> rend() const
            {
                return std::make_reverse_iterator(cbegin());
            }
            std::reverse_iterator<basic_iterator<iterator_type::Sibling>> rend()
            {
                return std::make_reverse_iterator(begin());
            }
        public:
            bool parent_is_root() const
            {
                return parent_node().is_root();
            }
            std::size_t depth() const
            {
                return our_node().depth();
            }
            std::size_t descendent_count() const
            {
                return our_node().descendent_count();
            }
            bool children_skipped() const
            {
                return our_node().children_skipped();
            }
            void skip_children()
            {
                our_node().skip_children();
            }
            void unskip_children()
            {
                our_node().unskip_children();
            }
        private:
            bool is_singular() const { return iParentNode == nullptr; }
            node& parent_node() const { if (is_singular()) throw singular_iterator(); return *iParentNode; }
            node& our_node() const { if (is_singular()) throw singular_iterator(); return *base(); }
            node_child_list_iterator base() const { if (is_singular()) throw singular_iterator(); return iBaseIterator; }
            node_child_list& children() const { if (is_singular()) throw singular_iterator(); return our_node().children(); }
        private:
            node* iParentNode;
            node_child_list_iterator iBaseIterator;
        };
        template <iterator_type Type>
        class basic_const_iterator
        {
            typedef basic_const_iterator<Type> self_type;
            friend class segmented_tree<T, N, Alloc>;
        public:
            typedef std::bidirectional_iterator_tag iterator_category; // todo: make iterator random access when logarithmic complexty indexing available
            typedef typename tree_type::value_type value_type;
            typedef typename tree_type::difference_type difference_type;
            typedef typename tree_type::const_pointer pointer;
            typedef typename tree_type::const_reference reference;
        public:
            basic_const_iterator() : iParentNode{}, iBaseIterator{} {}
            basic_const_iterator(node const& parentNode, node_child_list_const_iterator childIterator) : iParentNode{ &parentNode }, iBaseIterator{ childIterator } {}
            template <iterator_type Type2>
            basic_const_iterator(basic_const_iterator<Type2> const& other) : iParentNode{ other.iParentNode }, iBaseIterator{ other.iBaseIterator } {}
            template <iterator_type Type2>
            basic_const_iterator(basic_iterator<Type2> const& other) : iParentNode{ other.iParentNode }, iBaseIterator{ other.iBaseIterator } {}
        public:
            template <iterator_type Type2>
            bool operator==(basic_const_iterator<Type2> const& other) const
            {
                return iParentNode == other.iParentNode && iBaseIterator == other.iBaseIterator;
            }
            template <iterator_type Type2>
            bool operator!=(basic_const_iterator<Type2> const& other) const
            {
                return !(*this == other);
            }
        public:
            self_type& operator++()
            {
                if constexpr (Type == iterator_type::Sibling)
                    ++iBaseIterator;
                else
                {
                    if (children().empty() || (Type == iterator_type::Skip && children_skipped()))
                    {
                        ++iBaseIterator;
                        while (iBaseIterator == parent_node().children().end() && !parent_node().is_root())
                            *this = self_type{ parent_node().parent(), std::next(parent_node().parent().children().iter(parent_node())) };
                    }
                    else
                        *this = self_type{ our_node(), children().begin() };
                }
                return *this;
            }
            self_type operator++(int) const
            {
                self_type temp = *this;
                ++temp;
                return temp;
            }
            self_type& operator--()
            {
                if constexpr (Type == iterator_type::Sibling)
                    --iBaseIterator;
                else
                {
                    if (iBaseIterator == parent_node().children().begin())
                        *this = self_type{ parent_node().parent(), parent_node().parent().children().iter(parent_node()) };
                    else
                    {
                        --iBaseIterator;
                        while (!children().empty() && !(Type == iterator_type::Skip && children_skipped()))
                            *this = self_type{ our_node(), std::prev(children().end()) };
                    }
                }
                return *this;
            }
            self_type operator--(int) const
            {
                self_type temp = *this;
                --temp;
                return temp;
            }
        public:
            reference operator*() const
            {
                return our_node().value();
            }
            pointer operator->() const
            {
                return &(**this);
            }
        public:
            basic_const_iterator<iterator_type::Sibling> cparent() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ parent_node().parent(), parent_node().parent().children().iter(parent_node()) };
            }
            basic_const_iterator<iterator_type::Sibling> parent() const
            {
                return cparent();
            }
            basic_const_iterator<iterator_type::Sibling> cbegin() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ our_node(), children().begin() };
            }
            basic_const_iterator<iterator_type::Sibling> begin() const
            {
                return cbegin();
            }
            basic_const_iterator<iterator_type::Sibling> cend() const
            {
                return basic_const_iterator<iterator_type::Sibling>{ our_node(), children().end() };
            }
            basic_const_iterator<iterator_type::Sibling> end() const
            {
                return cend();
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> crbegin() const
            {
                return std::make_reverse_iterator(cend());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> rbegin() const
            {
                return std::make_reverse_iterator(cend());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> crend() const
            {
                return std::make_reverse_iterator(cbegin());
            }
            std::reverse_iterator<basic_const_iterator<iterator_type::Sibling>> rend() const
            {
                return std::make_reverse_iterator(cbegin());
            }
        public:
            bool parent_is_root() const
            {
                return parent_node().is_root();
            }
            std::size_t depth() const
            {
                return our_node().depth();
            }
            std::size_t descendent_count() const
            {
                return our_node().descendent_count();
            }
            bool children_skipped() const
            {
                return our_node().children_skipped();
            }
        private:
            bool is_singular() const { return iParentNode == nullptr; }
            node const& parent_node() const { if (is_singular()) throw singular_iterator(); return *iParentNode; }
            node const& our_node() const { if (is_singular()) throw singular_iterator(); return *base(); }
            node_child_list_const_iterator base() const { if (is_singular()) throw singular_iterator(); return iBaseIterator; }
            node_child_list const& children() const { if (is_singular()) throw singular_iterator(); return our_node().children(); }
        private:
            node const* iParentNode;
            node_child_list_const_iterator iBaseIterator;
        };
        typedef basic_iterator<iterator_type::Normal> iterator;
        typedef basic_const_iterator<iterator_type::Normal> const_iterator;
        typedef basic_iterator<iterator_type::Sibling> sibling_iterator;
        typedef basic_const_iterator<iterator_type::Sibling> const_sibling_iterator;
        typedef basic_iterator<iterator_type::Skip> skip_iterator;
        typedef basic_const_iterator<iterator_type::Skip> const_skip_iterator;

        // construction
    public:
        segmented_tree()
        {
        }
        ~segmented_tree() 
        {
        }

        // traversals
    public:
        bool empty() const 
        { 
            return root().empty();
        }
        std::size_t size() const
        {
            return root().descendent_count();
        }
        std::size_t ksize() const
        {
            return root().descendent_count() - root().skipped_descendent_count();
        }
        const_iterator cbegin() const
        {
            return const_iterator{ root(), root().children().begin() };
        }
        const_iterator begin() const
        {
            return cbegin();
        }
        iterator begin()
        {
            return iterator{ root(), root().children().begin() };
        }
        const_iterator cend() const
        {
            return const_iterator{ root(), root().children().end() };
        }
        const_iterator end() const
        {
            return cend();
        }
        iterator end()
        {
            return iterator{ root(), root().children().end() };
        }
        const_sibling_iterator csbegin() const
        {
            return const_sibling_iterator{ root(), root().children().begin() };
        }
        const_sibling_iterator sbegin() const
        {
            return csbegin();
        }
        sibling_iterator sbegin()
        {
            return sibling_iterator{ root(), root().children().begin() };
        }
        const_sibling_iterator csend() const
        {
            return const_sibling_iterator{ root(), root().children().end() };
        }
        const_sibling_iterator send() const
        {
            return csend();
        }
        sibling_iterator send()
        {
            return sibling_iterator{ root(), root().children().end() };
        }
        const_skip_iterator ckbegin() const
        {
            return const_skip_iterator{ root(), root().children().begin() };
        }
        const_skip_iterator kbegin() const
        {
            return ckbegin();
        }
        skip_iterator kbegin()
        {
            return skip_iterator{ root(), root().children().begin() };
        }
        const_skip_iterator ckend() const
        {
            return const_skip_iterator{ root(), root().children().end() };
        }
        const_skip_iterator kend() const
        {
            return cend();
        }
        skip_iterator kend()
        {
            return skip_iterator{ root(), root().children().end() };
        }
        std::reverse_iterator<const_iterator> crbegin() const
        {
            return std::make_reverse_iterator(cend());
        }
        std::reverse_iterator<const_iterator> rbegin() const
        {
            return std::make_reverse_iterator(cend());
        }
        std::reverse_iterator<iterator> rbegin()
        {
            return std::make_reverse_iterator(end());
        }
        std::reverse_iterator<const_iterator> crend() const
        {
            return std::make_reverse_iterator(cbegin());
        }
        std::reverse_iterator<const_iterator> rend() const
        {
            return std::make_reverse_iterator(cbegin());
        }
        std::reverse_iterator<iterator> rend()
        {
            return std::make_reverse_iterator(begin());
        }
        std::reverse_iterator<const_sibling_iterator> crsbegin() const
        {
            return std::make_reverse_iterator(csend());
        }
        std::reverse_iterator<const_sibling_iterator> rsbegin() const
        {
            return std::make_reverse_iterator(csend());
        }
        std::reverse_iterator<sibling_iterator> rsbegin()
        {
            return std::make_reverse_iterator(send());
        }
        std::reverse_iterator<const_sibling_iterator> crsend() const
        {
            return std::make_reverse_iterator(csbegin());
        }
        std::reverse_iterator<const_sibling_iterator> rsend() const
        {
            return std::make_reverse_iterator(csbegin());
        }
        std::reverse_iterator<sibling_iterator> rsend()
        {
            return std::make_reverse_iterator(sbegin());
        }
        std::reverse_iterator<const_skip_iterator> crkbegin() const
        {
            return std::make_reverse_iterator(ckend());
        }
        std::reverse_iterator<const_skip_iterator> rkbegin() const
        {
            return std::make_reverse_iterator(ckend());
        }
        std::reverse_iterator<skip_iterator> rkbegin()
        {
            return std::make_reverse_iterator(kend());
        }
        std::reverse_iterator<const_skip_iterator> crkend() const
        {
            return std::make_reverse_iterator(ckbegin());
        }
        std::reverse_iterator<const_skip_iterator> rkend() const
        {
            return std::make_reverse_iterator(ckbegin());
        }
        std::reverse_iterator<skip_iterator> rkend()
        {
            return std::make_reverse_iterator(kbegin());
        }

        // modifiers
    public:
        void clear()
        {
            iRoot.~node();
            new(&iRoot) node{};
        }
        sibling_iterator insert(const_sibling_iterator position, const value_type& value)
        {
            auto& parent = const_cast<node&>(position.parent_node());
            auto& children = const_cast<node_child_list&>(parent.children());
            auto result = sibling_iterator{ parent, children.emplace_insert(position.base(), parent, value) };
            parent.increment_descendent_count();
            return result;
        }
        void push_back(const value_type& value)
        {
            push_back(root(), value);
        }
        void push_back(const_iterator pos, const value_type& value)
        {
            push_back(to_node(pos), value);
        }
        void push_front(const value_type& value)
        {
            push_front(root(), value);
        }
        void push_front(const_iterator pos, const value_type& value)
        {
            push_front(to_node(pos), value);
        }
        iterator erase(const_iterator pos)
        {
            auto mutablePos = std::next(begin(), std::distance(cbegin(), pos));
            node& parent = mutablePos.parent_node();
            auto result = iterator{ parent, parent.children().erase(mutablePos.base()) };
            parent.decrement_descendent_count();
            return result;
        }
        void sort()
        {
            sort(std::less<value_type>{});
        }
        template <typename Predicate>
        void sort(Predicate pred)
        {
            sort(root(), pred);
        }

        // implementation
    private:
        node const& root() const
        {
            return iRoot;
        }
        node& root()
        {
            return iRoot;
        }
        const node& to_node(const_iterator pos) const
        {
            if (!pos.is_singular())
                return pos.our_node();
            return root();
        }
        node& to_node(const_iterator pos)
        {
            if (!pos.is_singular())
                return std::next(begin(), std::distance(cbegin(), pos)).our_node();
            return root();
        }
        node const& first_node() const
        {
            auto n = &root();
            while (!n->empty())
                n = &n->children()[0];
            return *n;
        }
        node& first_node()
        {
            return const_cast<node&>(const_cast<const self_type&>(*this).first_node());
        }
        node const& last_node() const
        {
            auto n = &root();
            while (!n->empty())
                n = &*std::prev(n->children().end());
            return *n;
        }
        node& last_node()
        {
            return const_cast<node&>(const_cast<const self_type&>(*this).last_node());
        }
    private:
        void push_back(node& parent, const value_type& value)
        {
            parent.children().emplace_back(parent, value);
            parent.increment_descendent_count();
        }
        void push_front(node& parent, const value_type& value)
        {
            parent.children().emplace_front(parent, value);
            parent.increment_descendent_count();
        }
    private:
        template <typename Predicate>
        void sort(node& parent, Predicate pred)
        {
            std::sort(parent.children().begin(), parent.children().end(), 
                [&pred](const node& lhs, const node& rhs) 
                { 
                    auto const& lhsValue = lhs.value();
                    auto const& rhsValue = rhs.value();
                    auto result = pred(lhsValue, rhsValue);
                    return result;
                });
            if (parent.children().size() != parent.descendent_count())
                for (auto& child : parent.children())
                    sort(child, pred);
        }

        // state
    private:
        node iRoot;
    };
}
