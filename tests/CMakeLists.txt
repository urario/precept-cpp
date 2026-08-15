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
                                      aligned_ptr_compile_test.cpp
                                      at_least_span_compile_test.cpp
                                      block_span_compile_test.cpp
                                      checked_span_compile_test.cpp
                                      set_once_compile_test.cpp)
target_link_libraries(precept_compile_requirements PRIVATE Precept::Precept)
target_compile_options(
  precept_compile_requirements
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

add_executable(precept_runtime_tests runtime_smoke_test.cpp aligned_ptr_test.cpp
                                     at_least_span_test.cpp block_span_test.cpp
                                     checked_span_test.cpp set_once_test.cpp)
target_link_libraries(precept_runtime_tests PRIVATE Precept::Precept GTest::gtest_main)
target_compile_options(
  precept_runtime_tests
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/EHsc>
          $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>)

include(GoogleTest)
# DISCOVERY_TIMEOUT raised from the CTest module's 5-second default: the MSVC runner has
# intermittently missed it, failing test discovery rather than any actual test.
gtest_discover_tests(precept_runtime_tests DISCOVERY_TIMEOUT 60)

if(CMAKE_CONFIGURATION_TYPES)
  set(precept_multi_config TRUE)
else()
  set(precept_multi_config FALSE)
endif()

set(precept_consumer_test_args
    -DPRECEPT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DPRECEPT_CONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/consumer
    -DPRECEPT_TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/consumer-smoke
    -DPRECEPT_GENERATOR=${CMAKE_GENERATOR}
    -DPRECEPT_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DPRECEPT_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DPRECEPT_BUILD_CONFIG=$<CONFIG>
    -DPRECEPT_MULTI_CONFIG=${precept_multi_config}
    -DPRECEPT_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
    -DPRECEPT_INSTALL_DATADIR=${CMAKE_INSTALL_DATADIR}
    -DPRECEPT_INSTALL_CMAKE_DIR=${precept_cmake_install_dir})

if(CMAKE_GENERATOR_PLATFORM)
  list(APPEND precept_consumer_test_args
       -DPRECEPT_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM})
endif()

if(CMAKE_GENERATOR_TOOLSET)
  list(APPEND precept_consumer_test_args -DPRECEPT_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET})
endif()

add_test(
  NAME precept_consumer_smoke
  COMMAND ${CMAKE_COMMAND} ${precept_consumer_test_args} -P
          ${CMAKE_CURRENT_SOURCE_DIR}/consumer/run_consumer_smoke.cmake)
set_tests_properties(precept_consumer_smoke PROPERTIES TIMEOUT 180)

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
precept_expect_compile_failure(block_span_zero block_span_zero.cpp)
precept_expect_compile_failure(block_span_dynamic_extent block_span_dynamic_extent.cpp)
precept_expect_compile_failure(block_span_fixed_non_divisible block_span_fixed_non_divisible.cpp)
precept_expect_compile_failure(block_span_fixed_try_from block_span_fixed_try_from.cpp)
precept_expect_compile_failure(aligned_ptr_strengthening aligned_ptr_strengthening.cpp)
precept_expect_compile_failure(aligned_ptr_const_removal aligned_ptr_const_removal.cpp)
precept_expect_compile_failure(aligned_ptr_array_covariance aligned_ptr_array_covariance.cpp)
precept_expect_compile_failure(aligned_ptr_non_power_of_two aligned_ptr_non_power_of_two.cpp)
precept_expect_compile_failure(aligned_ptr_below_natural_alignment
                               aligned_ptr_below_natural_alignment.cpp)
precept_expect_compile_failure(aligned_ptr_void aligned_ptr_void.cpp)
precept_expect_compile_failure(aligned_ptr_incomplete aligned_ptr_incomplete.cpp)
precept_expect_compile_failure(aligned_ptr_template_deduction aligned_ptr_template_deduction.cpp)
precept_expect_compile_failure(set_once_temporary_value set_once_temporary_value.cpp)

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
