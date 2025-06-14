#include "CoinPickup.hpp"
#include "Scene/TestScene.hpp"
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"



TestScene* CoinPickup::getTestScene() {
    return dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

CoinPickup::CoinPickup(float x, float y, float value) :
    Engine::Sprite("play/coin10-1.png", x, y, 0, 0, 0.25, 0.25),  // 用第一張圖初始化，並縮小為 25%
    value(value),
    frameCount(0),
    frameInterval(10)
{
    coinImgs = {
        "play/coin10-1.png",
        "play/coin10-2.png",
        "play/coin10-3.png"
    };
}

void CoinPickup::Update(float deltaTime) {
    Engine::Sprite::Update(deltaTime);
    auto scene = getTestScene();

    frameCount++;
    if (frameCount % frameInterval == 0) {
        int index = (frameCount / frameInterval) % coinImgs.size();
        bmp = Engine::Resources::GetInstance().GetBitmap(coinImgs[index]);
        
    }

    //偵測玩家是否撿起
    if (Engine::Collider::IsCircleOverlap(Position, 8, scene->GetPlayer()->Position, 16)) {
        scene->GetPlayer()->coin++;
        //scene->GetPlayer()->GainExp(20);
        scene->PickupGroup->RemoveObject(objectIterator); // 拿掉自己
    }
}
