#ifndef ITEM_PICKUP_HPP
#define ITEM_PICKUP_HPP

#include <vector>
#include <string>
#include "Engine/Sprite.hpp"

class TestScene;

class ItemPickup : public Engine::Sprite {
public:
    ItemPickup(float x, float y);

    void Update(float deltaTime) override;

private:
    TestScene* getTestScene();
    std::vector<std::string> potionImgs;
    std::string type;  // "red" 或 "blue"
};

#endif // ITEM_PICKUP_HPP
