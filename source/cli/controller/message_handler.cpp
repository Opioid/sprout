#include "message_handler.hpp"
#include <sstream>
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.hpp"
#include "base/string/string.hpp"
#include "camera_controller.hpp"
#include "core/logging/logging.hpp"
#include "core/rendering/rendering_driver_progressive.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/camera/camera.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"

namespace controller {

Message_handler::Message_handler(rendering::Driver_progressive& driver,
                                 resource::Manager& resource_manager, Camera& camera) noexcept
    : driver_(driver), resource_manager_(resource_manager), camera_(camera) {}

void Message_handler::handle(std::string const& message) noexcept {
    if ("restart" == message) {
        driver_.schedule_restart(false);
    } else if ("md:[" == message.substr(0, 4)) {
        float3 delta;
        sscanf(message.c_str(), "md:[%f,%f,%f]", &delta.v[0], &delta.v[1], &delta.v[2]);
        camera_.mouse_delta(delta, driver_.scene());
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

        if ("camera" == assignee) {
            handle_prop(driver_.scene().prop(driver_.camera().entity()), value, parameters, false);
        } else if ("entities" == assignee.substr(0, 8)) {
            if ('\"' == index.front() && '\"' == index.back()) {
                std::string index_string = index.substr(1, index.size() - 2);

                scene::prop::Prop* prop = driver_.scene().prop(index_string);
                handle_prop(prop, value, parameters, true);
            } else {
                uint32_t const index_number = static_cast<uint32_t>(std::stoul(index));

                scene::prop::Prop* prop = driver_.scene().prop(index_number);
                handle_prop(prop, value, parameters, true);
            }
        } else if ("materials" == assignee.substr(0, 9)) {
            if ('\"' == index.front() && '\"' == index.back()) {
                std::string index_string = index.substr(1, index.size() - 2);

                scene::material::Material* material =
                    resource_manager_.get<scene::material::Material>(index_string);
                handle_material(material, value, parameters);
            }
        } else {
            scene::prop::Prop* prop = driver_.scene().prop(assignee);
            handle_prop(prop, value, parameters, true);
        }
    }
}

std::string Message_handler::introduction() const noexcept {
    std::ostringstream stream;
    stream << "{";

    auto const d = driver_.camera().sensor_dimensions();
    stream << "\"resolution\":" << d << ",";

    stream << "\"iteration\":" << driver_.iteration();

    stream << "}";
    return stream.str();
}

std::string Message_handler::iteration() const noexcept {
    std::ostringstream stream;
    stream << "{";
    stream << "\"iteration\":" << driver_.iteration();
    stream << "}";
    return stream.str();
}

void Message_handler::handle_prop(scene::prop::Prop* prop, std::string const& value,
                                  std::string const& parameters, bool recompile) noexcept {
    if (!prop) {
        return;
    }

    std::string error;
    auto const  root = json::parse(parameters, error);
    if (!root) {
        logging::error(error);
        return;
    }

    if ("parameters" == value) {
        //    prop->set_parameters(*root);
    } else if ("transformation" == value) {
        math::Transformation t = prop->local_frame_0();
        json::read_transformation(*root, t);
        prop->set_transformation(t);
    } else {
        return;
    }

    driver_.schedule_restart(recompile);
}

void Message_handler::handle_material(scene::material::Material* /*material*/,
                                      std::string const& /*value*/,
                                      std::string const& /*parameters*/) noexcept {}

}  // namespace controller
