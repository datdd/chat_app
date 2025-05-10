#include "client/client.h"
#include "common/socket_factory.h"
#include "common/message_serialization.h"
#include "client/basic_client_file_transfer_handler.h" // Stub
#include <iostream>
#include <chrono>

namespace chat_app {
namespace client {

Client::Client() : connected_(false), client_id_(0) {
    file_transfer_handler_ = std::make_unique<BasicClientFileTransferHandler>();
    file_transfer_handler_->set_client_ptr(this); // For FT handler to send messages
    std::cout << "Client created." << std::endl;
}

Client::~Client() {
    disconnect();
    std::cout << "Client destroyed." << std::endl;
}

bool Client::connect_to_server(const std::string& ip_address, int port) {
    if (connected_) {
        std::cout << "Client: Already connected." << std::endl;
        return true;
    }

    socket_ = common::SocketFactory::create_socket();
    if (!socket_ || !socket_->connect_socket(ip_address, port)) {
        std::cerr << "Client: Failed to connect to server " << ip_address << ":" << port << std::endl;
        socket_.reset(); // Release socket
        return false;
    }

    connected_ = true;
    receive_thread_ = std::thread(&Client::receive_messages, this);
    send_thread_ = std::thread(&Client::send_messages, this);

    std::cout << "Client: Connected to server " << ip_address << ":" << port << "." << std::endl;
    // Note: Client ID is usually assigned by the server upon connection or login.
    // For this example, we'll assume the first message from the server might contain it,
    // or we can send a HELLO message and get a response. For now, client_id_ remains 0
    // until explicitly set (e.g. by a server message).
    return true;
}

void Client::disconnect() {
    if (!connected_) return;

    std::cout << "Client: Disconnecting..." << std::endl;
    connected_ = false; // Signal threads to stop

    // Notify send_thread to wake up and exit
    send_queue_cv_.notify_one(); 

    if (socket_ && socket_->is_valid()) {
        socket_->close_socket(); // This helps unblock receive_thread_
    }

    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    std::cout << "Client: Receive thread joined." << std::endl;
    
    if (send_thread_.joinable()) {
        send_thread_.join();
    }
    std::cout << "Client: Send thread joined." << std::endl;

    socket_.reset(); // Release socket
    // Clear send queue
    std::lock_guard<std::mutex> lock(send_queue_mutex_);
    std::queue<common::Message> empty;
    std::swap(send_queue_, empty);

    std::cout << "Client: Disconnected." << std::endl;
}

void Client::send_chat_message(const std::string& text) {
    if (!connected_) {
        std::cerr << "Client: Not connected. Cannot send message." << std::endl;
        return;
    }
    common::Message msg(common::MessageType::TEXT_MESSAGE, client_id_.load(), 0, text); // recipient 0 for broadcast to server
    add_message_to_send_queue(std::move(msg));
}

void Client::request_file_transfer(const std::string& recipient_id_str, const std::string& file_path) {
    if (!connected_) {
        std::cerr << "Client: Not connected. Cannot request file transfer." << std::endl;
        return;
    }
    file_transfer_handler_->request_file_transfer(recipient_id_str, file_path);
}


void Client::add_message_to_send_queue(common::Message msg) {
    {
        std::lock_guard<std::mutex> lock(send_queue_mutex_);
        send_queue_.push(std::move(msg));
    }
    send_queue_cv_.notify_one(); // Notify send_thread
}


void Client::receive_messages() {
    std::cout << "Client: Receive thread started." << std::endl;
    std::vector<char> temp_buffer(4096);

    while (connected_) {
        if (!socket_ || !socket_->is_valid()) {
            if(connected_) std::cerr << "Client: Socket became invalid in receive thread." << std::endl;
            connected_ = false;
            break;
        }

        int bytes_received = socket_->receive_data(temp_buffer, temp_buffer.size());

        if (bytes_received < 0) { // Error
            if (connected_) std::cerr << "Client: Receive error. Disconnecting." << std::endl;
            connected_ = false; // Signal other threads
            break;
        }
        if (bytes_received == 0) { // Connection closed by server
            if (connected_) std::cout << "Client: Connection closed by server." << std::endl;
            connected_ = false; // Signal other threads
            break;
        }
        
        { // Append to internal buffer
            std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
            receive_buffer_.insert(receive_buffer_.end(), temp_buffer.begin(), temp_buffer.begin() + bytes_received);
        }

        // Try to process messages from the buffer
        while (connected_) {
             common::Message msg;
            {
                std::lock_guard<std::mutex> lock(receive_buffer_mutex_);
                if (receive_buffer_.size() < common::HEADER_SIZE) break;

                common::MessageHeader temp_header;
                 if (!common::deserialize_header(receive_buffer_, temp_header)) {
                     std::cerr << "Client: Failed to deserialize header from buffer." << std::endl;
                     receive_buffer_.clear(); 
                     break;
                }

                if (receive_buffer_.size() < common::HEADER_SIZE + temp_header.payload_size) {
                    break; 
                }
                msg = common::deserialize_message_from_buffer(receive_buffer_);
            }

            if (msg.header.type == common::MessageType::ERROR_MESSAGE) {
                std::cerr << "Client: Failed to deserialize message from server or incomplete." << std::endl;
                break; 
            }
            process_incoming_message(msg);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // If non-blocking
    }
    std::cout << "Client: Receive thread finished." << std::endl;
    // If disconnected here, ensure main UI loop knows
    connected_ = false; 
    send_queue_cv_.notify_one(); // Wake up send thread if it's waiting, so it can exit
}

void Client::send_messages() {
    std::cout << "Client: Send thread started." << std::endl;
    while (connected_) {
        common::Message msg_to_send;
        {
            std::unique_lock<std::mutex> lock(send_queue_mutex_);
            send_queue_cv_.wait(lock, [this] {
                return !connected_ || !send_queue_.empty();
            });

            if (!connected_ && send_queue_.empty()) { // Check if woken up to exit
                break;
            }
            if (send_queue_.empty()) { // Spurious wakeup or woken to exit but queue is empty
                continue;
            }

            msg_to_send = std::move(send_queue_.front());
            send_queue_.pop();
        } // Mutex released

        if (socket_ && socket_->is_valid()) {
            auto serialized_msg = common::serialize_message(msg_to_send);
            if (socket_->send_data(serialized_msg) <= 0) {
                std::cerr << "Client: Failed to send message. Disconnecting." << std::endl;
                connected_ = false; // Signal other threads
                if (socket_ && socket_->is_valid()) socket_->close_socket(); // Help unblock receive
                break; 
            }
        } else {
            if(connected_) std::cerr << "Client: Socket invalid in send thread." << std::endl;
            connected_ = false;
            break;
        }
    }
    std::cout << "Client: Send thread finished." << std::endl;
}

void Client::process_incoming_message(const common::Message& msg) {
    // Simple console output for now
    std::string payload_str(msg.payload.begin(), msg.payload.end());

    switch (msg.header.type) {
        case common::MessageType::TEXT_MESSAGE:
            std::cout << "\n[" << (msg.header.sender_id == 0 ? "Server" : "User " + std::to_string(msg.header.sender_id))
                      << "]: " << payload_str << std::endl;
            break;
        case common::MessageType::CLIENT_JOINED:
            // If server assigns ID upon join, this is where we might get it.
            // For now, just print the notification.
            // if (client_id_ == 0 && msg.header.recipient_id == SOME_SPECIAL_ID_FOR_SELF) {
            //    client_id_ = msg.header.payload_as_uint32; // Example
            // }
            std::cout << "\n[Notification]: " << payload_str << std::endl;
            break;
        case common::MessageType::CLIENT_LEFT:
            std::cout << "\n[Notification]: " << payload_str << std::endl;
            break;
        case common::MessageType::SERVER_SHUTDOWN:
            std::cout << "\n[Server]: " << payload_str << ". Disconnecting." << std::endl;
            connected_ = false; // Trigger disconnect
            break;
        case common::MessageType::FILE_TRANSFER_REQUEST: // Fallthrough for stubs
        case common::MessageType::FILE_TRANSFER_DATA:
        case common::MessageType::FILE_TRANSFER_ACK:
            file_transfer_handler_->handle_message(msg);
            break;
        case common::MessageType::ERROR_MESSAGE:
             std::cout << "\n[Error from Server]: " << payload_str << std::endl;
             break;
        default:
            std::cout << "\nClient: Received unhandled message type: " << static_cast<int>(msg.header.type) << std::endl;
            break;
    }
    std::cout << "Enter message (or '/quit', '/file <id> <path>'): "; // Re-prompt
    std::cout.flush();
}


} // namespace client
} // namespace chat_app