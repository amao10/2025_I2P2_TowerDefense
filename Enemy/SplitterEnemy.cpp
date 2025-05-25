#include "SplitterEnemy.hpp"
#include "MiniEnemy.hpp"       // 新增的迷你敵人
#include "Scene/PlayScene.hpp"
#include "bits/stdc++.h"
#include "Enemy.hpp"

SplitterEnemy::SplitterEnemy(int x, int y) :
    Enemy("play/enemy-6.png", x, y, 12, 45, 15, 500) {
    // 半徑、速度、HP、金錢可自由調整
}

// void SplitterEnemy::OnExplode() {
//     std::cout << "SplitterEnemy exploded! Spawning mini enemies." << std::endl;
//     PlayScene* scene = getPlayScene();
//     // 先產生兩個迷你敵人
//     for (int i = 0; i < 2; ++i) {
//         // 位置可以稍微偏移避免重疊
//         float offsetX = (i == 0) ? -10.0f : 10.0f;
//         scene->EnemyGroup->AddNewObject(new MiniEnemy(Position.x + offsetX, Position.y));
//         std::cout << "Spawned MiniEnemy at " << Position.x + offsetX << ", " << Position.y << std::endl;
//     }
//     // 再呼叫基底的OnExplode處理動畫和移除自身
//     Enemy::OnExplode();
// }

void SplitterEnemy::OnExplode() {
    Enemy::OnExplode(); // 保留原本的爆炸動畫與音效

    PlayScene* scene = getPlayScene();

    if (!scene) return;

    // 在爆炸位置產生兩個 MiniEnemy
    Engine::Point spawn1(Position.x + 20, Position.y + 20);
    Engine::Point spawn2(Position.x - 20, Position.y - 20);

    Enemy* mini1 = new MiniEnemy(spawn1.x, spawn1.y);
    Enemy* mini2 = new MiniEnemy(spawn2.x, spawn2.y);

    // 設定路徑
    mini1->UpdatePath(scene->mapDistance);
    mini2->UpdatePath(scene->mapDistance);

    scene->EnemyGroup->AddNewObject(mini1);
    scene->EnemyGroup->AddNewObject(mini2);
}


