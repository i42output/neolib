// zip.h
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
#include <vector>
#include <memory>

namespace neolib
{
	class zip
	{
	public:
		typedef std::vector<uint8_t> buffer_type;
	public:
		struct zip_file_too_big : std::runtime_error { zip_file_too_big() : std::runtime_error("neolib::zip::zip_file_too_big") {} };
		struct file_not_found : std::runtime_error { file_not_found() : std::runtime_error("neolib::zip::file_not_found") {} };
	public:
		zip(const std::string& aZipFilePath);
		zip(const buffer_type& aZipFile);
		zip(buffer_type&& aZipFile);
		zip(const void* aZipFileData, std::size_t aZipFileDataLength);
	public:
		size_t file_count() const { return iFiles.size(); }
		std::size_t index_of(const std::string& aFile) const;
		bool extract(size_t aIndex, const std::string& aTargetDirectory);
		bool extract_to(size_t aIndex, buffer_type& aBuffer);
		std::string extract_to_string(size_t aIndex);
		const std::string& file_path(size_t aIndex) const;
		bool ok() const { return !iError; }
	private:
		bool parse();
		const uint8_t* data_front();
		const uint8_t* data_back();
	private:
		buffer_type iZipFile;
		const uint8_t* iZipFileData;
		std::size_t iZipFileDataLength;
		bool iError;
		struct dir_header;
		struct dir_file_header;
		struct local_header;
		typedef unsigned long dword;
		typedef unsigned short word;
		typedef unsigned char byte;
		std::vector<const dir_file_header*> iDirEntries;
		std::vector<std::string> iFiles;
	};
}
