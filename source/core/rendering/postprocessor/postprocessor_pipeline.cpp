#include "postprocessor_pipeline.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "image/typed_image.hpp"
#include "postprocessor.hpp"
#include "rendering/sensor/sensor.hpp"
#include "scene/camera/camera.hpp"

namespace rendering::postprocessor {

Pipeline::Pipeline() noexcept : scratch_(image::Description()) {}

Pipeline::~Pipeline() noexcept {
    for (auto pp : postprocessors_) {
        delete pp;
    }
}

bool Pipeline::empty() const noexcept {
    return postprocessors_.empty();
}

void Pipeline::clear() noexcept {
    for (auto pp : postprocessors_) {
        delete pp;
    }

    postprocessors_.clear();
}

void Pipeline::reserve(uint32_t num_pps) noexcept {
    postprocessors_.reserve(num_pps);
}

void Pipeline::add(Postprocessor* pp) noexcept {
    if (pp) {
        postprocessors_.push_back(pp);
    }
}

void Pipeline::init(scene::camera::Camera const& camera, thread::Pool& threads) noexcept {
    if (postprocessors_.empty()) {
        return;
    }

    scratch_.resize(camera.sensor_dimensions());

    for (auto pp : postprocessors_) {
        pp->init(camera, threads);
    }
}

bool Pipeline::has_alpha_transparency(bool alpha_in) const noexcept {
    for (auto const pp : postprocessors_) {
        alpha_in = pp->alpha_out(alpha_in);
    }

    return alpha_in;
}

void Pipeline::seed(sensor::Sensor const& sensor, image::Float4& target,
                    thread::Pool& threads) noexcept {
    if (postprocessors_.empty()) {
        sensor.resolve(threads, target);
    } else {
        if (0 == postprocessors_.size() % 2) {
            sensor.resolve(threads, target);
        } else {
            sensor.resolve(threads, scratch_);
        }
    }
}

void Pipeline::apply(image::Float4& target, thread::Pool& threads) noexcept {
    image::Float4* targets[2] = {&scratch_, &target};

    target.copy(scratch_);

    for (auto pp : postprocessors_) {
        pp->apply(*targets[0], *targets[1], threads);
        std::swap(targets[0], targets[1]);
    }

    if (0 == postprocessors_.size() % 2) {
        scratch_.copy(target);
    }
}

void Pipeline::apply(sensor::Sensor const& sensor, image::Float4& target,
                     thread::Pool& threads) noexcept {
    if (postprocessors_.empty()) {
        sensor.resolve(threads, target);
    } else {
        image::Float4* targets[2];

        if (0 == postprocessors_.size() % 2) {
            targets[0] = &target;
            targets[1] = &scratch_;
        } else {
            targets[0] = &scratch_;
            targets[1] = &target;
        }

        sensor.resolve(threads, *targets[0]);

        for (auto pp : postprocessors_) {
            pp->apply(*targets[0], *targets[1], threads);
            std::swap(targets[0], targets[1]);
        }
    }
}

void Pipeline::apply_accumulate(sensor::Sensor const& sensor, image::Float4& target,
                                thread::Pool& threads) noexcept {
    if (postprocessors_.empty()) {
        sensor.resolve_accumulate(threads, target);
    } else {
        image::Float4* targets[2];

        if (0 == postprocessors_.size() % 2) {
            targets[0] = &target;
            targets[1] = &scratch_;
        } else {
            targets[0] = &scratch_;
            targets[1] = &target;
        }

        sensor.resolve_accumulate(threads, *targets[0]);

        for (auto pp : postprocessors_) {
            pp->apply(*targets[0], *targets[1], threads);
            std::swap(targets[0], targets[1]);
        }
    }
}

}  // namespace rendering::postprocessor
