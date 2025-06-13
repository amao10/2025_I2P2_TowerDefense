#pragma once
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

#include "Map/MapSystem.hpp"
#include "Map/Teleport.hpp"
#include "Player/Player.hpp"
#include "Monster/Monster.hpp"
#include "Engine/Group.hpp"
#include "Engine/Collider.hpp"


enum class MonsterType {
    Mushroom,
    Snail,
    UNKNOWN
};
class Monster;
class TestScene final : public Engine::IScene{
public:

    MapSystem* GetMapSystem() const { return mapSystem_; }
    Player* GetPlayer() const { return player; }
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

    Group* PickupGroup;
    Group* EffectGroup; 
    Group* MonsterGroup;

private:
    MapSystem* mapSystem_;
    float elapsedTime_;
    Player* player = nullptr;
    
    // std::vector<Engine::Point> spawnPoints;
    // std::vector<MonsterType> spawnTypes;
    std::vector<Monster*> monsters;
    std::vector<float> respawnTimers;

    Monster* createMonsterByType(MonsterType type, float x, float y);
    void ClearTeleportTriggers();
    void CreateTeleportTriggers();
    std::vector<IObject*> teleportTriggers_;
    void LoadMonstersForCurrentMap();
};