#ifndef SLOWBULLET_HPP
#define SLOWBULLET_HPP
#include "Bullet.hpp"

class SlowBullet : public Bullet {
public:
    SlowBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret* parent);
    void OnExplode(Enemy* enemy) override;
};
#endif
