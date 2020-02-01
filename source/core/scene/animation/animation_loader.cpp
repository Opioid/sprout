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

Animation* load_keyframes(json::Value const&    keyframes_value,
                          Transformation const& default_transformation, Scene& scene);

Animation* load_sequence(json::Value const&    sequence_value,
                         Transformation const& default_transformation, Scene& scene);

void read_morphing(json::Value const& value, entity::Morphing& morphing);

Animation* load(json::Value const& animation_value, Transformation const& default_transformation,
                Scene& scene) {
    for (auto& n : animation_value.GetObject()) {
        std::string const       node_name  = n.name.GetString();
        rapidjson::Value const& node_value = n.value;

        if ("keyframes" == node_name) {
            return load_keyframes(node_value, default_transformation, scene);
        }

        if ("morph_sequence" == node_name) {
            return load_sequence(node_value, default_transformation, scene);
        }
    }

    return nullptr;
}

Animation* load_keyframes(json::Value const&    keyframes_value,
                          Transformation const& default_transformation, Scene& scene) {
    if (!keyframes_value.IsArray()) {
        return nullptr;
    }

    auto animation = scene.create_animation(keyframes_value.Size());

    auto const& keyframes = keyframes_value.GetArray();

    for (uint32_t i = 0, len = keyframes.Size(); i < len; ++i) {
        auto const& k = keyframes[i];

        Keyframe keyframe{{default_transformation}, {{0, 0}, 0.f}, 0};

        for (auto& n : k.GetObject()) {
            std::string const node_name = n.name.GetString();

            if ("time" == node_name) {
                keyframe.time = time(json::read_double(n.value));
            } else if ("transformation" == node_name) {
                json::read_transformation(n.value, keyframe.k.transformation);
            } else if ("morphing" == node_name) {
                read_morphing(n.value, keyframe.m);
            }
        }

        animation->set(i, keyframe);
    }

    return animation;
}

Animation* load_sequence(json::Value const&    sequence_value,
                         Transformation const& default_transformation, Scene& scene) {
    uint32_t start_frame       = 0;
    uint32_t num_frames        = 0;
    uint32_t frames_per_second = 0;

    for (auto& n : sequence_value.GetObject()) {
        std::string const node_name = n.name.GetString();

        if ("start_frame" == node_name) {
            start_frame = json::read_uint(n.value);
        } else if ("num_frames" == node_name) {
            num_frames = json::read_uint(n.value);
        } else if ("frames_per_second" == node_name) {
            frames_per_second = json::read_uint(n.value);
        }
    }

    if (!num_frames || !frames_per_second) {
        return nullptr;
    }

    auto animation = scene.create_animation(num_frames);

    uint64_t time = 0;

    uint64_t const time_increment = scene::Units_per_second / frames_per_second;

    for (uint32_t i = 0; i < num_frames; ++i) {
        uint32_t const target = start_frame + i;

        Keyframe const keyframe{{default_transformation}, {{target, target}, 0.f}, time};

        animation->set(i, keyframe);

        time += time_increment;
    }

    return animation;
}

void read_morphing(json::Value const& value, entity::Morphing& morphing) {
    for (auto& n : value.GetObject()) {
        std::string const node_name = n.name.GetString();

        if ("targets" == node_name) {
            if (n.value.IsArray() && n.value.Size() >= 2) {
                morphing.targets[0] = n.value[0].GetUint();
                morphing.targets[1] = n.value[1].GetUint();
            }
        } else if ("weight" == node_name) {
            morphing.weight = json::read_float(n.value);
        }
    }
}

}  // namespace scene::animation
