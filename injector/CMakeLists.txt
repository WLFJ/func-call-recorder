find_package(nlohmann_json 3.11.3 REQUIRED)
add_library(Injector SHARED injector.cpp)
target_link_libraries(Injector PRIVATE fmt nlohmann_json::nlohmann_json)

# copy lib(keep it's pybind target name) into source folder
add_custom_command(
  TARGET Injector
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Injector>
          ${CMAKE_SOURCE_DIR}/pyinjector/lib)
