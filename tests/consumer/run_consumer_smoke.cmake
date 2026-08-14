cmake_minimum_required(VERSION 3.21)

foreach(required_variable IN ITEMS
        PRECEPT_SOURCE_DIR
        PRECEPT_CONSUMER_SOURCE_DIR
        PRECEPT_TEST_BINARY_DIR
        PRECEPT_GENERATOR
        PRECEPT_INSTALL_INCLUDEDIR
        PRECEPT_INSTALL_DATADIR
        PRECEPT_INSTALL_CMAKE_DIR)
  if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
    message(FATAL_ERROR "${required_variable} must be provided")
  endif()
endforeach()

if(NOT DEFINED PRECEPT_CXX_COMPILER)
  set(PRECEPT_CXX_COMPILER "")
endif()
if(NOT DEFINED PRECEPT_BUILD_TYPE)
  set(PRECEPT_BUILD_TYPE "")
endif()
if(NOT DEFINED PRECEPT_BUILD_CONFIG)
  set(PRECEPT_BUILD_CONFIG "")
endif()
if(NOT DEFINED PRECEPT_MULTI_CONFIG)
  set(PRECEPT_MULTI_CONFIG FALSE)
endif()
if(NOT DEFINED PRECEPT_GENERATOR_PLATFORM)
  set(PRECEPT_GENERATOR_PLATFORM "")
endif()
if(NOT DEFINED PRECEPT_GENERATOR_TOOLSET)
  set(PRECEPT_GENERATOR_TOOLSET "")
endif()

function(run_checked)
  execute_process(
    COMMAND ${ARGV}
    RESULT_VARIABLE command_result
    OUTPUT_VARIABLE command_output
    ERROR_VARIABLE command_error)
  if(NOT command_result EQUAL 0)
    message(
      FATAL_ERROR
        "Command failed with exit code ${command_result}: ${ARGV}\n"
        "stdout:\n${command_output}\n"
        "stderr:\n${command_error}")
  endif()
endfunction()

set(work_dir "${PRECEPT_TEST_BINARY_DIR}")
set(precept_build_dir "${work_dir}/precept-build")
set(staging_dir "${work_dir}/staging")
set(relocated_dir "${work_dir}/relocated")
set(consumer_build_dir "${work_dir}/consumer-build")

file(REMOVE_RECURSE "${work_dir}")
file(MAKE_DIRECTORY "${work_dir}")

set(generator_args -G "${PRECEPT_GENERATOR}")
if(NOT "${PRECEPT_GENERATOR_PLATFORM}" STREQUAL "")
  list(APPEND generator_args -A "${PRECEPT_GENERATOR_PLATFORM}")
endif()
if(NOT "${PRECEPT_GENERATOR_TOOLSET}" STREQUAL "")
  list(APPEND generator_args -T "${PRECEPT_GENERATOR_TOOLSET}")
endif()

set(precept_configure_args
    -S "${PRECEPT_SOURCE_DIR}"
    -B "${precept_build_dir}"
    ${generator_args}
    -DBUILD_TESTING=OFF
    -DCMAKE_INSTALL_PREFIX=${staging_dir})

if(NOT "${PRECEPT_BUILD_TYPE}" STREQUAL "")
  list(APPEND precept_configure_args -DCMAKE_BUILD_TYPE=${PRECEPT_BUILD_TYPE})
endif()

if(NOT "${PRECEPT_CXX_COMPILER}" STREQUAL ""
   AND NOT "${PRECEPT_GENERATOR}" MATCHES "Visual Studio")
  list(APPEND precept_configure_args -DCMAKE_CXX_COMPILER=${PRECEPT_CXX_COMPILER})
endif()

run_checked("${CMAKE_COMMAND}" ${precept_configure_args})

set(precept_build_args --build "${precept_build_dir}")
if(PRECEPT_MULTI_CONFIG)
  if("${PRECEPT_BUILD_CONFIG}" STREQUAL "")
    set(PRECEPT_BUILD_CONFIG Debug)
  endif()
  list(APPEND precept_build_args --config "${PRECEPT_BUILD_CONFIG}")
endif()
run_checked("${CMAKE_COMMAND}" ${precept_build_args})

set(precept_install_args --install "${precept_build_dir}" --prefix "${staging_dir}")
if(PRECEPT_MULTI_CONFIG)
  list(APPEND precept_install_args --config "${PRECEPT_BUILD_CONFIG}")
endif()
run_checked("${CMAKE_COMMAND}" ${precept_install_args})

if(NOT EXISTS "${staging_dir}")
  message(FATAL_ERROR "Expected install staging directory does not exist: ${staging_dir}")
endif()
file(RENAME "${staging_dir}" "${relocated_dir}")
if(EXISTS "${staging_dir}")
  message(FATAL_ERROR "Install tree was not relocated away from staging")
endif()

foreach(header IN ITEMS
        at_least_span.hpp
        block_span.hpp
        checked_span.hpp
        non_empty_span.hpp)
  if(NOT EXISTS "${relocated_dir}/${PRECEPT_INSTALL_INCLUDEDIR}/precept/span/${header}")
    message(FATAL_ERROR "Installed public header is missing: ${header}")
  endif()
endforeach()

