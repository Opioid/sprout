#include "socket.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <cstring>

namespace net {

Socket::Socket() : socket_(-1) {}

Socket::Socket(const std::string& service) {
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
		if (-1 == socket_) {
			   continue;
		}

		int yes = 1;
		if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		//	std::cout << "setsockopt" << std::endl;
			return;
		}

		if (bind(socket_, p->ai_addr, p->ai_addrlen) == -1) {
			close();
			continue;
		}

		break;
	}

	freeaddrinfo(server_info);
}

Socket::~Socket() {}

bool Socket::is_valid() const {
	return socket_ != -1;
}

void Socket::close() {
	::close(socket_);
	socket_ = -1;
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
	return ::send(socket_, buffer, size, 0);
}

}
