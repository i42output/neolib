/*
 *  red_black_tree.hpp
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
    class red_black_tree
    {
    public:
        struct already_have_left_node : std::logic_error { already_have_left_node() : std::logic_error("neolib::red_black_tree::already_have_left_node") {} };
        struct already_have_right_node : std::logic_error { already_have_right_node() : std::logic_error("neolib::red_black_tree::already_have_right_node") {} };
    protected:
        class node
        {
        public:
            struct no_left_node : std::logic_error { no_left_node() : std::logic_error("neolib::red_black_tree::node::no_left_node") {} };
            struct no_right_node : std::logic_error { no_right_node() : std::logic_error("neolib::red_black_tree::node::no_right_node") {} };
            struct no_sibling : std::logic_error { no_sibling() : std::logic_error("neolib::red_black_tree::node::no_sibling") {} };
        public:
            enum color_e
            {
                NIL,
                BLACK,
                RED
            };

        public:
            node(color_e aColor = RED) :
                iColor{ aColor }, iParent{ aColor != NIL ? nullptr : this }, iLeft{ aColor != NIL ? nullptr : this }, iRight{ aColor != NIL ? nullptr : this }
            {
            }
            node(const node& aOther) :
                iColor{ aOther.iColor }, iParent{ aOther.iColor != NIL ? nullptr : this }, iLeft{ aOther.iColor != NIL ? nullptr : this }, iRight{ aOther.iColor != NIL ? nullptr : this }
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
            bool has_parent() const
            {
                return iParent != nullptr && !iParent->is_nil();
            }
            node* parent() const
            {
                return iParent;
            }
            void set_parent(node* aParent)
            {
                iParent = aParent;
            }
            bool has_left() const
            {
                return iLeft != nullptr && !iLeft->is_nil();
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
            bool has_right() const
            {
                return iRight != nullptr && !iRight->is_nil();
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
            bool has_sibling() const
            {
                return has_parent() && ((parent()->left() == this && parent()->has_right()) || (parent()->right() == this && parent()->has_left()));
            }
            node* sibling() const
            {
                if (has_sibling())
                    return parent()->left() == this ? parent()->right() : parent()->left();
                throw no_sibling();
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
        };

    public:
        red_black_tree() :
            iRoot{ nullptr }, iNil{ node::NIL }
        {
            set_root_node(nil_node());
        }
        ~red_black_tree()
        {
        }

    public:
        void clear()
        {
            *nil_node() = node{ node::NIL };
            set_root_node(nil_node());
        }
        node* nil_node() const
        {
            return &iNil;
        }
        node* root_node() const
        {
            return iRoot;
        }
        void set_root_node(node* aRoot)
        {
            iRoot = aRoot;
        }
        template <typename Predicate>
        void insert_node(node* aNode, Predicate aPredicate, node* aHint = nullptr)
        {
            node* z = aNode;
            node* y = nil_node();
            node* x = (aHint == nullptr ? root_node() : aHint);
            while (x != nil_node())
            {
                y = x;
                if (aPredicate(z, x))
                    x = x->left();
                else
                    x = x->right();
            }
            z->set_parent(y);
            if (y == nil_node())
                set_root_node(z);
            else
            {
                if (aPredicate(z, y))
                    y->set_left(z);
                else
                    y->set_right(z);
            }
            z->set_left(nil_node());
            z->set_right(nil_node());
            insert_fixup(z);
        }
        void delete_node(node* aNode)
        {
            node* z = aNode;
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
                y->replace(z, nil_node());
                if (root_node() == z)
                    set_root_node(y);
            }
            if (performDeleteFixup)
                delete_fixup(x);
        }
        void swap(red_black_tree& aOther)
        {
            std::swap(iRoot, aOther.iRoot);
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
        node* iRoot;
        mutable node iNil;
    };
}
