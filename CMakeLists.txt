# set cmake project.
cmake_minimum_required(VERSION 3.30)

project(func-call-recorder)

# add C++23
set(CMAKE_CXX_STANDARD 23)

# add compile_command support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(injector)
add_subdirectory(controller)
