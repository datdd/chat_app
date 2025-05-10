// Contents of winsock_socket.cc
#include "common/isocket.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton (newer SDKs) or use getaddrinfo
#include <vector>

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib

#ifdef _WIN32 // Guard for Winsock-specific code

namespace chat_app {
namespace common {

class WinsockInitializer {
public:
    WinsockInitializer() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            // Consider throwing an exception or exiting
        }
    }
    ~WinsockInitializer() {
        WSACleanup();
    }
};
// Ensure WSAStartup is called once
static WinsockInitializer ws_initializer;


class WinsockSocket : public ISocket {
public:
    WinsockSocket() : sock_(INVALID_SOCKET) {}
    explicit WinsockSocket(SOCKET s) : sock_(s) {} // For accepted sockets

    ~WinsockSocket() override {
        close_socket();
    }

    bool connect_socket(const std::string& ip_address, int port) override {
        sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "WinsockSocket: socket creation failed: " << WSAGetLastError() << std::endl;
            return false;
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(static_cast<u_short>(port));
        
        // inet_pton is preferred but might require newer SDK or MinGW setup.
        // Fallback to inet_addr if inet_pton is not available easily.
        // For robust solution, use getaddrinfo.
        if (InetPton(AF_INET, ip_address.c_str(), &serv_addr.sin_addr) != 1) {
             // Fallback for older systems or simpler setups
            // serv_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
            // if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
                std::cerr << "WinsockSocket: inet_pton failed for " << ip_address << ": " << WSAGetLastError() << std::endl;
                closesocket(sock_);
                sock_ = INVALID_SOCKET;
                return false;
            // }
        }


        if (connect(sock_, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            std::cerr << "WinsockSocket: connection failed: " << WSAGetLastError() << std::endl;
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
            return false;
        }
        // u_long mode = 1; // 1 to enable non-blocking socket
        // ioctlsocket(sock_, FIONBIO, &mode);
        return true;
    }

    bool bind_socket(int port) override {
        sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "WinsockSocket: socket creation failed: " << WSAGetLastError() << std::endl;
            return false;
        }

        BOOL optval = TRUE;
        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
            std::cerr << "WinsockSocket: setsockopt SO_REUSEADDR failed: " << WSAGetLastError() << std::endl;
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
            return false;
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(static_cast<u_short>(port));

        if (bind(sock_, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            std::cerr << "WinsockSocket: bind failed: " << WSAGetLastError() << std::endl;
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
            return false;
        }
        return true;
    }

    bool listen_socket(int backlog) override {
        if (listen(sock_, backlog) == SOCKET_ERROR) {
            std::cerr << "WinsockSocket: listen failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        return true;
    }

    std::unique_ptr<ISocket> accept_socket() override {
        sockaddr_in cli_addr{};
        int clilen = sizeof(cli_addr);
        SOCKET newsock = accept(sock_, (SOCKADDR*)&cli_addr, &clilen);
        if (newsock == INVALID_SOCKET) {
            // Can be non-fatal if server is shutting down (WSAEINTR)
            // int error = WSAGetLastError();
            // if (error != WSAEINTR && error != WSAECONNABORTED && error != WSAECONNRESET) { // WSAECONNRESET added for robustness
            //    std::cerr << "WinsockSocket: accept failed: " << error << std::endl;
            // }
            return nullptr;
        }
        // u_long mode = 1; // 1 to enable non-blocking socket
        // ioctlsocket(newsock, FIONBIO, &mode);
        return std::make_unique<WinsockSocket>(newsock);
    }

    int send_data(const std::vector<char>& data) override {
        if (sock_ == INVALID_SOCKET || data.empty()) return -1;
        int n = send(sock_, data.data(), static_cast<int>(data.size()), 0);
        if (n == SOCKET_ERROR) {
            std::cerr << "WinsockSocket: send failed: " << WSAGetLastError() << std::endl;
        }
        return n;
    }

    int receive_data(std::vector<char>& buffer, size_t max_len) override {
        if (sock_ == INVALID_SOCKET) return -1;
        buffer.resize(max_len);
        int n = recv(sock_, buffer.data(), static_cast<int>(max_len), 0);
        if (n == SOCKET_ERROR) {
            // int error = WSAGetLastError();
            // if (error != WSAEWOULDBLOCK) { // For non-blocking
            //    std::cerr << "WinsockSocket: receive failed: " << error << std::endl;
            // } else {
            //    n = 0; // No data available
            // }
            std::cerr << "WinsockSocket: receive failed: " << WSAGetLastError() << std::endl;
        } else if (n == 0) {
            // Connection gracefully closed
            return 0;
        }
        buffer.resize(n > 0 ? n : 0);
        return n;
    }

    void close_socket() override {
        if (sock_ != INVALID_SOCKET) {
            shutdown(sock_, SD_BOTH); // Graceful shutdown
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
    }
    bool is_valid() const override {
        return sock_ != INVALID_SOCKET;
    }
    int get_fd() const override { // Less relevant for Winsock's select but can return the SOCKET
        return static_cast<int>(sock_);
    }

private:
    SOCKET sock_;
};

} // namespace common
} // namespace chat_app

#endif // _WIN32