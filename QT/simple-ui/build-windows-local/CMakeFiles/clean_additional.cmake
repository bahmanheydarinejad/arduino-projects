# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\simple-ui_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\simple-ui_autogen.dir\\ParseCache.txt"
  "simple-ui_autogen"
  )
endif()
