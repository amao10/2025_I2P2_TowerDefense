// StartScene.hpp
#pragma once

#include "Engine/IScene.hpp"

namespace Engine {
    class StartScene : public IScene {
    public:
        StartScene();
        ~StartScene() override;

        // 建置場景物件（背景、按鈕、文字）
        void Initialize() override;

        // 繪製場景：會先清螢幕再呼叫 Group::Draw()
        void Draw() const override;

        // 終止場景：清除所有物件
        void Terminate() override;

    private:
        // 點擊「Start Game」後執行
        void StartOnClick();

        // 點擊「Settings」後執行
        void SettingsOnClick();
    };
}
