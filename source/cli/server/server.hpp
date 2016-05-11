#pragma once

#include "core/exporting/exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"
#include "base/net/socket.hpp"
#include <list>
#include <thread>

namespace server {

class Client;
class Message_handler;

class Server : public exporting::Sink {

public:

	Server(math::int2 dimensions, Message_handler& message_handler);
	~Server();

	void run();
	void shutdown();

	virtual void write(const image::Image_float_4& image, uint32_t frame,
					   thread::Pool& pool) final override;

private:

	void accept_loop();

	Message_handler& message_handler_;

	std::thread accept_thread_;

	net::Socket accept_socket_;

	bool shutdown_;

	std::list<Client*> clients_;

	image::encoding::Srgb_alpha srgb_;
};


}
