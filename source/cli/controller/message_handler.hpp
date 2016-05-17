#pragma once

#include "server/message_handler.hpp"

namespace rendering { class Driver_progressive; }

namespace controller {

class Message_handler : public server::Message_handler {

public:

	Message_handler(rendering::Driver_progressive& driver);

	virtual void handle(const std::string& message) final override;

private:

	rendering::Driver_progressive& driver_;
};

}
