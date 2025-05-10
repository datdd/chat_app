#pragma once

#include "common/isocket.h"
#include "client_handler.h" // For ClientHandler
#include "imessage_handler.h" // For IMessageHandler
#include "broadcast_message_handler.h" // Default handler
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory> // For std::unique_ptr
#include <queue>  // For finished_client_ids_

namespace chat_app {
namespace server {

class Server {
public:
    Server(int port);
    ~Server();

    void start();
    void stop();
    bool is_running_properly() const;

    void broadcast_message(const common::Message& msg, uint32_t sender_id_to_exclude = 0);
    void signal_client_finished(uint32_t client_id);

private:
    void accept_connections(); // Thread function for accepting new clients
    void cleanup_clients();    // Thread function for cleaning up disconnected clients
    void remove_client(uint32_t client_id);

    int port_;
    std::unique_ptr<common::ISocket> listen_socket_;
    std::atomic<bool> running_;
    std::atomic<uint32_t> next_client_id_;

    std::thread accept_thread_;
    std::thread cleanup_thread_;

    std::vector<std::unique_ptr<ClientHandler>> clients_;
    std::mutex clients_mutex_; // Protects clients_ vector

    std::queue<uint32_t> finished_client_ids_;
    std::mutex finished_clients_mutex_;
    std::condition_variable finished_clients_cv_;

    BroadcastMessageHandler default_message_handler_; // Example, can be more complex
    // std::unique_ptr<IMessageHandler> message_handler_; // More general
};

} // namespace server
} // namespace chat_app