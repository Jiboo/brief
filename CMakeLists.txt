cmake_minimum_required(VERSION 3.1)
project(brief)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(FlatBuffers REQUIRED)
find_package(Boost 1.57.0 COMPONENTS filesystem system REQUIRED)

FLATBUFFERS_GENERATE_C_HEADERS(RepoModel model/repo.fbs)
include_directories("inc" ${CMAKE_CURRENT_BINARY_DIR})

set(LIBBRIEF_HEADERS
  ${RepoModel_OUTPUTS}
  inc/brief/context.hpp
  inc/brief/vcs.hpp
  inc/brief/toolchain.hpp
  inc/brief/plugin.hpp
)
set(LIBBRIEF_SOURCES
  src/context.cpp
)
add_library(libbrief ${LIBBRIEF_SOURCES} ${LIBBRIEF_HEADERS})

set(BRIEF_SOURCES
  cli/brief.cpp
)
add_executable(brief ${BRIEF_SOURCES})
target_link_libraries(brief libbrief ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${Boost_SYSTEM_LIBRARY_RELEASE})
