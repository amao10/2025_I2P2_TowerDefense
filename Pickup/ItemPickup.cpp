#include "ItemPickup.hpp"
#include "Scene/TestScene.hpp"
#include "Engine/Collider.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include <cstdlib>
#include <ctime>
#include "Engine/LOG.hpp"

TestScene* ItemPickup::getTestScene() {
    return dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

ItemPickup::ItemPickup(float x, float y) :
    Engine::Sprite("play/red.png", x, y, 0, 0, 0.3, 0.3) // 調整 scale 為 0.3
{
    potionImgs = {
        "play/red.png",
        "play/blue.png"
    };

    // 初始化亂數（建議放 main，但這裡防多次執行）
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
    //bmp = Engine::Resources::GetInstance().GetBitmap("play/red.png");

    int index = rand() % potionImgs.size();
    
    Engine::LOG(Engine::INFO) << "Trying to load image: " << potionImgs[index];
    // ✅ 正確使用 potionImgs 而不是 imagePaths
    if (index >= 0 && index < potionImgs.size()) {
        bmp = Engine::Resources::GetInstance().GetBitmap(potionImgs[index]);
    }

    type = (index == 0) ? "red" : "blue"; // 記錄藥水類型
}

void ItemPickup::Update(float deltaTime) {
    Engine::Sprite::Update(deltaTime);
    auto scene = getTestScene();

    // 判斷是否撿取
    if (Engine::Collider::IsCircleOverlap(Position, 8, scene->GetPlayer()->Position, 16)) {
        if (type == "red") {
            scene->GetPlayer()->redPotion++;
        } else if (type == "blue") {
            scene->GetPlayer()->bluePotion++;
        }
        scene->PickupGroup->RemoveObject(objectIterator);
    }
}
