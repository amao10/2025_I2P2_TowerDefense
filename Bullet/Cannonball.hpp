#ifndef CANNONBALL_HPP
#define CANNONBALL_HPP
#include "Engine/Sprite.hpp"

class CannonBall : public Engine::Sprite {
public:
    float speed;
    float damage;
    int direction; // 1: RIGHT, -1: LEFT
    CannonBall(float x, float y, float dir, float dmg);
    void Update(float deltaTime) override;
    void Draw() const override;
};

#endif
