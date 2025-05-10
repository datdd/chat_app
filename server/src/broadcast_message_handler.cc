#include "server/broadcast_message_handler.h"
#include "server/server.h"       // For Server::broadcast_message
#include "server/client_handler.h" // For ClientHandler
#include "common/message.h"
#include <iostream>

namespace chat_app {
namespace server {

void BroadcastMessageHandler::handle_message(common::Message& msg, ClientHandler& client_handler, Server& server) {
    // For simplicity, we assume TEXT_MESSAGE is for broadcasting.
    // More sophisticated logic would check msg.header.type.
    if (msg.header.type == common::MessageType::TEXT_MESSAGE) {
        std::cout << "Server: Broadcasting message from client " << msg.header.sender_id << std::endl;
        server.broadcast_message(msg, client_handler.get_id()); // Broadcast, optionally excluding sender
    } else {
        std::cerr << "BroadcastMessageHandler: Received unhandled message type: " 
                  << static_cast<int>(msg.header.type) << std::endl;
        // Optionally send an error back to the client
    }
}

} // namespace server
} // namespace chat_app