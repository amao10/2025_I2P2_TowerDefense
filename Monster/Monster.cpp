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
#include "Pickup/ItemPickup.hpp"    // 如果有用到
#include "Scene/TestScene.hpp"
#include "UI/Animation/DirtyEffect.hpp"    // 如果有用到
#include "UI/Animation/ExplosionEffect.hpp"
#include "UI/Component/Image.hpp"

// 獲取 TestScene 實例的輔助函數
TestScene* Monster::getTestScene() {
    return dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

// 怪物爆炸時的處理
void Monster::OnExplode() {
    getTestScene()->EffectGroup->AddNewObject(new ExplosionEffect(Position.x, Position.y));
    getTestScene()->PickupGroup->AddNewObject(new CoinPickup(Position.x, Position.y, money));
}

// 怪物構造函數
Monster::Monster(std::string img, float x, float y, float radius, float speed, float hp, int money)
    : Engine::Sprite(img, x, y), speed(speed), hp(hp), money(money), Maxhp(hp) {
    CollisionRadius = radius;
    reachEndTime = 0;
    // 初始化新增的變數
    Velocity = Engine::Point(0, 0);    // 初始速度為 0
    onGround = false;                  // 初始時不在地面上（會受重力影響掉落）

    // 巡邏相關變數的預設值（您可以在創建怪物實例時修改這些值）
    patrolMode = PatrolMode::None;
    movingRight = true;
    moveSpeed = 100.0f;    // 預設巡邏速度
    flipHorizontal = false;
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

    const int tileSize = 60;
    const int mapWidth = static_cast<int>(mapDistance[0].size());
    const int mapHeight = static_cast<int>(mapDistance.size());

    int x = static_cast<int>(floor(Position.x / tileSize));
    int y = static_cast<int>(floor(Position.y / tileSize));
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
    path[0] = Engine::Point(mapWidth - 1, mapHeight - 1);    // 目的地瓦片
}

// 怪物主要的更新邏輯
void Monster::Update(float deltaTime) {
    auto* scene = getTestScene();
    MapSystem* map = nullptr;
    if (scene) {
        map = scene->GetMapSystem();
    }

    if (!map) {
        Sprite::Update(deltaTime);
        return;
    }

    const int tileSize = 60;
    // --- 重力與 Y 軸移動 ---
    Velocity.y += 1000 * deltaTime;
    Position.y += Velocity.y * deltaTime;

    float monsterBottomY = Position.y + GetBitmapHeight() / 2.0f;
    int gridYBelow = static_cast<int>((monsterBottomY + 1) / 60);
    gridYBelow = std::clamp(gridYBelow, 0, map->mapHeight - 1);

    const auto& tileData = map->GetTileData();
    bool wasOnGround = onGround;
    onGround = false;

    int startTileX = static_cast<int>(Position.x / 90);
    int endTileX = static_cast<int>((Position.x + GetBitmapWidth() - 1) / 90);

    startTileX = std::clamp(startTileX, 0, map->mapWidth - 1);
    endTileX = std::clamp(endTileX, 0, map->mapWidth - 1);

    for (int x = startTileX; x <= endTileX; ++x) {
        if (gridYBelow >= 0 && gridYBelow < tileData.size() && x >= 0 && x < tileData[0].size()) {
            int tileId = tileData[gridYBelow][x];
            if ((tileId == 1 || tileId == 2) && Velocity.y >= 0) {
                onGround = true;
                break;
            }
        }
    }

    if (onGround) {
        Velocity.y = 0;
        Position.y = gridYBelow * 60 - GetBitmapHeight() / 2.0f;
        if (!wasOnGround) {
            //Engine::LOG(Engine::INFO) << "Monster landed! New Y position: " << Position.y;
        }
    } else {
        if (Position.y > map->mapHeight * 60 + GetBitmapHeight()) {
            hp = 0; // Monster fell off map
        }
    }


    Engine::LOG(Engine::INFO) << "Monster onGround state: " << (onGround ? "TRUE" : "FALSE");

    // --- 根據 patrolMode 決定行為 ---
    if (patrolMode == PatrolMode::BottomRow) {
        Engine::LOG(Engine::INFO) << "Monster is in PatrolMode::BottomRow. OnGround: " << (onGround ? "TRUE" : "FALSE");
        if (onGround) {
            UpdatePatrol(deltaTime, map);
        } else {
            Velocity.x = 0;
            //Engine::LOG(Engine::INFO) << "Monster not on ground, patrol horizontal velocity set to 0.";
        }
    } else {
        Velocity.x = 0;    // 預設非巡邏怪物水平不動
        //Engine::LOG(Engine::INFO) << "Monster not in PatrolMode::BottomRow (Mode: " << static_cast<int>(patrolMode) << "), horizontal velocity set to 0.";
    }

    // 更新怪物的 Rotation
    // if (Velocity.x > 0) {
    //     Rotation = 0;
    // } else if (Velocity.x < 0) {
    //     Rotation = ALLEGRO_PI;
    // }
    if (Velocity.x > 0) {
        flipHorizontal = false; // 向右走，不翻轉
    } else if (Velocity.x < 0) {
        flipHorizontal = true;  // 向左走，水平翻轉
    }
    // 加入日誌：顯示最終的速度
    //Engine::LOG(Engine::INFO) << "Monster Final Velocity: X=" << Velocity.x << " Y=" << Velocity.y;

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
    al_draw_tinted_scaled_rotated_bitmap(
        currentBitmap,
        Tint,                       // From Sprite base class
        Anchor.x * GetBitmapWidth(),
        Anchor.y * GetBitmapHeight(),
        Position.x,
        Position.y,
        Size.x / GetBitmapWidth(), // Scale X factor
        Size.y / GetBitmapHeight(),// Scale Y factor
        Rotation,                   // From Sprite base class
        flags                       // Monster-specific flip flags
    );
    // 血條繪製
    const int barWidth = 40;
    const int barHeight = 5;
    float hpRatio = hp / static_cast<float>(Maxhp);
    float barX = Position.x - barWidth / 2;
    // 計算血條的 Y 座標，讓它在怪物頭部上方
    float barY = Position.y - GetBitmapHeight() / 2 - barHeight - 5; // 距離怪物圖片上方 5 像素

    al_draw_filled_rectangle(barX, barY, barX + barWidth, barY + barHeight, al_map_rgb(100, 100, 100));     // 背景
    al_draw_filled_rectangle(barX, barY, barX + barWidth * hpRatio, barY + barHeight, al_map_rgb(255, 0, 0)); // 血量
    al_draw_rectangle(barX, barY, barX + barWidth, barY + barHeight, al_map_rgb(0, 0, 0), 1.0);          // 邊框
}

// 巡邏邏輯的具體實作
void Monster::UpdatePatrol(float deltaTime, const MapSystem* mapSystem) {
    if (!mapSystem) {
        //Engine::LOG(Engine::ERROR) << "Monster::UpdatePatrol: Map system is null!";
        return;
    }

    const int patrolTileSize = mapSystem->tileHeight;    // 假設瓦片是正方形，寬高相同

    // 怪物當前腳下的瓦片座標 (使用中心點計算，這部分保持不變，因為用於判斷當前瓦片是否可走)
    int currentTileX = static_cast<int>(floor((Position.x + GetBitmapWidth() / 2.0f) / patrolTileSize));
    int tileY = static_cast<int>(floor((Position.y + GetBitmapHeight() / 2.0f) / patrolTileSize));

    currentTileX = std::clamp(currentTileX, 0, mapSystem->mapWidth - 1);
    tileY = std::clamp(tileY, 0, mapSystem->mapHeight - 1);

    // 確保怪物在最底行
    // if (tileY != mapSystem->mapHeight - 1) {
    //     //Engine::LOG(Engine::ERROR) << "UpdatePatrol: Monster not on bottom row! Current TileY: " << tileY;
    //     Velocity.x = 0;
    //     return;
    // }

    const auto& tileData = mapSystem->GetTileData();

    // 檢查怪物當前的瓦片是否可通行
    if (tileData[tileY][currentTileX] != 1 && tileData[tileY][currentTileX] != 2) {
        //Engine::LOG(Engine::ERROR) << "UpdatePatrol: Monster patrolling on impassable tile: (" << currentTileX << ", " << tileY << "). Type: " << tileData[tileY][currentTileX];
        Velocity.x = 0;
        return;
    }

    // 左右巡邏邏輯
    if (movingRight) {
        // 預計怪物在下一幀的右邊緣X座標
        float projectedRightX = Position.x + GetBitmapWidth() + moveSpeed * deltaTime;

        // 計算預計會踩到的瓦片索引 (以怪物右邊緣為基準)
        int nextTileToCheckX = static_cast<int>(floor(projectedRightX / patrolTileSize));
        // 這裡的 gridYBelow 用於確認瓦片ID，所以使用怪物底部 Y 座標計算，確保準確性
        int gridYBelow = static_cast<int>(floor((Position.y + GetBitmapHeight()) / patrolTileSize));
        // 限制索引在有效範圍內，防止越界讀取
        gridYBelow = std::clamp(gridYBelow, 0, mapSystem->mapHeight - 1);

        // Engine::LOG(Engine::INFO) << "UpdatePatrol: Moving Right. CurrentX: " << Position.x
        //                           << ", Right Edge X: " << (Position.x + GetBitmapWidth())
        //                           << ", Projected RightX: " << projectedRightX
        //                           << ", NextTileToCheckX: " << nextTileToCheckX
        //                           << ", MapWidth: " << mapSystem->mapWidth
        //                           << ", ActualGridYBelow: " << gridYBelow; // <-- 新增的日誌

        int checkedTileID = -1; // 初始化為一個不可能的值
        // 進行邊界檢查，防止 GetTileID 越界訪問
        if (nextTileToCheckX >= 0 && nextTileToCheckX < mapSystem->mapWidth &&
            gridYBelow >= 0 && gridYBelow < mapSystem->mapHeight) {
            checkedTileID = tileData[gridYBelow][nextTileToCheckX]; // 直接從 tileData 獲取
        }
        //Engine::LOG(Engine::INFO) << "DEBUG: Checking tile (" << nextTileToCheckX << ", " << gridYBelow << ") ID: " << checkedTileID; // <-- 新增的日誌

        // 判斷是否需要折返
        // 條件1: 如果預計踩到的瓦片索引已經超出地圖寬度 (mapWidth-1 是最大有效索引)
        // 條件2: 如果預計踩到的瓦片是地圖內的，但它的ID是0 (不可通行)
        if (nextTileToCheckX >= mapSystem->mapWidth || checkedTileID == 0) { // 使用 checkedTileID
            movingRight = false;
            Velocity.x = -moveSpeed;

            // 調整位置：讓怪物右邊緣對齊最右邊可通行瓦片的右邊緣
            Position.x = (mapSystem->mapWidth - 1) * patrolTileSize + patrolTileSize - GetBitmapWidth();
            // Engine::LOG(Engine::INFO) << "DEBUG: Monster turning Left! (Reason: "
            //                           << (nextTileToCheckX >= mapSystem->mapWidth ? "Hit map right boundary" : "Tile ahead impassable") << ")"; // <-- 新增的日誌
        } else {
            Velocity.x = moveSpeed;
        }
    } else {    // movingLeft
        // 預計怪物在下一幀的左邊緣X座標
        float projectedLeftX = Position.x - moveSpeed * deltaTime;

        // 計算預計會踩到的瓦片索引 (以怪物左邊緣為基準)
        int nextTileToCheckX = static_cast<int>(floor(projectedLeftX / patrolTileSize));
        // 這裡的 gridYBelow 用於確認瓦片ID，所以使用怪物底部 Y 座標計算，確保準確性
        int gridYBelow = static_cast<int>(floor((Position.y + GetBitmapHeight()) / patrolTileSize));
        // 限制索引在有效範圍內，防止越界讀取
        gridYBelow = std::clamp(gridYBelow, 0, mapSystem->mapHeight - 1);

        // Engine::LOG(Engine::INFO) << "UpdatePatrol: Moving Left. CurrentX: " << Position.x
        //                           << ", Left Edge X: " << Position.x
        //                           << ", Projected LeftX: " << projectedLeftX
        //                           << ", NextTileToCheckX: " << nextTileToCheckX
        //                           << ", ActualGridYBelow: " << gridYBelow; // <-- 新增的日誌
        
        int checkedTileID = -1; // 初始化為一個不可能的值
        // 進行邊界檢查，防止 GetTileID 越界訪問
        if (nextTileToCheckX >= 0 && nextTileToCheckX < mapSystem->mapWidth &&
            gridYBelow >= 0 && gridYBelow < mapSystem->mapHeight) {
            checkedTileID = tileData[gridYBelow][nextTileToCheckX]; // 直接從 tileData 獲取
        }
        //Engine::LOG(Engine::INFO) << "DEBUG: Checking tile (" << nextTileToCheckX << ", " << gridYBelow << ") ID: " << checkedTileID; // <-- 新增的日誌

        // 如果即將走出地圖左邊界，或者前方瓦片是不可通行區域 (ID 0)
        if (nextTileToCheckX < 0 || checkedTileID == 0) { // 使用 checkedTileID
            movingRight = true;
            Velocity.x = moveSpeed;

            // 調整位置：讓怪物左邊緣對齊最左邊可通行瓦片的左邊緣
            Position.x = 0 * patrolTileSize;
            // Engine::LOG(Engine::INFO) << "DEBUG: Monster turning Right! (Reason: "
            //                           << (nextTileToCheckX < 0 ? "Hit map left boundary" : "Tile ahead impassable") << ")"; // <-- 新增的日誌
        } else {
            Velocity.x = -moveSpeed;
        }
    }
    //Engine::LOG(Engine::INFO) << "UpdatePatrol: Final X Velocity: " << Velocity.x;
}