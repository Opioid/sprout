#include "message_handler.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"

#include <iostream>

namespace controller {

Message_handler::Message_handler(rendering::Driver_progressive& driver) : driver_(driver) {}

void Message_handler::handle(const std::string& message) {
	if ("restart" == message) {
		driver_.schedule_restart();
	} else {

		auto op = message.find_first_of("=");
		if (std::string::npos == op) {
			return;
		}

		std::string assignee = message.substr(0, op);

		auto dot = assignee.find_first_of('.');

		if (std::string::npos == dot) {
			return;
		}

		std::string value = assignee.substr(dot + 1);
		auto space = value.find_first_of(' ');
		if (std::string::npos != space) {
			value = value.substr(0, space);
		}

		std::string parameters = message.substr(op + 1);

		try {
			auto root = json::parse(parameters);

			if ("parameters" == value) {
				driver_.camera().set_parameters(*root);
			} else {
				return;
			}

			driver_.schedule_restart();
		} catch (const std::exception& e) {
			logging::error(e.what());
		}
	}
}

}
