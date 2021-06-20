// zip.cpp
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
#include <fstream>
#include <string>
#include <zlib.h>
#include <filesystem>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/crc.hpp>
#include <neolib/file/zip.hpp>

#pragma pack(1)

namespace neolib
{
    namespace
    {
        template <typename T>
        inline const char* byte_cast(const T* p)
        {
            return reinterpret_cast<const char*>(p);
        }
    }

    struct zip::local_header
    {
        enum { Signature = 0x04034b50 };
        dword iSignature;
        word iVersion;
        word iFlag;
        word iCompression;
        word iTime;
        word iDate;
        dword iCrc32;
        dword iCompressedSize;
        dword iUncompressedSize;
        word iFilenameLength;
        word iExtraLength;
    };

    struct zip::dir_header
    {
        enum { Signature = 0x06054b50 };
        dword iSignature;
        word iDisk;
        word iStartDisk;
        word iDirEntries;
        word iTotalDirEntries;
        dword iDirSize;
        dword iDirOffset;
        word iCommentLength;
    };

    struct zip::dir_file_header
    {
        enum { Signature = 0x02014b50 };
        dword iSignature;
        word iVersionMade;
        word iVersionNeeded;
        word iFlag;
        word iCompression;
        word iTime;
        word iDate;
        dword iCrc32;
        dword iCompressedSize;
        dword iUncompressedSize;
        word iFilenameLength;
        word iExtraLength;
        word iCommentLength;
        word iDiskStart;
        word iIntAttr;
        dword iExtAttr;
        dword iHeaderOffset;
    };

    #pragma pack()

    zip::zip(const std::string& aZipFilePath) : iZipFileData{}, iZipFileDataLength{}, iError{false}
    {
        auto fileSize = std::filesystem::file_size(aZipFilePath);
        if (fileSize > iZipFile.max_size())
            throw zip_file_too_big();
        iZipFile.resize(static_cast<std::size_t>(fileSize));
        iZipFileData = &iZipFile[0];
        iZipFileDataLength = iZipFile.size();
        std::ifstream input(aZipFilePath, std::ios::binary | std::ios::in);
        if (input.read(reinterpret_cast<char*>(&iZipFile[0]), iZipFile.size()))
            parse();
        else
            iError = true;
    }

    zip::zip(const buffer_type& aZipFile) : iZipFile{aZipFile}, iZipFileData{&iZipFile[0]}, iZipFileDataLength{iZipFile.size()}, iError{false}
    {
        parse();
    }

    zip::zip(buffer_type&& aZipFile) : iZipFile{std::move(aZipFile)}, iZipFileData{&iZipFile[0]}, iZipFileDataLength{iZipFile.size()}, iError{false}
    {
        parse();
    }

    zip::zip(const void* aZipFileData, std::size_t aZipFileDataLength) : iZipFileData{static_cast<const uint8_t*>(aZipFileData)}, iZipFileDataLength{aZipFileDataLength}, iError{ false }
    {
        parse();
    }

    std::size_t zip::index_of(const std::string& aFile) const
    {
        for (std::size_t i = 0; i < file_count(); ++i)
            if (file_path(i) == aFile)
                return i;
        throw file_not_found();
    }

    bool zip::extract(size_t aIndex, const std::string& aTargetDirectory)
    {
        buffer_type data;
        if (extract_to(aIndex, data))
        {
            std::ofstream out(aTargetDirectory + "/" + file_path(aIndex), std::ios::out | std::ios::binary);
            out.write(reinterpret_cast<const char*>(data[0]), data.size());
            if (out)
                return true;
        }
        iError = true;
        return false;
    }

