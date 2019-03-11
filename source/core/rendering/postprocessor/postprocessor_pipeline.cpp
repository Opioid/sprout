#include "postprocessor_pipeline.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "image/typed_image.hpp"
#include "postprocessor.hpp"
#include "rendering/sensor/sensor.hpp"

namespace rendering::postprocessor {

Pipeline::Pipeline() : scratch_(image::Image::Description(image::Image::Type::Float4)) {}

Pipeline::~Pipeline() {
    for (auto pp : postprocessors_) {
        delete pp;
    }
}

void Pipeline::reserve(size_t num_pps) {
    postprocessors_.reserve(num_pps);
}

void Pipeline::add(Postprocessor* pp) {
    if (pp) {
        postprocessors_.push_back(pp);
    }
}

void Pipeline::init(scene::camera::Camera const& camera, thread::Pool& pool) {
    if (postprocessors_.empty()) {
        return;
    }

    scratch_.resize(camera.sensor_dimensions());

    for (auto pp : postprocessors_) {
        pp->init(camera, pool);
    }
}

bool Pipeline::has_alpha_transparency(bool alpha_in) const {
    for (auto const pp : postprocessors_) {
        alpha_in = pp->alpha_out(alpha_in);
    }

    return alpha_in;
}

void Pipeline::apply(sensor::Sensor const& sensor, image::Float4& target, thread::Pool& pool) {
    if (postprocessors_.empty()) {
        sensor.resolve(pool, target);
    } else {
        image::Float4* targets[2];

        if (0 == postprocessors_.size() % 2) {
            targets[0] = &target;
            targets[1] = &scratch_;
        } else {
            targets[0] = &scratch_;
            targets[1] = &target;
        }

        sensor.resolve(pool, *targets[0]);

        for (auto pp : postprocessors_) {
            pp->apply(*targets[0], *targets[1], pool);
            std::swap(targets[0], targets[1]);
        }
    }
}

size_t Pipeline::num_bytes() const {
    size_t num_bytes = 0;
    for (auto const pp : postprocessors_) {
        num_bytes += pp->num_bytes();
    }

    return num_bytes;
}

}  // namespace rendering::postprocessor
