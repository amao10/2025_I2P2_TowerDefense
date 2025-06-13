#include <string>

#include "MushroomMonster.hpp"

MushroomMonster::MushroomMonster(int x,int y) : Monster("play/mushroom.png", x, y, 10, 50, 5, 5){
    damageToPlayer = 15;
}