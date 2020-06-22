cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

# meta
project(neolib VERSION 0.0.1 LANGUAGES CXX)

# deployment
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Default intstall prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(GNUInstallDirs)

# compiler support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# project
if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_UNICODE)
endif()

file(GLOB_RECURSE SOURCE_FILES src/app/*.cpp src/core/*.cpp src/ecs/*.cpp src/file/*.cpp  src/io/*.cpp src/plugin/*.cpp src/task/*.cpp)
if (WIN32)
  file(GLOB_RECURSE PLATFORM_SOURCE_FILES src/win32/*.*)
ELSE()
  file(GLOB_RECURSE PLATFORM_SOURCE_FILES src/posix/*.*)
ENDIF()
file(GLOB_RECURSE HEADER_FILES include/*.*)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${HEADER_FILES})

add_definitions(-DNEOLIB_HOSTED_ENVIRONMENT)
add_library(neolib STATIC ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${HEADER_FILES})
target_include_directories(neolib PUBLIC include)
set_property(TARGET neolib PROPERTY CXX_STANDARD 17)
set_property(TARGET neolib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# dependencies
find_package(Boost COMPONENTS system REQUIRED)
IF (Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()
find_package(OpenSSL REQUIRED)
IF (OpenSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
ENDIF()
find_package(ZLIB REQUIRED)
IF (ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ENDIF()

install(DIRECTORY include/neolib DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS neolib DESTINATION "${CMAKE_INSTALL_LIBDIR}")