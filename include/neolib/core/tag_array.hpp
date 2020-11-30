/*
*  tag_array.hpp
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

#include <neolib/neolib.hpp>
#include <memory>
#include <iterator>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/array_tree.hpp>

namespace neolib
{
    template <typename Tag, typename T, std::size_t ArraySize = 16, std::size_t VectorSize = 256, typename Alloc = std::allocator<T> >
    class tag_array : private neolib::array_tree<Alloc>
    {
        typedef neolib::array_tree<Alloc> base_type;
    public:
        typedef T value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef Alloc allocator_type;
        typedef typename std::allocator_traits<allocator_type>::pointer pointer;
        typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;
        typedef typename std::allocator_traits<allocator_type>::size_type size_type;
        typedef typename std::allocator_traits<allocator_type>::difference_type difference_type;
    private:
        class node : public base_type::node
        {
        public:
            typedef typename Tag::template rebind<node>::type tag_type;
            class data : public neolib::vecarray<T, ArraySize, VectorSize, neolib::nocheck>
            {
            public:
                data(node& aNode, const tag_type& aTag) :
                    iTag(aNode, aTag)
                {
                }
            public:
                const tag_type& tag() const
                {
                    return iTag;
                }
            private:
                tag_type iTag;
            };
            typedef data segment_type;
        public:
            node(const tag_type& aTag) :
                iSegment(*this, aTag) 
            {
            }
        public:
            const segment_type& segment() const
            {
                return iSegment;
            }
            segment_type& segment()
            {
                return iSegment;
            }
        private:
            segment_type iSegment;
        };
    public:
        typedef typename node::tag_type tag_type;
    private:
        typedef typename node::segment_type segment_type;
        typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<node> node_allocator_type;
    public:
        class iterator
        {
            friend class tag_array;
            friend class tag_array::const_iterator;

        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef tag_array::value_type value_type;
            typedef tag_array::difference_type difference_type;
            typedef tag_array::pointer pointer;
            typedef tag_array::reference reference;

        public:
            iterator() :
                iContainer(), iNode(), iContainerPosition(), iSegmentPosition()
            {
            }
            iterator(const iterator& aOther) :
                iContainer(aOther.iContainer), iNode(aOther.iNode), iContainerPosition(aOther.iContainerPosition), iSegmentPosition(aOther.iSegmentPosition)
            {
            }
            iterator& operator=(const iterator& aOther)
            {
                iContainer = aOther.iContainer;
                iNode = aOther.iNode;
                iContainerPosition = aOther.iContainerPosition;
                iSegmentPosition = aOther.iSegmentPosition;
                return *this;
            }
        private:
            iterator(tag_array& aContainer, size_type aContainerPosition) :
                iContainer(&aContainer), iNode(0), iContainerPosition(aContainerPosition), iSegmentPosition(0)
            {
                iNode = iContainer->find_node(aContainerPosition, iSegmentPosition);
                if (iNode->is_nil())
                    *this = iContainer->end();
            }
            iterator(tag_array& aContainer, node* aNode, size_type aContainerPosition, size_type aSegmentPosition) :
                iContainer(&aContainer), iNode(aNode), iContainerPosition(aContainerPosition), iSegmentPosition(aSegmentPosition)
            {
            }

        public:
            iterator& operator++()
            {
                ++iContainerPosition;
                if (++iSegmentPosition == static_cast<node*>(iNode)->segment().size())
                {
                    if (iNode != iContainer->back_node())
                    {
                        iNode = static_cast<node*>(iNode->next());
                        iSegmentPosition = 0;
                    }
                }
                return *this;
            }
            iterator& operator--()
            {
                --iContainerPosition;
                if (iSegmentPosition-- == 0)
                {
                    iNode = static_cast<node*>(iNode->previous());
                    iSegmentPosition = static_cast<node*>(iNode)->segment().size() - 1;
                }
                return *this;
            }
            iterator operator++(int) { iterator ret(*this); operator++(); return ret; }
            iterator operator--(int) { iterator ret(*this); operator--(); return ret; }
            iterator& operator+=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator-=(-aDifference);
                else if (iNode == nullptr || aDifference >= static_cast<difference_type>(segment().size() - iSegmentPosition))
                    *this = iterator(*iContainer, iContainerPosition + aDifference);
                else
                {
                    iContainerPosition += aDifference;
                    iSegmentPosition += aDifference;
                }
                return *this;
            }
            iterator& operator-=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator+=(-aDifference);
                else if (aDifference > static_cast<difference_type>(iSegmentPosition))
                    *this = iterator(*iContainer, iContainerPosition - aDifference);
                else
                {
                    iContainerPosition -= aDifference;
                    iSegmentPosition -= aDifference;
                }
                return *this;
            }
            iterator operator+(difference_type aDifference) const { iterator result(*this); result += aDifference; return result; }
            iterator operator-(difference_type aDifference) const { iterator result(*this); result -= aDifference; return result; }
            reference operator[](difference_type aDifference) const { return *((*this) + aDifference); }
            difference_type operator-(const iterator& aOther) const { return static_cast<difference_type>(iContainerPosition)-static_cast<difference_type>(aOther.iContainerPosition); }
            reference operator*() const { return segment()[iSegmentPosition]; }
            pointer operator->() const { return &operator*(); }
            bool operator==(const iterator& aOther) const { return iContainerPosition == aOther.iContainerPosition; }
            bool operator!=(const iterator& aOther) const { return iContainerPosition != aOther.iContainerPosition; }
            bool operator<(const iterator& aOther) const { return iContainerPosition < aOther.iContainerPosition; }
            bool operator<=(const iterator& aOther) const { return iContainerPosition <= aOther.iContainerPosition; }
            bool operator>(const iterator& aOther) const { return iContainerPosition > aOther.iContainerPosition; }
            bool operator>=(const iterator& aOther) const { return iContainerPosition >= aOther.iContainerPosition; }

        private:
            segment_type& segment() const { return iNode->segment(); }

        private:
            tag_array* iContainer;
            node* iNode;
            size_type iContainerPosition;
            size_type iSegmentPosition;
        };
        class const_iterator
        {
            friend class tag_array;

        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef tag_array::value_type value_type;
            typedef tag_array::difference_type difference_type;
            typedef tag_array::const_pointer pointer;
            typedef tag_array::const_reference reference;

        public:
            const_iterator() :
                iContainer(), iNode(), iContainerPosition(), iSegmentPosition()
            {
            }
            const_iterator(const const_iterator& aOther) :
                iContainer(aOther.iContainer), iNode(aOther.iNode), iContainerPosition(aOther.iContainerPosition), iSegmentPosition(aOther.iSegmentPosition)
            {
            }
            const_iterator(const typename tag_array::iterator& aOther) :
                iContainer(aOther.iContainer), iNode(aOther.iNode), iContainerPosition(aOther.iContainerPosition), iSegmentPosition(aOther.iSegmentPosition)
            {
            }
            const_iterator& operator=(const const_iterator& aOther)
            {
                iContainer = aOther.iContainer;
                iNode = aOther.iNode;
                iContainerPosition = aOther.iContainerPosition;
                iSegmentPosition = aOther.iSegmentPosition;
                return *this;
            }
            const_iterator& operator=(const typename tag_array::iterator& aOther)
            {
                iContainer = aOther.iContainer;
                iNode = aOther.iNode;
                iContainerPosition = aOther.iContainerPosition;
                iSegmentPosition = aOther.iSegmentPosition;
                return *this;
            }
        private:
            const_iterator(const tag_array& aContainer, size_type aContainerPosition) :
                iContainer(&aContainer), iNode(nullptr), iContainerPosition(aContainerPosition), iSegmentPosition(0)
            {
                iNode = iContainer->find_node(aContainerPosition, iSegmentPosition);
                if (iNode->is_nil())
                    *this = iContainer->end();
            }
            const_iterator(const tag_array& aContainer, node* aNode, size_type aContainerPosition, size_type aSegmentPosition) :
                iContainer(&aContainer), iNode(aNode), iContainerPosition(aContainerPosition), iSegmentPosition(aSegmentPosition)
            {
            }

        public:
            const_iterator& operator++()
            {
                ++iContainerPosition;
                if (++iSegmentPosition == static_cast<node*>(iNode)->segment().size())
                {
                    if (iNode != iContainer->back_node())
                    {
                        iNode = static_cast<node*>(iNode->next());
                        iSegmentPosition = 0;
                    }
                }
                return *this;
            }
            const_iterator& operator--()
            {
                --iContainerPosition;
                if (iSegmentPosition-- == 0)
                {
                    iNode = static_cast<node*>(iNode->previous());
                    iSegmentPosition = static_cast<node*>(iNode)->segment().size() - 1;
                }
                return *this;
            }
            const_iterator operator++(int) { const_iterator ret(*this); operator++(); return ret; }
            const_iterator operator--(int) { const_iterator ret(*this); operator--(); return ret; }
            const_iterator& operator+=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator-=(-aDifference);
                else if (iNode == nullptr || aDifference >= static_cast<difference_type>(segment().size() - iSegmentPosition))
                    *this = const_iterator(*iContainer, iContainerPosition + aDifference);
                else
                {
                    iContainerPosition += aDifference;
                    iSegmentPosition += aDifference;
                }
                return *this;
            }
            const_iterator& operator-=(difference_type aDifference)
            {
                if (aDifference < 0)
                    return operator+=(-aDifference);
                else if (aDifference > static_cast<difference_type>(iSegmentPosition))
                    *this = const_iterator(*iContainer, iContainerPosition - aDifference);
                else
                {
                    iContainerPosition -= aDifference;
                    iSegmentPosition -= aDifference;
                }
                return *this;
            }
            const_iterator operator+(difference_type aDifference) const { const_iterator result(*this); result += aDifference; return result; }
            const_iterator operator-(difference_type aDifference) const { const_iterator result(*this); result -= aDifference; return result; }
            const_reference operator[](difference_type aDifference) const { return *((*this) + aDifference); }
            difference_type operator-(const const_iterator& aOther) const { return static_cast<difference_type>(iContainerPosition)-static_cast<difference_type>(aOther.iContainerPosition); }
            const_reference operator*() const { return segment()[iSegmentPosition]; }
            const_pointer operator->() const { return &operator*(); }
            bool operator==(const const_iterator& aOther) const { return iContainerPosition == aOther.iContainerPosition; }
            bool operator!=(const const_iterator& aOther) const { return iContainerPosition != aOther.iContainerPosition; }
            bool operator<(const const_iterator& aOther) const { return iContainerPosition < aOther.iContainerPosition; }
            bool operator<=(const const_iterator& aOther) const { return iContainerPosition <= aOther.iContainerPosition; }
            bool operator>(const const_iterator& aOther) const { return iContainerPosition > aOther.iContainerPosition; }
            bool operator>=(const const_iterator& aOther) const { return iContainerPosition >= aOther.iContainerPosition; }

        private:
            segment_type& segment() const { return iNode->segment(); }

        private:
            const tag_array* iContainer;
            node* iNode;
            size_type iContainerPosition;
            size_type iSegmentPosition;
        };
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    public:
        tag_array(const Alloc& aAllocator = Alloc()) :
            iAllocator(aAllocator), iSize(0)
        {
        }
        tag_array(const tag_type& aTag, const size_type aCount, const value_type& aValue, const Alloc& aAllocator = Alloc()) :
            iAllocator(aAllocator), iSize(0)
        {
            insert(aTag, begin(), aCount, aValue);
        }
        template <typename InputIterator>
        tag_array(const tag_type& aTag, InputIterator aFirst, InputIterator aLast, const Alloc& aAllocator = Alloc()) :
            iAllocator(aAllocator), iSize(0)
        {
            insert(aTag, begin(), aFirst, aLast);
        }
        ~tag_array()
        {
            erase(begin(), end());
        }
    public:
        tag_array& operator=(const tag_array& aRhs)
        {
            clear();
            insert(begin(), aRhs.begin(), aRhs.end());
            return *this;
        }
    public:
        bool operator==(const tag_array& aRhs) const
        {
            return size() == aRhs.size() && std::equal(begin(), end(), aRhs.begin(), aRhs.end());
        }
        bool operator!=(const tag_array& aRhs) const
        {
            return !(*this == aRhs);
        }
    public:
        size_type size() const
        {
            return iSize;
        }
        bool empty() const
        {
            return iSize == 0;
        }
        const_iterator begin() const
        {
            return const_iterator(*this, static_cast<node*>(base_type::front_node()), 0, 0);
        }
        const_iterator end() const
        {
            return const_iterator(*this, static_cast<node*>(base_type::back_node()), iSize, base_type::back_node() ? static_cast<node*>(base_type::back_node())->segment().size() : 0);
        }
        iterator begin()
        {
            return iterator(*this, static_cast<node*>(base_type::front_node()), 0, 0);
        }
        iterator end()
        {
            return iterator(*this, static_cast<node*>(base_type::back_node()), iSize, base_type::back_node() ? static_cast<node*>(base_type::back_node())->segment().size() : 0);
        }
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }
        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }
        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }
        const_reference front() const
        {
            return *begin();
        }
        reference front()
        {
            return *begin();
        }
        const_reference back() const
        {
            return *--end();
        }
        reference back()
        {
            return *--end();
        }
        iterator insert(const tag_type& aTag, const_iterator aPosition, const value_type& aValue)
        {
            return insert(aTag, aPosition, 1, aValue);
        }
        const_reference operator[](size_type aIndex) const
        {
            return *(begin() + aIndex);
        }
        reference operator[](size_type aIndex)
        {
            return *(begin() + aIndex);
        }
        iterator insert(const_iterator aPosition, const_iterator aFirst, const_iterator aLast)
        {
            auto pos = aPosition.iContainerPosition;
            while (aFirst != aLast)
            {
                aPosition = insert(aFirst.segment().tag(), aPosition, 1, *aFirst++);
                ++aPosition;
            }
            return iterator(*this, pos);
        }
        template <class InputIterator>
        typename std::enable_if<!std::is_integral<InputIterator>::value, iterator>::type
        insert(const tag_type& aTag, const_iterator aPosition, InputIterator aFirst, InputIterator aLast)
        {
            return do_insert(aTag, aPosition, aFirst, aLast);
        }
        iterator insert(const tag_type& aTag, const_iterator aPosition, size_type aCount, const value_type& aValue)
        {
            auto pos = aPosition.iContainerPosition;
            while (aCount > 0)
            {
                aPosition = insert(aTag, aPosition, &aValue, &aValue+1);
                ++aPosition;
                --aCount;
            }
            return iterator(*this, pos);
        }
        void clear()
        {
            erase(begin(), end());
        }
        void push_front(const tag_type& aTag, const value_type& aValue)
        {
            insert(aTag, begin(), aValue);
        }
        void push_back(const tag_type& aTag, const value_type& aValue)
        {
            insert(aTag, end(), aValue);
        }
        iterator erase(const_iterator aPosition)
        {
            erase(aPosition, aPosition + 1);
            return iterator(*this, aPosition.iContainerPosition);
        }
        iterator erase(const_iterator aFirst, const_iterator aLast)
        {
            if (aFirst == aLast)
                return iterator(*this, aFirst.iNode, aFirst.iContainerPosition, aFirst.iSegmentPosition);
            segment_type& segmentFirst = aFirst.iNode->segment();
            if (aFirst.iNode == aLast.iNode)
            {
                segmentFirst.erase(segmentFirst.begin() + aFirst.iSegmentPosition, segmentFirst.begin() + aLast.iSegmentPosition);
                iSize -= (aLast.iSegmentPosition - aFirst.iSegmentPosition);
                aFirst.iNode->set_size(aFirst.iNode->size() - (aLast.iSegmentPosition - aFirst.iSegmentPosition));
                if (segmentFirst.empty())
                    free_node(aFirst.iNode);
            }
            else
            {
                segment_type& segmentLast = aLast.iNode->segment();
                for (node* inbetweenNode = static_cast<node*>(aFirst.iNode->next()); inbetweenNode != aLast.iNode;)
                {
                    node* next = static_cast<node*>(inbetweenNode->next());
                    size_type inbetweenRemoved = inbetweenNode->segment().size();
                    free_node(inbetweenNode);
                    iSize -= inbetweenRemoved;
                    inbetweenNode = next;
                }
                size_type firstRemoved = segmentFirst.size() - aFirst.iSegmentPosition;
                size_type secondRemoved = aLast.iSegmentPosition;
                segmentFirst.erase(segmentFirst.begin() + aFirst.iSegmentPosition, segmentFirst.end());
                segmentLast.erase(segmentLast.begin(), segmentLast.begin() + aLast.iSegmentPosition);
                if (segmentFirst.empty())
                    free_node(aFirst.iNode);
                else
                    aFirst.iNode->set_size(aFirst.iNode->size() - firstRemoved);
                iSize -= firstRemoved;
                if (segmentLast.empty())
                    free_node(aLast.iNode);
                else
                    aLast.iNode->set_size(aLast.iNode->size() - secondRemoved);
                iSize -= secondRemoved;
            }
            return iterator(*this, aFirst.iContainerPosition);
        }
        void pop_front()
        {
            erase(begin());
        }
        void pop_back()
        {
            erase(--end());
        }
        void swap(tag_array& aOther)
        {
            base_type::swap(aOther);
            std::swap(iAllocator, aOther.iAllocator);
            std::swap(iSize, aOther.iSize);
        }
        const tag_type& tag(const_iterator aWhere) const
        {
            return aWhere.segment().tag();
        }

    private:
        template <class InputIterator>
        typename std::enable_if<!std::is_same<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value, iterator>::type
        do_insert(const tag_type& aTag, const_iterator aPosition, InputIterator aFirst, InputIterator aLast)
        {
            size_type count = std::distance(aFirst, aLast);
            if (count == 0)
                return iterator(*this, aPosition.iNode, aPosition.iContainerPosition, aPosition.iSegmentPosition);
            if (aPosition.iNode != nullptr && static_cast<node*>(aPosition.iNode)->segment().tag() != aTag &&
                aPosition.iNode->previous() != nullptr && static_cast<node*>(aPosition.iNode->previous())->segment().tag() == aTag &&
                static_cast<node*>(aPosition.iNode->previous())->segment().available() >= count &&
                aPosition.iSegmentPosition == 0)
            {
                aPosition.iNode = static_cast<node*>(aPosition.iNode->previous());
                aPosition.iSegmentPosition = aPosition.iNode->segment().size();
            }
            node* before = aPosition.iNode;
            node* after = aPosition.iNode ? static_cast<node*>(aPosition.iNode->next()) : nullptr;
            node* lastNode = aPosition.iNode;
            if (aPosition.iNode != nullptr && count <= static_cast<node*>(aPosition.iNode)->segment().available() && static_cast<node*>(aPosition.iNode)->segment().tag() == aTag)
            {
                segment_type& segment = static_cast<node*>(aPosition.iNode)->segment();
                segment.insert(segment.begin() + aPosition.iSegmentPosition, aFirst, aLast);
                iSize += count;
                aPosition.iNode->set_size(aPosition.iNode->size() + count);
            }
            else
            {
                lastNode = allocate_space(aTag, aPosition, count);
                if (aPosition.iNode == nullptr)
                    aPosition = begin();
                segment_type& segment = aPosition.iNode->segment();
                typename segment_type::const_iterator tailEnd = segment.end();
                typename segment_type::const_iterator tailStart = tailEnd - (segment.size() - aPosition.iSegmentPosition);
                if (tailStart != tailEnd)
                {
                    lastNode->segment().insert(lastNode->segment().begin(), tailStart, tailEnd);
                    lastNode->set_size(lastNode->size() + (tailEnd - tailStart));
                    aPosition.iNode->set_size(aPosition.iNode->size() - (tailEnd - tailStart));
                    segment.erase(tailStart, tailEnd);
                }
                for (node* nextNode = segment.tag() == aTag ? aPosition.iNode : static_cast<node*>(aPosition.iNode->next()); count > 0 && nextNode != lastNode; nextNode = static_cast<node*>(nextNode->next()))
                {
                    size_type addCount = std::min(count, nextNode->segment().available());
                    if (addCount != 0)
                    {
                        InputIterator stop = aFirst;
                        std::advance(stop, addCount);
                        nextNode->segment().insert(nextNode->segment().begin() + (nextNode == aPosition.iNode ? aPosition.iSegmentPosition : 0), aFirst, stop);
                        aFirst = stop;
                        iSize += addCount;
                        count -= addCount;
                        nextNode->set_size(nextNode->size() + addCount);
                    }
                }
                if (count > 0)
                {
                    InputIterator stop = aFirst;
                    std::advance(stop, count);
                    lastNode->segment().insert(lastNode->segment().begin() + (lastNode == aPosition.iNode ? aPosition.iSegmentPosition : 0), aFirst, stop);
                    lastNode->set_size(lastNode->size() + count);
                    iSize += count;
                }
            }
            size_type index = aPosition.iContainerPosition - aPosition.iSegmentPosition;
            for (node* newNode = aPosition.iNode;; newNode = static_cast<node*>(newNode->next()))
            {
                if (newNode != before && newNode != after)
                    base_type::insert_node(newNode, index);
                index += newNode->segment().size();
                if (newNode == lastNode)
                    break;
            }
            if (aPosition.iNode->segment().empty())
            {
                auto insertionPoint = iterator(*this, static_cast<node*>(aPosition.iNode->next()), aPosition.iContainerPosition, 0);
                free_node(aPosition.iNode);
                return insertionPoint;
            }
            else if (aPosition.iSegmentPosition != aPosition.iNode->segment().size()) // was not end
                return iterator(*this, aPosition.iNode, aPosition.iContainerPosition, aPosition.iSegmentPosition);
            else
                return iterator(*this, static_cast<node*>(aPosition.iNode->next()), aPosition.iContainerPosition, 0);
        }
        template <class InputIterator>
        typename std::enable_if<std::is_same<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value, iterator>::type
        do_insert(const tag_type& aTag, const_iterator aPosition, InputIterator aFirst, InputIterator aLast)
        {
            auto pos = aPosition.iContainerPosition;
            while (aFirst != aLast)
            {
                aPosition = insert(aTag, aPosition, 1, *aFirst++);
                ++aPosition;
            }
            return iterator(*this, pos);
        }
        node* find_node(size_type aContainerPosition, size_type& aSegmentPosition) const
        {
            size_type nodeIndex = 0;
            node* result = static_cast<node*>(base_type::find_node(aContainerPosition, nodeIndex));
            aSegmentPosition = aContainerPosition - nodeIndex;
            return result;
        }
        node* allocate_space(const tag_type& aTag, const_iterator aPosition, size_type aCount)
        {
            if (aCount == 0)
                return aPosition.iNode;
            if (aPosition.iNode && aPosition.iNode->segment().tag() == aTag)
                aCount -= std::min(aCount, (aPosition.iNode->segment().available()));
            if (aCount == 0)
                return aPosition.iNode;
            node* lastNode = nullptr;
            if (aCount > 0 && aPosition.iNode && aPosition.iNode->next() != nullptr && aCount <= static_cast<node*>(aPosition.iNode->next())->segment().available() && static_cast<node*>(aPosition.iNode->next())->segment().tag() == aTag)
            {
                if (aPosition.iNode->segment().tag() == aTag || aPosition.iSegmentPosition == aPosition.iNode->segment().size())
                {
                    lastNode = static_cast<node*>(aPosition.iNode->next());
                    aCount -= std::min(aCount, lastNode->segment().available());
                }
            }
            node* nextNode = aPosition.iNode;
            while (aCount > 0)
                aCount -= std::min(aCount, (nextNode = allocate_node(aTag, nextNode))->segment().available());
            if (aPosition.iNode == nullptr)
                aPosition = begin();
            segment_type& segment = aPosition.iNode->segment();
            if (aPosition.iSegmentPosition < segment.size() && (nextNode->segment().available() < segment.size() - aPosition.iSegmentPosition || segment.tag() != aTag))
                lastNode = allocate_node(segment.tag(), nextNode);
            return lastNode ? lastNode : nextNode;
        }
        node* allocate_node(const tag_type& aTag, node* aAfter)
        {
            node* newNode = iAllocator.allocate(1);
            iAllocator.construct(newNode, node(aTag));
            if (aAfter == nullptr)
            {
                base_type::set_front_node(newNode);
                base_type::set_back_node(newNode);
            }
            else
            {
                newNode->set_previous(aAfter);
                if (aAfter->next() != nullptr)
                {
                    newNode->set_next(aAfter->next());
                    aAfter->next()->set_previous(newNode);
                }
                aAfter->set_next(newNode);
                if (base_type::back_node() == aAfter)
                    base_type::set_back_node(newNode);
            }
            return newNode;
        }
        void free_node(node* aNode)
        {
            if (aNode)
            {
                if (aNode->next())
                    aNode->next()->set_previous(aNode->previous());
                if (aNode->previous())
                    aNode->previous()->set_next(aNode->next());
                if (base_type::back_node() == aNode)
                    base_type::set_back_node(aNode->previous());
                if (base_type::front_node() == aNode)
                    base_type::set_front_node(aNode->next());
                base_type::delete_node(aNode);
            }
            iAllocator.destroy(aNode);
            iAllocator.deallocate(aNode, 1);
        }

    private:
        node_allocator_type iAllocator;
        size_type iSize;
    };
}
