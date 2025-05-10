#pragma once

#include "common/isocket.h"
#include "common/message.h"
#include "iclient_file_transfer_handler.h" // Stub
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <memory> // For std::unique_ptr

namespace chat_app {
namespace client {

class Client {
public:
    Client();
    ~Client();

    bool connect_to_server(const std::string& ip_address, int port);
    void disconnect();
    void send_chat_message(const std::string& text);
    
    // For file transfer stub
    void request_file_transfer(const std::string& recipient_id_str, const std::string& file_path);


    // For internal use by threads or handlers
    void add_message_to_send_queue(common::Message msg);


private:
    void receive_messages(); // Thread for receiving messages from server
    void send_messages();    // Thread for sending messages from queue

    void process_incoming_message(const common::Message& msg);

    std::unique_ptr<common::ISocket> socket_;
    std::atomic<bool> connected_;
    std::atomic<uint32_t> client_id_; // Assigned by server (or could be part of login)

    std::thread receive_thread_;
    std::thread send_thread_;

    std::queue<common::Message> send_queue_;
    std::mutex send_queue_mutex_;
    std::condition_variable send_queue_cv_;

    std::vector<char> receive_buffer_; // Buffer for incoming data stream
    std::mutex receive_buffer_mutex_;

    std::unique_ptr<IClientFileTransferHandler> file_transfer_handler_; // Stub
};

} // namespace client
} // namespace chat_app