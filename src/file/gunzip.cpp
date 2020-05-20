// gunzip.cpp
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

#include <neolib/neolib.hpp>
#include <zlib/zlib.h>
#include <neolib/file/gunzip.hpp>

namespace neolib
{

    gunzip::gunzip(const compressed_data_t& aGzipData) : iOk(false)
    {
        compressed_data_t::const_iterator i = aGzipData.begin();
        if (aGzipData.size() < 10)
            return;
        unsigned char id1 = static_cast<unsigned char>(*i++);
        unsigned char id2 = static_cast<unsigned char>(*i++);
        unsigned char cm = static_cast<unsigned char>(*i++);
        unsigned char flg = static_cast<unsigned char>(*i++);
        unsigned long mtime = static_cast<unsigned char>(*i++); 
        mtime += (static_cast<unsigned char>(*i++) << 8);
        mtime += (static_cast<unsigned char>(*i++) << 16); 
        mtime += (static_cast<unsigned char>(*i++) << 24);
        unsigned char xfl = static_cast<unsigned char>(*i++); (void)xfl;
        unsigned char os = static_cast<unsigned char>(*i++); (void)os;
        if (id1 != 0x1F)
            return;
        if (id2 != 0x8B)
            return;
        if (cm != 8)
            return;
        enum flg_e { FTEXT = 0x1, FHCRC = 0x2, FEXTRA = 0x4, FNAME = 0x8, FCOMMENT = 0x10 };
        if (flg & FEXTRA)
        {
            if (aGzipData.end() - i < 2)
                return;
            unsigned short xlen = static_cast<unsigned char>(*i++);
            xlen += (static_cast<unsigned char>(*i++) << 8);
            if (aGzipData.end() - i < xlen)
                return;
            i += xlen;
        }
        if (flg & FNAME)
        {
            while (i != aGzipData.end())
                if (*i++ == 0)
                    break;
            if (i == aGzipData.end())
                return;
        }
        if (flg & FCOMMENT)
        {
            while (i != aGzipData.end())
                if (*i++ == 0)
                    break;
            if (i == aGzipData.end())
                return;
        }
        if (flg & FHCRC)
        {
            if (aGzipData.end() - i < 2)
                return;
            unsigned short crc16 = static_cast<unsigned char>(*i++);
            crc16 += (static_cast<unsigned char>(*i++) << 8);
        }
        if (aGzipData.end() - i < 8)
                return;
        const char* compressedData = &*i;
        std::size_t compressedSize = aGzipData.end() - i - 8;
        i += compressedSize; 
        unsigned long crc32 = static_cast<unsigned char>(*i++);
        crc32 += (static_cast<unsigned char>(*i++) << 8);
        crc32 += (static_cast<unsigned char>(*i++) << 16); 
        crc32 += (static_cast<unsigned char>(*i++) << 24);    
        unsigned long isize = static_cast<unsigned char>(*i++); 
        isize += (static_cast<unsigned char>(*i++) << 8);
        isize += (static_cast<unsigned char>(*i++) << 16); 
        isize += (static_cast<unsigned char>(*i++) << 24);
        unsigned long uncompressedSize = isize;
        z_stream stream;
        stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressedData));
        stream.avail_in = static_cast<uInt>(compressedSize);
        iUncompressedData.resize(uncompressedSize);
        stream.next_out = reinterpret_cast<Bytef*>(&iUncompressedData[0]);
        stream.avail_out = static_cast<uInt>(iUncompressedData.size());
        stream.zalloc = static_cast<alloc_func>(0);
        stream.zfree = static_cast<free_func>(0);
        int result = inflateInit2(&stream, -MAX_WBITS);
        if (result == Z_OK)
        {
            result = inflate(&stream, Z_FINISH);
            inflateEnd(&stream);
            if (result == Z_STREAM_END)
                result = Z_OK;
            inflateEnd(&stream);
        }
        iOk = (result == Z_OK);
        if (!iOk)
        {
            iUncompressedData.clear();
        }
    }

}