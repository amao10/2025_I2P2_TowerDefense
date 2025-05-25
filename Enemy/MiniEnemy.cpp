#include "MiniEnemy.hpp"

MiniEnemy::MiniEnemy(float x, float y) :
    Enemy("play/enemy-10.png", x, y, 8, 15, 3, 100) {
    // 半徑更小、速度更快、HP更少、獎金更少
}
