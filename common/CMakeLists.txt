cmake_minimum_required(VERSION 3.10)

add_library(common_lib STATIC
    # Headers are implicitly included by being in include paths
    # Or explicitly list them if you prefer, but not strictly needed for compilation here
    # "include/common/isocket.h"
    # "include/common/message.h"
    # "include/common/message_serialization.h"
    # "include/common/socket_factory.h"
    src/message_serialization.cc
    src/socket_factory.cc 
    src/error_codes.cc
    # posix_socket.cc and winsock_socket.cc are #included by socket_factory.cc
    # If you compile them separately:
    # $<IF:$<PLATFORM_ID:Windows>,src/winsock_socket.cc,src/posix_socket.cc>
)

target_include_directories(common_lib PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/common/include" # So server/client can find common/isocket.h etc.
)

if(WIN32)
    target_link_libraries(common_lib PRIVATE ws2_32) # For Winsock
endif()

# If posix_socket.cc and winsock_socket.cc were compiled separately:
# target_sources(common_lib PRIVATE
#     src/message_serialization.cc
#     src/socket_factory.cc
# )
# if(WIN32)
#     target_sources(common_lib PRIVATE src/winsock_socket.cc)
#     target_link_libraries(common_lib PRIVATE ws2_32) # For Winsock
# else()
#     target_sources(common_lib PRIVATE src/posix_socket.cc)
#     # No special link for POSIX sockets, covered by standard libraries
# endif()