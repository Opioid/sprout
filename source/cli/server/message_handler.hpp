#pragma once

#include <string>

namespace server {

class Message_handler {

public:

	virtual void handle(const std::string& message) = 0;

	virtual std::string introduction() const = 0;
};

}
