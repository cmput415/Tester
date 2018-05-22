
# Gather our source files in this directory.
set(
  toolchain_src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/Command.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ToolChain.cpp"
)

add_library(toolchain STATIC ${toolchain_src_files})
target_link_libraries(toolchain stdc++fs)
