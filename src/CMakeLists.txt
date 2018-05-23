add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/toolchain")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/testharness")

# Gather our source files in this directory.
set(
  tester_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

# Gather the libraries we create for the tester.
set(
  tester_libs
    testharness
)

# Build our executable from the source files.
add_executable(tester ${tester_src_files})

# Link in the filesystem library
target_link_libraries(tester ${tester_libs})

# Symbolic link our executable to the base directory so we don't have to go searching for it.
symlink_to_bin("tester")
