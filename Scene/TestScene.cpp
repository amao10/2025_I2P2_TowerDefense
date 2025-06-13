#include <allegro5/allegro.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>
// using namespace std; // 通常不推薦在頭文件中使用，但在 .cpp 中可以接受

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"

#include "UI/Component/Label.hpp"

#include "TestScene.hpp"
#include "Player/Player.hpp"
#include "Monster/Monster.hpp"
#include "Monster/MushroomMonster.hpp"
#include "Monster/SnailMonster.hpp"
#include "Map/MapSystem.hpp"

TestScene::TestScene()
    : mapSystem_(nullptr), elapsedTime_(0.0f) {}

TestScene::~TestScene() {
    Terminate();
}

void TestScene::Initialize() {
    // 1. 取得顯示器
    ALLEGRO_DISPLAY* display = al_get_current_display();
    if (!display) {
        throw std::ios_base::failure("[TestScene] al_get_current_display() returned nullptr");
    }

    // 2. 建立地圖系統
    mapSystem_ = new MapSystem(display);
    //CreateTeleportTriggers();

    // 3. 嘗試載入地圖
    try {
        mapSystem_->loadMap("Resource/testMap.txt", "Resource/testObject.txt","Resource/testMonsterSpawns.txt");
        Engine::LOG(Engine::INFO) << "Map loaded successfully.";
        Engine::LOG(Engine::INFO) << "Confirmed Map height is: " << mapSystem_->mapHeight;
    } catch (const std::exception& e) {
        std::cerr << "[TestScene] loadMap exception: " << e.what() << std::endl;
        Engine::LOG(Engine::ERROR) << "Map loading failed: " << e.what();
    }

    player = new Player(400, 200, 300, 100, 50, 30, 0); // 可調位置和屬性
    AddNewObject(player); // 讓 engine 控制 update & draw

    CreateTeleportTriggers();

    // 4. 初始化计时器
    elapsedTime_ = 0.0f;


    // 4. 初始化 Group 和 Player
    AddNewObject(MonsterGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    AddNewObject(PickupGroup = new Group());
    LoadMonstersForCurrentMap();
    

    elapsedTime_ = 0.0f;
    Engine::LOG(Engine::INFO) << "TestScene initialized successfully.";
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
    mapSystem_->update(deltaTime, 0, 0); // cameraX 和 cameraY 暫時固定為 0
    IScene::Update(deltaTime);

    int px = static_cast<int>(player->Position.x);
    int py = static_cast<int>(player->Position.y);
    mapSystem_->update(deltaTime, px, py);

    // 計算玩家當前格子（與 MapSystem 同樣的 tileW/H）
    const int tileW = 90;
    const int tileH = 60;
    int gx = px / tileW;
    int gy = (py + player->Size.y/2) / tileH;

    // 更新 Player
    if (player) {
        player->Update(deltaTime);
    }

    // 更新所有怪物
    MonsterGroup->Update(deltaTime); 

    // --- 新增：玩家與怪物碰撞檢測及扣血邏輯 ---
    if (player && MonsterGroup) {
        // 獲取玩家的中心點
        // 根據您的 Player::Draw() 判斷，player->Position 就是中心點
        Engine::Point playerCenter = player->Position; 
        float playerRadius = player->CollisionRadius;

        for (auto& obj : MonsterGroup->GetObjects()) {
            Monster* monster = dynamic_cast<Monster*>(obj);
            if (monster && !monster->Removed()) {
                // 獲取怪物的中心點
                // 根據您的 Monster 構造函數和 Draw() 判斷，monster->Position 是左上角
                Engine::Point monsterCenter = monster->Position + 
                                              Engine::Point(monster->GetBitmapWidth() / 2.0f, monster->GetBitmapHeight() / 2.0f);
                float monsterRadius = monster->CollisionRadius;

                // 使用 Collider 進行圓形碰撞檢測
                if (Engine::Collider::IsCircleOverlap(playerCenter, playerRadius, monsterCenter, monsterRadius)) {
                    // 碰撞發生！
                    // 接下來是處理玩家無敵幀和扣血的邏輯
                    // 我沿用之前建議的 "Player 自身處理無敵幀" 的方式，因為這通常更健壯
                    player->TakeDamage(monster->GetDamage()); 
                    // AudioHelper::PlayAudio("player_hit.wav"); // 播放受傷音效
                }
            }
        }
    }

    // 偵測任何傳送點
    // for (auto& tp : mapSystem_->GetTeleports()) {
    //     if (tp.x == gx && tp.y == gy) {
    //         // 1. 清除舊觸發器
    //         ClearTeleportTriggers();
    //         for (Monster* m : monsters) {
    //             if (m) delete m; // 確保刪除怪物物件
    //         }
    //         monsters.clear();
    //         MonsterGroup->Clear();// 清空 Group，但不會 delete 記憶體
    //         respawnTimers.clear();
    //         // 2. 換圖
    //         mapSystem_->unloadMap();
    //         mapSystem_->loadMap(tp.targetMapFile, tp.targetObjFile,tp.targetMonsterSpawnFile);
    //         // 3. 重設玩家位置
    //         player->Position.x = static_cast<float>(tp.targetX);
    //         player->Position.y = static_cast<float>(tp.targetY);
    //         // 5. 為新地圖生成怪物 (重要！)
    //         LoadMonstersForCurrentMap();
    //         // 4. 重建觸發器（新的地圖裡可能有新的 teleports）
    //         CreateTeleportTriggers();
    //         break;  // 一次只能觸發一個
    //     }
    // }
    // 怪物復活邏輯 - 需要修改為基於 mapSystem_->GetMonsterSpawns() 來獲取原始生成點
    // 您不再需要全局的 spawnPoints 和 spawnTypes
    for (size_t i = 0; i < monsters.size(); ++i) {
        if (monsters[i] == nullptr || monsters[i]->Removed()) {
            respawnTimers[i] += deltaTime;
            if (respawnTimers[i] >= 5.0f) {
                // 從 mapSystem_ 中獲取對應的原始生成資訊
                const auto& monsterSpawns = mapSystem_->GetMonsterSpawns();
                if (i < monsterSpawns.size()) { // 確保索引有效
                    const MonsterSpawnInfo& spawnInfo = monsterSpawns[i];
                    Engine::LOG(Engine::INFO) << "Monster respawn at (" << spawnInfo.position.x << ", " << spawnInfo.position.y << ") type: " << (int)spawnInfo.type;

                    Monster* monster = createMonsterByType(spawnInfo.type, spawnInfo.position.x, spawnInfo.position.y);
                    if (monster) {
                        monster->patrolMode = Monster::PatrolMode::BottomRow;
                        monster->movingRight = true;
                        // 這裡的速度應該從 spawnInfo 讀取，而不是硬編碼
                        //monster->moveSpeed = spawnInfo.initialSpeed; // 使用從配置讀取的速度

                        MonsterGroup->AddNewObject(monster);
                        monsters[i] = monster;
                        respawnTimers[i] = 0.0f;
                        Engine::LOG(Engine::INFO) << "Monster " << i << " respawned and patrol mode set.";
                    } else {
                        Engine::LOG(Engine::ERROR) << "Monster respawn failed for type " << (int)spawnInfo.type;
                    }
                } else {
                    Engine::LOG(Engine::ERROR) << "Respawn index " << i << " out of bounds for current map monster spawns.";
                }
            }
        }
    }
}

void TestScene::LoadMonstersForCurrentMap() {
    // 清空之前地圖的怪物數據 (在切換地圖前已經做過了，這裡再加一層安全檢查)
    for (Monster* m : monsters) {
        if (m) delete m;
    }
    monsters.clear();
    respawnTimers.clear();
    MonsterGroup->Clear(); // 清空 Group

    const auto& monsterSpawns = mapSystem_->GetMonsterSpawns();
    monsters.resize(monsterSpawns.size(), nullptr);
    respawnTimers.resize(monsterSpawns.size(), 0.0f);

    // 根據當前地圖的怪物生成數據生成怪物
    for (size_t i = 0; i < monsterSpawns.size(); ++i) {
        const MonsterSpawnInfo& spawnInfo = monsterSpawns[i];
        Engine::LOG(Engine::INFO) << "Loading monster at (" << spawnInfo.position.x << ", " << spawnInfo.position.y << ") type: " << (int)spawnInfo.type;

        Monster* monster = createMonsterByType(spawnInfo.type, spawnInfo.position.x, spawnInfo.position.y);
        if (monster) {
            monster->patrolMode = Monster::PatrolMode::BottomRow;
            monster->movingRight = true;
            //monster->moveSpeed = spawnInfo.initialSpeed; // 使用從配置讀取的速度

            MonsterGroup->AddNewObject(monster);
            monsters[i] = monster;
            Engine::LOG(Engine::INFO) << "Monster " << i << " loaded to MonsterGroup, patrol mode set.";
        } else {
            Engine::LOG(Engine::ERROR) << "Monster creation failed for type " << (int)spawnInfo.type;
        }
    }
}
void TestScene::Draw() const {
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

// 怪物工廠方法
Monster* TestScene::createMonsterByType(MonsterType type, float x, float y) {
    switch (type) {
        case MonsterType::Mushroom:
            Engine::LOG(Engine::INFO) << "Creating MushroomMonster.";
            // 注意：MushroomMonster 的內部構造函數可能也需要調整以匹配 Monster 的參數
            // MushroomMonster("resources/images/mushroom.png", x, y, radius, speed, hp, money)
            return new MushroomMonster(x, y); 
        case MonsterType::Snail:
            Engine::LOG(Engine::INFO) << "Creating SnailMonster.";
            // SnailMonster("resources/images/snail.png", x, y, radius, speed, hp, money)
            return new SnailMonster(x, y);
        default:
            Engine::LOG(Engine::ERROR) << "Unknown monster type: " << (int)type;
            return nullptr;
    }
}

// void TestScene::Draw() const {
//     // 1. 自行清空螢幕
//     al_clear_to_color(al_map_rgb(0, 0, 0));
//     // 2. 先把地圖畫出來
//     mapSystem_->render(nullptr);

//     ALLEGRO_TRANSFORM t;
//     al_identity_transform(&t);
//     al_translate_transform(&t, -mapSystem_->GetCameraX(), -mapSystem_->GetCameraY());
//     al_use_transform(&t);
//     // 3. 再把玩家和其他場景物件畫上去
//     Group::Draw();   // 或者 Engine::Group::Draw();

//     al_identity_transform(&t);
//     al_use_transform(&t);
// }


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