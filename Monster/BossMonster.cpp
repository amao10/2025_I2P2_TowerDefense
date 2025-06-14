#include <string>
#include <cmath> // For std::hypot
#include <vector> // For std::vector

#include "BossMonster.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/TestScene.hpp"
#include "Player/Player.hpp" // 確保 Player.hpp 的路徑正確
#include "Engine/Group.hpp" // 用於訪問 Group 及其方法
#include "Engine/LOG.hpp" // 用於日誌輸出

// 常數定義，用於數學計算，例如 Pi
// 如果你的 Engine/Point.hpp 或其他公共頭文件中有定義 Engine::MATH_PI，建議使用它
const float PI_F = 3.1415926535f;

// BossMonster 構造函數
// 初始化 BossMonster 的基本屬性並創建環繞的球體
BossMonster::BossMonster(int x, int y) : Monster("play/mistKnight.png", x, y, 80, 50, 100, 100) {
    damageToPlayer = 20; // Boss 對玩家的基礎傷害
    speed = 80.0f;      // Boss 的移動速度
    patrolMode = PatrolMode::None; // Boss 不使用巡邏模式，將追蹤玩家
    hp = 100;
    // 在構造時創建 Boss 的環繞球體
    CreateOrbs();
}

// BossMonster 析構函數
// 當 BossMonster 被銷毀時，呼叫 DestroyOrbs 來清理其創建的球體，防止記憶體洩漏
BossMonster::~BossMonster() {
    DestroyOrbs();
}

// 創建環繞 Boss 的球體
void BossMonster::CreateOrbs() {
    // 創建三個 BossOrb 實例，並賦予它們不同的初始角度偏移，使其分佈均勻
    orbs.emplace_back(new BossOrb(this, 0.0f));                  // 第一個球，0 度
    orbs.emplace_back(new BossOrb(this, 2 * PI_F / 3.0f));       // 第二個球，120 度
    orbs.emplace_back(new BossOrb(this, 4 * PI_F / 3.0f));       // 第三個球，240 度

    // 獲取當前遊戲場景的實例
    TestScene* scene = getTestScene();
    // 檢查場景和 BossOrbGroup 是否有效，然後將球體添加到場景的 BossOrbGroup 中
    if (scene && scene->BossOrbGroup) {
        for (BossOrb* orb : orbs) {
            scene->BossOrbGroup->AddNewObject(orb);
            // 可選：用於除錯的日誌輸出
            // Engine::LOG(Engine::INFO) << "Added BossOrb to BossOrbGroup!";
        }
    } else {
        // 可選：用於除錯的錯誤日誌
        // Engine::LOG(Engine::ERROR) << "BossMonster::CreateOrbs: TestScene or BossOrbGroup is null. Orbs not added to scene.";
    }
}

// 銷毀 BossMonster 及其環繞的球體
void BossMonster::DestroyOrbs() {
    // 獲取當前遊戲場景的實例
    TestScene* scene = getTestScene();

    // 檢查場景和 BossOrbGroup 是否有效
    if (scene && scene->BossOrbGroup) {
        // 遍歷 BossMonster 擁有的所有 BossOrb
        for (BossOrb* orb : orbs) { // 使用 'orb' 作為迴圈變數名稱
            // *** 關鍵修正：使用 GetObjectList() 來獲取內部列表的引用 ***
            auto& group_internal_list = scene->BossOrbGroup->GetObjectList();

            // 在 Group 的內部列表中尋找當前的 BossOrb，並透過迭代器將其移除
            for (auto it = group_internal_list.begin(); it != group_internal_list.end(); ++it) {
                // 列表儲存的是 std::pair<bool, IObject*>，我們比較 IObject* 指針
                if (it->second == orb) { // 檢查物件指針是否匹配
                    scene->BossOrbGroup->RemoveObject(it); // 使用迭代器移除物件
                    break; // 找到並移除後，跳出內部迴圈，處理下一個 orb
                }
            }
            //delete orb; // 釋放 BossOrb 物件所佔用的記憶體
        }
    }
    orbs.clear(); // 清空儲存 BossOrb 指針的向量
    // 可選：用於除錯的日誌輸出
    // Engine::LOG(Engine::INFO) << "Destroyed all BossOrbs.";
}


// BossMonster 的更新邏輯
void BossMonster::Update(float deltaTime) {
    // 獲取當前遊戲場景的實例
    TestScene* scene = getTestScene();
    if (!scene) {
        Sprite::Update(deltaTime); // 仍然呼叫基類的 Sprite 更新
        return;
    }

    // 獲取玩家實例
    Player* player = scene->GetPlayer();
    if (!player) {
        Sprite::Update(deltaTime); // 仍然呼叫基類的 Sprite 更新
        // 可選：用於除錯的錯誤日誌
        // Engine::LOG(Engine::ERROR) << "BossMonster::Update: Player is null!";
        return;
    }

    // 更新攻擊冷卻時間
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
    }

    // --- 追蹤玩家邏輯 ---
    // 計算 Boss Monster 的中心點
    Engine::Point bossCenter(Position.x + GetBitmapWidth() / 2.0f, Position.y + GetBitmapHeight() / 2.0f);
    // 計算玩家的中心點
    Engine::Point playerCenter(player->Position.x + player->GetBitmapWidth() / 2.0f, player->Position.y + player->GetBitmapHeight() / 2.0f);

    // 計算從 Boss 到玩家的方向向量
    Engine::Point direction = playerCenter - bossCenter;
    float distance = std::hypot(direction.x, direction.y); // 計算歐幾里得距離

    float attackRange = 100.0f; // Boss 自身的攻擊範圍閾值
    if (distance < attackRange) {
        Velocity = Engine::Point(0, 0); // 如果玩家在攻擊範圍內，Boss 停止移動
        // 在這裡可以觸發 Boss 對玩家的攻擊邏輯，例如：
        // if (attackCooldown <= 0) { player->TakeDamage(damageToPlayer); attackCooldown = 1.0f; }
    } else {
        // 標準化方向向量並乘以移動速度
        direction = direction.Normalize();
        Velocity.x = direction.x * speed;
        Velocity.y = direction.y * speed;
    }

    // 應用水平和垂直移動
    Position.x += Velocity.x * deltaTime;
    Position.y += Velocity.y * deltaTime;

    // 根據水平移動方向設置 Boss 的圖片翻轉
    if (Velocity.x > 0) {
        flipHorizontal = false; // 向右走，不翻轉
    } else if (Velocity.x < 0) {
        flipHorizontal = true;  // 向左走，水平翻轉
    } else {
        // 如果沒有水平移動，根據玩家相對位置決定朝向
        if (playerCenter.x < bossCenter.x) {
            flipHorizontal = true;
        } else if (playerCenter.x > bossCenter.x) {
            flipHorizontal = false;
        }
    }

    // 最後呼叫基類的 Sprite 更新方法，處理基礎繪圖、透明度等
    Sprite::Update(deltaTime);
}