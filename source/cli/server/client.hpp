#pragma once

#include "websocket.hpp"
#include <deque>
#include <thread>

namespace server {

class Client {

public:

	Client(net::Socket socket);
	~Client();

	bool run(std::string const& introduction);
	void shutdown();

	bool send(std::string const& text);
	bool send(const char* data, size_t size);

	bool pop_message(std::string& message);

private:

	void loop();

	void push_message(std::string const& message);

	Websocket websocket_;

	std::thread thread_;

	std::deque<std::string> messages_;
};

}
