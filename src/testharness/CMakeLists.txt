set(
  testharness_src_files
    "${CMAKE_CURRENT_SOURCE_DIR}/TestHarness.cpp"
)


add_library(testharness STATIC ${testharness_src_files})
target_link_libraries(testharness toolchain stdc++fs)
