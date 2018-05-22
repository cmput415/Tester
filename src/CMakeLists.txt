# Gather our source files in this directory.
set(
  tester_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestHarness.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ToolChain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Command.cpp"
)

# Build our executable from the source files.
add_executable(tester ${tester_src_files})

# Link in the filesystem library
target_link_libraries(tester stdc++fs)

# Symbolic link our executable to the base directory so we don't have to go searching for it.
symlink_to_bin("tester")
