# Function that creates a custom target that depends on a target of your choosing and symlinks its
# result to a new folder in the main source tree called "bin" so that target executables (or
# libraries, etc.) are easily accessible.

function(symlink_to_bin target)
  if(WIN32)
    # https://cmake.org/cmake/help/v3.4/manual/cmake.1.html#unix-specific-command-line-tools
    # There is a tool to make symlinks on Windows (mklink) but we'd have to do some hacky stuff to
    # run just Windows commands.
    message(WARN "CMake does not support generating symlinks on Windows.")
  else()
    message(STATUS "Generating custom command for symlinking ${target}.")
    add_custom_target(
      "symlink_${target}" ALL
      DEPENDS ${target}
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E create_symlink
        "$<TARGET_FILE:${target}>"
        "${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:${target}>"
      COMMENT
        "Symlinking target ${target} to ${CMAKE_SOURCE_DIR}/bin"
    )
  endif()
endfunction(symlink_to_bin)

