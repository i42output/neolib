// intrusive_sort.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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

namespace neolib 
{
    namespace detail
    {
        template <typename RandomIt, typename Swapper, typename Compare>
        inline RandomIt partition(RandomIt first, RandomIt last, Swapper swapper, Compare comp)
        {
            auto lo = first;
            auto hi = std::prev(last);
            auto mid = lo + std::distance(lo, hi) / 2;
            if (comp(*mid, *lo))
                swapper(lo, mid);
            if (comp(*hi, *lo))
                swapper(lo, hi);
            if (comp(*mid, *hi))
                swapper(mid, hi);
            auto& pivot = *hi;
            auto i = lo;
            for (auto j = lo; j != hi; ++j)
                if (comp(*j, pivot))
                {
                    swapper(i, j);
                    ++i;
                }
            swapper(i, hi);
            return i;
        }

        template <typename RandomIt>
        inline RandomIt heap_parent(RandomIt first, RandomIt node)
        {
            return first + (std::distance(first, node) - 1) / 2;
        }

        template <typename RandomIt>
        inline RandomIt heap_left_child(RandomIt first, RandomIt node)
        {
            return first + 2 * std::distance(first, node) + 1;
        }

        template <typename RandomIt>
        inline RandomIt heap_right_child(RandomIt first, RandomIt node)
        {
            return first + 2 * std::distance(first, node) + 2;
        }

        template <typename RandomIt, typename Swapper, typename Compare>
        inline void siftDown(RandomIt first, RandomIt start, RandomIt end, Swapper swapper, Compare comp)
        {
            auto root = start;
            while (heap_left_child(first, root) < end)
            {
                auto child = heap_left_child(first, root);
                auto swap = root;
                if (comp(*swap, *child))
                    swap = child;
                ++child;
                if (child < end && comp(*swap, *child))
                    swap = child;
                if (swap == root)
                    return;
                swapper(root, swap);
                root = swap;
            }
        }

        template <typename RandomIt, typename Swapper, typename Compare>
        inline void heapify(RandomIt first, RandomIt last, Swapper swapper, Compare comp)
        {
            auto end = std::prev(last);
            auto start = std::next(heap_parent(first, end));
            while (start > first)
            {
                --start;
                siftDown(first, start, end, swapper, comp);
            }
        }
            
        template <typename RandomIt, typename Swapper, typename Compare>
        inline void heapsort(RandomIt first, RandomIt last, Swapper swapper, Compare comp)
        {
            heapify(first, last, swapper, comp);

            auto end = std::prev(last);
            while (end > first)
            {
                swapper(end, first);
                siftDown(first, first, end, swapper, comp);
                --end;
            }
        }

        template <typename RandomIt, typename Swapper, typename Compare>
        inline void introsort(RandomIt first, RandomIt last, Swapper swapper, Compare comp, uint32_t depth)
        {
            if (std::distance(first, last) > 1)
            {
                if (depth == 0)
                    heapsort(first, last, swapper, comp);
                else
                {
                    auto p = partition(first, last, swapper, comp);
                    introsort(first, p, swapper, comp, depth - 1);
                    introsort(p + 1, last, swapper, comp, depth - 1);
                }
            }
        }
    }
        
    template <typename RandomIt, typename Swapper, typename Compare>
    inline void intrusive_sort(RandomIt first, RandomIt last, Swapper swapper, Compare comp)
    {
        auto n = std::distance(first, last);
        if (n <= 1)
            return;
        uint32_t maxdepth = static_cast<uint32_t>(std::log2(n)) * 2;
        detail::introsort(first, last, swapper, comp, maxdepth);
    }

    template <typename RandomIt, typename Swapper>
    inline void intrusive_sort(RandomIt first, RandomIt last, Swapper swapper)
    {
        intrusive_sort(first, last, swapper, std::less<typename std::iterator_traits<RandomIt>::value_type>{});
    }
}
