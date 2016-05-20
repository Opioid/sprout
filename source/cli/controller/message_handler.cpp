#include "message_handler.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/camera/camera.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"

namespace controller {

Message_handler::Message_handler(rendering::Driver_progressive& driver) : driver_(driver) {}

void Message_handler::handle(const std::string& message) {
	if ("restart" == message) {
		driver_.schedule_restart(false);
	} else {

		size_t op = message.find_first_of("=");
		if (std::string::npos == op) {
			return;
		}

		std::string operand = message.substr(0, op);

		size_t dot = operand.find_first_of('.');
		if (std::string::npos == dot) {
			return;
		}


		auto assignee_begin = std::find_if_not(operand.begin(), operand.begin() + dot,
											   string::is_space);

		auto assigne_end = std::find_if(assignee_begin, operand.begin() + dot, string::is_space);

		std::string assignee(assignee_begin, assigne_end);


		size_t index_begin = operand.find_first_of('[');
		size_t index_end   = operand.find_first_of(']', index_begin);

		std::string index;

		if (index_begin != std::string::npos && index_end != std::string::npos) {
			++index_begin;
			index.assign(operand.substr(index_begin, index_end - index_begin));
			string::trim(index);
		}

		auto value_begin = std::find_if_not(operand.begin() + dot + 1, operand.end(),
											string::is_space);

		auto value_end = std::find_if(value_begin, operand.end(), string::is_space);

		std::string value(value_begin, value_end);

		std::string parameters = message.substr(op + 1);

		scene::entity::Entity* entity = nullptr;
		bool recompile = false;

		if ("camera" == assignee) {
			entity = &driver_.camera();
		} else if ("entities" == assignee.substr(0, 8)) {

			try {
				uint32_t number = std::stoul(index);
				entity = driver_.scene().entity(number);
				recompile = true;
			} catch (...) {}
		}

		if (!entity) {
			return;
		}

		try {
			auto root = json::parse(parameters);

			if ("parameters" == value) {
				entity->set_parameters(*root);
			} else if ("transformation" == value) {
				math::transformation t = entity->local_frame_a();
				json::read_transformation(*root, t);
				entity->set_transformation(t);
			} else {
				return;
			}

			driver_.schedule_restart(recompile);
		} catch (const std::exception& e) {
			logging::error(e.what());
		}
	}
}

}
