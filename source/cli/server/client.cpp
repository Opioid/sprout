#include "client.hpp"
#include <iostream>

namespace server {

Client::Client(net::Socket socket) : websocket_(socket) {}

Client::~Client() {}

bool Client::run() {
	if (!websocket_.handshake()) {
		return false;
	}

	thread_ = std::thread(&Client::loop, this);

	return true;
}

void Client::shutdown() {
	websocket_.shutdown();

	thread_.join();
}

bool Client::send(const char* data, size_t size) {
	return websocket_.send(data, size);
}

void Client::loop() {
	std::vector<char> buffer(512);

	std::string text;

	for (;;) {
		int read_bytes = websocket_.receive(buffer.data(), buffer.size());

		// read_bytes >  0 means something was received
		// read_bytes == 0 means the socket is done with receiving
		// read_bytes <  0 means there was some kind of error

		if (read_bytes <= 0) {
			return;
		}

		if (Websocket::is_text(buffer.data(), read_bytes)) {
			Websocket::decode_text(buffer.data(), read_bytes, text);

			std::cout << text << std::endl;
		}
	}
}

}
