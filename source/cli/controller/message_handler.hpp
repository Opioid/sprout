#pragma once

#include "server/message_handler.hpp"

namespace rendering {
class Driver_progressive;
}

namespace resource {
class Manager;
}

namespace scene {

namespace prop {
class Prop;
}

namespace material {
class Material;
}

}  // namespace scene

namespace controller {

class Camera;

class Message_handler : public server::Message_handler {
  public:
    Message_handler(rendering::Driver_progressive& driver, resource::Manager& resource_manager,
                    Camera& camera) noexcept;

    void handle(std::string const& message) noexcept override final;

    std::string introduction() const noexcept override final;

    std::string iteration() const noexcept override final;

  private:
    void handle_prop(scene::prop::Prop* prop, std::string const& value,
                     std::string const& parameters, bool recompile) noexcept;

    void handle_material(scene::material::Material* material, std::string const& value,
                         std::string const& parameters) noexcept;

    rendering::Driver_progressive& driver_;

    resource::Manager& resource_manager_;

    Camera& camera_;
};

}  // namespace controller
