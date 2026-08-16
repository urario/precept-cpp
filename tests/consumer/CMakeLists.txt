cmake_minimum_required(VERSION 3.21)

project(PreceptConsumer LANGUAGES CXX)

find_package(Precept 0.3 CONFIG REQUIRED)

set(expected_public_headers
    precept/aligned_ptr.hpp
    precept/index_below.hpp
    precept/narrow_exact.hpp
    precept/never_decrease.hpp
    precept/non_overlapping.hpp
    precept/nonzero.hpp
    precept/same_size.hpp
    precept/set_once.hpp
    precept/span/at_least_span.hpp
    precept/span/block_span.hpp
    precept/span/checked_span.hpp
    precept/span/non_empty_span.hpp)

get_target_property(precept_include_dirs Precept::Precept INTERFACE_INCLUDE_DIRECTORIES)
set(actual_public_headers "")
foreach(include_dir IN LISTS precept_include_dirs)
  if(EXISTS "${include_dir}/precept")
    file(GLOB_RECURSE found_public_headers
         LIST_DIRECTORIES FALSE
         RELATIVE "${include_dir}"
         "${include_dir}/precept/*")
    list(APPEND actual_public_headers ${found_public_headers})
  endif()
endforeach()

list(REMOVE_DUPLICATES actual_public_headers)
list(SORT actual_public_headers)
list(SORT expected_public_headers)

if(NOT "${actual_public_headers}" STREQUAL "${expected_public_headers}")
  message(
    FATAL_ERROR
      "Installed public header surface does not match the expected set.\n"
      "Expected: ${expected_public_headers}\n"
      "Actual:   ${actual_public_headers}")
endif()

add_executable(precept_consumer main.cpp)
target_link_libraries(precept_consumer PRIVATE Precept::Precept)
