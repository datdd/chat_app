#include "common/error_codes.h"

namespace chat_app {
namespace common {

std::string error_code_to_string(ErrorCode ec) {
    switch (ec) {
        case ErrorCode::SUCCESS: return "SUCCESS";
        case ErrorCode::SOCKET_CREATE_FAILED: return "SOCKET_CREATE_FAILED";
        case ErrorCode::SOCKET_BIND_FAILED: return "SOCKET_BIND_FAILED";
        case ErrorCode::SOCKET_LISTEN_FAILED: return "SOCKET_LISTEN_FAILED";
        case ErrorCode::SOCKET_CONNECT_FAILED: return "SOCKET_CONNECT_FAILED";
        case ErrorCode::SOCKET_ACCEPT_FAILED: return "SOCKET_ACCEPT_FAILED";
        case ErrorCode::SOCKET_SEND_FAILED: return "SOCKET_SEND_FAILED";
        case ErrorCode::SOCKET_RECEIVE_FAILED: return "SOCKET_RECEIVE_FAILED";
        case ErrorCode::SOCKET_CLOSE_FAILED: return "SOCKET_CLOSE_FAILED";
        case ErrorCode::SOCKET_INVALID_ADDRESS: return "SOCKET_INVALID_ADDRESS";
        case ErrorCode::SOCKET_OPTION_SET_FAILED: return "SOCKET_OPTION_SET_FAILED";
        case ErrorCode::SOCKET_INVALID_STATE: return "SOCKET_INVALID_STATE";
        case ErrorCode::MESSAGE_SERIALIZE_FAILED: return "MESSAGE_SERIALIZE_FAILED";
        case ErrorCode::MESSAGE_DESERIALIZE_HEADER_FAILED: return "MESSAGE_DESERIALIZE_HEADER_FAILED";
        case ErrorCode::MESSAGE_DESERIALIZE_PAYLOAD_FAILED: return "MESSAGE_DESERIALIZE_PAYLOAD_FAILED";
        case ErrorCode::MESSAGE_INCOMPLETE: return "MESSAGE_INCOMPLETE";
        case ErrorCode::MESSAGE_INVALID_TYPE: return "MESSAGE_INVALID_TYPE";
        case ErrorCode::MESSAGE_PAYLOAD_TOO_LARGE: return "MESSAGE_PAYLOAD_TOO_LARGE";
        case ErrorCode::SERVER_ALREADY_RUNNING: return "SERVER_ALREADY_RUNNING";
        case ErrorCode::SERVER_NOT_RUNNING: return "SERVER_NOT_RUNNING";
        case ErrorCode::SERVER_CLIENT_HANDLER_ERROR: return "SERVER_CLIENT_HANDLER_ERROR";
        case ErrorCode::CLIENT_ALREADY_CONNECTED: return "CLIENT_ALREADY_CONNECTED";
        case ErrorCode::CLIENT_NOT_CONNECTED: return "CLIENT_NOT_CONNECTED";
        case ErrorCode::FILE_TRANSFER_OPEN_FAILED: return "FILE_TRANSFER_OPEN_FAILED";
        case ErrorCode::FILE_TRANSFER_READ_FAILED: return "FILE_TRANSFER_READ_FAILED";
        case ErrorCode::FILE_TRANSFER_WRITE_FAILED: return "FILE_TRANSFER_WRITE_FAILED";
        case ErrorCode::THREAD_CREATE_FAILED: return "THREAD_CREATE_FAILED";
        case ErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
        case ErrorCode::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case ErrorCode::OPERATION_TIMED_OUT: return "OPERATION_TIMED_OUT";
        case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
        default: return "UNMAPPED_ERROR_CODE";
    }
}

} // namespace common
} // namespace chat_app