set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS chrono filesystem system REQUIRED)

function(add_neolib_test_executable TARGET)
  add_executable(${TARGET} ${ARGN})
  target_include_directories(${TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
  set_property(TARGET ${TARGET}  PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_link_libraries(${TARGET} PRIVATE neolib)
  add_test(${TARGET} ${TARGET})
endfunction()

add_neolib_test_executable(NoFussJSON NoFussJSON/src/NoFussJSONTest.cpp)
add_neolib_test_executable(Event Event/src/Event.cpp)
add_neolib_test_executable(Containers Containers/Containers.cpp)
add_neolib_test_executable(Logger Logger/Logger.cpp)
