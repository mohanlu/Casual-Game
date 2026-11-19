#pragma once

namespace zombie {

class Windows {
public:
    // 窗口与界面流程的入口，返回程序退出码。
    int run();

private:
    // 开始菜单界面。
    void showMainMenu();

    // TODO: 添加窗口资源、菜单状态和按钮交互所需的成员。
};

} // namespace zombie
