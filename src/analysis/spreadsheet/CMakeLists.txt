# Gather our source files in this directory.
set(
  spreadsheet_src_files
  "${CMAKE_CURRENT_SOURCE_DIR}/Cell.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Sheet.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Table.cpp"
)

# Build the library from the source files.
add_library(spreadsheet STATIC ${spreadsheet_src_files})
