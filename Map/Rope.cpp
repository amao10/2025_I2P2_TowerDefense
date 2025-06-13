// Rope.cpp
#include "Rope.hpp"
#include "Player/Player.hpp"
#include "MapSystem.hpp"
#include <allegro5/allegro_image.h>
#include <stdexcept>

ALLEGRO_BITMAP* Rope::bmp_ = nullptr;

Rope::Rope(Player* player,
           Engine::MapSystem* mapSystem,
           const RopeConfig& cfg)
    : IObject(
        // 在世界坐标系中，绳子的 X = cfg.x * tileWidth
        cfg.x * mapSystem->tileWidth,
        // Y 坐标对应 tile 底端
        cfg.bottomY * mapSystem->tileHeight,
        // 宽度一个 tile
        mapSystem->tileWidth,
        // 高度 = (顶端行 - 底端行) * tileHeight
        (cfg.topY - cfg.bottomY) * mapSystem->tileHeight,
        0, 0),
      player_(player),
      mapSystem_(mapSystem),
      cfg_(cfg)
{
    if (!bmp_) {
        bmp_ = al_load_bitmap("Resource/images/rope.png");
        if (!bmp_)
            throw std::runtime_error("Failed to load rope.png");
    }
}

Rope::~Rope() {
    // 不在这里销毁 bmp_，让所有 Rope 共享
}

void Rope::Update(float deltaTime) {
    // 玩家中心点
    float px = player_->Position.x;
    float py = player_->Position.y;

    // 判断玩家是否在绳子水平区域（窄一点，让体验更好）
    float ropeX = Position.x + Size.x * 0.5f;
    if (px > ropeX - 10 && px < ropeX + 10
        && py > Position.y && py < Position.y + Size.y)
    {
        // 进入攀爬模式
        player_->changeOnRope(true);
        //Player與Rope交互
        //player_->velocity.y = 0; // 关闭重力

        ALLEGRO_KEYBOARD_STATE ks;
        al_get_keyboard_state(&ks);
        if (al_key_down(&ks, ALLEGRO_KEY_UP)) {
            player_->Position.y -= climbSpeed_ * deltaTime;
        } else if (al_key_down(&ks, ALLEGRO_KEY_DOWN)) {
            player_->Position.y += climbSpeed_ * deltaTime;
        }
    }
    else if (player_->isOnRope()) {
        // 离开绳子区域，恢复正常下落
        player_->changeOnRope(false);
    }
}

void Rope::Draw() const {
    al_draw_scaled_bitmap(
        bmp_,
        0, 0,
        al_get_bitmap_width(bmp_),
        al_get_bitmap_height(bmp_),
        Position.x, Position.y,
        Size.x, Size.y,
        0
    );
}
