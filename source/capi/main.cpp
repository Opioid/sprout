#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_std_out.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/take/take.hpp"
#include "core/take/take_loader.hpp"
#include "visibility.h"

#include <iostream>

extern "C" {

using namespace scene;

using Shape_ptr = resource::Resource_ptr<shape::Shape>;

struct Context {
    Context() noexcept
        : threads(0),
          resources(threads),
          image_resources(resources.register_provider(image_provider)),
          texture_provider(true),
          texture_resources(resources.register_provider(texture_provider)),
          material_provider(true),
          material_resources(resources.register_provider(material_provider)),
          shape_resources(resources.register_provider(mesh_provider)),
          scene_loader(resources, material_provider.create_fallback_material()),
          scene(scene_loader.null_shape(), shape_resources,
                material_resources,
                texture_resources),
          max_sample_size(material_provider.max_sample_size()) {}

    thread::Pool threads;

    resource::Manager resources;

    image::Provider image_provider;
    std::vector<image::Image*> const& image_resources;

    image::texture::Provider texture_provider;
    std::vector<image::texture::Texture*> const& texture_resources;

    material::Provider material_provider;
    std::vector<material::Material*> const& material_resources;

    shape::triangle::Provider mesh_provider;
    std::vector<shape::Shape*> const& shape_resources;

    scene::Loader scene_loader;

    Scene scene;

    take::Take take;

    uint32_t const max_sample_size;
};

static Context* context = nullptr;

DLL_PUBLIC char const* su_platform_revision(void) noexcept {
    return platform::revision().c_str();
}

DLL_PUBLIC void su_init(void) noexcept {
    context = new Context;

    logging::init(logging::Type::Std_out);
}

DLL_PUBLIC void su_release(void) noexcept {
    delete context;
    context = nullptr;
}

DLL_PUBLIC int32_t su_load_take(/*char *const take*/ void) noexcept {
    if (!context) {
        return 0;
    }

    bool success = true;

    std::string take_name;

    std::string take = "takes/imrod.take";

    {
        bool const is_json = string::is_json(take);


        auto stream = is_json ? file::Stream_ptr(new std::stringstream(take))
                              : context->resources.filesystem().read_stream(take, take_name);

        if (!stream || !take::Loader::load(context->take, *stream, take_name, context->scene,
                                           context->resources)) {
            //     logging::error("Loading take %S: ", args.take);
            success = false;
        }

        logging::error("3");
    }

    if (success && !context->scene_loader.load(context->take.scene_filename, take_name,
                                               context->take, context->scene)) {
        //    logging::error("Loading scene %S: ", take.scene_filename);
        success = false;
    }

    logging::error("4");

    return success ? 1 : 0;
}

DLL_PUBLIC int32_t su_render() noexcept {
    if (!context) {
        return 0;
    }

    progress::Std_out progressor;

    if (context->take.view.camera) {
        rendering::Driver_finalframe driver(context->take, context->scene, context->threads,
                                            context->max_sample_size, progressor);

        driver.render(context->take.exporters);
    }
}

DLL_PUBLIC int square(int i) noexcept {
    return i * i;
}
}
