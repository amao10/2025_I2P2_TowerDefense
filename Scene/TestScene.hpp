#pragma once
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

#include "Map/MapSystem.hpp"
#include "Player/Player.hpp"


class TestScene final : public Engine::IScene{
public:

    MapSystem* GetMapSystem() const { return mapSystem_; }

    TestScene();
    ~TestScene();

    // 初始化場景、載入地圖
    void Initialize() override;

    // 清理釋放
    void Terminate() override;

    // 更新鏡頭位置（這裡固定在 (0,0)）
    void Update(float deltaTime) override;

    // 繪製地圖
    void Draw() const override;

private:
    MapSystem* mapSystem_;
    float elapsedTime_;
    Player* player = nullptr;

};