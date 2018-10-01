# Gather our source files in this directory.
set(
  testharness_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/TestHarness.cpp"
)

# Gather the libs we use for the testharness lib.
set(
  testharness_libs
    config
    toolchain
    tests
    stdc++fs
)

# Build the library from the source files.
add_library(testharness STATIC ${testharness_src_files})
target_link_libraries(testharness ${testharness_libs})
