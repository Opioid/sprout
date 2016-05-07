#include "server.hpp"
#include "websocket.hpp"
#include "base/net/socket.hpp"

#include <vector>
#include <iostream>

namespace server {

Server::Server() {}

Server::~Server() {}

void Server::run() {
	net::Socket listen_socket("8080");

	listen_socket.listen(10);

	net::Socket connection_socket = listen_socket.accept();

	if (!connection_socket.is_valid()) {
		return;
	}

	std::vector<char> buffer(1024);
	int read_bytes = connection_socket.receive(buffer.data(), buffer.size() - 1);
	if (read_bytes < 0) {
		return;
	}

	buffer[read_bytes] = '\0';

	std::cout << "receieved " << read_bytes << std::endl;
	std::cout << buffer.data() << std::endl;

	std::string handshake_response = Websocket::handshake_response(buffer.data());

		std::vector<uint8_t> header;

		size_t message_size = handshake_response.size();

		enum opcode_type {
					CONTINUATION = 0x0,
					TEXT_FRAME = 0x1,
					BINARY_FRAME = 0x2,
					CLOSE = 8,
					PING = 9,
					PONG = 0xa,
				};

		header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0), 0);
		header[0] = 0x80 | opcode_type::TEXT_FRAME;

		if (message_size < 126) {


			header[1] = message_size & 0xff;

		} else if (message_size < 65536) {
			  header[1] = 126;
			  header[2] = (message_size >> 8) & 0xff;
			  header[3] = (message_size >> 0) & 0xff;

		}

		std::vector<char> txbuf;

		txbuf.insert(txbuf.end(), header.begin(), header.end());
		txbuf.insert(txbuf.end(), handshake_response.begin(), handshake_response.end());

	connection_socket.send(txbuf.data(), txbuf.size());
}

}
