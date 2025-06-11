#ifndef COIN_PICKUP_HPP
#define COIN_PICKUP_HPP
#include "Engine/Sprite.hpp"
#include "Scene/PlayScene.hpp"
class CoinPickup : public Engine::Sprite {
public:
    float value;
    CoinPickup(float x, float y, float value);
    void Update(float deltaTime) override;
protected:
    PlayScene *getPlayScene();
};
#endif
