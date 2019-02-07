// os_version.cpp
/* 
 * Parts Copyright (c) 2007 Leigh Johnston.
 * The author makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 */

#include <neolib/neolib.hpp>
#ifdef _WIN32
#include <tchar.h> // _tcslen
#endif // _WIN32
#include <neolib/string_utils.hpp>
#include <neolib/version.hpp>
#include <neolib/application_info.hpp>

#ifdef _WIN32
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#endif // _WIN32

namespace neolib
{
	std::string os_name()
	{
		std::string result;

#ifdef _WIN32
		OSVERSIONINFOEX osvi;
		SYSTEM_INFO si;
		PGNSI pGNSI;
		PGPI pGPI;
		BOOL bOsVersionInfoEx;
		DWORD dwType;

		ZeroMemory(&si, sizeof(SYSTEM_INFO));
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
		if (!bOsVersionInfoEx)
		{
			return "Microsoft Windows";
		}

		// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

		pGNSI = (PGNSI)GetProcAddress(
			GetModuleHandle(TEXT("kernel32.dll")),
			"GetNativeSystemInfo");
		if (NULL != pGNSI)
			pGNSI(&si);
		else GetSystemInfo(&si);

		if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
			osvi.dwMajorVersion > 4)
		{
			result = ("Microsoft ");

			// Test for the specific product.

			if (osvi.dwMajorVersion == 6 && (osvi.dwMinorVersion == 0 || osvi.dwMinorVersion == 1))
			{
				if (osvi.dwMinorVersion == 0)
				{
					if (osvi.wProductType == VER_NT_WORKSTATION)
						result += ("Windows Vista ");
					else
						result += ("Windows Server 2008 ");
				}
				else if (osvi.dwMinorVersion == 1)
				{
					if (osvi.wProductType == VER_NT_WORKSTATION)
						result += ("Windows 7 ");
					else
						result += ("Windows Server 2008 R2 ");
				}

				pGPI = (PGPI)GetProcAddress(
					GetModuleHandle(TEXT("kernel32.dll")),
					"GetProductInfo");

				pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

				switch (dwType)
				{
				case PRODUCT_ULTIMATE:
					result += ("Ultimate Edition");
					break;
				case PRODUCT_HOME_PREMIUM:
					result += ("Home Premium Edition");
					break;
				case PRODUCT_HOME_BASIC:
					result += ("Home Basic Edition");
					break;
				case PRODUCT_ENTERPRISE:
					result += ("Enterprise Edition");
					break;
				case PRODUCT_BUSINESS:
					result += ("Business Edition");
					break;
				case PRODUCT_STARTER:
					result += ("Starter Edition");
					break;
				case PRODUCT_CLUSTER_SERVER:
					result += ("Cluster Server Edition");
					break;
				case PRODUCT_DATACENTER_SERVER:
					result += ("Datacenter Edition");
					break;
				case PRODUCT_DATACENTER_SERVER_CORE:
					result += ("Datacenter Edition (core installation)");
					break;
				case PRODUCT_ENTERPRISE_SERVER:
					result += ("Enterprise Edition");
					break;
				case PRODUCT_ENTERPRISE_SERVER_CORE:
					result += ("Enterprise Edition (core installation)");
					break;
				case PRODUCT_ENTERPRISE_SERVER_IA64:
					result += ("Enterprise Edition for Itanium-based Systems");
					break;
				case PRODUCT_SMALLBUSINESS_SERVER:
					result += ("Small Business Server");
					break;
				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
					result += ("Small Business Server Premium Edition");
					break;
				case PRODUCT_STANDARD_SERVER:
					result += ("Standard Edition");
					break;
				case PRODUCT_STANDARD_SERVER_CORE:
					result += ("Standard Edition (core installation)");
					break;
				case PRODUCT_WEB_SERVER:
					result += ("Web Server Edition");
					break;
				}
				switch (si.wProcessorArchitecture)
				{
				case PROCESSOR_ARCHITECTURE_AMD64:
				case PROCESSOR_ARCHITECTURE_IA64:
				case PROCESSOR_ARCHITECTURE_ALPHA64:
				case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
					result += (", 64-bit");
					break;
				case PROCESSOR_ARCHITECTURE_INTEL:
					result += (", 32-bit");
					break;
				}
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
			{
				if (GetSystemMetrics(SM_SERVERR2))
					result += ("Windows Server 2003 R2, ");
				else if (osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
					result += ("Windows Storage Server 2003");
				else if (osvi.wProductType == VER_NT_WORKSTATION &&
					si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					result += ("Windows XP Professional x64 Edition");
				}
				else result += ("Windows Server 2003, ");

				// Test for the irc::server type.
				if (osvi.wProductType != VER_NT_WORKSTATION)
				{
					if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							result += ("Datacenter Edition for Itanium-based Systems");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							result += ("Enterprise Edition for Itanium-based Systems");
					}

					else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					{
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							result += ("Datacenter x64 Edition");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							result += ("Enterprise x64 Edition");
						else result += ("Standard x64 Edition");
					}

					else
					{
						if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
							result += ("Compute Cluster Edition");
						else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
							result += ("Datacenter Edition");
						else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
							result += ("Enterprise Edition");
						else if (osvi.wSuiteMask & VER_SUITE_BLADE)
							result += ("Web Edition");
						else result += ("Standard Edition");
					}
				}
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			{
				result += ("Windows XP ");
				if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
					result += ("Home Edition");
				else result += ("Professional");
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			{
				result += ("Windows 2000 ");

				if (osvi.wProductType == VER_NT_WORKSTATION)
				{
					result += ("Professional");
				}
				else
				{
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						result += ("Datacenter Server");
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						result += ("Advanced Server");
					else result += ("Server");
				}
			}

			if (result == ("Microsoft "))
				result += ("Windows");

			// Include service pack (if any) and build number.

			if (_tcslen(osvi.szCSDVersion) > 0)
			{
				result += (" ");
				result += any_to_utf8(std::u16string(reinterpret_cast<char16_t*>(osvi.szCSDVersion)));
			}

			TCHAR buf[80];

			_snwprintf(buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
			result += any_to_utf8(std::u16string(reinterpret_cast<char16_t*>(buf)));
		}
		else
		{
			result = "Microsoft Windows";
		}
#endif // _WIN32
		return result;
	}

	application_info get_application_info(const application_info& aAppInfo)
	{
#ifdef _WIN32
		std::string appName = aAppInfo.name().to_std_string();
		version appVersion = aAppInfo.version();
		std::string appVersionName = aAppInfo.version().name().to_std_string();
		std::string appCopyright = aAppInfo.copyright().to_std_string();
		wchar_t szFullPath[MAX_PATH];
		GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
		DWORD dwVerHnd;
		DWORD dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
		if (dwVerInfoSize)
		{
			// If we were able to get the information, process it:
			UINT    uVersionLen;
			BOOL    bRetCode;
			LPTSTR   lpVersion;
			LPTSTR   lpstrVffInfo;
			HANDLE  hMem;
			hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
			lpstrVffInfo = (wchar_t *)GlobalLock(hMem);
			GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
			uVersionLen = 0;
			lpVersion = NULL;
			bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, L"\\StringFileInfo\\080904B0\\ProductName",
				(LPVOID *)&lpVersion, (PUINT)&uVersionLen);
			if (bRetCode && uVersionLen && lpVersion)
				appName = any_to_utf8(reinterpret_cast<char16_t*>(lpVersion));
			uVersionLen = 0;
			lpVersion = NULL;
			bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, L"\\StringFileInfo\\080904B0\\ProductVersion",
				(LPVOID *)&lpVersion, (PUINT)&uVersionLen);
			if (bRetCode && uVersionLen && lpVersion)
			{
				vecarray<std::string, 4> ver;
				tokens(any_to_utf8(reinterpret_cast<char16_t*>(lpVersion)), std::string("."), ver, 4);
				appVersion = version(
					string_to_uint32(ver[0]), 
					string_to_uint32(ver[1]), 
					string_to_uint32(ver[2]),
					string_to_uint32(ver[3]));
			}
			uVersionLen = 0;
			lpVersion = NULL;
			bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, L"\\StringFileInfo\\080904B0\\ProductVersionName",
				(LPVOID *)&lpVersion, (PUINT)&uVersionLen);
			if (bRetCode && uVersionLen && lpVersion)
				appVersion = version(appVersion.major(), appVersion.minor(), appVersion.maintenance(), appVersion.build(), any_to_utf8(reinterpret_cast<char16_t*>(lpVersion)));
			if (appVersion.build() == 0)
				appVersion = version(appVersion.major(), appVersion.minor(), appVersion.maintenance(), aAppInfo.version().build(), any_to_utf8(reinterpret_cast<char16_t*>(lpVersion)));
			uVersionLen = 0;
			lpVersion = NULL;
			bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, L"\\StringFileInfo\\080904B0\\LegalCopyright",
				(LPVOID *)&lpVersion, (PUINT)&uVersionLen);
			if (bRetCode && uVersionLen && lpVersion)
				appCopyright = any_to_utf8(reinterpret_cast<char16_t*>(lpVersion));
			GlobalUnlock(hMem);
			GlobalFree(hMem);
			return application_info(aAppInfo.arguments(), appName, aAppInfo.company().to_std_string(), appVersion, appCopyright,
				aAppInfo.application_folder().to_std_string(), aAppInfo.settings_folder().to_std_string(), aAppInfo.data_folder().to_std_string());
		} // if (dwVerInfoSize)
#endif // _WIN32
		
		return aAppInfo;
	}
}