cmake_minimum_required(VERSION 3.20)
project(precept_issue_41_final_gate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(issue_41_final_positive positive.cpp)
if(MSVC)
  target_compile_options(issue_41_final_positive PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(issue_41_final_positive PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME issue_41_final_positive COMMAND issue_41_final_positive)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/diagnostics")

function(expect_compile_failure name source)
  try_compile(
    compiled
    "${CMAKE_BINARY_DIR}/try-${name}"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    CMAKE_FLAGS
      "-DCMAKE_CXX_STANDARD=20"
      "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
      "-DCMAKE_CXX_EXTENSIONS=OFF"
      "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
    OUTPUT_VARIABLE output
  )

  file(WRITE "${CMAKE_BINARY_DIR}/diagnostics/${name}.txt" "${output}")
  if(compiled)
    message(FATAL_ERROR "Issue #41 negative compile case unexpectedly succeeded: ${name}")
  endif()
endfunction()

expect_compile_failure(strengthening bad_strengthening.cpp)
expect_compile_failure(const_drop bad_const_drop.cpp)
expect_compile_failure(array_covariance bad_covariance.cpp)
expect_compile_failure(non_power_of_two bad_power_of_two.cpp)
expect_compile_failure(below_natural_alignment bad_natural_alignment.cpp)
expect_compile_failure(void_element bad_void.cpp)
expect_compile_failure(incomplete_element bad_incomplete.cpp)
expect_compile_failure(template_deduction_boundary template_deduction_boundary.cpp)
