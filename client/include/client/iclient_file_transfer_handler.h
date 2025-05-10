#pragma once
#include "common/message.h"

namespace chat_app {
namespace client {

class Client; // Forward declaration

class IClientFileTransferHandler {
public:
    virtual ~IClientFileTransferHandler() = default;
    virtual void request_file_transfer(const std::string& recipient_id_str, const std::string& file_path) = 0;
    virtual void handle_message(const common::Message& msg) = 0;
    virtual void set_client_ptr(Client* client_ptr) = 0; // To send messages
};

} // namespace client
} // namespace chat_app