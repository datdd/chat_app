cmake_minimum_required(VERSION 3.10)

add_executable(client_app
    src/main.cc
    src/client.cc
    src/basic_client_file_transfer_handler.cc # Stub
)

target_include_directories(client_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/common/include" # To find common headers
)

target_link_libraries(client_app PRIVATE common_lib)

if(NOT WIN32)
    target_link_libraries(client_app PRIVATE Threads::Threads) # For pthreads
endif()