cmake_minimum_required(VERSION 3.24)

include(cmake/prelude.cmake)

project(
    stamd
    VERSION 0.2.9
    DESCRIPTION "Static Markdown Page Generator"
    HOMEPAGE_URL "https://git.dimitrijedobrota.com/stamd.git"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare dependencies ----

find_package(md4c CONFIG REQUIRED)
find_package(poafloc 1 CONFIG REQUIRED)
find_package(hemplate 0.1 CONFIG REQUIRED)


# ---- Declare library ----

add_library(
    stamd_lib OBJECT
    source/article.cpp
    source/indexer.cpp
)

target_link_libraries(stamd_lib PUBLIC hemplate::hemplate poafloc::poafloc)

target_include_directories(
    stamd_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(stamd_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(stamd_exe source/main.cpp)
add_executable(stamd::exe ALIAS stamd_exe)

set_property(TARGET stamd_exe PROPERTY OUTPUT_NAME stamd)

target_compile_features(stamd_exe PRIVATE cxx_std_20)

target_link_libraries(stamd_exe PRIVATE poafloc md4c::md4c-html stamd_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT stamd_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of stamd"
  )
endif()

include(cmake/dev-mode.cmake)
