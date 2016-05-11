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

	bool pop_message(std::string& message);

private:

	void loop();

	void push_message(const std::string& message);

	Websocket websocket_;

	std::thread thread_;

	std::vector<std::string> messages_;
};

}
