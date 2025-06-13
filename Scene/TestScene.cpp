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
    CreateTeleportTriggers();

    // 3. 嘗試載入地圖
    try {
        mapSystem_->loadMap("Resource/testMap.txt", "Resource/testObject.txt");
        Engine::LOG(Engine::INFO) << "Map loaded successfully.";
        Engine::LOG(Engine::INFO) << "Confirmed Map height is: " << mapSystem_->mapHeight;
    } catch (const std::exception& e) {
        std::cerr << "[TestScene] loadMap exception: " << e.what() << std::endl;
        Engine::LOG(Engine::ERROR) << "Map loading failed: " << e.what();
    }

    player = new Player(400, 200, 300, 100, 50, 30, 10); // 可調位置和屬性
    AddNewObject(player); // 讓 engine 控制 update & draw

    CreateTeleportTriggers();

    // 4. 初始化计时器
    elapsedTime_ = 0.0f;


    // 4. 初始化 Group 和 Player
    AddNewObject(MonsterGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    AddNewObject(PickupGroup = new Group());

    // player = new Player(400, 200, 300, 100, 50, 30, 10);
    // AddNewObject(player);

    // --- 關鍵修正點：根據地圖高度和瓦片大小計算精確的 Y 座標 ---
    const int tileSize = mapSystem_->tileHeight; // 從 MapSystem 取得瓦片高度 (60)

    // 怪物圖片的實際像素高度，您已確認為 50x50
    float monsterBitmapHeight = 50.0f; // **這裡設定為 50.0f**

    // 計算最底層瓦片的頂部 Y 座標 (mapHeight 是 12)
    float bottomRowTopY = (mapSystem_->mapHeight - 1) * tileSize; // (12 - 1) * 60 = 11 * 60 = 660

    // 計算怪物圖片中心點的 Y 座標，使其腳部剛好落在 bottomRowTopY
    float adjustedSpawnY = bottomRowTopY - (monsterBitmapHeight / 2.0f); // 660 - (50 / 2.0f) = 635

    // 5. 初始化怪物生成點與類型
    spawnPoints = {
        // 使用計算出的調整後 Y 座標 635
        Engine::Point(225, 295),  // Platform 1, standing on tile (2,1)
        Engine::Point(135, 495), // Platform 3, standing on tile (1,6)
        Engine::Point(495, 515), // Platform 4, standing on tile (5,8)
        Engine::Point(875, 635),
        Engine::Point(975, 435)   
    };
    spawnTypes = {
        MonsterType::Mushroom,
        MonsterType::Mushroom,
        MonsterType::Snail,
        MonsterType::Snail,
        MonsterType::Mushroom
    };
    monsters.resize(spawnPoints.size(), nullptr);
    respawnTimers.resize(spawnPoints.size(), 0.0f);

    // 生成初始怪物
    for (size_t i = 0; i < spawnPoints.size(); ++i) {
        Engine::Point pos = spawnPoints[i];
        Engine::LOG(Engine::INFO) << "Spawning monster at (" << pos.x << ", " << pos.y << ") type: " << (int)spawnTypes[i];
        
        Monster* monster = createMonsterByType(spawnTypes[i], pos.x, pos.y);
        if (monster) {
            // 為怪物設定巡邏模式和速度
            monster->patrolMode = Monster::PatrolMode::BottomRow;
            monster->movingRight = true;  // 初始向右移動
            monster->moveSpeed = 50.0f;  // 設定巡邏速度 (例如 120 像素/秒)

            MonsterGroup->AddNewObject(monster);
            monsters[i] = monster;
            Engine::LOG(Engine::INFO) << "Monster " << i << " added to MonsterGroup, patrol mode set.";
        } else {
            Engine::LOG(Engine::ERROR) << "Monster creation failed for type " << (int)spawnTypes[i];
        }
    }

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
    // 怪物復活邏輯
    for (size_t i = 0; i < monsters.size(); ++i) {
        if (monsters[i] == nullptr || monsters[i]->Removed()) {
            respawnTimers[i] += deltaTime;
            if (respawnTimers[i] >= 5.0f) {
                Engine::Point pos = spawnPoints[i];
                Engine::LOG(Engine::INFO) << "Monster respawn at (" << pos.x << ", " << pos.y << ") type: " << (int)spawnTypes[i];
                
                Monster* monster = createMonsterByType(spawnTypes[i], pos.x, pos.y);
                if (monster) {
                    // 復活的怪物也要設定巡邏模式和速度
                    monster->patrolMode = Monster::PatrolMode::BottomRow;
                    monster->movingRight = true;
                    monster->moveSpeed = 120.0f;

                    // 如果不需要尋路，這行可以移除
                    // auto mapDistance = mapSystem_->GetMapDistance();
                    // if (!mapDistance.empty() && !mapDistance[0].empty()) {
                    //     monster->UpdatePath(mapDistance);
                    // }

                    MonsterGroup->AddNewObject(monster);
                    monsters[i] = monster;
                    respawnTimers[i] = 0.0f;
                    Engine::LOG(Engine::INFO) << "Monster " << i << " respawned and patrol mode set.";
                } else {
                    Engine::LOG(Engine::ERROR) << "Monster respawn failed for type " << (int)spawnTypes[i];
                }
            }
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