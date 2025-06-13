#ifndef MONSTER_HPP
#define MONSTER_HPP

#include <list>
#include <string>
#include <vector>
#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class TestScene;
class MapSystem; // 加入 MapSystem 前置宣告

class Monster : public Engine::Sprite {
public:
    enum class PatrolMode {
        None,       // 不移動
        BottomRow,  // 在最底下一行來回移動（Snail）
        CustomPath  // 自訂路徑（Mushroom 或其他）
    };

    Monster(std::string img, float x, float y, float radius, float speed, float hp, int money);
    void Hit(float damage);
    void UpdatePath(const std::vector<std::vector<int>>& mapDistance);
    void Update(float deltaTime) override;
    void Draw() const override;
    bool Removed() const;
    int damageToPlayer; // 確保有這個成員變數
    float attackCooldown;
    //Engine::Point velocity; // 怪物的速度，包含垂直速度
    bool onGround = false;  
    // 新增巡邏相關方法
    void SetPatrolMode(PatrolMode mode) { patrolMode = mode; }
    void UpdatePatrol(float deltaTime, const MapSystem* mapSystem);
    PatrolMode patrolMode = PatrolMode::None;
    bool movingRight = true;
    float moveSpeed = 25.0f;
    bool flipHorizontal;
    int GetDamage() const { return damageToPlayer; } 

    // 宣告 CanAttackPlayer() 和 ResetAttackCooldown()
    bool CanAttackPlayer() const { return attackCooldown <= 0; }
    void ResetAttackCooldown() { attackCooldown = 1.0f; }

protected:   
    std::vector<Engine::Point> path;
    float speed;
    float hp;
    int money;
    float Maxhp;
    float reachEndTime;
    float turnAroundCooldown; 
    TestScene* getTestScene();
    virtual void OnExplode();

    // 巡邏模式控制
     
};

#endif