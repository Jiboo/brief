cmake_minimum_required(VERSION 3.1)
project(brief)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(CURL REQUIRED)
find_package(LibGit2 REQUIRED)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
find_package(GTest REQUIRED)
find_package(Boost 1.57.0 COMPONENTS filesystem system REQUIRED)

include_directories("inc")

set(LIBBRIEF_HEADERS
  inc/brief/serial.hpp
  inc/brief/json.hpp
  inc/brief/msgpack.hpp

  inc/brief/description.hpp
  inc/brief/task.hpp
  inc/brief/repository.hpp

  inc/brief/context.hpp
  inc/brief/vcs.hpp
  inc/brief/toolchain.hpp
  inc/brief/plugin.hpp
  inc/brief/builder.hpp
  inc/brief/trunks.hpp
)
set(LIBBRIEF_SOURCES
  src/context.cpp
  src/builder.cpp
  src/trunks.cpp
  src/task.cpp
)
add_library(libbrief ${LIBBRIEF_SOURCES} ${LIBBRIEF_HEADERS})
link_libraries(libbrief ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE})

set(BRIEF_SOURCES
  cli/brief.cpp
)
add_executable(brief ${BRIEF_SOURCES})
target_link_libraries(brief libbrief libcurl libgit2)

set(TESTS_SOURCES
  tst/unit/main.cpp
  tst/unit/json.cpp
  tst/unit/msgpack.cpp
  tst/unit/context.cpp
)
add_executable(tests ${LIBBRIEF_HEADERS} ${TESTS_SOURCES})
target_link_libraries(tests gtest libbrief)

add_test(NAME unittests COMMAND tests)
