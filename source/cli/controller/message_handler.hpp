#pragma once

#include "server/message_handler.hpp"

namespace rendering {
class Driver_progressive;
}

namespace resource {
class Manager;
}

namespace scene {

namespace entity {
class Entity;
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
                    Camera& camera);

    virtual void handle(std::string const& message) override final;

    virtual std::string introduction() const override final;

    virtual std::string iteration() const override final;

  private:
    void handle_entity(scene::entity::Entity* entity, std::string const& value,
                       std::string const& parameters, bool recompile);

    void handle_material(scene::material::Material* material, std::string const& value,
                         std::string const& parameters);

    rendering::Driver_progressive& driver_;
    resource::Manager&             resource_manager_;
    Camera&                        camera_;
};

}  // namespace controller
