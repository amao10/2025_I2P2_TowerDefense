#ifndef SLOWTURRET_HPP
#define SLOWTURRET_HPP
#include "Turret.hpp"

class SlowTurret : public Turret {
public:
    static const int Price;
    SlowTurret(float x, float y);
    void CreateBullet() override;
};
#endif   // SLOWTURRET_HPP
