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

add_library(
  precept_compile_requirements OBJECT compile_requirements.cpp
                                      at_least_span_compile_test.cpp
                                      checked_span_compile_test.cpp)
target_link_libraries(precept_compile_requirements PRIVATE Precept::Precept)
target_compile_options(
  precept_compile_requirements
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

add_executable(precept_runtime_tests runtime_smoke_test.cpp at_least_span_test.cpp
                                     checked_span_test.cpp)
target_link_libraries(precept_runtime_tests PRIVATE Precept::Precept GTest::gtest_main)
target_compile_options(
  precept_runtime_tests
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/EHsc>
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

include(GoogleTest)
gtest_discover_tests(precept_runtime_tests)

function(precept_expect_compile_failure test_name source_file)
  try_compile(
    ${test_name}_compiled
    ${CMAKE_CURRENT_BINARY_DIR}/negative/${test_name}
    ${CMAKE_CURRENT_SOURCE_DIR}/negative/${source_file}
    CMAKE_FLAGS -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON
                -DCMAKE_CXX_EXTENSIONS=OFF
                -DINCLUDE_DIRECTORIES=${PROJECT_SOURCE_DIR}/include
    OUTPUT_VARIABLE compile_output)
  if(${test_name}_compiled)
    message(FATAL_ERROR "Negative compile test ${test_name} unexpectedly compiled")
  endif()
endfunction()

precept_expect_compile_failure(at_least_span_zero at_least_span_zero.cpp)
precept_expect_compile_failure(at_least_span_dynamic_extent at_least_span_dynamic_extent.cpp)
precept_expect_compile_failure(at_least_span_fixed_too_small at_least_span_fixed_too_small.cpp)
precept_expect_compile_failure(at_least_span_dynamic_direct at_least_span_dynamic_direct.cpp)
precept_expect_compile_failure(at_least_span_fixed_try_from at_least_span_fixed_try_from.cpp)
precept_expect_compile_failure(checked_span_fixed_unequal checked_span_fixed_unequal.cpp)
precept_expect_compile_failure(checked_span_dynamic_extent checked_span_dynamic_extent.cpp)

# Knowledge bundle checks. These are development-only: the Python interpreter and its YAML
# parser are never usage requirements of Precept::Precept, and a BUILD_TESTING=OFF
# configuration never looks for them.
option(PRECEPT_REQUIRE_KNOWLEDGE_CHECK
       "Fail configuration when the knowledge checker cannot run" OFF)

find_package(Python3 COMPONENTS Interpreter QUIET)

set(precept_knowledge_check_blocker "")
if(NOT Python3_FOUND)
  set(precept_knowledge_check_blocker "no Python 3 interpreter was found")
else()
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import yaml"
    RESULT_VARIABLE precept_yaml_import
    OUTPUT_QUIET ERROR_QUIET)
  if(NOT precept_yaml_import EQUAL 0)
    set(precept_knowledge_check_blocker
        "${Python3_EXECUTABLE} cannot import yaml (python -m pip install pyyaml)")
  endif()
endif()

if(precept_knowledge_check_blocker)
  if(PRECEPT_REQUIRE_KNOWLEDGE_CHECK)
    message(
      FATAL_ERROR
        "Knowledge checks are required but cannot run: ${precept_knowledge_check_blocker}")
  endif()
  message(STATUS "Precept: knowledge checks not registered - ${precept_knowledge_check_blocker}")
else()
  add_test(NAME knowledge_bundle
           COMMAND ${Python3_EXECUTABLE} tools/check_knowledge.py knowledge)
  add_test(NAME knowledge_checker
           COMMAND ${Python3_EXECUTABLE} -m unittest --quiet tools.test_check_knowledge)
  set_tests_properties(knowledge_bundle knowledge_checker
                       PROPERTIES WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
