#pragma once

#include "message.h"
#include <vector>

namespace chat_app {
namespace common {

// Serializes a Message object into a byte vector
std::vector<char> serialize_message(const Message& msg);

// Deserializes a byte vector (header part) into a MessageHeader
// Returns false if not enough data for header
bool deserialize_header(const std::vector<char>& buffer, MessageHeader& header);

// Deserializes a complete message from a raw buffer.
// Assumes buffer starts with a complete header.
// Modifies buffer by removing the processed message data.
// Returns an empty Message with an invalid type if deserialization fails.
Message deserialize_message_from_buffer(std::vector<char>& buffer);

} // namespace common
} // namespace chat_app