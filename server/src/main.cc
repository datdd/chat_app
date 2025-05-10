#include "server/server.h"
#include <iostream>
#include <string>
#include <csignal> // For signal handling
#include <memory>  // For std::unique_ptr

std::unique_ptr<chat_app::server::Server> server_instance;

void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    if (server_instance) {
        std::cout << "Shutting down server..." << std::endl;
        server_instance->stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << argv[1] << ". Using default " << port << std::endl;
        }
    }

    signal(SIGINT, signal_handler);  // Handle Ctrl+C
    signal(SIGTERM, signal_handler); // Handle termination signal

    server_instance = std::make_unique<chat_app::server::Server>(port);
    server_instance->start();

    std::cout << "Server is running. Press Ctrl+C to exit." << std::endl;
    
    // Keep main thread alive, server operations are in other threads.
    // The signal handler will take care of shutdown.
    while (true) {
        // Can add a command processing loop here for server commands if needed
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
        if (!server_instance || !server_instance->is_running_properly()) { // Add is_running_properly() to server if needed
             break; // Or if server signals it's no longer running
        }
    }
    
    // Fallback if loop exits for other reasons
    if (server_instance) {
        server_instance->stop();
    }

    return 0;
}

// Add to Server class for the main loop check:
// bool Server::is_running_properly() const {
//     return running_ && listen_socket_ && listen_socket_->is_valid();
// }