cmake_minimum_required(VERSION 3.21)

project(Precept LANGUAGES CXX)

add_library(precept INTERFACE)
add_library(Precept::Precept ALIAS precept)

target_compile_features(precept INTERFACE cxx_std_20)
target_include_directories(
  precept
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()
