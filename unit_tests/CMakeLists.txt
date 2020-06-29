function(add_neolib_test_executable TARGET)
  add_executable(${TARGET} ${ARGN})
  target_link_libraries(${TARGET} PRIVATE neolib)
  add_test(${TARGET} ${TARGET})
endfunction()

find_package(RapidJSON REQUIRED)
add_neolib_test_executable(NoFussJSON NoFussJSON/src/NoFussJSONTest.cpp)
target_include_Directories(NoFussJSON PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

add_neolib_test_executable(Event Event/src/Event.cpp)

add_neolib_test_executable(Containers Containers/Containers.cpp)
