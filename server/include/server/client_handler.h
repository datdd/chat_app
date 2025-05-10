#pragma once

#include "common/isocket.h"
#include "common/message.h"
#include "imessage_handler.h" // For IMessageHandler
#include <thread>
#include <atomic>
#include <memory> // For std::unique_ptr
#include <vector> // For internal buffer
#include <mutex>  // For receive_buffer_mutex_

namespace chat_app {
namespace server {

class Server; // Forward declaration

class ClientHandler {
public:
    ClientHandler(uint32_t id, std::unique_ptr<common::ISocket> socket, Server& server_ref, IMessageHandler& msg_handler);
    ~ClientHandler();

    void start();
    void stop(); // Signals the thread to stop and joins it
    void send_message(const common::Message& msg);
    uint32_t get_id() const;
    bool is_running() const;

private:
    void run(); // Thread function

    uint32_t id_;
    std::unique_ptr<common::ISocket> socket_;
    Server& server_; // Reference to the main server
    IMessageHandler& message_handler_; // Reference to the message handler strategy
    
    std::thread thread_;
    std::atomic<bool> running_;

    std::vector<char> receive_buffer_;
    std::mutex receive_buffer_mutex_; // Protects receive_buffer_
};

} // namespace server
} // namespace chat_app