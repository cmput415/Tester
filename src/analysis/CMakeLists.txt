# Add sub libraries.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/spreadsheet")

# Gather our source files in this directory.
set(
  analysis_src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/Grader.cpp"
)

# Gather the libraries we use for the analysis lib.
set(
  analysis_libs
    testharness
    toolchain
    spreadsheet
)

# Build the library from the source files.
add_library(analysis STATIC ${analysis_src_files})
target_link_libraries(analysis ${analysis_libs})
