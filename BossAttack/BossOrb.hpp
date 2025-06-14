#ifndef BOSSORB_HPP
#define BOSSORB_HPP

#include "Engine/Sprite.hpp"
#include "Engine/Point.hpp"
#include "Scene/TestScene.hpp"

class Player; // 前向聲明 Player 類，避免循環引用
class BossMonster; // 前向聲明 BossMonster 類

class BossOrb : public Engine::Sprite {
public:
    // 構造函數，需要知道它所屬的 Boss
    BossOrb(BossMonster* boss, float angleOffset);
    //void CreateOrbs();
    // 更新球的位置和碰撞檢測
    void Update(float deltaTime) override;
    // 繪製球
    void Draw() const override;

    // 獲取碰撞半徑
    float GetCollisionRadius() const { return CollisionRadius; }

private:
    BossMonster* parentBoss; // 指向父級 Boss Monster 的指針
    float orbitRadius;       // 軌道半徑
    float orbitSpeed;        // 軌道速度 (角度每秒)
    float currentAngle;      // 當前角度 (弧度)
    float CollisionRadius;   // 球的碰撞半徑
    int damageToPlayer;      // 球對玩家造成的傷害
    float attackCooldown;    // 球的攻擊冷卻
    
    // 獲取 TestScene 實例的輔助函數
    TestScene* getTestScene();
};

#endif // BOSSORB_HPP