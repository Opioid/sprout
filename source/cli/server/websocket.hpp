#pragma once

#include <string>

namespace server {

class Websocket {

public:

	static std::string handshake_response(const char* header);

	static std::string sec_websocket_accept(const char* header);

	static std::string sec_websocket_key(const char* header);

};

}
