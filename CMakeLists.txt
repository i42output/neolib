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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

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
add_library(${PROJECT_NAME} ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION ${PROJECT_VERSION_MAJOR})
set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME include/${PROJECT_NAME}/${PROJECT_NAME}_export.hpp)
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>) # for ${PROJECT_NAME}_export.hpp

# dependencies
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
if (WIN32)
	set(Boost_USE_STATIC_LIBS   ON)
	set(Boost_USE_STATIC_RUNTIME ON)
else()
	set(LIB_ATOMIC atomic)
endif()
find_package(Boost COMPONENTS system chrono filesystem thread fiber locale REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system )
# need Boost::chrono and Boost::thread for ${PROJECT_NAME}::thread::elasped_ns()
# need Boost::filesystem for boost dll API
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::chrono Boost::filesystem Boost::thread Boost::fiber Boost::locale ${LIB_ATOMIC})
find_package(OpenSSL REQUIRED COMPONENTS SSL)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}Config.cmake INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake COMPATIBILITY AnyNewerVersion)

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}" EXPORT
	${PROJECT_NAME})
install(DIRECTORY include/${PROJECT_NAME}
	"${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(EXPORT ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_CMAKEDIR}" FILE
	${PROJECT_NAME}Targets.cmake NAMESPACE ${PROJECT_NAME})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
install(FILES "LICENSE" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}")

option(NEOLIB_TESTS "Build ${PROJECT_NAME} tests" ON)
if(NEOLIB_TESTS)

  enable_testing()
  
  function(add_${PROJECT_NAME}_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
    set_property(TARGET ${TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    target_link_libraries(${TARGET} PRIVATE ${PROJECT_NAME})
    add_test(${TARGET} ${TARGET})
  endfunction()

  add_neolib_test_executable(Task unit_tests/Task/Task.cpp)
  add_neolib_test_executable(StringUtils unit_tests/StringUtils/src/StringUtils.cpp)
  add_neolib_test_executable(NoFussJSON unit_tests/NoFussJSON/src/NoFussJSONTest.cpp)
  add_neolib_test_executable(Event unit_tests/Event/src/Event.cpp)
  add_neolib_test_executable(Containers unit_tests/Containers/Containers.cpp unit_tests/Containers/Tree.cpp)
  add_neolib_test_executable(Logger unit_tests/Logger/Logger.cpp)

endif()
