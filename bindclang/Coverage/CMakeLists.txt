if(NOT TARGET LLVM)
  find_package(LLVM REQUIRED CONFIG)
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  include(AddLLVM)

  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} at ${LLVM_INCLUDE_DIRS}")

  # if not found, print error message.
  if(NOT LLVM_FOUND)
    message(FATAL_ERROR "LLVM package can't be found.")
  endif()
endif()

add_library(Coverage source/CoverageMapping.cpp)
target_include_directories(
  Coverage
  PUBLIC ./include/
  PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(Coverage PRIVATE LLVM)
