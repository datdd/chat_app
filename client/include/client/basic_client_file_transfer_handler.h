#pragma once
#include "iclient_file_transfer_handler.h"
#include <iostream>

namespace chat_app {
namespace client {

class BasicClientFileTransferHandler : public IClientFileTransferHandler {
public:
    BasicClientFileTransferHandler() : client_(nullptr) {}
    void request_file_transfer(const std::string& recipient_id_str, const std::string& file_path) override {
        std::cout << "[File Transfer STUB] Requesting transfer of '" << file_path 
                  << "' to client '" << recipient_id_str << "'" << std::endl;
        // In a real implementation, you'd create a FILE_TRANSFER_REQUEST message
        // and send it via client_->send_message_to_queue(...);
    }

    void handle_message(const common::Message& msg) override {
        std::cout << "[File Transfer STUB] Received file transfer message type: " 
                  << static_cast<int>(msg.header.type) << std::endl;
    }
    void set_client_ptr(Client* client_ptr) override {
        client_ = client_ptr;
    }
private:
    Client* client_;
};

} // namespace client
} // namespace chat_app