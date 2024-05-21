# Gather our source files in this directory.
set(
  tests_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/testRunning.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestFinding.cpp"
)

# Build the library from the source files.
add_library(tests STATIC ${tests_src_files})
target_link_libraries(tests toolchain)