    bool zip::extract_to(size_t aIndex, buffer_type& aBuffer)
    {
        if (iError)
            return false;
        if (aIndex > iFiles.size())
            return false;
        const local_header* lh = reinterpret_cast<const local_header*>(data_front() + iDirEntries[aIndex]->iHeaderOffset);
        if (byte_cast(lh) < byte_cast(data_front()) ||
            byte_cast(lh) > byte_cast(data_back()) - sizeof(local_header) + 1)
        {
            iError = true;
            return false;
        }
        if (lh->iSignature != local_header::Signature)
        {
            iError = true;
            return false;
        }
        const uint8_t* compressedData = reinterpret_cast<const uint8_t*>(lh + 1) + lh->iFilenameLength + lh->iExtraLength;
        if (byte_cast(compressedData) < byte_cast(data_front()) ||
            byte_cast(compressedData) + lh->iCompressedSize - 1 > byte_cast(data_back()))
        {
            iError = true;
            return false;
        }
        if (lh->iCompression == Z_NO_COMPRESSION)
        {
            if (crc32(reinterpret_cast<const uint8_t*>(compressedData), lh->iCompressedSize) != lh->iCrc32)
            {
                iError = true;
                return false;
            }
            aBuffer.resize(lh->iCompressedSize);
            std::copy(compressedData, compressedData + aBuffer.size(), &aBuffer[0]);
            return true;
        }
        else if (lh->iCompression != Z_DEFLATED)
        {
            iError = true;
            return false;
        }
        z_stream stream;
        stream.next_in = static_cast<Bytef*>(const_cast<uint8_t*>(compressedData));
        stream.avail_in = static_cast<uInt>(lh->iCompressedSize);
        aBuffer.resize(lh->iUncompressedSize);
        stream.next_out = static_cast<Bytef*>(&aBuffer[0]);
        stream.avail_out = static_cast<uInt>(aBuffer.size());
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
        if (result != Z_OK)
        {
            iError = true;
            return false;
        }
        if (crc32(reinterpret_cast<const uint8_t*>(&aBuffer[0]), static_cast<uint32_t>(aBuffer.size())) != lh->iCrc32)
        {
            iError = true;
            return false;
        }
        return true;
    }

    std::string zip::extract_to_string(size_t aIndex)
    {
        buffer_type buffer;
        extract_to(aIndex, buffer);
        return std::string(buffer.begin(), buffer.end());
    }

    const std::string& zip::file_path(size_t aIndex) const
    {
        return iFiles[aIndex];
    }

    bool zip::parse()
    {
        if (iZipFileDataLength < sizeof(dir_header))
        {
            iError = true;
            return false;
        }
        const dir_header* dh = reinterpret_cast<const dir_header*>(data_back() - sizeof(dir_header) + 1);
        if (dh->iSignature != dir_header::Signature)
        {
            iError = true;
            return false;
        }
        const dir_file_header* fh = reinterpret_cast<const dir_file_header*>(data_back() - (dh->iDirSize + sizeof(dir_header)) + 1);
        for (size_t i = 0; i < dh->iDirEntries; ++i)
        {
            if (byte_cast(fh) < byte_cast(data_front()) ||
                byte_cast(fh) + sizeof(fh->iSignature) >= byte_cast(dh))
            {
                iError = false;
                return false;
            }
            if (fh->iSignature != dir_file_header::Signature)
            {
                iError = false;
                return false;
            }
            else
            {
                if (byte_cast(reinterpret_cast<const char*>(fh) + sizeof(*fh) + fh->iFilenameLength + fh->iExtraLength + fh->iCommentLength) > 
                    byte_cast(dh))
                {
                    iError = false;
                    return false;
                }
                iDirEntries.push_back(fh);
                std::string filename(reinterpret_cast<const char*>(fh + 1), fh->iFilenameLength);
                iFiles.push_back(filename);
                fh = reinterpret_cast<const dir_file_header*>(reinterpret_cast<const char*>(fh) + sizeof(*fh) + fh->iFilenameLength + fh->iExtraLength + fh->iCommentLength);
            }
        }
        return true;
    }

    const uint8_t* zip::data_front()
    {
        return iZipFileData;
    }

    const uint8_t* zip::data_back()
    {
        return (iZipFileData + iZipFileDataLength - 1);
    }
}
