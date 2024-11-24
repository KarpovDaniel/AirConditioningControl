# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AirConditioningControl_autogen"
  "CMakeFiles\\AirConditioningControl_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\AirConditioningControl_autogen.dir\\ParseCache.txt"
  )
endif()
