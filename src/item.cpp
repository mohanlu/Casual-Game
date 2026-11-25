#include "zombie/item.h"

namespace zombie {

Item::Item(ItemType type, int amount) : type_(type), amount_(amount) {
    // TODO: 补充道具初始化规则。
}

void Item::applyTo(Entity& /*entity*/) {
    // TODO: 根据道具类型提供弹药或恢复生命，并标记已消耗。
}

} // namespace zombie
