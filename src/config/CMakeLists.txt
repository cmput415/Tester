# Gather our source files in this directory.
set(
  config_src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.cpp"
)

# Build the library from the source files.
add_library(config STATIC ${config_src_files})
target_link_libraries(config toolchain)
