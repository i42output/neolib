// zip.h
/*
 *  Copyright (c) 2012 Leigh Johnston.
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
	template <typename CharT = char>
	class basic_zip
	{
	public:
		typedef std::basic_string<CharT> path_type;
		typedef std::vector<char> buffer_t;
		typedef std::tr1::shared_ptr<buffer_t> buffer_ptr_t;
	public:
		basic_zip(const buffer_t& aZipFile, const path_type& aTargetDirectory);
		basic_zip(const buffer_ptr_t& aZipFilePtr, const path_type& aTargetDirectory);
	public:
		size_t size() const { return iFiles.size(); }
		bool extract(size_t aIndex);
		path_type file_path(size_t aIndex) const;
		bool ok() const { return !iError; }
	private:
		bool parse();
	private:
		const buffer_t& iZipFile;
		buffer_ptr_t iZipFilePtr;
		path_type iTargetDirectory;
		bool iError;
		struct dir_header;
		struct dir_file_header;
		struct local_header;
		typedef unsigned long dword;
		typedef unsigned short word;
		typedef unsigned char byte;
		std::vector<const dir_file_header*> iDirEntries;
		std::vector<path_type> iFiles;
	};

	typedef basic_zip<char> zip;
	typedef basic_zip<wchar_t> wzip;
}
