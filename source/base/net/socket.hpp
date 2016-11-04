#pragma once

#include <string>
#include <cstdint>

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <winsock2.h>
#	undef min
#	undef max
#endif

namespace net {

class Socket {

public:

	Socket();
	Socket(const std::string& service);
	~Socket();

	bool is_valid() const;

	void cancel_blocking_accept();
	void shutdown() const;
	void close();

	bool listen(int backlog) const;

	Socket accept() const;

	int receive(char* buffer, uint32_t size) const;

	int send(char* buffer, uint32_t size) const;

	static bool init();
	static void release();

public:

#ifdef WIN32
	using Socket_handle = SOCKET;

	static constexpr Socket_handle Invalid_socket = INVALID_SOCKET;
#else
	using Socket_handle = int;

	static constexpr Socket_handle Invalid_socket = -1;
#endif

	Socket_handle socket_;
};

}
