cmake_minimum_required(VERSION 3.5)
project(brief)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

find_package(CURL REQUIRED)
find_package(LibGit2 REQUIRED)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
find_package(GTest REQUIRED)
find_package(Boost 1.58.0 COMPONENTS filesystem system REQUIRED)
add_subdirectory(deps/backward-cpp)

set(LIBBRIEF_DEPS
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_FILESYSTEM_LIBRARY_RELEASE}
  ${Boost_SYSTEM_LIBRARY_RELEASE}
  ${CURL_LIBRARY}
  ${LIBGIT2_LIBRARIES}
)

include_directories("inc")

set(LIBBRIEF_HEADERS
  inc/brief/serial.hpp
  inc/brief/json.hpp
  inc/brief/msgpack.hpp
  inc/brief/model/description.hpp
  inc/brief/model/task.hpp
  inc/brief/model/repository.hpp
  inc/brief/logger.hpp
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
  src/repository.cpp
)

set(BRIEF_SOURCES
  cli/brief.cpp
)

set(TESTS_SOURCES
  tst/unit/json.cpp
  tst/unit/msgpack.cpp
  tst/unit/context.cpp
)

add_library(libbrief ${LIBBRIEF_SOURCES} ${LIBBRIEF_HEADERS})
link_libraries(libbrief ${LIBBRIEF_DEPS})
set_target_properties(libbrief PROPERTIES PREFIX "")

add_executable(brief ${BRIEF_SOURCES} ${BACKWARD_ENABLE})
target_link_libraries(brief libbrief)
add_backward(brief)

enable_testing()

add_executable(tests ${LIBBRIEF_HEADERS} ${TESTS_SOURCES} ${BACKWARD_ENABLE})
target_link_libraries(tests libbrief ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})
add_backward(tests)

add_test(NAME unittests COMMAND tests --gtest_catch_exceptions=0)
