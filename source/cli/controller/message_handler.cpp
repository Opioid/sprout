#include "message_handler.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"

namespace controller {

Message_handler::Message_handler(rendering::Driver_progressive& driver) : driver_(driver) {}

void Message_handler::handle(const std::string& message) {
	if ("restart" == message) {
		driver_.schedule_restart();
	} else {

		try {
			auto root = json::parse(message);

			driver_.camera().set_parameters(*root);
			driver_.schedule_restart();
		} catch (const std::exception& e) {
			logging::error(e.what());
		}
	}
}

}
