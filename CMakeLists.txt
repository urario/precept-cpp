cmake_minimum_required(VERSION 3.21)

project(Precept VERSION 0.1.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(precept INTERFACE)
add_library(Precept::Precept ALIAS precept)

set_target_properties(precept PROPERTIES EXPORT_NAME Precept)

target_compile_features(precept INTERFACE cxx_std_20)
target_include_directories(
  precept
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(precept_cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Precept")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PreceptConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PreceptConfig.cmake"
  INSTALL_DESTINATION "${precept_cmake_install_dir}")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PreceptConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
  ARCH_INDEPENDENT)

install(
  TARGETS precept
  EXPORT PreceptTargets)

install(
  EXPORT PreceptTargets
  FILE PreceptTargets.cmake
  NAMESPACE Precept::
  DESTINATION "${precept_cmake_install_dir}")

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PreceptConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PreceptConfigVersion.cmake"
  DESTINATION "${precept_cmake_install_dir}")

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/Precept")

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)

  if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
  endif()
endif()
