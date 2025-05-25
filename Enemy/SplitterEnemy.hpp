#ifndef SPLITTERENEMY_HPP
#define SPLITTERENEMY_HPP
#include "Enemy.hpp"

class SplitterEnemy : public Enemy {
public:
    SplitterEnemy(int x, int y);
    void OnExplode() override;
};
#endif // SPLITTERENEMY_HPP
