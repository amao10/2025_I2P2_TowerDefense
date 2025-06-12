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
    // 3. 再把玩家和其他場景物件畫上去
    Group::Draw();   // 或者 Engine::Group::Draw();
}

// void TestScene::Draw() const {
//     // 清空螢幕
//     IScene::Draw();
//     // 直接將地圖繪製到後備緩衝區
//     mapSystem_->render(nullptr);
// }