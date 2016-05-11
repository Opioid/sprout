#pragma once

#include "base/net/socket.hpp"
#include <string>
#include <vector>

namespace server {

class Websocket {

public:

	Websocket(net::Socket socket);
	~Websocket();

	void shutdown();

	bool handshake();

	void ping(const std::string& text);

	int receive(char* data, size_t size);

	bool send(const std::string& text);

	bool send(const char* data, size_t size);

	static std::string handshake_response(const char* header);

	static std::string sec_websocket_accept(const char* header);

	static std::string sec_websocket_key(const char* header);

	static bool is_pong(const char* buffer, size_t size);
	static bool is_text(const char* buffer, size_t size);

	static void decode_text(const char* buffer, size_t size, std::string& text);

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
