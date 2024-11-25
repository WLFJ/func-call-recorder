if(NOT TARGET pybind11::module)
  find_package(pybind11 REQUIRED)
endif()

pybind11_add_module(Controller SHARED controller.cpp)

add_custom_command(
  TARGET Controller
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Controller>
          ${CMAKE_SOURCE_DIR}/pyinjector/controller)
