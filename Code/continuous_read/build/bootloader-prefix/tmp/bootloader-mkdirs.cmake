# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/esp-idf/components/bootloader/subproject"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/tmp"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/src/bootloader-stamp"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/src"
  "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/hall_sensor_array/Code/continuous_read/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
