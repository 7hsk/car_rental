# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-src")
  file(MAKE_DIRECTORY "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-build"
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix"
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/tmp"
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/src/cjson-populate-stamp"
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/src"
  "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/src/cjson-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/src/cjson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/mouad/CLionProjects/car_rental/cmake-build-debug/_deps/cjson-subbuild/cjson-populate-prefix/src/cjson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
