#pragma once

#include "base/net/socket.hpp"
#include <string>
#include <vector>

namespace server {

class Websocket {

public:

	Websocket(const net::Socket& socket);
	~Websocket();

	bool handshake();

	void send(const std::string& text);

	void send(const char* data, size_t size);

	static std::string handshake_response(const char* header);

	static std::string sec_websocket_accept(const char* header);

	static std::string sec_websocket_key(const char* header);

private:

	enum class Opcode {
		Continuation = 0x0,
		Text_frame = 0x1,
		Binary_frame = 0x2,
		Close = 8,
		Ping = 9,
		Pong = 0xa,
	};

	void prepare_header(size_t payload_length, Opcode opcode);

	net::Socket socket_;

	std::vector<char> buffer_;
};

}
