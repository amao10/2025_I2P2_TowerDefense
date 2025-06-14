#ifndef BOSSMONSTER_HPP
#define BOSSMONSTER_HPP

#include <string>
#include <vector>
#include "Monster.hpp"
#include "BossAttack/BossOrb.hpp"

class BossMonster : public Monster {
public:
    BossMonster(int x, int y);
    ~BossMonster() override;

    void Update(float deltaTime) override;

    // --- CHANGE THESE TO PUBLIC ---
    float GetBitmapWidth() const { return Sprite::GetBitmapWidth(); }
    float GetBitmapHeight() const { return Sprite::GetBitmapHeight(); }
    // ----------------------------

private:
    std::vector<BossOrb*> orbs;
    void CreateOrbs();
    void DestroyOrbs();
};

#endif // BOSSMONSTER_HPP