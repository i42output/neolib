// os_version.hpp
/* 
 * Parts Copyright (c) 2007 Leigh Johnston.
 * The author makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 */

#include "neolib.hpp"
#include "string_utils.hpp"
#include "version.hpp"
#include "application_info.hpp"

namespace neolib
{
	std::string os_name();
	application_info get_application_info(const application_info& aAppInfo = application_info());
}