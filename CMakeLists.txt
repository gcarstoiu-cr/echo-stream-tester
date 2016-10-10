cmake_minimum_required(VERSION 3.1)
project(echo_stream_tester)
set(CMAKE_CXX_STANDARD 11)

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable( echo_stream_tester 
    echo_stream_tester.cpp
)

cmake_policy(SET CMP0015 NEW)
link_directories(
    ../lib
)

target_link_libraries( echo_stream_tester 
    -lpthread
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libnabto_client_api.so
)

