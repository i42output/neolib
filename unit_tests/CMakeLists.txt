set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS system chrono filesystem REQUIRED)

function(add_neolib_test_executable TARGET)
  add_executable(${TARGET} ${ARGN})
  target_include_directories(${TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
    "${CMAKE_INSTALL_PREFIX}/include")
  set_property(TARGET ${TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
  target_link_libraries(${TARGET} PRIVATE neolib)
  add_test(${TARGET} ${TARGET})
endfunction()

add_neolib_test_executable(NoFussJSON NoFussJSON/src/NoFussJSONTest.cpp)
add_neolib_test_executable(Event Event/src/Event.cpp)
add_neolib_test_executable(Containers Containers/Containers.cpp)
add_neolib_test_executable(Logger Logger/Logger.cpp)
