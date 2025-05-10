#pragma once

#include "imessage_handler.h"
#include <iostream> // For cout

namespace chat_app {
namespace server {

class BroadcastMessageHandler : public IMessageHandler {
public:
    void handle_message(common::Message& msg, ClientHandler& client_handler, Server& server) override;
};

} // namespace server
} // namespace chat_app