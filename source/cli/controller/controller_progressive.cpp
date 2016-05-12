#include "controller_progressive.hpp"
#include "server/message_handler.hpp"
#include "server/server.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/scene/scene.hpp"
#include "core/take/take.hpp"
#include "core/progress/progress_sink.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"
#include <iostream>

namespace controller {

class Message_handler : public server::Message_handler {

public:

	Message_handler(rendering::Driver_progressive& driver) : driver_(driver) {}

	virtual void handle(const std::string& message) final override;

private:

	rendering::Driver_progressive& driver_;
};

void progressive(const take::Take& take, scene::Scene& scene, thread::Pool& thread_pool) {
	logging::info("Progressive mode... type stuff to interact");

	if (!take.view.camera) {
		return;
	}

	rendering::Driver_progressive driver(take.surface_integrator_factory,
										 take.volume_integrator_factory,
										 take.sampler, scene, take.view,
										 thread_pool);

	Message_handler handler(driver);
	server::Server server(take.view.camera->sensor_dimensions(), handler);

	server.run();

	driver.render(server);

	for (;;) {
		std::string input;
		std::cin >> input;

		if ("abort" == input || "exit" == input || "quit" == input) {
			break;
		} else if ("iteration" == input) {
			logging::info(string::to_string(driver.iteration()));
		} else {
			handler.handle(input);
		}
	}

	driver.abort();

	server.shutdown();
}

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
