#include "animation_loader.hpp"
#include "animation.hpp"
#include "base/json/json.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.hpp"
#include "base/math/vector3.inl"
#include "scene/entity/keyframe.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"

namespace scene::animation {

static Animation* load_keyframes(json::Value const& value, Transformation const& default_trafo,
                                 uint32_t entity, Scene& scene);

static Animation* load_sequence(json::Value const& value, Transformation const& default_trafo,
                                uint32_t entity, Scene& scene);

static void read_morphing(json::Value const& value, entity::Morphing& morphing);

Animation* load(json::Value const& value, Transformation const& default_trafo, uint32_t entity,
                Scene& scene) {
    for (auto& n : value.GetObject()) {
        if ("keyframes" == n.name) {
            return load_keyframes(n.value, default_trafo, entity, scene);
        }

        if ("morph_sequence" == n.name) {
            return load_sequence(n.value, default_trafo, entity, scene);
        }
    }

    return nullptr;
}

static Animation* load_keyframes(json::Value const& value, Transformation const& default_trafo,
                                 uint32_t entity, Scene& scene) {
    if (!value.IsArray()) {
        return nullptr;
    }

    auto animation = scene.create_animation(entity, value.Size());

    for (uint32_t i = 0; auto const& k : value.GetArray()) {
        Keyframe keyframe{{default_trafo}, {{0, 0}, 0.f}, 0};

        for (auto& n : k.GetObject()) {
            if ("time" == n.name) {
                keyframe.time = time(json::read_double(n.value));
            } else if ("transformation" == n.name) {
                json::read_transformation(n.value, keyframe.k.trafo);
            } else if ("morphing" == n.name) {
                read_morphing(n.value, keyframe.m);
            }
        }

        animation->set(i++, keyframe);
    }

    return animation;
}

Animation* load_sequence(json::Value const& value, Transformation const& default_trafo,
                         uint32_t entity, Scene& scene) {
    uint32_t start_frame       = 0;
    uint32_t num_frames        = 0;
    uint32_t frames_per_second = 0;

    for (auto& n : value.GetObject()) {
        if ("start_frame" == n.name) {
            start_frame = json::read_uint(n.value);
        } else if ("num_frames" == n.name) {
            num_frames = json::read_uint(n.value);
        } else if ("frames_per_second" == n.name) {
            frames_per_second = json::read_uint(n.value);
        }
    }

    if (!num_frames || !frames_per_second) {
        return nullptr;
    }

    auto animation = scene.create_animation(entity, num_frames);

    uint64_t time = 0;

    uint64_t const time_increment = scene::Units_per_second / frames_per_second;

    for (uint32_t i = 0; i < num_frames; ++i) {
        uint32_t const target = start_frame + i;

        Keyframe const keyframe{{default_trafo}, {{target, target}, 0.f}, time};

        animation->set(i, keyframe);

        time += time_increment;
    }

    return animation;
}

void read_morphing(json::Value const& value, entity::Morphing& morphing) {
    for (auto& n : value.GetObject()) {
        if ("targets" == n.name) {
            if (n.value.IsArray() && n.value.Size() >= 2) {
                morphing.targets[0] = n.value[0].GetUint();
                morphing.targets[1] = n.value[1].GetUint();
            }
        } else if ("weight" == n.name) {
            morphing.weight = json::read_float(n.value);
        }
    }
}

}  // namespace scene::animation
