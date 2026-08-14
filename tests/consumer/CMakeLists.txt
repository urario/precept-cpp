cmake_minimum_required(VERSION 3.21)

project(PreceptConsumer LANGUAGES CXX)

find_package(Precept 0.1 CONFIG REQUIRED)

add_executable(precept_consumer main.cpp)
target_link_libraries(precept_consumer PRIVATE Precept::Precept)
