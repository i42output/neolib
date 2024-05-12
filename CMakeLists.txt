cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

# meta
project(neolib VERSION 0.0.1 LANGUAGES CXX)

# deployment
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Default intstall prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# compiler support
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if (MSVC)   
    add_compile_options(/std:c++latest)
endif()

# project
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)
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
add_library(neolib ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${HEADER_FILES})
target_include_directories(neolib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set_property(TARGET neolib PROPERTY CXX_STANDARD 20)
set_property(TARGET neolib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET neolib PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET neolib PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})
set_property(TARGET neolib PROPERTY VERSION ${PROJECT_VERSION})

include(GenerateExportHeader)
generate_export_header(neolib EXPORT_FILE_NAME include/neolib/neolib_export.hpp)
target_include_directories(neolib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>) # for neolib_export.hpp

# dependencies
find_package(Threads REQUIRED)
target_link_libraries(neolib PUBLIC Threads::Threads)
if (WIN32)
	set(Boost_USE_STATIC_LIBS   ON)
	set(Boost_USE_STATIC_RUNTIME ON)
else()
	set(LIB_ATOMIC atomic)
endif()
find_package(Boost COMPONENTS system chrono filesystem thread fiber locale REQUIRED)
target_link_libraries(neolib PRIVATE Boost::system )
# need Boost::chrono and Boost::thread for neolib::thread::elasped_ns()
# need Boost::filesystem for boost dll API
target_link_libraries(neolib PUBLIC Boost::chrono Boost::filesystem Boost::thread Boost::fiber Boost::locale ${LIB_ATOMIC})
find_package(OpenSSL REQUIRED COMPONENTS SSL)
target_link_libraries(neolib PRIVATE OpenSSL::SSL)
find_package(ZLIB REQUIRED)
target_link_libraries(neolib PRIVATE ZLIB::ZLIB)

include(CMakePackageConfigHelpers)
configure_package_config_file(neolibConfig.cmake.in neolibConfig.cmake INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
write_basic_package_version_file(neolibConfigVersion.cmake COMPATIBILITY AnyNewerVersion)

install(TARGETS neolib DESTINATION "${CMAKE_INSTALL_LIBDIR}" EXPORT neolib)
install(DIRECTORY include/neolib "${CMAKE_CURRENT_BINARY_DIR}/include/neolib" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS neolib DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(EXPORT neolib DESTINATION "${CMAKE_INSTALL_CMAKEDIR}" FILE neolibTargets.cmake NAMESPACE neolib)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/neolibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/neolibConfigVersion.cmake" DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
install(FILES "LICENSE" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}")

option(NEOLIB_TESTS "Build neolib tests" ON)
if(NEOLIB_TESTS)

  enable_testing()
  
  function(add_neolib_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
    set_property(TARGET ${TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    target_link_libraries(${TARGET} PRIVATE neolib)
    add_test(${TARGET} ${TARGET})
  endfunction()

  add_neolib_test_executable(Containers unit_tests/Containers/Containers.cpp unit_tests/Containers/Tree.cpp)
  add_neolib_test_executable(StringUtils unit_tests/StringUtils/src/StringUtils.cpp)
  add_neolib_test_executable(Logger unit_tests/Logger/Logger.cpp)
  add_neolib_test_executable(NoFussJSON unit_tests/NoFussJSON/src/NoFussJSONTest.cpp)
  add_neolib_test_executable(Task unit_tests/Task/Task.cpp)
  add_neolib_test_executable(Event unit_tests/Event/src/Event.cpp)
  add_neolib_test_executable(App unit_tests/App/src/App.cpp)
  add_neolib_test_executable(File unit_tests/File/File.cpp)

endif()
