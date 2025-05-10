#include "client/client.h"
#include <iostream>
#include <string>
#include <vector> // For string splitting
#include <sstream> // For string splitting

// Helper to split string by delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


int main(int argc, char* argv[]) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;

    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        try {
            server_port = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << argv[2] << ". Using default " << server_port << std::endl;
        }
    }

    chat_app::client::Client client;

    if (!client.connect_to_server(server_ip, server_port)) {
        return 1;
    }

    std::cout << "Connected to server. Type '/quit' to exit." << std::endl;
    std::cout << "Type '/file <recipient_id> <file_path>' to request a file transfer (stub)." << std::endl;
    std::string line;

    while (true) {
        std::cout << "Enter message (or '/quit', '/file <id> <path>'): ";
        std::getline(std::cin, line);

        if (std::cin.eof() || line == "/quit") {
            break;
        }
        
        if (line.rfind("/file", 0) == 0) { // Check if line starts with /file
            auto parts = split(line, ' ');
            if (parts.size() == 3) {
                client.request_file_transfer(parts[1], parts[2]);
            } else {
                std::cout << "Usage: /file <recipient_id> <file_path>" << std::endl;
            }
        } else if (!line.empty()) {
            client.send_chat_message(line);
        }
        // Check if client got disconnected by server or error
        // This is a bit crude; a better way is an atomic flag or callback
        // For now, rely on threads setting connected_ to false.
        // A small sleep can reduce CPU usage in this simple input loop.
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    client.disconnect();
    std::cout << "Exiting client." << std::endl;
    return 0;
}