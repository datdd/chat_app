#include "server/server.h"
#include "common/socket_factory.h"
#include "common/message.h"
#include <iostream>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h> // Ensure Winsock headers are included for Windows
    // If SOMAXCONN is not defined by winsock2.h (it usually isn't directly),
    // we can define a reasonable default or use specific Winsock constants.
    #ifndef SOMAXCONN
        // Winsock uses a different default, SOMAXCONN_HINT_LISTEN_BACKLOG (Windows 10 1607+)
        // or often defaults to 5 for older versions if a large number is passed.
        // Using a common default that works well across platforms.
        #define SOMAXCONN 128 // Or a smaller value like 5 or 10 for basic Windows compatibility
    #endif
#else
    #include <sys/socket.h> // For SOMAXCONN on POSIX
#endif

namespace chat_app {
namespace server {

Server::Server(int port)
    : port_(port), running_(false), next_client_id_(1) {
    listen_socket_ = common::SocketFactory::create_socket();
    // message_handler_ = std::make_unique<BroadcastMessageHandler>(); // If using unique_ptr
    std::cout << "Server created for port " << port_ << "." << std::endl;
}

Server::~Server() {
    stop();
    std::cout << "Server destroyed." << std::endl;
}

void Server::start() {
    if (running_) return;

    if (!listen_socket_ || !listen_socket_->bind_socket(port_)) {
        std::cerr << "Server: Failed to bind to port " << port_ << std::endl;
        return;
    }
    if (!listen_socket_->listen_socket(SOMAXCONN)) { // SOMAXCONN is a common backlog value
        std::cerr << "Server: Failed to listen on socket." << std::endl;
        return;
    }

    running_ = true;
    accept_thread_ = std::thread(&Server::accept_connections, this);
    cleanup_thread_ = std::thread(&Server::cleanup_clients, this);

    std::cout << "Server started and listening on port " << port_ << "." << std::endl;
}

void Server::stop() {
    if (!running_) return;
    running_ = false;

    std::cout << "Server stopping..." << std::endl;

    // Close the listening socket to unblock accept_thread_'s accept call
    if (listen_socket_ && listen_socket_->is_valid()) {
        listen_socket_->close_socket(); 
    }

    // Notify cleanup thread to wake up and exit
    finished_clients_cv_.notify_one();

    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    std::cout << "Accept thread joined." << std::endl;
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    std::cout << "Cleanup thread joined." << std::endl;

    // Stop all client handlers
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& client_handler : clients_) {
        if (client_handler) {
            client_handler->stop();
        }
    }
    clients_.clear(); // This will call destructors of ClientHandler unique_ptrs
    std::cout << "All client handlers stopped and cleared." << std::endl;
    std::cout << "Server stopped." << std::endl;
}

bool Server::is_running_properly() const {
    // running_ is atomic, listen_socket_ is a unique_ptr
    bool socket_ok = false;
    if (listen_socket_) { // Check if unique_ptr is not null
        socket_ok = listen_socket_->is_valid();
    }
    return running_.load() && socket_ok;
}

void Server::accept_connections() {
    std::cout << "Accept thread started." << std::endl;
    while (running_) {
        if (!listen_socket_ || !listen_socket_->is_valid()) {
             if (running_) std::cerr << "Accept thread: Listen socket became invalid." << std::endl;
             break; // Exit if socket is closed (e.g. during shutdown)
        }

        auto client_socket = listen_socket_->accept_socket();
        if (!client_socket || !client_socket->is_valid()) {
            if (running_) { // Only log error if we are supposed to be running
                // This can happen if listen_socket_ is closed during shutdown
                // std::cerr << "Server: Failed to accept new connection or server shutting down." << std::endl;
            }
            // Small pause to prevent busy loop if accept fails repeatedly but server is running
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::cout << "Server: Accepted new connection." << std::endl;
        uint32_t client_id = next_client_id_++;
        
        auto client_handler = std::make_unique<ClientHandler>(client_id, std::move(client_socket), *this, default_message_handler_);
        client_handler->start();

        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.push_back(std::move(client_handler));
        }
        
        // Notify other clients about the new join (optional)
        common::Message join_msg(common::MessageType::CLIENT_JOINED, 0, 0, "Client " + std::to_string(client_id) + " joined.");
        join_msg.header.sender_id = client_id; // Or 0 for server notification
        broadcast_message(join_msg, client_id); // Don't send to the new client itself yet
    }
    std::cout << "Accept thread finished." << std::endl;
}

void Server::broadcast_message(const common::Message& msg, uint32_t sender_id_to_exclude) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    // std::cout << "Server broadcasting message from " << msg.header.sender_id << " (excluding " << sender_id_to_exclude << ")" << std::endl;
    for (const auto& client_handler : clients_) {
        if (client_handler && client_handler->is_running()) {
            if (sender_id_to_exclude == 0 || client_handler->get_id() != sender_id_to_exclude) {
                client_handler->send_message(msg);
            }
        }
    }
}

void Server::signal_client_finished(uint32_t client_id) {
    std::cout << "Server: Client " << client_id << " signaled finished." << std::endl;
    {
        std::lock_guard<std::mutex> lock(finished_clients_mutex_);
        finished_client_ids_.push(client_id);
    }
    finished_clients_cv_.notify_one(); // Notify cleanup thread
}

void Server::cleanup_clients() {
    std::cout << "Cleanup thread started." << std::endl;
    while (running_) {
        std::unique_lock<std::mutex> lock(finished_clients_mutex_);
        // Wait until notified or server is stopping or queue is not empty
        finished_clients_cv_.wait(lock, [this] { 
            return !running_ || !finished_client_ids_.empty(); 
        });

        if (!running_ && finished_client_ids_.empty()) {
            break; // Server is stopping and no more clients to cleanup
        }

        while (!finished_client_ids_.empty()) {
            uint32_t client_id = finished_client_ids_.front();
            finished_client_ids_.pop();
            lock.unlock(); // Unlock while removing client to avoid holding lock too long

            remove_client(client_id);

            lock.lock(); // Re-lock for the loop condition and wait
        }
    }
    std::cout << "Cleanup thread finished." << std::endl;
}

void Server::remove_client(uint32_t client_id) {
    std::cout << "Server: Attempting to remove client " << client_id << std::endl;
    std::unique_ptr<ClientHandler> handler_to_delete = nullptr;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = std::find_if(clients_.begin(), clients_.end(),
                               [client_id](const auto& handler_ptr) {
                                   return handler_ptr && handler_ptr->get_id() == client_id;
                               });

        if (it != clients_.end()) {
            handler_to_delete = std::move(*it); // Move ownership out of vector
            clients_.erase(it);
            std::cout << "Server: Client " << client_id << " removed from active list." << std::endl;
        } else {
            std::cout << "Server: Client " << client_id << " not found for removal (possibly already removed)." << std::endl;
        }
    } // clients_mutex_ released

    if (handler_to_delete) {
        handler_to_delete->stop(); // This joins the thread
        // The unique_ptr will delete the ClientHandler object when it goes out of scope here
        std::cout << "Server: ClientHandler for " << client_id << " stopped and resources released." << std::endl;
        
        // Notify other clients about the departure (optional)
        common::Message leave_msg(common::MessageType::CLIENT_LEFT, 0, 0, "Client " + std::to_string(client_id) + " left.");
        leave_msg.header.sender_id = client_id; // Or 0 for server notification
        broadcast_message(leave_msg);
    }
}

} // namespace server
} // namespace chat_app