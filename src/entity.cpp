#include "zombie/entity.h"

namespace zombie {

Entity::Entity(ActorType type) : type_(type) {
    // TODO: 根据实体类型初始化对应参数。
}

void Entity::update(double /*deltaTime*/) {
    // TODO: 更新移动、朝向及受伤／死亡状态。
}

void Entity::takeDamage(int /*damage*/) {
    // TODO: 处理扣血、受伤反馈和死亡。
}

} // namespace zombie
