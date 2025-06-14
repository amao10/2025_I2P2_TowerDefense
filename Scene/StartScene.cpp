// StartScene.cpp
#include "StartScene.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

namespace Engine {

StartScene::StartScene() = default;
StartScene::~StartScene() = default;

void StartScene::Initialize() {
    auto& engine = GameEngine::GetInstance();
    int w = engine.GetScreenSize().x;
    int h = engine.GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 1. 全螢幕背景
    auto* bg = new Sprite("login.png", 540, 360);
    AddNewObject(bg);

    // 2. 標題文字
    // AddNewObject(new Label(
    //     "My 2D Adventure",   // 可自行替換
    //     "pirulen.ttf", 100,
    //     halfW, halfH / 3,
    //     255, 255, 255, 255,
    //     0.5f, 0.5f
    // ));

    // 3. 「Start Game」按鈕
    {
        auto* startBtn = new ImageButton(
            "ui/button_up.png",    // 未按下
            "ui/button_down.png",  // 按下時
            halfW - 100, 80,    // x, y
            400, 70               // 寬, 高
        );
        startBtn->SetOnClickCallback([this]() { StartOnClick(); });
        AddNewControlObject(startBtn);

        // 按鈕文字
        AddNewObject(new Label(
            "Start Game",
            "pirulen.ttf", 36,
            halfW + 100, 120,
            0, 0, 0, 255,
            0.5f, 0.5f
        ));
    }

    // 4. 「Settings」按鈕
    // {
    //     auto* settingsBtn = new ImageButton(
    //         "ui/button_up.png",
    //         "ui/button_down.png",
    //         halfW - 200, halfH + 150,
    //         400, 100
    //     );
    //     settingsBtn->SetOnClickCallback([this]() { SettingsOnClick(); });
    //     AddNewControlObject(settingsBtn);

    //     AddNewObject(new Label(
    //         "Settings",
    //         "pirulen.ttf", 36,
    //         halfW, halfH + 200,
    //         0, 0, 0, 255,
    //         0.5f, 0.5f
    //     ));
    // }
}

void StartScene::Draw() const {
    // 先清螢幕後由 Group::Draw() 繪製所有物件 :contentReference[oaicite:1]{index=1}
    IScene::Draw();
}

void StartScene::Terminate() {
    // 釋放場景所有物件 :contentReference[oaicite:2]{index=2}
    IScene::Terminate();
}

void StartScene::StartOnClick() {
    GameEngine::GetInstance().ChangeScene("test");
}

void StartScene::SettingsOnClick() {
    GameEngine::GetInstance().ChangeScene("settings");
}

} // namespace Engine
