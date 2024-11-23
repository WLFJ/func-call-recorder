# import pybind11
find_package(pybind11 REQUIRED)
pybind11_add_module(Controller SHARED
    controller.cpp
)

add_custom_command(
  TARGET Controller
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Controller> ${CMAKE_SOURCE_DIR}/pyinjector/lib)
