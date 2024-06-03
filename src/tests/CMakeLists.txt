# Gather our source files in this directory.
set(
  tests_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/TestRunning.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestFile.cpp"
)

# Build the library from the source files.
add_library(tests STATIC ${tests_src_files})
target_link_libraries(tests toolchain)
