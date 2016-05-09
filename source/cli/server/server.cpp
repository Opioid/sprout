#include "server.hpp"
#include "websocket.hpp"
#include "core/logging/logging.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"
#include "miniz/miniz.hpp"

#include <iostream>

namespace server {

Server::Server(math::int2 dimensions) : srgb_(dimensions) {}

Server::~Server() {
	for (Websocket* c : clients_) {
		delete c;
	}

	net::Socket::release();
}

void Server::run() {
	if (!net::Socket::init()) {
		logging::error("Could not start socket environment.");
		return;
	}

	accept_thread_ = std::thread(&Server::accept_loop, this);
}

void Server::write(const image::Image_float_4& image, uint32_t /*frame*/, thread::Pool& pool) {
	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end) {
		srgb_.to_sRGB(image, begin, end); }, 0, d.x * d.y);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(srgb_.data(), d.x, d.y,
															   4, &buffer_len);

	if (!png_buffer) {
		return;
	}

	for (auto c = clients_.begin(); c != clients_.end();) {
		// Here we are assuming that the client disconnected if the send fails.
		// The nice solution probably is listening for a close message.
		if (!(*c)->send(static_cast<char*>(png_buffer), buffer_len)) {
			delete *c;
			c = clients_.erase(c);
		} else {
			++c;
		}
	}


	mz_free(png_buffer);
}

void Server::accept_loop() {
	net::Socket listen_socket("8080");

	if (!listen_socket.is_valid()) {
		logging::error("Could not establish listen socket.");
		return;
	}

	listen_socket.listen(10);

	for (;;) {
		net::Socket connection_socket = listen_socket.accept();

		if (!connection_socket.is_valid()) {
			continue;
		}

		Websocket* client = new Websocket(connection_socket);

		if (!client->handshake()) {
			delete client;
			continue;
		}

		clients_.push_back(client);
	}
}

}
