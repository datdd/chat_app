cmake_minimum_required(VERSION 3.10)

add_executable(server_app
    src/main.cc
    src/server.cc
    src/client_handler.cc
    src/broadcast_message_handler.cc
)

target_include_directories(server_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/common/include" # To find common headers
)

target_link_libraries(server_app PRIVATE common_lib)

if(NOT WIN32)
    target_link_libraries(server_app PRIVATE Threads::Threads) # For pthreads
endif()