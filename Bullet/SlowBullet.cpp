#include <allegro5/base.h>
#include "SlowBullet.hpp"
#include "Enemy/Enemy.hpp"

SlowBullet::SlowBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret* parent) :
    Bullet("play/bullet-9.png", 500, 1, position, forwardDirection, rotation, parent) {
}

void SlowBullet::OnExplode(Enemy* enemy) {
    if (enemy) {
        enemy->Slow(1.0f); // 先讓敵人停下來 1 秒
    }
    Bullet::OnExplode(enemy); // 再執行原本爆炸邏輯
}
