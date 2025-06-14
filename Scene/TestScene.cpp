#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <map>
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
#include "Monster/BossMonster.hpp"
#include "Map/MapSystem.hpp"

static void SavePlayerStatus(const std::string& path, Player* p) {
    std::ofstream ofs(path);
    if (!ofs.is_open()) return;
    ofs << "PositionX=" << int(p->Position.x) << "\n";
    ofs << "PositionY=" << int(p->Position.y) << "\n";
    ofs << "Speed="    << p->GetSpeed()   << "\n";
    ofs << "HP="       << p->GetHP()      << "\n";
    ofs << "MP="       << p->GetMP()      << "\n";
    ofs << "Level="    << p->GetLevel()    << "\n";
    ofs << "Attack="   << p->GetAtk()  << "\n";
    ofs << "Defense="  << p->GetDef() << "\n";
    ofs << "Coin="     << p->coin    << "\n";
    ofs << "RedPotion="<< p->redPotion   << "\n";
    ofs << "BluePotion="<< p->bluePotion  << "\n";
}

TestScene::TestScene()
    : mapSystem_(nullptr), elapsedTime_(0.0f) {}

TestScene::~TestScene() {
    Terminate();
}

void TestScene::Initialize() {
    teleportTriggers_.clear();

    al_init_primitives_addon();

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

    int px = 400, py = 200;
    int speed = 300, level = 1, hp = 100, mp = 50, atk = 30, def = 0;
    int coin = 0, redPotion = 0, bluePotion = 0;

    {
        std::ifstream ifs("Resource/PlayerStatus.txt");
        if (!ifs.is_open())
            throw std::ios_base::failure("無法開啟 PlayerStatus.txt");
        std::string line;
        std::map<std::string,std::string> kv;
        auto trim = [](std::string &s) {
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            if (a == std::string::npos) { s.clear(); return; }
            s = s.substr(a, b - a + 1);
        };
        while (std::getline(ifs, line)) {
            if (line.empty() || line[0]=='#') continue;
            auto p = line.find('=');
            if (p == std::string::npos) continue;
            std::string key = line.substr(0, p);
            std::string val = line.substr(p+1);
            trim(key); trim(val);
            kv[key] = val;
        }
        px         = std::stoi(kv["PositionX"]);
        py         = std::stoi(kv["PositionY"]);
        speed      = std::stoi(kv["Speed"]);
        level      = std::stoi(kv["Level"]);
        hp         = std::stoi(kv["HP"]);
        mp         = std::stoi(kv["MP"]);
        atk        = std::stoi(kv["Attack"]);
        def        = std::stoi(kv["Defense"]);
        coin       = std::stoi(kv["Coin"]);
        redPotion  = std::stoi(kv["RedPotion"]);
        bluePotion = std::stoi(kv["BluePotion"]);
    }

    if(hp <= 0) hp = level * 100;

    player = new Player(px, py, speed, level, hp, mp, atk, def); // 可調位置和屬性
    player->coin = coin;
    player->redPotion = redPotion;
    player->bluePotion = bluePotion;
// =======
//     player = new Player(400, 200, 300, 1000, 500, 10, 10); // 可調位置和屬性
// >>>>>>> upstream/main
    AddNewObject(player); // 讓 engine 控制 update & draw

    CreateTeleportTriggers();

    bgBmp = Engine::Resources::GetInstance()
          .GetBitmap("background.png").get();

    coinBmp = Engine::Resources::GetInstance()
             .GetBitmap("ui/coin.png").get();
    redBmp  = Engine::Resources::GetInstance()
                .GetBitmap("ui/red_potion.png").get();
    blueBmp = Engine::Resources::GetInstance()
                .GetBitmap("ui/blue_potion.png").get();
    uiFont  = Engine::Resources::GetInstance()
                .GetFont("pirulen.ttf", 16).get();

    // 4. 初始化计时器
    elapsedTime_ = 0.0f;


    // 4. 初始化 Group 和 Player
    AddNewObject(MonsterGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    AddNewObject(PickupGroup = new Group());
    AddNewObject(BulletGroup = new Engine::Group());

    AddNewObject(BossOrbGroup = new Group());
    LoadMonstersForCurrentMap();
    
    //playBGM
    // AudioHelper::PlayBGM("Fairytale.ogg");
    // elapsedTime_ = 0.0f;
    bgmId_ = AudioHelper::PlayBGM("Fairytale.ogg");
    Engine::LOG(Engine::INFO) << "TestScene initialized successfully.";
}

void TestScene::Terminate() {
    AudioHelper::StopBGM(bgmId_);
    ClearTeleportTriggers();

    monsters.clear();
    respawnTimers.clear();

    IScene::Terminate();

    if (mapSystem_) {
        delete mapSystem_;
        mapSystem_ = nullptr;
    }

    player = nullptr;

    MonsterGroup = nullptr;
    EffectGroup = nullptr;
    PickupGroup = nullptr;
    BossOrbGroup = nullptr;
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
                    //Engine::LOG(Engine::ERROR) << "Respawn index " << i << " out of bounds for current map monster spawns.";
                }
            }
        }
    }
    // ---- 在所有物件都跑完 Update() 之後 ----
    TeleportPoint pendingCfg;
    bool needReload = false;
    for (auto* tp : teleportTriggers_) {
        if (tp->triggered_) {
            pendingCfg = tp->cfg_;  // 儲存要切換的設定
            needReload = true;
            break;
        }
    }
    if (needReload) {
        // 1. 卸載／載入地圖
        mapSystem_->unloadMap();
        mapSystem_->loadMap(
            pendingCfg.targetMapFile,
            pendingCfg.targetObjFile,
            pendingCfg.targetMonsterSpawnFile
        );
        // 2. 更新玩家位置
        player->Position.x = float(pendingCfg.targetX);
        player->Position.y = float(pendingCfg.targetY);
        // 3. 清除、重建傳送點（此時已不在迴圈內，不會 invalidated）
        ClearTeleportTriggers();
        CreateTeleportTriggers();
    }

    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);
    static bool prevQ = false;
    bool curQ = al_key_down(&keyState, ALLEGRO_KEY_Q);
    //按Q存檔回StartScene
    if(curQ && !prevQ){
        SavePlayerStatus("Resource/PlayerStatus.txt", player);
        Engine::GameEngine::GetInstance().ChangeScene("start");
    }
    prevQ = curQ;
    //死亡進EndScene，不存檔
    if (player->GetHP() <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("end");

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
            monster->speed = spawnInfo.initialSpeed;
            monster->hp = spawnInfo.initialHP;
            monster->Maxhp = spawnInfo.initialHP; // 通常 Maxhp 也需要設置為初始 HP
            monster->damageToPlayer = spawnInfo.initialDamage;// 使用從配置讀取的速度

            MonsterGroup->AddNewObject(monster);
            monsters[i] = monster;
            Engine::LOG(Engine::INFO) << "Monster " << i << " loaded to MonsterGroup, patrol mode set.";
        } else {
            Engine::LOG(Engine::ERROR) << "Monster creation failed for type " << (int)spawnInfo.type;
        }
    }
}
void TestScene::Draw() const {
    // 0. 先切回「螢幕座標系」畫背景
    ALLEGRO_TRANSFORM uiTrans;
    al_identity_transform(&uiTrans);
    al_use_transform(&uiTrans);
    // (如果需要清屏就打開下一行)
    // al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_scaled_bitmap(
        bgBmp,
        0, 0,
        al_get_bitmap_width(bgBmp),
        al_get_bitmap_height(bgBmp),
        0, 0,
        Engine::GameEngine::GetInstance().GetScreenSize().x,
        Engine::GameEngine::GetInstance().GetScreenSize().y,
        0
    );

    mapSystem_->render(nullptr);

    // 1. 切到「世界座標 → 螢幕座標」的相機 transform
    ALLEGRO_TRANSFORM cam;
    al_identity_transform(&cam);
    al_translate_transform(
        &cam,
        -mapSystem_->GetCameraX(),
        -mapSystem_->GetCameraY()
    );
    al_use_transform(&cam);

    // 2. 用相機 transform 畫地圖和所有世界物件
    Group::Draw();      // 玩家 + 其他物件

    // 3. 切回「螢幕座標系」──接下來畫 UI 層
    al_use_transform(&uiTrans);

    // 4. 繪製小地圖（右上角）
    {
        const float miniW   = 200;
        const float miniH   = 200;
        const float margin  = 10;
        float screenW = Engine::GameEngine::GetInstance().GetScreenSize().x;
        float screenH = Engine::GameEngine::GetInstance().GetScreenSize().y;
        float miniX   = screenW - miniW - margin;
        float miniY   = margin;

        // 小地圖背景
        al_draw_filled_rectangle(
            miniX, miniY,
            miniX + miniW, miniY + miniH,
            al_map_rgba(0, 0, 0, 150)
        );

        // 地圖格子資料
        const auto& tileData = mapSystem_->GetTileData();
        int mapH = tileData.size();
        int mapW = mapH > 0 ? tileData[0].size() : 0;
        float gridW = miniW / mapW;
        float gridH = miniH / mapH;

        // 畫 1號／2號格子
        for (int y = 0; y < mapH; ++y) {
            for (int x = 0; x < mapW; ++x) {
                int id = tileData[y][x];
                if (id == 1) {
                    al_draw_filled_rectangle(
                        miniX + x*gridW,       miniY + y*gridH,
                        miniX + (x+1)*gridW,   miniY + (y+1)*gridH,
                        al_map_rgb(200,200,200)
                    );
                }
                else if (id == 2) {
                    al_draw_filled_rectangle(
                        miniX + x*gridW,       miniY + y*gridH,
                        miniX + (x+1)*gridW,   miniY + (y+1)*gridH,
                        al_map_rgb(150,150,150)
                    );
                }
            }
        }

        // 畫傳送點
        for (auto& tp : mapSystem_->GetTeleports()) {
            float cx = miniX + (tp.x + 0.5f) * gridW;
            float cy = miniY + (tp.y + 0.5f) * gridH;
            float r  = std::min(gridW, gridH) * 0.3f;
            al_draw_filled_circle(cx, cy, r, al_map_rgb(255,0,0));
        }

        // 畫玩家位置
        float mapPixelW = mapW * mapSystem_->tileWidth;
        float mapPixelH = mapH * mapSystem_->tileHeight;
        float px = player->Position.x / mapPixelW * miniW;
        float py = player->Position.y / mapPixelH * miniH;
        float pr = std::min(gridW, gridH) * 0.4f;
        al_draw_filled_circle(miniX + px, miniY + py, pr, al_map_rgb(0,0,255));
    }

    // 5. 最後畫 UI：金幣、藥水圖示＆數量
    const int uiX = 20, uiY = 20, spacing = 40;
    // 金幣
    al_draw_bitmap(coinBmp, uiX, uiY, 0);
    Engine::Label(
        std::to_string(player->coin),
        "pirulen.ttf", 16,
        uiX + 32, uiY + 8,
        255, 255, 0, 255
    ).Draw();
    // 紅藥水
    al_draw_bitmap(redBmp, uiX, uiY + spacing, 0);
    Engine::Label(
        std::to_string(player->redPotion),
        "pirulen.ttf", 16,
        uiX + 32, uiY + spacing + 8,
        255,   0,   0, 255
    ).Draw();
    // 藍藥水
    al_draw_bitmap(blueBmp, uiX, uiY + spacing*2, 0);
    Engine::Label(
        std::to_string(player->bluePotion),
        "pirulen.ttf", 16,
        uiX + 32, uiY + spacing*2 + 8,
          0,   0, 255, 255
    ).Draw();
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
        case MonsterType::Boss:
            Engine::LOG(Engine::INFO) << "Creating BossMonster.";
            return new BossMonster(x,y);
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