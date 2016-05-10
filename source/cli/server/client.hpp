#pragma once

#include "websocket.hpp"
#include <thread>

namespace server {

class Client {

public:

	Client(net::Socket socket);
	~Client();

	bool run();
	void shutdown();

	bool send(const char* data, size_t size);

private:

	void loop();

	Websocket websocket_;

	std::thread thread_;
};

}
