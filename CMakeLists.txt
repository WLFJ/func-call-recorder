# set cmake project.
cmake_minimum_required(VERSION 3.26)

project(func-call-recorder)

# add C++23
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_BUILD_TYPE Debug)

# set linker to lld
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

# add compile_command support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(injector)
add_subdirectory(controller)
add_subdirectory(bindclang)
