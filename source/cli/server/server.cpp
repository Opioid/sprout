#include "server.hpp"
#include "websocket.hpp"
#include "base/net/socket.hpp"
#include "base/thread/thread_pool.hpp"

#include <vector>
#include <iostream>



#include "base/math/vector.inl"
#include "miniz/miniz.hpp"


namespace server {

Server::Server(math::int2 dimensions) : srgb_(dimensions) {}

Server::~Server() {
	for (Websocket* c : clients_) {
		delete c;
	}
}

void Server::run() {
	net::Socket listen_socket("8080");

	listen_socket.listen(10);

	net::Socket connection_socket = listen_socket.accept();

	if (!connection_socket.is_valid()) {
		return;
	}

	Websocket* client = new Websocket(connection_socket);

	if (!client->handshake()) {
		return;
	}

	math::int2 dimensions(512, 512);
	math::byte3* rgb = new math::byte3[dimensions.x * dimensions.y];

	for (int32_t y = 0; y < dimensions.y; ++y) {
		for (int32_t x = 0; x < dimensions.x; ++x) {
			auto& pixel = rgb[y * dimensions.x + x];

			pixel.x = static_cast<uint8_t>(255.f * static_cast<float>(x) / static_cast<float>(dimensions.x - 1));
			pixel.y = static_cast<uint8_t>(255.f * static_cast<float>(y) / static_cast<float>(dimensions.y - 1));
			pixel.z = 127;
		}
	}

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgb, dimensions.x, dimensions.y,
															   3, &buffer_len);

	if (!png_buffer) {
		delete [] rgb;
		return;
	}

	client->send(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	delete [] rgb;

	clients_.push_back(client);
}

void Server::write(const image::Image_float_4& image, uint32_t /*frame*/, thread::Pool& pool) {
	auto d = image.description().dimensions;
	pool.run_range([this, &image](uint32_t begin, uint32_t end) {
		srgb_.to_sRGB(image, begin, end); }, 0, d.x * d.y);

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(srgb_.rgba(), d.x, d.y,
															   4, &buffer_len);

	if (!png_buffer) {
		return;
	}

	for (Websocket* c : clients_) {
		c->send(static_cast<char*>(png_buffer), buffer_len);
	}

	mz_free(png_buffer);
}

void Server::accept_loop() {

}

}
