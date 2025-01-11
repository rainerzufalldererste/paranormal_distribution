cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(paranormal_distribution)
add_executable(paranormal_distribution_test paranormal_distribution_test.cpp)

target_include_directories(paranormal_distribution_test PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           )
