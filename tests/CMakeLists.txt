include(FetchContent)

set(BUILD_GMOCK OFF CACHE BOOL "Do not build GoogleMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Do not install GoogleTest" FORCE)

if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "Use the shared MSVC runtime for GoogleTest" FORCE)
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.18.0
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(googletest)

add_library(precept_compile_requirements OBJECT compile_requirements.cpp)
target_link_libraries(precept_compile_requirements PRIVATE Precept::Precept)
target_compile_options(
  precept_compile_requirements
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

add_executable(precept_runtime_tests runtime_smoke_test.cpp)
target_link_libraries(precept_runtime_tests PRIVATE Precept::Precept GTest::gtest_main)
target_compile_options(
  precept_runtime_tests
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/EHsc>
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

include(GoogleTest)
gtest_discover_tests(precept_runtime_tests)
