#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/LaserBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "LaserTurret.hpp"
#include "Scene/PlayScene.hpp"
#include "Bullet/SlowBullet.hpp"
#include "Turret/SlowTurret.hpp"

const int SlowTurret::Price = 100;
SlowTurret::SlowTurret(float x, float y) : 
    Turret("play/tower-base.png", "play/turret-6.png", x, y, 300, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
}
void SlowTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    getPlayScene()->BulletGroup->AddNewObject(new SlowBullet(Position + normalized * 36, diff, rotation, this));
    AudioHelper::PlayAudio("missile.wav");
}
