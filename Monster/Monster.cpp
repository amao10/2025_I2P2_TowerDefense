#include "Monster.hpp"

#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>

#include <algorithm>    // for std::clamp
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Map/MapSystem.hpp"
#include "Pickup/CoinPickup.hpp"
#include "Pickup/ItemPickup.hpp"
#include "Scene/TestScene.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/ExplosionEffect.hpp"
#include "UI/Component/Image.hpp"

// 獲取 TestScene 實例的輔助函數
TestScene* Monster::getTestScene() {
    return dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

// 怪物爆炸時的處理
void Monster::OnExplode() {
    // Position 是左上角，所以中心點是 Position + (width/2, height/2)
    float centerX = Position.x + GetBitmapWidth() / 2.0f;
    float centerY = Position.y + GetBitmapHeight() / 2.0f;
    getTestScene()->GetPlayer()->GainExp(20);
    getTestScene()->EffectGroup->AddNewObject(new ExplosionEffect(centerX, centerY));
    getTestScene()->PickupGroup->AddNewObject(new CoinPickup(centerX, centerY, money));
}

// 怪物構造函數
Monster::Monster(std::string img, float x, float y, float radius, float speed, float hp, int money)
    : Engine::Sprite(img, x, y), speed(speed), hp(hp), money(money), Maxhp(hp) {
    CollisionRadius = radius;
    reachEndTime = 0;
    // 初始化新增的變數
    Velocity = Engine::Point(0, 0);     // 初始速度為 0
    onGround = false;                   // 初始時不在地面上（會受重力影響掉落）
    damageToPlayer = 10; // 預設傷害值，請根據您的怪物類型調整
    attackCooldown = 0.0f; 
    // 巡邏相關變數的預設值（您可以在創建怪物實例時修改這些值）
    patrolMode = PatrolMode::None;
    movingRight = true;
    moveSpeed = 30.0f;      // 進一步降低巡邏速度到 30.0f
    flipHorizontal = false;
    // 新增用於緩衝抖動的計時器
    turnAroundCooldown = 0.0f;
    this->damageToPlayer = 10; // <--- 這裡給一個預設值，例如 10
    this->attackCooldown = 0.0f; // 初始時可以攻擊

    // 確保不使用構造函數傳入的 speed 參數影響巡邏
    Engine::LOG(Engine::INFO) << "Monster created with constructor speed: " << speed 
                             << ", but using moveSpeed: " << moveSpeed;
}

// 判斷怪物是否應該被移除
bool Monster::Removed() const {
    return hp <= 0;
}

// 怪物受到傷害
void Monster::Hit(float damage) {
    hp -= damage;
    if (hp <= 0) {
        OnExplode();
        // 建議這裡使用一個標誌來標記怪物為「待移除」，而不是直接移除
        // 因為直接在 Update 循環中移除物件可能導致迭代器失效
        getTestScene()->MonsterGroup->RemoveObject(objectIterator);
        AudioHelper::PlayAudio("explosion.wav");
    }
}

// 尋路方法 (如果不需要怪物尋路，這個函數將不會被呼叫)
void Monster::UpdatePath(const std::vector<std::vector<int>>& mapDistance) {
    // 這裡的邏輯是為尋路設計的，如果怪物只巡邏，則不會執行。
    // 如果您看到這裡的 LOG，表示有程式碼仍然嘗試為巡邏怪物呼叫此函數。

    auto* scene = getTestScene();
    MapSystem* map = nullptr;
    if (scene) {
        map = scene->GetMapSystem();
    }
    if (!map) {
        Engine::LOG(Engine::ERROR) << "Monster::UpdatePath: Map system is null!";
        path.clear();
        return;
    }

    const int tileSizeX = map->tileWidth;
    const int tileSizeY = map->tileHeight;

    const int mapWidth = static_cast<int>(mapDistance[0].size());
    const int mapHeight = static_cast<int>(mapDistance.size());

    // Position 是左上角，計算怪物中心點所在的瓦片
    float centerX = Position.x + GetBitmapWidth() / 2.0f;
    float centerY = Position.y + GetBitmapHeight() / 2.0f;
    
    int x = static_cast<int>(floor(centerX / tileSizeX));
    int y = static_cast<int>(floor(centerY / tileSizeY));
    x = std::clamp(x, 0, mapWidth - 1);
    y = std::clamp(y, 0, mapHeight - 1);

    Engine::Point pos(x, y);
    int num = mapDistance[y][x];

    if (num == -1) {
        path.clear();
        return;
    }

    path = std::vector<Engine::Point>(num + 1);

    std::vector<Engine::Point> directions = {
        Engine::Point(1, 0), Engine::Point(-1, 0),
        Engine::Point(0, 1), Engine::Point(0, -1)};

    while (num != 0) {
        std::vector<Engine::Point> nextHops;
        for (auto& dir : directions) {
            int nx = pos.x + dir.x;
            int ny = pos.y + dir.y;
            if (nx < 0 || nx >= mapWidth || ny < 0 || ny >= mapHeight)
                continue;
            if (mapDistance[ny][nx] == num - 1)
                nextHops.emplace_back(nx, ny);
        }

        if (nextHops.empty()) {
            path.clear();
            return;
        }

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> dist(0, nextHops.size() - 1);
        pos = nextHops[dist(rng)];
        path[num] = pos;
        num--;
    }
    path[0] = Engine::Point(mapWidth - 1, mapHeight - 1);     // 目的地瓦片
}

// 怪物主要的更新邏輯
void Monster::Update(float deltaTime) {
    auto* scene = getTestScene();
    MapSystem* map = nullptr;
    if (scene) {
        map = scene->GetMapSystem();
    }
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
    }
    if (!map) {
        Sprite::Update(deltaTime);
        return;
    }

    // 使用 MapSystem 提供的實際瓦片尺寸
    const int tileWidth = map->tileWidth;
    const int tileHeight = map->tileHeight;

    // --- 重力與 Y 軸移動 ---
    Velocity.y += 1000 * deltaTime;
    Position.y += Velocity.y * deltaTime;

    // Y 軸碰撞檢測與地面判斷
    // Position.y 是左上角，所以怪物底部是 Position.y + GetBitmapHeight()
    float monsterBottomY = Position.y + GetBitmapHeight();
    int gridYBelow = static_cast<int>(floor((monsterBottomY + 1) / tileHeight));
    gridYBelow = std::clamp(gridYBelow, 0, map->mapHeight - 1);

    const auto& tileData = map->GetTileData();
    bool wasOnGround = onGround;
    onGround = false;

    // X 軸瓦片範圍判斷：獲取怪物「整個寬度」所跨越的瓦片範圍
    // Position.x 是左上角，所以左邊緣就是 Position.x，右邊緣是 Position.x + GetBitmapWidth()
    int startTileX = static_cast<int>(floor((Position.x + 1.0f) / tileWidth)); // 怪物左邊緣
    int endTileX = static_cast<int>(floor((Position.x + GetBitmapWidth() - 1.0f) / tileWidth)); // 怪物右邊緣

    startTileX = std::clamp(startTileX, 0, map->mapWidth - 1);
    endTileX = std::clamp(endTileX, 0, map->mapWidth - 1);

    // 檢查怪物底部所在的瓷磚是否有平台
    for (int x = startTileX; x <= endTileX; ++x) {
        if (gridYBelow >= 0 && gridYBelow < tileData.size() && x >= 0 && x < tileData[0].size()) {
            int tileId = tileData[gridYBelow][x];
            if ((tileId == 1 || tileId == 2) && Velocity.y >= 0) { // 只有當向下移動時才考慮碰撞
                onGround = true;
                break; // 只要有一個瓦片是平台就算站在地面上
            }
        }
    }

    if (onGround) {
        Velocity.y = 0;
        // 將怪物 Y 座標調整到地圖瓦片網格的正確位置
        // 讓怪物底部對齊平台頂部，Position.y 是左上角
        Position.y = gridYBelow * tileHeight - GetBitmapHeight();
        if (!wasOnGround) {
            //Engine::LOG(Engine::INFO) << "Monster landed! New Y position: " << Position.y;
        }
    } else {
        // 如果怪物掉落到地圖下方，銷毀它
        if (Position.y > map->mapHeight * tileHeight + GetBitmapHeight()) {
            hp = 0; // Monster fell off map
        }
    }

    Engine::LOG(Engine::INFO) << "Monster onGround state: " << (onGround ? "TRUE" : "FALSE");

    // --- 根據 patrolMode 決定行為 ---
    if (patrolMode == PatrolMode::BottomRow) {
        Engine::LOG(Engine::INFO) << "Monster is in PatrolMode::BottomRow. OnGround: " << (onGround ? "TRUE" : "FALSE");
        // 處理轉向冷卻
        if (turnAroundCooldown > 0) {
            turnAroundCooldown -= deltaTime;
            Velocity.x = 0; // 在冷卻期間暫停水平移動，防止抖動
            Engine::LOG(Engine::INFO) << "Monster in cooldown, Velocity.x = 0. Remaining cooldown: " << turnAroundCooldown;
        } else {
            // 只有在冷卻時間結束後才執行 UpdatePatrol
            if (onGround) {
                UpdatePatrol(deltaTime, map);
            } else {
                Velocity.x = 0;
                Engine::LOG(Engine::INFO) << "Monster not on ground, patrol horizontal velocity set to 0.";
            }
        }
    } else {
        Velocity.x = 0;     // 預設非巡邏怪物水平不動
        //Engine::LOG(Engine::INFO) << "Monster not in PatrolMode::BottomRow (Mode: " << static_cast<int>(patrolMode) << "), horizontal velocity set to 0.";
    }

    // 應用水平移動
    Position.x += Velocity.x * deltaTime;

    // 加入調試日誌：顯示實際的移動距離
    Engine::LOG(Engine::INFO) << "Monster deltaTime: " << deltaTime 
                             << ", moveSpeed: " << moveSpeed 
                             << ", Velocity.x: " << Velocity.x 
                             << ", Movement this frame: " << (Velocity.x * deltaTime);

    // 根據移動方向設置翻轉
    if (Velocity.x > 0) {
        flipHorizontal = false; // 向右走，不翻轉
    } else if (Velocity.x < 0) {
        flipHorizontal = true;  // 向左走，水平翻轉
    }
    
    // 加入日誌：顯示最終的速度
    Engine::LOG(Engine::INFO) << "Monster Final Velocity: X=" << Velocity.x << " Y=" << Velocity.y;

    Sprite::Update(deltaTime);
}

