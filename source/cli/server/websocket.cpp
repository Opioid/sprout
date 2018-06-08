#include "websocket.hpp"
#include "base/crypto/base64.hpp"
#include "base/crypto/sha1.hpp"

namespace server {

Websocket::Websocket(net::Socket socket) : socket_(socket) {
    buffer_.reserve(1024);
}

Websocket::~Websocket() {
    socket_.close();
}

void Websocket::shutdown() {
    //	socket_.cancel_blocking();

    socket_.shutdown();

    //	socket_.close();
}

bool Websocket::handshake() {
    buffer_.resize(1024);

    int read_bytes = socket_.receive(buffer_.data(), static_cast<uint32_t>(buffer_.size() - 1));
    if (read_bytes < 0) {
        return false;
    }

    buffer_[read_bytes] = '\0';

    std::string response = handshake_response(buffer_.data());
    send(response);

    return true;
}

void Websocket::ping(std::string const& text) {
    prepare_header(text.size(), Opcode::Ping);

    buffer_.insert(buffer_.end(), text.begin(), text.end());

    socket_.send(buffer_.data(), static_cast<uint32_t>(buffer_.size()));
}

int Websocket::receive(char* data, size_t size) {
    return socket_.receive(data, static_cast<uint32_t>(size));
}

bool Websocket::send(std::string const& text) {
    prepare_header(text.size(), Opcode::Text_frame);

    buffer_.insert(buffer_.end(), text.begin(), text.end());

    return socket_.send(buffer_.data(), static_cast<uint32_t>(buffer_.size())) >= 0;
}

bool Websocket::send(const char* data, size_t size) {
    prepare_header(size, Opcode::Binary_frame);

    buffer_.insert(buffer_.end(), data, data + size);

    return socket_.send(buffer_.data(), static_cast<uint32_t>(buffer_.size())) >= 0;
}

std::string Websocket::handshake_response(const char* header) {
    std::string key_accept = Websocket::sec_websocket_accept(header);

    return "HTTP/1.0 101 Switching Protocols\r\n"
           "Content-Type: application/json; charset=utf-8\r\n"
           "Upgrade: websocket\r\n"
           "Connection: Upgrade\r\n"
           "Sec-WebSocket-Accept: " +
           key_accept + "\r\n\r\n";
}

std::string Websocket::sec_websocket_accept(const char* header) {
    std::string key = sec_websocket_key(header);

    std::string global_uid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::vector<uint8_t> hash = crypto::sha1::encode(key + global_uid);

    return crypto::base64::encode(hash.data(), hash.size());
}

std::string Websocket::sec_websocket_key(const char* header) {
    std::string string = header;

    std::string key_label      = "Sec-WebSocket-Key: ";
    auto        key_iter       = string.find(key_label);
    auto        key_end_iter   = string.find("\r\n", key_iter);
    auto        key_start_iter = key_iter + key_label.size();

    return string.substr(key_start_iter, key_end_iter - key_start_iter);
}

bool Websocket::is_pong(const char* buffer, size_t size) {
    if (!size) {
        return false;
    }

    return (0x80 | static_cast<char>(Opcode::Pong)) == (buffer[0] & 0xff);
}

bool Websocket::is_text(const char* buffer, size_t size) {
    if (!size) {
        return false;
    }

    return (0x80 | static_cast<char>(Opcode::Text_frame)) == (buffer[0] & 0xff);
}

void Websocket::decode_text(const char* buffer, size_t size, std::string& text) {
    uint64_t payload_length = 0;

    bool masked = (buffer[1] & 0x80) == 0x80;

    uint32_t mask_start = 0;

    uint32_t const length_marker = buffer[1] & 0x7f;
    if (length_marker < 126) {
        payload_length = length_marker;
        mask_start     = 2;
    } else if (length_marker == 126) {
        payload_length = buffer[2] << 8 | buffer[3];
        mask_start     = 4;
    }

    if (payload_length > size) {
        // to really support websockets we need a mechanism
        // to decode data which is larger than the current buffer
        text.clear();
        return;
    }

    char     mask[4];
    uint32_t payload_start = 0;

    if (masked) {
        mask[0] = buffer[mask_start + 0];
        mask[1] = buffer[mask_start + 1];
        mask[2] = buffer[mask_start + 2];
        mask[3] = buffer[mask_start + 3];

        payload_start = mask_start + 4;
    } else {
        mask[0] = 0;
        mask[1] = 0;
        mask[2] = 0;
        mask[3] = 0;

        payload_start = mask_start;
    }

    text.clear();

    for (uint64_t i = 0; i < payload_length; ++i) {
        uint64_t index = payload_start + i;
        text.insert(text.end(), buffer[index] ^ mask[i % 4]);
    }
}

void Websocket::prepare_header(size_t payload_length, Opcode opcode) {
    buffer_.clear();

    buffer_.resize(2 + (payload_length >= 126 ? 2 : 0) + (payload_length >= 65536 ? 6 : 0));

    buffer_[0] = 0x80 | static_cast<char>(opcode);

    if (payload_length < 126) {
        buffer_[1] = payload_length & 0xff;
    } else if (payload_length < 65536) {
        buffer_[1] = 126;
        buffer_[2] = (payload_length >> 8) & 0xff;
        buffer_[3] = (payload_length >> 0) & 0xff;
    } else {
        buffer_[1] = 127;
        buffer_[2] = (payload_length >> 56) & 0xff;
        buffer_[3] = (payload_length >> 48) & 0xff;
        buffer_[4] = (payload_length >> 40) & 0xff;
        buffer_[5] = (payload_length >> 32) & 0xff;
        buffer_[6] = (payload_length >> 24) & 0xff;
        buffer_[7] = (payload_length >> 16) & 0xff;
        buffer_[8] = (payload_length >> 8) & 0xff;
        buffer_[9] = (payload_length >> 0) & 0xff;
    }
}

}  // namespace server
