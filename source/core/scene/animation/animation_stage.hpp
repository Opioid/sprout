#pragma once


namespace thread { class Pool; }

namespace scene {

namespace entity {

class Entity;

}

namespace animation {

class Animation;

class Stage {
public:

    Stage(entity::Entity* entity, Animation* animation);

	void update(thread::Pool& pool);

private:

    entity::Entity* entity_;
    Animation*   animation_;
};

}}

