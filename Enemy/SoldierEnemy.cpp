#include <string>

#include "SoldierEnemy.hpp"
#include "Pickup/CoinPickup.hpp"

// TODO HACKATHON-3 (1/3): You can imitate the 2 files: 'SoldierEnemy.hpp', 'SoldierEnemy.cpp' to create a new enemy.
SoldierEnemy::SoldierEnemy(int x, int y) : Enemy("play/enemy-1.png", x, y, 10, 50, 5, 5) {
}

void SoldierEnemy::OnExplode() {
    auto scene = getPlayScene();
    scene->PickupGroup->AddNewObject(new CoinPickup(Position.x, Position.y, 10));
    //scene->PickupGroup->AddNewObject(new ItemPickup(Position.x + 10, Position.y, "heal")); // 或其他 itemType
    Enemy::OnExplode(); // 若你還有爆炸動畫
}