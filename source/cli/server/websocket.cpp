#include "websocket.hpp"
#include "base/crypto/base64.hpp"
#include "base/crypto/sha1.hpp"

namespace server {

std::string Websocket::handshake_response(const char* header) {
	std::string key_accept = Websocket::sec_websocket_accept(header);

	return "HTTP/1.0 101 Switching Protocols\r\nContent-Type: application/json; "
		   "charset=utf-8\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
		   "Sec-WebSocket-Accept: " + key_accept + "\r\n\r\n";
}

std::string Websocket::sec_websocket_accept(const char* header) {
	std::string key = sec_websocket_key(header);

	std::string global_uid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	std::vector<uint8_t> hash = crypto::sha1::encode(key + global_uid);

	return crypto::base64::encode(hash.data(), hash.size());
}

std::string Websocket::sec_websocket_key(const char* header) {
	std::string string = header;

	std::string key_label = "Sec-WebSocket-Key: ";
	auto key_iter = string.find(key_label);
	auto key_end_iter = string.find("\r\n", key_iter);
	auto key_start_iter = key_iter + key_label.size();

	return string.substr(key_start_iter, key_end_iter - key_start_iter);
}

}
