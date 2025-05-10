#include "server/client_handler.h"
#include "server/server.h" // For Server::signal_client_finished
#include "common/message_serialization.h"
#include <iostream>
#include <chrono>   // For sleep_for

namespace chat_app {
namespace server {

ClientHandler::ClientHandler(uint32_t id, std::unique_ptr<common::ISocket> socket, Server& server_ref, IMessageHandler& msg_handler)
    : id_(id), socket_(std::move(socket)), server_(server_ref), message_handler_(msg_handler), running_(false) {
    std::cout << "ClientHandler " << id_ << " created." << std::endl;
}

ClientHandler::~ClientHandler() {
    stop(); // Ensure thread is stopped and joined
    std::cout << "ClientHandler " << id_ << " destroyed." << std::endl;
    // The document mentions a detach() as a safety net, but it's better to ensure stop() is always called.
    // if (thread_.joinable()) {
    //     std::cerr << "Warning: ClientHandler " << id_ << " destroyed with joinable thread. Detaching." << std::endl;
    //     thread_.detach(); 
    // }
}

void ClientHandler::start() {
    if (running_) return;
    running_ = true;
    thread_ = std::thread(&ClientHandler::run, this);
    std::cout << "ClientHandler " << id_ << " started." << std::endl;
}

void ClientHandler::stop() {
    running_ = false; // Signal thread to stop
    if (socket_ && socket_->is_valid()) {
        socket_->close_socket(); // This can help unblock recv
    }
    if (thread_.joinable()) {
        thread_.join();
    }
    std::cout << "ClientHandler " << id_ << " stopped." << std::endl;
}

void ClientHandler::send_message(const common::Message& msg) {
    if (!socket_ || !socket_->is_valid() || !running_) {
        std::cerr << "ClientHandler " << id_ << ": Cannot send message, socket invalid or not running." << std::endl;
        return;
    }
    auto serialized_msg = common::serialize_message(msg);
    if (socket_->send_data(serialized_msg) <= 0) {
        std::cerr << "ClientHandler " << id_ << ": Failed to send message." << std::endl;
        // Consider this a disconnect
        running_ = false; 
    }
}

uint32_t ClientHandler::get_id() const {
    return id_;
}

bool ClientHandler::is_running() const {
    return running_;
}

void ClientHandler::run() {
    std::cout << "ClientHandler " << id_ << " thread running." << std::endl;
    std::vector<char> temp_buffer(4096); // Temporary buffer for each recv call

    while (running_) {
        if (!socket_ || !socket_->is_valid()) {
            std::cerr << "ClientHandler " << id_ << ": Socket became invalid." << std::endl;
            running_ = false;
            break;
        }

        int bytes_received = socket_->receive_data(temp_buffer, temp_buffer.size());

        if (bytes_received < 0) { // Error
            std::cerr << "ClientHandler " << id_ << ": Receive error. Disconnecting." << std::endl;
            running_ = false;
            break;
        }
        if (bytes_received == 0) { // Connection closed by peer
            std::cout << "ClientHandler " << id_ << ": Connection closed by peer." << std::endl;
            running_ = false;
            break;
        }

        // Append received data to internal buffer
        {
            std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
            receive_buffer_.insert(receive_buffer_.end(), temp_buffer.begin(), temp_buffer.begin() + bytes_received);
        }
        
        // Try to process messages from the buffer
        while (true) {
            common::Message msg;
            {
                std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
                if (receive_buffer_.size() < common::HEADER_SIZE) break; // Not enough for header

                // Peek at header to get payload size
                common::MessageHeader temp_header;
                if (!common::deserialize_header(receive_buffer_, temp_header)) {
                     std::cerr << "ClientHandler " << id_ << ": Failed to deserialize header from buffer." << std::endl;
                     receive_buffer_.clear(); // Corrupted, clear buffer
                     break;
                }

                if (receive_buffer_.size() < common::HEADER_SIZE + temp_header.payload_size) {
                    break; // Not enough for full message yet
                }
                
                // Now deserialize the full message
                msg = common::deserialize_message_from_buffer(receive_buffer_);
            }


            if (msg.header.type == common::MessageType::ERROR_MESSAGE) { // Deserialization failed for a full message
                std::cerr << "ClientHandler " << id_ << ": Failed to deserialize message or incomplete message." << std::endl;
                // If it's truly an error, might disconnect. If incomplete, we'd normally wait.
                // For this simple example, if deserialize_message_from_buffer returns ERROR, we break the inner loop.
                // The outer loop will try to receive more data.
                break; 
            }
            
            // Ensure sender ID is set correctly by the server for messages from this client
            msg.header.sender_id = id_; 
            
            std::cout << "ClientHandler " << id_ << ": Received message of type " 
                      << static_cast<int>(msg.header.type) << " size " << msg.header.payload_size << std::endl;
            message_handler_.handle_message(msg, *this, server_);
        }
         // Small sleep to prevent busy-looping if socket is non-blocking and no data
         // std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    }

    std::cout << "ClientHandler " << id_ << " thread finishing." << std::endl;
    if (socket_ && socket_->is_valid()) {
        socket_->close_socket();
    }
    server_.signal_client_finished(id_);
}

} // namespace server
} // namespace chat_app