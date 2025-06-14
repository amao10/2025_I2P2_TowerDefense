#include "BossOrb.hpp"
#include "Monster/BossMonster.hpp" // 需要包含 BossMonster 的完整定義
#include "Player/Player.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/TestScene.hpp"
#include "Engine/Group.hpp" // 用於訪問 PlayerGroup
#include "Engine/LOG.hpp"
#include <cmath> // For sin, cos

// 獲取 TestScene 實例的輔助函數
TestScene* BossOrb::getTestScene() {
    return dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

BossOrb::BossOrb(BossMonster* boss, float angleOffset)
    : Engine::Sprite("play/mistKnight_circle.png", 0, 0), // 初始位置隨意，會在 Update 中計算
      parentBoss(boss),
      orbitRadius(100.0f), // 調整軌道半徑
      orbitSpeed(3.1415926 / 1.5f), // 調整軌道速度，例如每 1.5 秒轉一圈
      currentAngle(angleOffset), // 初始角度偏移
      CollisionRadius(20.0f), // 球的碰撞半徑
      damageToPlayer(20),     // 球的傷害值
      attackCooldown(0.0f) {
    // 調整球的尺寸，如果 mistKnight_circle.png 很大
    Size = Engine::Point(40, 40); // 假設球的大小為 40x40 像素
}

void BossOrb::Update(float deltaTime) {
    Sprite::Update(deltaTime); // 呼叫基類的 Sprite 更新

    if (!parentBoss) {
        //Engine::LOG(Engine::ERROR) << "BossOrb has no parent boss!";
        return;
    }

    // 更新攻擊冷卻
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
    }

    // 更新當前角度
    currentAngle += orbitSpeed * deltaTime;
    if (currentAngle >= 2 * 3.1415926) {
        currentAngle -= 2 * 3.1415926;
    }

    // 計算球的新位置
    // Boss Monster 的中心作為軌道中心
    float bossCenterX = parentBoss->Position.x + parentBoss->GetBitmapWidth() / 2.0f;
    float bossCenterY = parentBoss->Position.y + parentBoss->GetBitmapHeight() / 2.0f;

    // 計算球的相對位置
    float relativeX = orbitRadius * std::cos(currentAngle);
    float relativeY = orbitRadius * std::sin(currentAngle);

    // 設置球的絕對位置 (左上角)
    Position.x = bossCenterX + relativeX - GetBitmapWidth() / 2.0f;
    Position.y = bossCenterY + relativeY - GetBitmapHeight() / 2.0f;

    // --- 碰撞檢測與傷害玩家 ---
    TestScene* scene = getTestScene();
    if (scene && attackCooldown <= 0) {
        Player* player = scene->GetPlayer();
        if (player) {
            // 計算球和玩家中心點之間的距離
            Engine::Point orbCenter(Position.x + GetBitmapWidth() / 2.0f, Position.y + GetBitmapHeight() / 2.0f);
            Engine::Point playerCenter(player->Position.x + player->GetBitmapWidth() / 2.0f, player->Position.y + player->GetBitmapHeight() / 2.0f);

            float distance = std::hypot(orbCenter.x - playerCenter.x, orbCenter.y - playerCenter.y);
            float combinedRadius = CollisionRadius + player->CollisionRadius; // 假設玩家也有 CollisionRadius

            if (distance < combinedRadius) {
                // 發生碰撞，對玩家造成傷害
                player->TakeDamage(damageToPlayer);
                //Engine::LOG(Engine::INFO) << "Player hit by BossOrb! Player HP: " << player->hp;
                attackCooldown = 0.5f; // 設置攻擊冷卻，防止頻繁傷害
            }
        }
    }
}

void BossOrb::Draw() const {
    Sprite::Draw(); // 呼叫基類的 Sprite 繪製，它會處理圖片和透明度
    // 如果需要，可以在這裡添加球的特殊繪製效果 (例如光暈)
}