#include <allegro5/allegro.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>  //
#include <memory>
#include <queue>
#include <string>
#include <vector>
using namespace std;

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"

#include "UI/Component/Label.hpp"

#include "TestScene.hpp"
#include "Player/Player.hpp"

TestScene::TestScene()
    : mapSystem_(nullptr), elapsedTime_(0.0f)
{}

TestScene::~TestScene() {
    Terminate();
}
void TestScene::Initialize() {
    // 1. 从 Allegro 拿到当前显示器
    ALLEGRO_DISPLAY* display = al_get_current_display();
    if (!display) {
        // 如果拿不到，就抛异常交由上层处理
        throw std::ios_base::failure("[TestScene] al_get_current_display() returned nullptr");
    }

    // 2. 构造 MapSystem
    mapSystem_ = new MapSystem(display);
    CreateTeleportTriggers();

    // 3. 尝试加载地图和物件文件（路径根据你的 Resource 文件夹结构调整）
    try {
        mapSystem_->loadMap("Resource/testMap.txt",
                            "Resource/testObject.txt");
    }
    catch (const std::exception& e) {
        std::cerr << "[TestScene] loadMap exception: " << e.what() << std::endl;
        // 视情况可以提前返回，或保留空地图继续执行
    }

    player = new Player(400, 200, 300, 100, 50, 30, 10); // 可調位置和屬性
    AddNewObject(player); // 讓 engine 控制 update & draw

    CreateTeleportTriggers();

    // 4. 初始化计时器
    elapsedTime_ = 0.0f;
}


void TestScene::Terminate() {
    if (mapSystem_) {
        delete mapSystem_;
        mapSystem_ = nullptr;
    }
    if (player) {
        delete player;
        player = nullptr;
    }

    IScene::Terminate();
}

void TestScene::Update(float deltaTime) {
    elapsedTime_ += deltaTime;
    // 1. 先 update 場景（包含玩家）
    IScene::Update(deltaTime);
    // 2. 再把玩家新座標傳給 MapSystem
    int px = static_cast<int>(player->Position.x);
    int py = static_cast<int>(player->Position.y);
    mapSystem_->update(deltaTime, px, py);

    // 計算玩家當前格子（與 MapSystem 同樣的 tileW/H）
    const int tileW = 90;
    const int tileH = 60;
    int gx = px / tileW;
    int gy = (py + player->Size.y/2) / tileH;

    // 偵測任何傳送點
    for (auto& tp : mapSystem_->GetTeleports()) {
        if (tp.x == gx && tp.y == gy) {
            // 1. 清除舊觸發器
            ClearTeleportTriggers();
            // 2. 換圖
            mapSystem_->unloadMap();
            mapSystem_->loadMap(tp.targetMapFile, tp.targetObjFile);
            // 3. 重設玩家位置
            player->Position.x = static_cast<float>(tp.targetX);
            player->Position.y = static_cast<float>(tp.targetY);
            // 4. 重建觸發器（新的地圖裡可能有新的 teleports）
            CreateTeleportTriggers();
            break;  // 一次只能觸發一個
        }
    }
}


// void TestScene::Update(float deltaTime) {
//     elapsedTime_ += deltaTime;
//     // 将 player 世界坐标传给 MapSystem
//     int px = static_cast<int>(player->Position.x);
//     int py = static_cast<int>(player->Position.y);
//     mapSystem_->update(deltaTime, px, py);  // 原来是 (dt, 0, 0)
//     IScene::Update(deltaTime);
// }



void TestScene::Draw() const {
    // 1. 自行清空螢幕
    al_clear_to_color(al_map_rgb(0, 0, 0));
    // 2. 先把地圖畫出來
    mapSystem_->render(nullptr);

    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(&t, -mapSystem_->GetCameraX(), -mapSystem_->GetCameraY());
    al_use_transform(&t);
    // 3. 再把玩家和其他場景物件畫上去
    Group::Draw();   // 或者 Engine::Group::Draw();

    al_identity_transform(&t);
    al_use_transform(&t);
}


void TestScene::ClearTeleportTriggers() {
    for (auto* obj : teleportTriggers_) {
        RemoveObject(obj->GetObjectIterator());    // 從場景物件群裡拔掉並 delete
    }
    teleportTriggers_.clear();
}

void TestScene::CreateTeleportTriggers() {
    for (auto& tp : mapSystem_->GetTeleports()) {
        auto* trigger = new TeleportTrigger(player, mapSystem_, tp);
        AddNewObject(trigger);
        teleportTriggers_.push_back(trigger);
    }
}