cmake_minimum_required(VERSION 3.1)
project(echo_stream_tester)
find_package(Threads)

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable( echo_stream_tester 
    echo_stream_tester.cpp
)

target_compile_features(echo_stream_tester PRIVATE cxx_range_for)

link_directories(
    ../lib
)

target_link_libraries( echo_stream_tester 
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libnabto_client_api.so
)