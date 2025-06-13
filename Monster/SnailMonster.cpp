#include <string>

#include "SnailMonster.hpp"

SnailMonster::SnailMonster(int x,int y) : Monster("play/snail.png", x, y, 10, 50, 5, 5){
    damageToPlayer = 25;
}