// 繪製怪物（包含血條）
void Monster::Draw() const {
    // Determine drawing flags for this Monster
    int flags = 0;
    if (flipHorizontal) { // Use the Monster's own flipHorizontal
        flags |= ALLEGRO_FLIP_HORIZONTAL;
    }

    // Get the bitmap from the base Image class (inherited by Sprite, then Monster)
    ALLEGRO_BITMAP* currentBitmap = GetBitmap(); // Assuming Image has GetBitmap()

    if (!currentBitmap) // Safety check
        return;
    if (!Visible) // Inherited from IObject
        return;

    // Draw the monster with tint, scale, rotation, and custom flip flags
    // Position.x, Position.y 是左上角，所以直接使用
    al_draw_tinted_scaled_rotated_bitmap(
        currentBitmap,
        Tint,                   // From Sprite base class
        Anchor.x * GetBitmapWidth(),
        Anchor.y * GetBitmapHeight(),
        Position.x + GetBitmapWidth() / 2.0f,  // 繪製中心點 X
        Position.y + GetBitmapHeight() / 2.0f, // 繪製中心點 Y
        Size.x / GetBitmapWidth(), // Scale X factor
        Size.y / GetBitmapHeight(),// Scale Y factor
        Rotation,                   // From Sprite base class
        flags                       // Monster-specific flip flags
    );
    
    // 血條繪製
    const int barWidth = 40;
    const int barHeight = 5;
    float hpRatio = hp / static_cast<float>(Maxhp);
    
    // Position.x, Position.y 是左上角，計算怪物中心和血條位置
    float centerX = Position.x + GetBitmapWidth() / 2.0f;
    float barX = centerX - barWidth / 2.0f;
    // 血條在怪物頂部上方 5 像素
    float barY = Position.y - barHeight - 5;

    al_draw_filled_rectangle(barX, barY, barX + barWidth, barY + barHeight, al_map_rgb(100, 100, 100));     // 背景
    al_draw_filled_rectangle(barX, barY, barX + barWidth * hpRatio, barY + barHeight, al_map_rgb(255, 0, 0)); // 血量
    al_draw_rectangle(barX, barY, barX + barWidth, barY + barHeight, al_map_rgb(0, 0, 0), 1.0);           // 邊框
}

