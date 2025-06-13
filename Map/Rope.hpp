// Rope.hpp
#pragma once

#include "Engine/IObject.hpp"
#include <allegro5/allegro.h>
//#include "MapSystem.hpp"

namespace Engine { class MapSystem; }
class Player;

// 配置数据结构（与 MapSystem.cpp 中的 Engine::Rope 对应）
struct RopeConfig {
    int x;        // 绳子所在列（tile index）
    int bottomY;  // 绳子底端所在行（tile index）
    int topY;     // 绳子顶端所在行（tile index）
};

class Rope : public Engine::IObject {
public:
    Rope(Player* player,
         Engine::MapSystem* mapSystem,
         const RopeConfig& cfg);
    ~Rope() override;

    // 每帧处理玩家是否正在攀爬
    void Update(float deltaTime) override;
    // 绘制整根绳子
    void Draw() const override;

private:
    Player* player_;
    Engine::MapSystem* mapSystem_;
    RopeConfig cfg_;

    // 静态位图，所有 Rope 实例共享
    static ALLEGRO_BITMAP* bmp_;

    // 攀爬相关
    float climbSpeed_ = 100.0f;  // 像素/秒
};
