#include "socket.hpp"

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <ws2tcpip.h>
#	undef min
#	undef max
#else
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <netdb.h>
#	include <unistd.h>
#endif

#include <cstring>

namespace net {

Socket::Socket() : socket_(Invalid_socket) {}

Socket::Socket(const std::string& service) : socket_(Invalid_socket) {
	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	addrinfo* server_info;
	int error = getaddrinfo(nullptr, service.c_str(), &hints, &server_info);
	if (error != 0) {
		//   fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for (addrinfo* p = server_info; p != nullptr; p = p->ai_next) {
		socket_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (Invalid_socket == socket_) {
			continue;
		}

		const int enable = 1;
		if (-1 == setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
							 reinterpret_cast<const char*>(&enable),
							 sizeof(int))) {
			return;
		}

		if (-1 == bind(socket_, p->ai_addr, static_cast<int>(p->ai_addrlen))) {
			close();
			continue;
		}

		break;
	}

	freeaddrinfo(server_info);
}

Socket::~Socket() {}

bool Socket::is_valid() const {
	return socket_ != Invalid_socket;
}

void Socket::cancel_blocking_accept() {
#ifdef WIN32
	close();
#else
	shutdown();
#endif
}

void Socket::shutdown() const {
#ifdef WIN32
	int how = SD_BOTH;
#else
	int how = SHUT_RDWR;
#endif

	::shutdown(socket_, how);
}

void Socket::close() {
#ifdef WIN32
	closesocket(socket_);
#else
	::close(socket_);
#endif

	socket_ = Invalid_socket;
}

bool Socket::listen(int backlog) const {
	if (::listen(socket_, backlog) == -1) {
		return false;
	}

	return true;
}

Socket Socket::accept() const {
	Socket connection;

	connection.socket_ = ::accept(socket_, nullptr, nullptr);

	return connection;
}

int Socket::receive(char* buffer, uint32_t size) const {
	return recv(socket_, buffer, size, 0);
}

int Socket::send(char* buffer, uint32_t size) const {
#ifdef WIN32
	int flags = 0;
#else
	int flags = MSG_NOSIGNAL;
#endif

	return ::send(socket_, buffer, size, flags);
}

bool Socket::init() {
#ifdef WIN32
	// Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
	WORD version_requested = MAKEWORD(2, 2);
	WSAData wsa_data;
	return 0 == WSAStartup(version_requested, &wsa_data);
#else
	return true;
#endif
}

void Socket::release() {
#ifdef WIN32
	WSACleanup();
#endif
}

}
