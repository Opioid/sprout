#pragma once

#include "core/exporting/exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"
#include "base/net/socket.hpp"
#include <list>
#include <thread>

namespace server {

class Websocket;

class Server : public exporting::Sink {

public:

	Server(math::int2 dimensions);
	~Server();

	void run();

	virtual void write(const image::Image_float_4& image, uint32_t frame,
					   thread::Pool& pool) final override;

private:

	void accept_loop();

	std::thread accept_thread_;

	std::list<Websocket*> clients_;

	image::encoding::Srgb_alpha srgb_;
};


}
