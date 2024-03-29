#===============================================================================
# Copyright 2021 Intel Corporation
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

set(API_TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_logical_tensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_tensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_op.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_partition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_op.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_add_op.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_compile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_logical_tensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_compile_parametrized.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_api_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_compiled_partition_cache_mt.cpp
    )

add_executable(${LIB_NAME}_api_test ${API_TEST_SRCS})

target_link_libraries(${LIB_NAME}_api_test
    LINK_PRIVATE
    ${DNNL_GRAPH_TEST_LINK_FLAGS}
    ${LIB_NAME}_gtest
    ${LIB_NAME} # Use shared to prevent accidental access to private symbols
    ${DNNL_GRAPH_SYCL_LINK_FLAGS}
    ${DNNL_GRAPH_COMMON_LINK_FLAGS} # FIXME: consider target_link_options from cmake 3.13
    )

target_include_directories(${LIB_NAME}_api_test
    PRIVATE
    ${PROJECT_BINARY_DIR}/third_party/oneDNN/include
    ${PROJECT_SOURCE_DIR}/third_party/oneDNN/include
    ${PROJECT_SOURCE_DIR}/tests # gtest related headers
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    )

if(NOT DNNL_GRAPH_ENABLE_COMPILED_PARTITION_CACHE)
    target_compile_definitions(${LIB_NAME}_api_test
        PRIVATE DNNL_GRAPH_DISABLE_COMPILED_PARTITION_CACHE)
endif()

set(TEST_FILES_NEED_VERSION ${CMAKE_CURRENT_SOURCE_DIR}/test_c_api_version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpp_api_version.cpp)
APPLY_VERSION_DEFINITIONS("${TEST_FILES_NEED_VERSION}")

add_test("${LIB_NAME}_api_test" "${LIB_NAME}_api_test")
maybe_configure_windows_test("${LIB_NAME}_api_test" TEST)
