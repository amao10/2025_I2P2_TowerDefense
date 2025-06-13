#include "Teleport.hpp"
#include "Player/Player.hpp"
#include "MapSystem.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

ALLEGRO_BITMAP* TeleportTrigger::bmp_ = nullptr;

TeleportTrigger::TeleportTrigger(Player* player,
                                 MapSystem* mapSystem,
                                 const TeleportPoint& cfg)
    : IObject(cfg.x * 90,
              cfg.y * 60,
              90,
              60,
              0, 0)
    , player_(player)
    , mapSystem_(mapSystem)
    , cfg_(cfg)
    , triggered_(false)
{   
    if(!bmp_)
        bmp_ = al_load_bitmap("Resource/images/teleport.png");

    if (!bmp_) {
        throw std::runtime_error("Failed to load teleport.png");
    }
}

TeleportTrigger::~TeleportTrigger() {
    //if (bmp_) al_destroy_bitmap(bmp_);
}

void TeleportTrigger::Update(float /*deltaTime*/) {
    if (triggered_) return;

    // 简易 AABB 碰撞检测
    float px = player_->Position.x;
    float py = player_->Position.y;
    if (px > Position.x && px < Position.x + Size.x
     && py > Position.y && py < Position.y + Size.y)
    {
        triggered_ = true;
        mapSystem_->unloadMap();
        mapSystem_->loadMap(cfg_.targetMapFile, cfg_.targetObjFile, cfg_.targetMonsterSpawnFile);
        player_->Position.x = static_cast<float>(cfg_.targetX);
        player_->Position.y = static_cast<float>(cfg_.targetY);
    }
}

void TeleportTrigger::Draw() const {
    // 直接用共用的 static 貼圖
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
