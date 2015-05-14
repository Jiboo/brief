cmake_minimum_required(VERSION 3.1)
project(brief)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(FlatBuffers REQUIRED)
find_package(GTest REQUIRED)
find_package(Boost 1.57.0 COMPONENTS filesystem system REQUIRED)

FLATBUFFERS_GENERATE_C_HEADERS(RepoModel model/repo.fbs)
include_directories("inc" ${CMAKE_CURRENT_BINARY_DIR})

set(LIBBRIEF_HEADERS
#  ${RepoModel_OUTPUTS}
#  inc/brief/context.hpp
#  inc/brief/vcs.hpp
#  inc/brief/toolchain.hpp
#  inc/brief/plugin.hpp
  inc/brief/json.hpp
  inc/brief/msgpack.hpp
)
set(LIBBRIEF_SOURCES
#  src/context.cpp
)
#add_library(libbrief ${LIBBRIEF_SOURCES} ${LIBBRIEF_HEADERS})
#link_libraries(libbrief ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE})

set(BRIEF_SOURCES
#  cli/brief.cpp
)
#add_executable(brief ${BRIEF_SOURCES})
#target_link_libraries(brief libbrief)

set(TESTS_SOURCES
  tst/unit/main.cpp
  tst/unit/json.cpp
  tst/unit/msgpack.cpp
#  tst/unit/context.cpp
)
add_executable(tests ${LIBBRIEF_HEADERS} ${TESTS_SOURCES})
target_link_libraries(tests gtest)

add_test(NAME unittests COMMAND tests)