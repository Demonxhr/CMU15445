# Install script for directory: C:/Users/34279/Desktop/leveldb/cmu15445/third_party

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/BusTub")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "D:/CLion 2022.3.3/bin/mingw/bin/objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/murmur3/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/libpg_query/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/googletest/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/fmt/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/linenoise/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/libfort/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/argparse/cmake_install.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/third_party/utf8proc/cmake_install.cmake")

endif()

