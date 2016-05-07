#pragma once

#include <string>
#include <cstdint>

namespace net {

class Socket {

public:

	Socket();
	Socket(const std::string& service);
	~Socket();

	bool is_valid() const;

	void close();

	bool listen(int backlog) const;

	Socket accept() const;

	int receive(char* buffer, uint32_t size) const;

	int send(char* buffer, uint32_t size) const;

public:

	using Socket_handle = int;

	Socket_handle socket_;
};

}
