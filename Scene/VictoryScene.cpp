#include <functional>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

#include "VictoryScene.hpp"
#include "PlayScene.hpp"
//using namespace Engine;
VictoryScene::VictoryScene() = default;
VictoryScene::~VictoryScene() = default;

void VictoryScene::Initialize() {
    auto& engine = Engine::GameEngine::GetInstance();
    int w = engine.GetScreenSize().x;
    int h = engine.GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 1. 全螢幕背景（可替換為你專案中的「Game Over」圖片）
    AddNewObject(new Engine::Image(
        "lose/benjamin-happy.png", // 圖片路徑
        halfW, halfH,              // 中心點
        0, 0,                      // 原圖大小自動縮放
        0.5f, 0.5f                 // 錨點置中
    ));

    // 2. 文字提示
    AddNewObject(new Engine::Label(
        "Game Over",               // 顯示文字
        "pirulen.ttf",             // 字型
        48,                        // 字級
        halfW, halfH / 4 + 10,     // 位置
        255, 255, 255, 255,        // 顏色（白）
        0.5f, 0.5f                 // 錨點置中
    ));

    // 3. 重試按鈕
    auto* btn = new Engine::ImageButton(
        "ui/button_up.png",        // 未按下
        "ui/button_down.png",      // 按下時
        halfW - 200, halfH * 7 / 4 - 50,
        400, 100
    );
    btn->SetOnClickCallback(std::bind(&VictoryScene::BackOnClick, this, 0));
    AddNewControlObject(btn);

    AddNewObject(new Engine::Label(
        "Retry",                   // 按鈕文字
        "pirulen.ttf", 48,
        halfW, halfH * 7 / 4,
        0, 0, 0, 255,              // 顏色（黑）
        0.5f, 0.5f
    ));

    // 4. 背景音（可換成你的 gameover 音效）
    bgmInstance = AudioHelper::PlaySample(
        "astronomia.ogg",          // 音檔路徑
        false,
        AudioHelper::BGMVolume,
        PlayScene::DangerTime       // 可自由指定長度
    );
}

void VictoryScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance.reset();
    IScene::Terminate();
}

void VictoryScene::BackOnClick(int) {
    // 回到重試場景，依專案需求改成 "test" 或其他
    Engine::GameEngine::GetInstance().ChangeScene("start");
}
