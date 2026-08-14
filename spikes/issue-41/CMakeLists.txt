cmake_minimum_required(VERSION 3.20)
project(precept_issue_41_gate_c LANGUAGES CXX)

add_executable(issue_41_aligned_ptr_gate_c main.cpp)
target_compile_features(issue_41_aligned_ptr_gate_c PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(issue_41_aligned_ptr_gate_c PRIVATE /W4 /permissive-)
else()
  target_compile_options(issue_41_aligned_ptr_gate_c PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME issue_41_aligned_ptr_gate_c COMMAND issue_41_aligned_ptr_gate_c)
