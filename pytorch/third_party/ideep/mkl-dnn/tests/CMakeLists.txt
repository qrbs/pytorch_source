#===============================================================================
# Copyright 2020-2021 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

if(NOT DNNL_GRAPH_BUILD_TESTS)
    return()
endif()

# propagate common flags
append(CMAKE_CXX_FLAGS "${DNNL_GRAPH_COMMON_COMPILATION_FLAGS}")
append(CMAKE_C_FLAGS "${DNNL_GRAPH_COMMON_COMPILATION_FLAGS}")

set(CMAKE_TEST_CCXX_NOWARN_FLAGS)

# propagate no warning flags
append(CMAKE_TEST_CCXX_NOWARN_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")

if(WIN32 AND NOT MINGW)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        # 2415: unused variable
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/Qdiag-disable:2415")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # c4244: conversion with possible loss of data
        # c4996: unsafe / deprecated functions
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/wd4996 /wd4244" "/wd4267")
    endif()
endif()

# propagate TEST specific flags
append(CMAKE_C_FLAGS "${CMAKE_TEST_CCXX_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_TEST_CCXX_FLAGS}")

# propagate TEST specific flags
append(CMAKE_C_FLAGS "${CMAKE_TEST_CCXX_NOWARN_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_TEST_CCXX_NOWARN_FLAGS}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    append_if(WIN32 CMAKE_CXX_FLAGS "/fp:precise")
    append_if(UNIX  CMAKE_CXX_FLAGS "-fp-model precise")
    append(CMAKE_CXX_FLAGS "-Wno-unknown-pragmas")
    append(CMAKE_C_FLAGS "-Wno-unknown-pragmas")
endif()

if(WIN32)
    append(CMAKE_CXX_FLAGS "-DNOMINMAX") # to allow std::max on Windows with parentheses
endif()

add_subdirectory(cpp)
add_subdirectory(benchdnn)
