#include "message_handler.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"

#include <iostream>

namespace controller {

Message_handler::Message_handler(rendering::Driver_progressive& driver) : driver_(driver) {}

void Message_handler::handle(const std::string& message) {
	if ("restart" == message) {
		driver_.schedule_restart();
	} else {

		size_t op = message.find_first_of("=");
		if (std::string::npos == op) {
			return;
		}

		std::string assignee = message.substr(0, op);

		size_t dot = assignee.find_first_of('.');
		if (std::string::npos == dot) {
			return;
		}

		auto value_begin = std::find_if_not(assignee.begin() + dot + 1, assignee.end(),
											std::isspace);

		auto value_end = std::find_if(value_begin, assignee.end(), std::isspace);

		std::string value(value_begin, value_end);

		std::cout << "|" << value << "|" << std::endl;

		std::string parameters = message.substr(op + 1);

		try {
			auto root = json::parse(parameters);

			if ("parameters" == value) {
				driver_.camera().set_parameters(*root);
			} else if ("transformation" == value) {
				math::transformation t = driver_.camera().local_frame_a();
				json::read_transformation(*root, t);
				driver_.camera().set_transformation(t);
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
