// container_helper.hpp - v1.0
/*
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
#include <type_traits>
#include "reference_counted.hpp"
#include "pair.hpp"
#include "i_iterator.hpp"

namespace neolib
{
    namespace container
    {
        namespace helper
        {
            template <typename AbstractType, typename ConcreteType>
            struct converter
            {
                typedef AbstractType abstract_type;
                typedef ConcreteType concrete_type;
                static concrete_type& to_concrete_type(abstract_type& aAbstractObject)
                {
                    return static_cast<concrete_type&>(aAbstractObject);
                }
                static abstract_type& to_abstract_type(concrete_type& aConcreteObject)
                {
                    return static_cast<abstract_type&>(aConcreteObject);
                }
            };
            template <typename T1, typename T2, typename ConcreteType1, typename ConcreteType2>
            struct converter<neolib::i_pair<const T1, T2>, std::pair<const ConcreteType1, neolib::pair<const T1, T2, const ConcreteType1, ConcreteType2>>>
            {
                typedef neolib::i_pair<const T1, T2> abstract_type;
                typedef std::pair<const ConcreteType1, neolib::pair<const T1, T2, const ConcreteType1, ConcreteType2>> concrete_type;
                static concrete_type to_concrete_type(abstract_type& aAbstractObject)
                {
                    return concrete_type(aAbstractObject.first(), neolib::pair<T1, T2, ConcreteType1, ConcreteType2>(aAbstractObject.first(), aAbstractObject.second()));
                }
                static abstract_type& to_abstract_type(concrete_type& aConcreteObject)
                {
                    return static_cast<abstract_type&>(aConcreteObject.second);
                }
            };
            template <typename T1, typename T2, typename ConcreteType1, typename ConcreteType2>
            struct converter<const neolib::i_pair<const T1, T2>, const std::pair<const ConcreteType1, neolib::pair<const T1, T2, const ConcreteType1, ConcreteType2>>>
            {
                typedef const neolib::i_pair<const T1, T2> abstract_type;
                typedef const std::pair<const ConcreteType1, neolib::pair<const T1, T2, const ConcreteType1, ConcreteType2>> concrete_type;
                static concrete_type to_concrete_type(abstract_type& aAbstractObject)
                {
                    return concrete_type(aAbstractObject.first(), neolib::pair<T1, T2, ConcreteType1, ConcreteType2>(aAbstractObject.first(), aAbstractObject.second()));
                }
                static abstract_type& to_abstract_type(concrete_type& aConcreteObject)
                {
                    return static_cast<abstract_type&>(aConcreteObject.second);
                }
            };
        }
    }
}
