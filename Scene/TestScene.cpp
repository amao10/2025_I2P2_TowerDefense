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

    // 4. 初始化计时器
    elapsedTime_ = 0.0f;
}


void TestScene::Terminate() {
    if (mapSystem_) {
        delete mapSystem_;
        mapSystem_ = nullptr;
    }
    IScene::Terminate();
}

void TestScene::Update(float deltaTime) {
    elapsedTime_ += deltaTime;
    // 固定鏡頭在 (0,0) 以測試整張地圖的渲染
    mapSystem_->update(deltaTime, 0, 0);
}

void TestScene::Draw() const {
    // 清空螢幕
    IScene::Draw();
    // 直接將地圖繪製到後備緩衝區
    mapSystem_->render(nullptr);
}