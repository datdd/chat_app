#pragma once

#include <string>
#include <vector>
#include <cstddef> // For size_t
#include <memory>  // For std::unique_ptr

namespace chat_app {
namespace common {

class ISocket {
public:
    virtual ~ISocket() = default;

    virtual bool connect_socket(const std::string& ip_address, int port) = 0;
    virtual bool bind_socket(int port) = 0;
    virtual bool listen_socket(int backlog) = 0;
    virtual std::unique_ptr<ISocket> accept_socket() = 0;
    virtual int send_data(const std::vector<char>& data) = 0;
    virtual int receive_data(std::vector<char>& buffer, size_t max_len) = 0;
    virtual void close_socket() = 0;
    virtual bool is_valid() const = 0;
    virtual int get_fd() const = 0; // For select/poll if used later
};

} // namespace common
} // namespace chat_app