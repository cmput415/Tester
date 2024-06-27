# Add sublibraries.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/analysis")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/testharness")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/toolchain")

# Gather our source files in this directory.
set(
  tester_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

# Gather the libraries we create for the tester.
set(
  tester_libs
    analysis
    config
    testharness
    tests
)

# Build our executable from the source files.
add_executable(tester ${tester_src_files})

# Link in the filesystem library
target_link_libraries(tester ${tester_libs})