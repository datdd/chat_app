#include "common/message_serialization.h"
#include <cstring> // For memcpy
#include <algorithm> // for std::copy, std::min
#include <iostream>  // For debug

namespace chat_app {
namespace common {

std::vector<char> serialize_message(const Message& msg) {
    std::vector<char> buffer(HEADER_SIZE + msg.payload.size());
    // Serialize header
    std::memcpy(buffer.data(), &msg.header, HEADER_SIZE);
    // Serialize payload
    if (!msg.payload.empty()) {
        std::memcpy(buffer.data() + HEADER_SIZE, msg.payload.data(), msg.payload.size());
    }
    return buffer;
}

bool deserialize_header(const std::vector<char>& buffer, MessageHeader& header) {
    if (buffer.size() < HEADER_SIZE) {
        return false; // Not enough data for a header
    }
    std::memcpy(&header, buffer.data(), HEADER_SIZE);
    return true;
}


Message deserialize_message_from_buffer(std::vector<char>& buffer) {
    Message msg;
    if (!deserialize_header(buffer, msg.header)) {
        msg.header.type = MessageType::ERROR_MESSAGE; // Indicate failure
        return msg;
    }

    if (msg.header.payload_size > 0) {
        if (buffer.size() < HEADER_SIZE + msg.header.payload_size) {
            // Not enough data for payload yet
            // This indicates an incomplete message in the buffer,
            // The caller should wait for more data.
            // For simplicity here, we'll return an error message.
            // In a real scenario, you'd buffer this and wait.
            // std::cerr << "Incomplete message: need " << msg.header.payload_size << " payload bytes, have " << (buffer.size() - HEADER_SIZE) << std::endl;
            msg.header.type = MessageType::ERROR_MESSAGE;
            return msg;
        }
        msg.payload.resize(msg.header.payload_size);
        std::memcpy(msg.payload.data(), buffer.data() + HEADER_SIZE, msg.header.payload_size);
    }

    // Remove the processed message from the beginning of the buffer
    size_t total_message_size = HEADER_SIZE + msg.header.payload_size;
    if (buffer.size() >= total_message_size) {
        buffer.erase(buffer.begin(), buffer.begin() + total_message_size);
    } else {
        // Should not happen if checks above are correct
        buffer.clear();
        msg.header.type = MessageType::ERROR_MESSAGE; // Should not happen
    }

    return msg;
}


} // namespace common
} // namespace chat_app