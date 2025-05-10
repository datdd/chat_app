// Contents of posix_socket.cc
#include "common/isocket.h"
#include <iostream>
#include <cstring>      // For strerror, memset
#include <unistd.h>     // For close, read, write
#include <sys/socket.h> // For socket, bind, listen, accept, connect
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_pton
#include <fcntl.h>      // For fcntl, O_NONBLOCK

#ifndef _WIN32 // Guard for Posix-specific code

namespace chat_app {
namespace common {

class PosixSocket : public ISocket {
public:
    PosixSocket() : sockfd_(-1) {}
    explicit PosixSocket(int fd) : sockfd_(fd) {} // For accepted sockets

    ~PosixSocket() override {
        close_socket();
    }

    bool connect_socket(const std::string& ip_address, int port) override {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            perror("PosixSocket: socket creation failed");
            return false;
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip_address.c_str(), &serv_addr.sin_addr) <= 0) {
            perror("PosixSocket: invalid address / address not supported");
            close_socket();
            return false;
        }

        if (connect(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("PosixSocket: connection failed");
            close_socket();
            return false;
        }
        // Set non-blocking for receives
        // int flags = fcntl(sockfd_, F_GETFL, 0);
        // fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);
        return true;
    }

    bool bind_socket(int port) override {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            perror("PosixSocket: socket creation failed");
            return false;
        }

        int opt = 1;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("PosixSocket: setsockopt SO_REUSEADDR failed");
            close_socket();
            return false;
        }


        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        if (bind(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("PosixSocket: bind failed");
            close_socket();
            return false;
        }
        return true;
    }

    bool listen_socket(int backlog) override {
        if (listen(sockfd_, backlog) < 0) {
            perror("PosixSocket: listen failed");
            return false;
        }
        return true;
    }

    std::unique_ptr<ISocket> accept_socket() override {
        sockaddr_in cli_addr{};
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd_, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            // Can be non-fatal if server is shutting down
            // perror("PosixSocket: accept failed");
            return nullptr;
        }
        // Set non-blocking for receives on the new socket
        // int flags = fcntl(newsockfd, F_GETFL, 0);
        // fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);
        return std::make_unique<PosixSocket>(newsockfd);
    }

    int send_data(const std::vector<char>& data) override {
        if (sockfd_ < 0 || data.empty()) return -1;
        ssize_t n = send(sockfd_, data.data(), data.size(), 0); // MSG_NOSIGNAL might be useful
        if (n < 0) {
            perror("PosixSocket: send failed");
        }
        return static_cast<int>(n);
    }

    int receive_data(std::vector<char>& buffer, size_t max_len) override {
        if (sockfd_ < 0) return -1;
        buffer.resize(max_len); // Ensure buffer has space
        ssize_t n = recv(sockfd_, buffer.data(), max_len, 0);
        if (n < 0) {
            // EAGAIN or EWOULDBLOCK means no data on non-blocking, not an error
            // if (errno != EAGAIN && errno != EWOULDBLOCK) {
            //     perror("PosixSocket: receive failed");
            // } else {
            //     n = 0; // No data available
            // }
             perror("PosixSocket: receive failed"); // For blocking socket, any error is an issue
        } else if (n == 0) {
            // Connection closed by peer
            return 0;
        }
        buffer.resize(n > 0 ? n : 0); // Resize to actual data received
        return static_cast<int>(n);
    }

    void close_socket() override {
        if (sockfd_ >= 0) {
            close(sockfd_);
            sockfd_ = -1;
        }
    }

    bool is_valid() const override {
        return sockfd_ >= 0;
    }
    
    int get_fd() const override {
        return sockfd_;
    }

private:
    int sockfd_;
};

} // namespace common
} // namespace chat_app

#endif // !_WIN32