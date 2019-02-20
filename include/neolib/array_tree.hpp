/*
 *  array_tree.hpp
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

namespace neolib
{
    template <typename Alloc>
    class array_tree
    {
    public:
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        typedef Alloc allocator_type;
    protected:
        class node
        {
            friend array_tree;

        public:
            struct no_left_node : std::logic_error { no_left_node() : std::logic_error("neolib::array_tree::node::no_left_node") {} };
            struct no_right_node : std::logic_error { no_right_node() : std::logic_error("neolib::array_tree::node::no_right_node") {} };

        public:
            enum color_e
            {
                NIL,
                BLACK,
                RED
            };

        public:
            node(color_e aColor = RED) :
                iColor{ aColor }, iParent{ aColor != NIL ? nullptr : this }, iLeft{ aColor != NIL ? nullptr : this }, iRight{ aColor != NIL ? nullptr : this }, iPrevious{ nullptr }, iNext{ nullptr }, iSize { 0 }
            {
            }
            node(const node& aOther) :
                iColor{ aOther.iColor }, iParent{ aOther.iColor != NIL ? nullptr : this }, iLeft{ aOther.iColor != NIL ? nullptr : this }, iRight{ aOther.iColor != NIL ? nullptr : this }, iPrevious{ nullptr }, iNext{ nullptr }, iSize{ 0 }
            {
            }
            ~node()
            {
            }

        public:
            bool is_nil() const
            {
                return iColor == NIL;
            }    
            color_e color() const
            {
                return iColor != NIL ? iColor : BLACK;
            }
            void set_color(color_e aColor)
            {
                if (iColor != NIL)
                    iColor = aColor;
            }
            node* parent() const
            {
                return iParent;
            }
            void set_parent(node* aParent)
            {
                iParent = aParent;
            }
            node* left() const
            {
                if (iLeft != nullptr)
                    return iLeft;
                throw no_left_node();
            }
            void set_left(node* aLeft)
            {
                iLeft = aLeft;
            }
            node* right() const
            {
                if (iRight != nullptr)
                    return iRight;
                throw no_right_node();
            }
            void set_right(node* aRight)
            {
                iRight = aRight;
            }
            node* previous() const
            {
                return iPrevious;
            }
            void set_previous(node* aPrevoius)
            {
                iPrevious = aPrevoius;
            }
            node* next() const
            {
                return iNext;
            }
            void set_next(node* aNext)
            {
                iNext = aNext;
            }
            size_type size() const
            {
                return iColor != NIL ? iSize : 0;
            }
            size_type left_size() const
            {
                return left() ? left()->size() : 0;
            }
            size_type right_size() const
            {
                return right() ? right()->size() : 0;
            }
            void set_size(size_type aSize)
            {
                if (!is_nil())
                {
                    difference_type difference = aSize - iSize;
                    if (difference != 0)
                    {
                        iSize += difference;
                        if (parent() != nullptr && !parent()->is_nil())
                            parent()->set_size(parent()->size() + difference);
                    }
                }
            }
            void replace(node* aGarbage, node* aNil)
            {
                set_color(aGarbage->color());
                set_parent(aGarbage->parent());
                set_left(aGarbage->left());
                set_right(aGarbage->right());
                if (parent()->left() == aGarbage)
                    parent()->set_left(this);
                else if (parent()->right() == aGarbage)
                    parent()->set_right(this);
                if (!left()->is_nil())
                    left()->set_parent(this);
                if (!right()->is_nil())
                    right()->set_parent(this);
                aGarbage->set_parent(nullptr);
                aGarbage->set_left(nullptr);
                aGarbage->set_right(nullptr);
                if (aNil->parent() == aGarbage)
                    aNil->set_parent(this);
                if (aNil->left() == aGarbage)
                    aNil->set_left(this);
                if (aNil->right() == aGarbage)
                    aNil->set_right(this);
            }

        private:
            color_e iColor;
            node* iParent;
            node* iLeft;
            node* iRight;
            node* iPrevious;
            node* iNext;
            size_type iSize;
        };
    private:
        typedef typename allocator_type:: template rebind<node>::other node_allocator_type;

    public:
        array_tree(const Alloc& aAllocator = Alloc()) :
            iAllocator(aAllocator),
            iRoot(0),
            iFront(0),
            iBack(0),
            iNil(0)
        {
            iNil = std::allocator_traits<node_allocator_type>::allocate(iAllocator, 1);
            try
            {
                std::allocator_traits<node_allocator_type>::construct(iAllocator, iNil, node(node::NIL));
            }
            catch (...)
            {
                std::allocator_traits<node_allocator_type>::deallocate(iAllocator, iNil, 1);
                throw;
            }
            set_root_node(iNil);
        }
        ~array_tree()
        {
            std::allocator_traits<node_allocator_type>::destroy(iAllocator, iNil);
            std::allocator_traits<node_allocator_type>::deallocate(iAllocator, iNil, 1);
        }

    public:
        node* nil_node() const
        {
            return iNil;
        }
        node* root_node() const
        {
            return iRoot;
        }
        void set_root_node(node* aRoot)
        {
            iRoot = aRoot;
        }
        node* front_node() const
        {
            return iFront;
        }
        void set_front_node(node* aFront)
        {
            iFront = aFront;
        }
        node* back_node() const
        {
            return iBack;
        }
        void set_back_node(node* aBack)
        {
            iBack = aBack;
        }
        static size_type size(node* aNode)
        {
            return aNode ? aNode->size() : 0;
        }
        static size_type size_parent(node* aNode)
        {
            return aNode && aNode->parent() ? aNode->parent()->size() : 0;
        }
        static size_type size_left(node* aNode)
        {
            return aNode && aNode->left() ? aNode->left()->size() : 0;
        }
        static size_type size_right(node* aNode)
        {
            return aNode && aNode->right() ? aNode->right()->size() : 0;
        }
        node* find_node(size_type aPosition, size_type& aNodeIndex) const
        {
            node* x = root_node();
            size_type index = size_left(x);
            while (x != nil_node() && (aPosition < index || aPosition >= index + (size(x) - size_left(x) - size_right(x))))
            {
                if (aPosition < index)
                {
                    x = x->left();
                    index -= (size(x) - size_left(x));
                }
                else
                {
                    index += size(x) - size_left(x) - size_right(x) + size_left(x->right());
                    x = x->right();
                }
            }
            aNodeIndex = index;
            return x;
        }
        void insert_node(node* aNode, size_type aPosition)
        {
            node* z = aNode;
            node* y = nil_node();
            node* x = root_node();
            size_type index = size_left(x);
            size_type previousIndex = index;
            while (x != nil_node())
            {
                previousIndex = index;
                y = x;
                if (aPosition <= index)
                {
                    x = x->left();
                    index -= (size(x) - size_left(x));
                }
                else
                {
                    index += size(x) - size_left(x) - size_right(x) + size_left(x->right());
                    x = x->right();
                }
            }
            z->set_parent(y);
            if (y == nil_node())
                set_root_node(z);
            else
            {
                if (aPosition <= previousIndex)
                    y->set_left(z);
                else
                    y->set_right(z);
            }
            z->set_left(nil_node());
            z->set_right(nil_node());
            if (z->parent() != nil_node())
                z->parent()->set_size(z->parent()->size() + z->size());
            insert_fixup(z);
        }
        void delete_node(node* aNode)
        {
            node* z = aNode;
            z->set_size(z->left_size() + z->right_size());
            node *y;
            if (z->left() == nil_node() || z->right() == nil_node())
                y = z;
            else
                y = tree_successor(z);
            node* x;
            if (y->left() != nil_node())
                x = y->left();
            else
                x = y->right();
            if (y != z)
            {
                y->parent()->set_size(y->parent()->size() - y->size());
                y->parent()->set_size(y->parent()->size() + x->size());
            }
            x->set_parent(y->parent());
            if (y->parent() == nil_node())
                set_root_node(x);
            else
            {
                if (y == y->parent()->left())
                    y->parent()->set_left(x);
                else
                    y->parent()->set_right(x);
            }
            bool performDeleteFixup = (y->color() == node::BLACK);
            if (y != z)
            {
                z->parent()->set_size(z->parent()->size() - z->size());
                y->iSize = y->size() - size_left(y) - size_right(y);
                y->replace(z, nil_node());
                if (root_node() == z)
                    set_root_node(y);
                y->iSize = y->size() + size_left(y) + size_right(y);
                y->parent()->set_size(y->parent()->size() + y->size());
            }
            if (performDeleteFixup)
                delete_fixup(x);
        }
        void swap(array_tree& aOther)
        {
            std::swap(iAllocator, aOther.iAllocator);
            std::swap(iRoot, aOther.iRoot);
            std::swap(iFront, aOther.iFront);
            std::swap(iBack, aOther.iBack);
            std::swap(iNil, aOther.iNil);
        }

    private:
        void insert_fixup(node* aNode)
        {
            node* z = aNode;
            while (z->parent()->color() == node::RED)
            {
                if (z->parent() == z->parent()->parent()->left())
                {
                    node* y = z->parent()->parent()->right();
                    if (y->color() == node::RED)
                    {
                        z->parent()->set_color(node::BLACK);
                        y->set_color(node::BLACK);
                        z->parent()->parent()->set_color(node::RED);
                        z = z->parent()->parent();
                    }
                    else
                    {
                        if (z == z->parent()->right())
                        {
                            z = z->parent();
                            left_rotate(z);
                        }
                        z->parent()->set_color(node::BLACK);
                        z->parent()->parent()->set_color(node::RED);
                        right_rotate(z->parent()->parent());
                    }
                }
                else
                {
                    node* y = z->parent()->parent()->left();
                    if (y->color() == node::RED)
                    {
                        z->parent()->set_color(node::BLACK);
                        y->set_color(node::BLACK);
                        z->parent()->parent()->set_color(node::RED);
                        z = z->parent()->parent();
                    }
                    else
                    {
                        if (z == z->parent()->left())
                        {
                            z = z->parent();
                            right_rotate(z);
                        }
                        z->parent()->set_color(node::BLACK);
                        z->parent()->parent()->set_color(node::RED);
                        left_rotate(z->parent()->parent());
                    }
                }
            }
            root_node()->set_color(node::BLACK);
        }
        void left_rotate(node* aNode)
        {
            node* x = aNode;
            node* y = x->right();
            x->set_right(y->left());
            if (y->left() != nil_node())
                y->left()->set_parent(x);
            y->set_parent(x->parent());
            if (x->parent() == nil_node())
                set_root_node(y);
            else
            {
                if (x == x->parent()->left())
                    x->parent()->set_left(y);
                else
                    x->parent()->set_right(y);
            }
            y->set_left(x);
            x->set_parent(y);
            size_type previousSize = y->size();
            /* do not use set_size() as we don't want to propagate to ancestors */
            y->iSize = x->size();
            x->iSize -= previousSize;
            x->iSize += x->right()->size();
        }
        void right_rotate(node* aNode)
        {
            node* y = aNode;
            node* x = y->left();
            y->set_left(x->right());
            if (x->right() != nil_node())
                x->right()->set_parent(y);
            x->set_parent(y->parent());
            if (y->parent() == nil_node())
                set_root_node(x);
            else
            {
                if (y == y->parent()->right())
                    y->parent()->set_right(x);
                else
                    y->parent()->set_left(x);
            }
            x->set_right(y);
            y->set_parent(x);
            size_type previousSize = x->size();
            /* do not use set_size() as we don't want to propagate to ancestors */
            x->iSize = y->size();
            y->iSize -= previousSize;
            y->iSize += y->left()->size();
        }
        node* tree_minimum(node* aNode)
        {
            node* x = aNode;
            while (x->left() != nil_node())
                x = x->left();
            return x;
        }
        node* tree_successor(node* aNode)
        {
            node* x = aNode;
            if (x->right() != nil_node())
                return tree_minimum(x->right());
            node* y = x->parent();
            while (y != nil_node() && x == y->right())
            {
                x = y;
                y = y->parent();
            }
            return y;
        }
        void delete_fixup(node* aNode)
        {
            node* x = aNode;
            while (x != root_node() && x->color() == node::BLACK)
            {
                if (x == x->parent()->left())
                {
                    node* w = x->parent()->right();
                    if (w->color() == node::RED)
                    {
                        w->set_color(node::BLACK);
                        x->parent()->set_color(node::RED);
                        left_rotate(x->parent());
                        w = x->parent()->right();
                    }
                    if (w->left()->color() == node::BLACK && w->right()->color() == node::BLACK)
                    {
                        w->set_color(node::RED);
                        x = x->parent();
                    }
                    else
                    {
                        if (w->right()->color() == node::BLACK)
                        {
                            w->left()->set_color(node::BLACK);
                            w->set_color(node::RED);
                            right_rotate(w);
                            w = x->parent()->right();
                        }
                        w->set_color(x->parent()->color());
                        x->parent()->set_color(node::BLACK);
                        w->right()->set_color(node::BLACK);
                        left_rotate(x->parent());
                        x = root_node();
                    }
                }
                else
                {
                    node* w = x->parent()->left();
                    if (w->color() == node::RED)
                    {
                        w->set_color(node::BLACK);
                        x->parent()->set_color(node::RED);
                        right_rotate(x->parent());
                        w = x->parent()->left();
                    }
                    if (w->right()->color() == node::BLACK && w->left()->color() == node::BLACK)
                    {
                        w->set_color(node::RED);
                        x = x->parent();
                    }
                    else
                    {
                        if (w->left()->color() == node::BLACK)
                        {
                            w->right()->set_color(node::BLACK);
                            w->set_color(node::RED);
                            left_rotate(w);
                            w = x->parent()->left();
                        }
                        w->set_color(x->parent()->color());
                        x->parent()->set_color(node::BLACK);
                        w->left()->set_color(node::BLACK);
                        right_rotate(x->parent());
                        x = root_node();
                    }
                }
            }
            x->set_color(node::BLACK);
        }

    private:
        node_allocator_type iAllocator;
        node* iRoot;
        node* iFront;
        node* iBack;
        node* iNil;
    };
}
