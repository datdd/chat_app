#pragma once

#include <string>

namespace chat_app {
namespace common {

enum class ErrorCode {
    SUCCESS = 0,

    // Socket Errors
    SOCKET_CREATE_FAILED,
    SOCKET_BIND_FAILED,
    SOCKET_LISTEN_FAILED,
    SOCKET_CONNECT_FAILED,
    SOCKET_ACCEPT_FAILED,
    SOCKET_SEND_FAILED,
    SOCKET_RECEIVE_FAILED,
    SOCKET_CLOSE_FAILED,
    SOCKET_INVALID_ADDRESS,
    SOCKET_OPTION_SET_FAILED,
    SOCKET_INVALID_STATE, // e.g., trying to send on a non-connected socket

    // Message Errors
    MESSAGE_SERIALIZE_FAILED,
    MESSAGE_DESERIALIZE_HEADER_FAILED,
    MESSAGE_DESERIALIZE_PAYLOAD_FAILED,
    MESSAGE_INCOMPLETE, // Not enough data received yet for a full message
    MESSAGE_INVALID_TYPE,
    MESSAGE_PAYLOAD_TOO_LARGE,

    // Server Errors
    SERVER_ALREADY_RUNNING,
    SERVER_NOT_RUNNING,
    SERVER_CLIENT_HANDLER_ERROR,

    // Client Errors
    CLIENT_ALREADY_CONNECTED,
    CLIENT_NOT_CONNECTED,

    // File Transfer Errors (Stub)
    FILE_TRANSFER_OPEN_FAILED,
    FILE_TRANSFER_READ_FAILED,
    FILE_TRANSFER_WRITE_FAILED,

    // General Errors
    THREAD_CREATE_FAILED,
    RESOURCE_NOT_FOUND,
    INVALID_ARGUMENT,
    OPERATION_TIMED_OUT,
    UNKNOWN_ERROR
};

// Helper function to convert ErrorCode to string (optional but good for logging)
std::string error_code_to_string(ErrorCode ec);

// Basic Result struct (could be more advanced with std::expected in C++23)
template<typename T>
struct Result {
    T value;
    ErrorCode error;
    std::string error_message_detail; // Optional detailed OS error message

    Result(T val) : value(val), error(ErrorCode::SUCCESS) {}
    Result(ErrorCode ec, std::string detail = "") : error(ec), error_message_detail(std::move(detail)) {}
    // Result(T val, ErrorCode ec) : value(val), error(ec) {} // If value might be valid on some errors

    bool is_success() const { return error == ErrorCode::SUCCESS; }
    bool is_error() const { return error != ErrorCode::SUCCESS; }

    // explicit operator bool() const { return is_success(); } // For if(result)
};

// Specialization for void results
struct VoidResult {
    ErrorCode error;
    std::string error_message_detail;

    VoidResult() : error(ErrorCode::SUCCESS) {}
    VoidResult(ErrorCode ec, std::string detail = "") : error(ec), error_message_detail(std::move(detail)) {}

    bool is_success() const { return error == ErrorCode::SUCCESS; }
    bool is_error() const { return error != ErrorCode::SUCCESS; }
    // explicit operator bool() const { return is_success(); }
};


} // namespace common
} // namespace chat_app