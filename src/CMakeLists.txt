# Gather our source files in this directory.
set(
  tester_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

# Build our executable from the source files.
add_executable(tester ${tester_src_files})

# Symbolic link our executable to the base directory so we don't have to go searching for it.
symlink_to_bin("tester")
