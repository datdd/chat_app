#pragma once

#include "isocket.h"
#include <memory>

namespace chat_app {
namespace common {

class SocketFactory {
public:
    static std::unique_ptr<ISocket> create_socket();
};

} // namespace common
} // namespace chat_app