foreach(package_file IN ITEMS
        PreceptConfig.cmake
        PreceptConfigVersion.cmake
        PreceptTargets.cmake)
  if(NOT EXISTS "${relocated_dir}/${PRECEPT_INSTALL_CMAKE_DIR}/${package_file}")
    message(FATAL_ERROR "Installed package metadata is missing: ${package_file}")
  endif()
endforeach()

if(NOT EXISTS "${relocated_dir}/${PRECEPT_INSTALL_DATADIR}/licenses/Precept/LICENSE")
  message(FATAL_ERROR "Installed license file is missing")
endif()

file(GLOB_RECURSE installed_cmake_files "${relocated_dir}/*.cmake")
if(NOT installed_cmake_files)
  message(FATAL_ERROR "No installed CMake package metadata was found")
endif()

set(forbidden_paths "${PRECEPT_SOURCE_DIR}" "${precept_build_dir}" "${staging_dir}")
foreach(forbidden_path IN LISTS forbidden_paths)
  file(TO_CMAKE_PATH "${forbidden_path}" normalized_forbidden_path)
  string(TOLOWER "${normalized_forbidden_path}" normalized_forbidden_path)
  list(APPEND normalized_forbidden_paths "${normalized_forbidden_path}")
endforeach()

foreach(installed_cmake_file IN LISTS installed_cmake_files)
  file(READ "${installed_cmake_file}" installed_cmake_content)
  string(TOLOWER "${installed_cmake_content}" normalized_cmake_content)
  foreach(forbidden_path IN LISTS normalized_forbidden_paths)
    string(FIND "${normalized_cmake_content}" "${forbidden_path}" forbidden_path_index)
    if(NOT forbidden_path_index EQUAL -1)
      message(
        FATAL_ERROR
          "Installed CMake metadata contains a forbidden absolute path (${forbidden_path}): "
          "${installed_cmake_file}")
    endif()
  endforeach()
endforeach()

set(consumer_configure_args
    -S "${PRECEPT_CONSUMER_SOURCE_DIR}"
    -B "${consumer_build_dir}"
    ${generator_args}
    -DCMAKE_PREFIX_PATH=${relocated_dir}
    -DPrecept_DIR=${relocated_dir}/${PRECEPT_INSTALL_CMAKE_DIR}
    -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
    -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
    -DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=OFF
    -DCMAKE_FIND_USE_PACKAGE_ROOT_PATH=OFF
    -DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=OFF)

if(NOT "${PRECEPT_BUILD_TYPE}" STREQUAL "")
  list(APPEND consumer_configure_args -DCMAKE_BUILD_TYPE=${PRECEPT_BUILD_TYPE})
endif()

if(NOT "${PRECEPT_CXX_COMPILER}" STREQUAL ""
   AND NOT "${PRECEPT_GENERATOR}" MATCHES "Visual Studio")
  list(APPEND consumer_configure_args -DCMAKE_CXX_COMPILER=${PRECEPT_CXX_COMPILER})
endif()

run_checked("${CMAKE_COMMAND}" ${consumer_configure_args})

set(consumer_cache_file "${consumer_build_dir}/CMakeCache.txt")
if(NOT EXISTS "${consumer_cache_file}")
  message(FATAL_ERROR "Consumer CMake cache was not produced")
endif()
file(TO_CMAKE_PATH "${relocated_dir}/${PRECEPT_INSTALL_CMAKE_DIR}" expected_precept_dir)
string(TOLOWER "${expected_precept_dir}" expected_precept_dir)
file(READ "${consumer_cache_file}" consumer_cache_content)
string(REPLACE "\n" ";" consumer_cache_lines "${consumer_cache_content}")
set(found_relocated_precept_dir FALSE)
foreach(consumer_cache_line IN LISTS consumer_cache_lines)
  string(TOLOWER "${consumer_cache_line}" normalized_consumer_cache_line)
  if(normalized_consumer_cache_line MATCHES "^precept_dir:[^=]*=")
    string(REGEX REPLACE "^[^=]*=" "" actual_precept_dir "${normalized_consumer_cache_line}")
    if("${actual_precept_dir}" STREQUAL "${expected_precept_dir}")
      set(found_relocated_precept_dir TRUE)
    endif()
  endif()
endforeach()
if(NOT found_relocated_precept_dir)
  message(FATAL_ERROR "Consumer did not configure against the relocated Precept_DIR")
endif()

set(consumer_build_args --build "${consumer_build_dir}")
if(PRECEPT_MULTI_CONFIG)
  list(APPEND consumer_build_args --config "${PRECEPT_BUILD_CONFIG}")
endif()
run_checked("${CMAKE_COMMAND}" ${consumer_build_args})

set(consumer_executable "${consumer_build_dir}/precept_consumer")
if(PRECEPT_MULTI_CONFIG)
  set(consumer_executable "${consumer_build_dir}/${PRECEPT_BUILD_CONFIG}/precept_consumer")
endif()
if(WIN32)
  set(consumer_executable "${consumer_executable}.exe")
endif()

if(NOT EXISTS "${consumer_executable}")
  message(FATAL_ERROR "Consumer executable was not produced: ${consumer_executable}")
endif()
run_checked("${consumer_executable}")
