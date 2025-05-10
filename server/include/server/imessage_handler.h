#pragma once

#include "common/message.h"
#include <cstdint> // For uint32_t

namespace chat_app {
namespace server {

// Forward declaration
class Server; 
class ClientHandler;

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    // Server pointer might be needed for broadcasting or accessing server state
    // ClientHandler pointer might be needed to send a response directly to the sender
    virtual void handle_message(common::Message& msg, ClientHandler& client_handler, Server& server) = 0;
};

} // namespace server
} // namespace chat_app