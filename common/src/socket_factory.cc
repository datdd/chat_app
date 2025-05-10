#include "common/socket_factory.h"

#ifdef _WIN32
#include "winsock_socket.cc" // Include .cc directly for simplicity here, or link separately
#else
#include "posix_socket.cc"   // Include .cc directly for simplicity here, or link separately
#endif

namespace chat_app {
namespace common {

std::unique_ptr<ISocket> SocketFactory::create_socket() {
#ifdef _WIN32
    return std::make_unique<WinsockSocket>();
#else
    return std::make_unique<PosixSocket>();
#endif
}

} // namespace common
} // namespace chat_app