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

    //Engine::Point velocity; // 怪物的速度，包含垂直速度
    bool onGround = false;  
    // 新增巡邏相關方法
    void SetPatrolMode(PatrolMode mode) { patrolMode = mode; }
    void UpdatePatrol(float deltaTime, const MapSystem* mapSystem);
    PatrolMode patrolMode = PatrolMode::None;
    bool movingRight = true;
    float moveSpeed = 50.0f;
    bool flipHorizontal;

protected:   
    std::vector<Engine::Point> path;
    float speed;
    float hp;
    int money;
    float Maxhp;
    float reachEndTime;
    TestScene* getTestScene();
    virtual void OnExplode();

    // 巡邏模式控制
     
};

#endif