// 巡邏邏輯的具體實作
void Monster::UpdatePatrol(float deltaTime, const MapSystem* mapSystem) {
    if (!mapSystem) {
        Engine::LOG(Engine::ERROR) << "Monster::UpdatePatrol: Map system is null!";
        return;
    }

    const int patrolTileWidth = mapSystem->tileWidth; // 90
    const int patrolTileHeight = mapSystem->tileHeight; // 60

    float monsterWidth = GetBitmapWidth(); // 50
    float monsterHeight = GetBitmapHeight(); // 50

    // Position.x, Position.y 是左上角
    // 獲取怪物中心點所在的瓦片座標
    float monsterCenterX = Position.x + monsterWidth / 2.0f;
    int monsterCenterTileX = static_cast<int>(floor(monsterCenterX / patrolTileWidth));
    
    // 獲取怪物底部所在的瓦片 Y 座標 (腳下)
    float monsterBottomY = Position.y + monsterHeight;
    int currentYTileAtBottom = static_cast<int>(floor((monsterBottomY - 1.0f) / patrolTileHeight)); // 減去 1.0f 確保在瓦片內

    // 確保瓦片座標在有效範圍內，防止越界訪問
    monsterCenterTileX = std::clamp(monsterCenterTileX, 0, mapSystem->mapWidth - 1);
    currentYTileAtBottom = std::clamp(currentYTileAtBottom, 0, mapSystem->mapHeight - 1);

    const auto& tileData = mapSystem->GetTileData();

    // 在執行任何移動前，確保怪物不在冷卻中
    if (turnAroundCooldown > 0) {
        Velocity.x = 0; // 冷卻期間保持靜止
        Engine::LOG(Engine::INFO) << "Monster in cooldown, Velocity.x = " << Velocity.x << ". Remaining cooldown: " << turnAroundCooldown;
        turnAroundCooldown -= deltaTime;
        if (turnAroundCooldown < 0) turnAroundCooldown = 0; // 避免負數
        return;
    }

    // 判斷下一個要檢查的瓦片 X 座標
    int nextCheckTileX;
    float lookAheadOffset = 30.0f; // 提前判斷的像素距離

    if (movingRight) {
        // 向右走時，檢查怪物右邊緣 + lookAheadOffset 處的瓦片
        float rightEdge = Position.x + monsterWidth;
        nextCheckTileX = static_cast<int>(floor((rightEdge + lookAheadOffset) / patrolTileWidth));
    } else {
        // 向左走時，檢查怪物左邊緣 - lookAheadOffset 處的瓦片
        float leftEdge = Position.x;
        nextCheckTileX = static_cast<int>(floor((leftEdge - lookAheadOffset) / patrolTileWidth));
    }

    // 檢查 Y 座標 (始終檢查怪物底部下方一格的瓦片，這是懸崖判斷的 Y 座標)
    int checkYBelow = currentYTileAtBottom + 1;

    bool turnAround = false;

    // Log 輸出，方便調試
    Engine::LOG(Engine::INFO) << "UpdatePatrol: Moving " << (movingRight ? "Right" : "Left")
                             << ". CurrentX: " << Position.x
                             << ", MonsterCenterTileX: " << monsterCenterTileX
                             << ", NextCheckTileX: " << nextCheckTileX
                             << ", CurrentYTileAtBottom: " << currentYTileAtBottom
                             << ", CheckYBelow: " << checkYBelow
                             << ", MapWidth: " << mapSystem->mapWidth
                             << ", MapHeight: " << mapSystem->mapHeight;

    // 1. 檢查是否超出地圖邊界 (nextCheckTileX)
    if (nextCheckTileX < 0 || nextCheckTileX >= mapSystem->mapWidth) {
        turnAround = true;
        Engine::LOG(Engine::INFO) << "DEBUG: Monster turning " << (movingRight ? "Left" : "Right") << "! (Reason: Hit map boundary)";
    } else {
        // 2. 檢查前方瓦片本身是否是不可通行的實體 (ID 1 或 2)
        // 判斷怪物前方的瓦片 (在 currentYTileAtBottom 這一層) 是否為實體方塊
        if (currentYTileAtBottom >= 0 && currentYTileAtBottom < mapSystem->mapHeight) {
            int frontTileID = tileData[currentYTileAtBottom][nextCheckTileX];
            if (frontTileID == 1 || frontTileID == 2) { // 如果前方是實體瓦片，則轉向
                turnAround = true;
                Engine::LOG(Engine::INFO) << "DEBUG: Monster turning " << (movingRight ? "Left" : "Right") << "! (Reason: Front tile is solid block, ID: " << frontTileID << ")";
            }
        } else {
            // 如果 currentYTileAtBottom 超出範圍，這是不正常的，也轉向
            turnAround = true;
            Engine::LOG(Engine::INFO) << "DEBUG: Monster turning " << (movingRight ? "Left" : "Right") << "! (Reason: Current Y Tile at Bottom Out of Bounds)";
        }

        // 3. 檢查前方瓦片下方是否是懸崖 (即下方瓦片是空的 ID 0)
        if (!turnAround) { // 只有當前面沒有觸發轉向時才檢查懸崖
            if (checkYBelow >= 0 && checkYBelow < mapSystem->mapHeight) {
                int tileBelowFrontID = tileData[checkYBelow][nextCheckTileX];
                if (tileBelowFrontID == 0) { // 如果下方瓦片是 ID 0 (空氣/懸崖)，則轉向
                    turnAround = true;
                    Engine::LOG(Engine::INFO) << "DEBUG: Monster turning " << (movingRight ? "Left" : "Right") << "! (Reason: Cliff ahead, TileBelowFrontID: " << tileBelowFrontID << ")";
                }
            } else {
                // 如果下方超出地圖範圍，也視為懸崖
                turnAround = true;
                Engine::LOG(Engine::INFO) << "DEBUG: Monster turning " << (movingRight ? "Left" : "Right") << "! (Reason: Cliff ahead, below map boundary)";
            }
        }
    }

    // 執行轉向
    if (turnAround) {
        movingRight = !movingRight;
        Velocity.x = (movingRight ? moveSpeed : -moveSpeed);
        turnAroundCooldown = 0.5f; // 設置冷卻時間

        // 轉向後，將怪物位置調整到安全位置，避免立即再次觸發轉向
        float safetyBuffer = 20.0f; // 增加安全偏移量

        if (movingRight) { // 剛才向左走，現在向右轉
            // 將怪物左邊緣（Position.x）拉到 monsterCenterTileX 瓦片的左邊界 + safetyBuffer 處
            Position.x = monsterCenterTileX * patrolTileWidth + safetyBuffer;
        } else { // 剛才向右走，現在向左轉
            // 將怪物右邊緣拉到 (monsterCenterTileX + 1) 瓦片的右邊界 - safetyBuffer 處
            // Position.x 是左上角，所以右邊緣是 Position.x + monsterWidth
            Position.x = (monsterCenterTileX + 1) * patrolTileWidth - monsterWidth - safetyBuffer;
        }

        Engine::LOG(Engine::INFO) << "Monster turned around. New Velocity.x: " << Velocity.x
                                 << ". Monster position adjusted to: " << Position.x;
    } else {
        Velocity.x = (movingRight ? moveSpeed : -moveSpeed);
    }

    Engine::LOG(Engine::INFO) << "UpdatePatrol: Final X Velocity: " << Velocity.x;
}