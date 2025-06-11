#include "CoinPickup.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"

PlayScene *CoinPickup::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}

CoinPickup::CoinPickup(float x, float y, float value) :
    Engine::Sprite("play/coin10-1.png", x, y , 0, 0, 0.25, 0.25), value(value) {}

void CoinPickup::Update(float deltaTime) {
    Engine::Sprite::Update(deltaTime);
    auto scene = getPlayScene();
    // 偵測玩家是否撿起
    // if (Engine::Collider::IsCircleOverlap(Position, 8, scene->Player->Position, 16)) {
    //     scene->EarnMoney(value); // 加錢
    //     scene->PickupGroup->RemoveObject(objectIterator); // 拿掉自己
    // }
}
