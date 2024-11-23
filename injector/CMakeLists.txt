# add a shard library target
add_library(Injector SHARED injector.cpp)

# add dependency: fmt
target_link_libraries(Injector fmt)

# copy lib(keep it's pybind target name) into source folder
add_custom_command(
  TARGET Injector
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Injector>
          ${CMAKE_SOURCE_DIR}/pyinjector/lib)
