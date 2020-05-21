// version.hpp
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
#include <string>
#include <ostream>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/string.hpp>
#include <neolib/app/i_version.hpp>

namespace neolib
{
    class version : public i_version
    {
        // construction
    public:
        version(uint32_t aMajor, uint32_t aMinor, uint32_t aMaintenance, uint32_t aBuild = 0, const std::string& aName = "") :
            iMajor(aMajor), iMinor(aMinor), iMaintenance(aMaintenance), iBuild(aBuild), iName(aName)
        {
        }
        version(const std::string& aVersionString = "") :
            iMajor(0), iMinor(0), iMaintenance(0), iBuild(0), iName("")
        {
            neolib::vecarray<std::string, 5> bits;
            neolib::tokens(aVersionString, std::string(". "), bits, 4);
            if (bits.size() > 0)
                iMajor = neolib::string_to_uint32(bits[0]);
            if (bits.size() > 1)
                iMinor = neolib::string_to_uint32(bits[1]);
            if (bits.size() > 2)
                iMaintenance = neolib::string_to_uint32(bits[2]);
            if (bits.size() > 3)
                iBuild = neolib::string_to_uint32(bits[3]);
            if (bits.size() > 4)
                iName = bits[3];
        }
        version(const i_version& aOther) :
            iMajor(aOther.version_major()), iMinor(aOther.version_minor()), iMaintenance(aOther.version_maintenance()), iBuild(aOther.version_build()), iName(aOther.version_name())
        {
        }

        // operations
    public:
        bool operator<(const version& aOther) const
        {
            return std::make_tuple(iMajor, iMinor, iMaintenance, iBuild) < std::make_tuple(aOther.iMajor, aOther.iMinor, iMaintenance, aOther.iBuild);
        }
        bool operator>(const version& aOther) const
        {
            return aOther < *this;
        }
        bool operator==(const version& aOther) const
        {
            return iMajor == aOther.iMajor && iMinor == aOther.iMinor && iMaintenance == aOther.iMaintenance && iBuild == aOther.iBuild;
        }
        // implementation
    public:
        // from i_version
        uint32_t version_major() const
        {
            return iMajor;
        }
        uint32_t version_minor() const
        {
            return iMinor;
        }
        uint32_t version_maintenance() const
        {
            return iMaintenance;
        }
        uint32_t version_build() const
        {
            return iBuild;
        }
        const i_string& version_name() const
        {
            return iName;
        }
    private:
        uint32_t iMajor;
        uint32_t iMinor;
        uint32_t iMaintenance;
        uint32_t iBuild;
        string iName;
    };

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const version& aVersion)
    {
        aStream << aVersion.version_major() << "." << aVersion.version_minor() << "." << aVersion.version_maintenance() << "." << aVersion.version_build();
        if (!aVersion.version_name().empty())
            aStream << " " << aVersion.version_name();
        return aStream;
    }

}
