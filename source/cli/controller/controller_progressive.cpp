#include "controller_progressive.hpp"
#include "message_handler.hpp"
#include "server/server.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/scene/scene.hpp"
#include "core/take/take.hpp"
#include "core/progress/progress_sink.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"
#include <iostream>

namespace controller {

void progressive(const take::Take& take, scene::Scene& scene, thread::Pool& thread_pool) {
	logging::info("Progressive mode... type stuff to interact");

	if (!take.view.camera) {
		return;
	}

	rendering::Driver_progressive driver(take.surface_integrator_factory,
										 take.volume_integrator_factory,
										 take.sampler, scene, take.view,
										 thread_pool);

	driver.set_force_statistics(true);

	Message_handler handler(driver);
	server::Server server(take.view.camera->sensor_dimensions(), handler);

	server.run();

	driver.render(server);

	std::string input_line;

	for (;;) {
		std::getline(std::cin, input_line);

		string::trim(input_line);

		if ("abort" == input_line || "exit" == input_line || "quit" == input_line) {
			break;
		} else if ("iteration" == input_line) {
			logging::info(string::to_string(driver.iteration()));
		} else if ("statistics" == input_line || "stats" == input_line) {
			driver.schedule_statistics();
		} else {
			handler.handle(input_line);
		}
	}

	driver.abort();

	server.shutdown();
}


}
