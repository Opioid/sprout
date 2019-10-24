#include "controller_progressive.hpp"
#include <iostream>
#include "base/math/vector4.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "camera_controller.hpp"
#include "core/logging/logging.hpp"
#include "core/progress/progress_sink.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/scene/scene.hpp"
#include "core/take/take.hpp"
#include "message_handler.hpp"
#include "server/server.hpp"

namespace controller {

void progressive(take::Take& take, scene::Scene& scene, resource::Manager& resource_manager,
                 thread::Pool& threads, uint32_t max_sample_size) {
    logging::info("Progressive mode... type stuff to interact");

    if (!take.view.camera) {
        return;
    }

    rendering::Driver_progressive driver(take, scene, threads, max_sample_size);

    //	driver.set_force_statistics(true);

    Camera camera(take.view.camera->entity(), scene);

    Message_handler handler(driver, resource_manager, camera);

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

}  // namespace controller
