#pragma once

#include <vector>
#include <string>
#include <cstdint> // For uint_t types

namespace chat_app {
namespace common {

enum class MessageType : uint8_t {
    TEXT_MESSAGE,
    CLIENT_JOINED,
    CLIENT_LEFT,
    SERVER_SHUTDOWN,
    FILE_TRANSFER_REQUEST, // Stub
    FILE_TRANSFER_DATA,    // Stub
    FILE_TRANSFER_ACK,     // Stub
    ERROR_MESSAGE
};

struct MessageHeader {
    MessageType type;
    uint32_t sender_id;    // 0 for server
    uint32_t recipient_id; // 0 for broadcast or server
    uint32_t payload_size;

    MessageHeader() : type(MessageType::TEXT_MESSAGE), sender_id(0), recipient_id(0), payload_size(0) {}
};

const size_t HEADER_SIZE = sizeof(MessageHeader);

struct Message {
    MessageHeader header;
    std::vector<char> payload;

    Message() = default;
    Message(MessageType type, uint32_t sender, uint32_t recipient, const std::string& text_payload) {
        header.type = type;
        header.sender_id = sender;
        header.recipient_id = recipient;
        payload.assign(text_payload.begin(), text_payload.end());
        header.payload_size = static_cast<uint32_t>(payload.size());
    }
};

} // namespace common
} // namespace chat_app