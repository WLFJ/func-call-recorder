if(NOT TARGET pybind11::module)
  find_package(pybind11 REQUIRED)
endif()

# 先将依赖的子模块构建好
add_subdirectory(Coverage)

pybind11_add_module(bindclang SHARED bindclang.cpp)
target_link_libraries(bindclang PRIVATE Coverage)
target_include_directories(bindclang PRIVATE Coverage)

add_custom_command(
  TARGET bindclang
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bindclang>
          ${CMAKE_SOURCE_DIR}/pyinjector/bindclang)
