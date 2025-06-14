#include "Cannonball.hpp"
#include "Monster/Monster.hpp"
#include "Scene/TestScene.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Collider.hpp"
#include <allegro5/color.h>
#include <allegro5/allegro.h>
#include "Engine/Resources.hpp"

CannonBall::CannonBall(float x, float y, float dir, float dmg)
    : Engine::Sprite("Player_handcannon/cannonball.png", x, y), speed(600 * dir), damage(dmg), direction((dir > 0) ? 1 : -1) {
    Velocity.x = speed;
    CollisionRadius = GetBitmapWidth() / 2;
}


void CannonBall::Update(float deltaTime) {
    Sprite::Update(deltaTime);
    Position.x += Velocity.x * deltaTime;

    auto* scene = dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
    if (!scene) return;
    for (auto& obj : scene->MonsterGroup->GetObjects()) {
        Monster* monster = dynamic_cast<Monster*>(obj);
        if (!monster || !monster->Visible) continue;

        if (Engine::Collider::IsCircleOverlap(Position, CollisionRadius,
                                              monster->Position, monster->CollisionRadius)) {
            monster->Hit(damage); // 不刪除，讓它繼續穿透
        }
    }

    // 超出畫面後移除自己
    int screenW = Engine::GameEngine::GetInstance().GetScreenSize().x;
    if (Position.x < -50 || Position.x > screenW + 50)
        Visible = false;
}

void CannonBall::Draw() const {
    ALLEGRO_BITMAP* bmp = Engine::Resources::GetInstance().GetBitmap("Player_handcannon/cannonball.png").get();
    if (bmp) {
        al_draw_bitmap(
            bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - al_get_bitmap_height(bmp) / 2,
            direction == 1 ? ALLEGRO_FLIP_HORIZONTAL : 0
        );
    }
}


