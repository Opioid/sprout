#include "animation_loader.hpp"
#include "animation.hpp"
#include "base/json/json.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "scene/scene_constants.hpp"

namespace scene::animation {

std::shared_ptr<animation::Animation> load_keyframes(
    json::Value const& keyframes_value, math::Transformation const& default_transformation);

std::shared_ptr<animation::Animation> load_sequence(
    json::Value const& keyframes_value, math::Transformation const& default_transformation);

void read_morphing(json::Value const& value, entity::Keyframe::Morphing& morphing);

std::shared_ptr<animation::Animation> load(json::Value const&          animation_value,
                                           math::Transformation const& default_transformation) {
    for (auto n = animation_value.MemberBegin(); n != animation_value.MemberEnd(); ++n) {
        std::string const       node_name  = n->name.GetString();
        rapidjson::Value const& node_value = n->value;

        if ("keyframes" == node_name) {
            return load_keyframes(node_value, default_transformation);
        } else if ("morph_sequence" == node_name) {
            return load_sequence(node_value, default_transformation);
        }
    }

    return nullptr;
}

std::shared_ptr<animation::Animation> load_keyframes(
    json::Value const& keyframes_value, math::Transformation const& default_transformation) {
    if (!keyframes_value.IsArray()) {
        return nullptr;
    }

    auto animation = std::make_shared<animation::Animation>();

    animation->init(keyframes_value.Size());

    for (auto const& k : keyframes_value.GetArray()) {
        entity::Keyframe keyframe;
        keyframe.transformation = default_transformation;

        keyframe.morphing.targets[0] = 0;
        keyframe.morphing.targets[1] = 0;
        keyframe.morphing.weight     = 0.f;

        keyframe.time = 0;

        for (auto& n : k.GetObject()) {
            std::string const node_name = n.name.GetString();

            if ("time" == node_name) {
                keyframe.time = time(json::read_double(n.value));
            } else if ("transformation" == node_name) {
                json::read_transformation(n.value, keyframe.transformation);
            } else if ("morphing" == node_name) {
                read_morphing(n.value, keyframe.morphing);
            }
        }

        animation->push_back(keyframe);
    }

    return animation;
}

std::shared_ptr<animation::Animation> load_sequence(
    json::Value const& sequence_value, math::Transformation const& default_transformation) {
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

    auto animation = std::make_shared<animation::Animation>();

    animation->init(num_frames);

    uint64_t time           = 0;
    uint64_t time_increment = scene::Units_per_second / frames_per_second;

    for (uint32_t i = 0; i < num_frames; ++i) {
        entity::Keyframe keyframe;

        keyframe.time = time;

        keyframe.transformation = default_transformation;

        uint32_t target              = start_frame + i;
        keyframe.morphing.targets[0] = target;
        keyframe.morphing.targets[1] = target;
        keyframe.morphing.weight     = 0.f;

        animation->push_back(keyframe);

        time += time_increment;
    }

    return animation;
}

void read_morphing(json::Value const& value, entity::Keyframe::Morphing& morphing) {
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
