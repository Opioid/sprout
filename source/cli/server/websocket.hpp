#pragma once

#include <string>
#include <vector>
#include "base/net/socket.hpp"

namespace server {

class Websocket {
  public:
    Websocket(net::Socket socket);

    ~Websocket();

    void shutdown();

    bool handshake();

    void ping(std::string const& text);

    int receive(char* data, size_t size);

    bool send(std::string const& text);

    bool send(char const* data, size_t size);

    static std::string handshake_response(char const* header);

    static std::string sec_websocket_accept(char const* header);

    static std::string sec_websocket_key(char const* header);

    static bool is_pong(char const* buffer, size_t size);

    static bool is_text(char const* buffer, size_t size);

    static void decode_text(char const* buffer, size_t size, std::string& text);

  private:
    enum class Opcode {
        Continuation = 0x0,
        Text_frame   = 0x1,
        Binary_frame = 0x2,
        Close        = 8,
        Ping         = 9,
        Pong         = 0xa,
    };

    void prepare_header(size_t payload_length, Opcode opcode);

    net::Socket socket_;

    std::vector<char> buffer_;
};

}  // namespace server
