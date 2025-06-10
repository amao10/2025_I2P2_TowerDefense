#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <allegro5/allegro.h>
#include <string>
#include <vector>
#include "Engine/Sprite.hpp"

class Player : public Engine::Sprite {
protected:
    int speed;
    int hp, maxHp;
    int mp, maxMp;
    int exp;
    int attack, defense;
    //bool onRope;

    std::vector<ALLEGRO_BITMAP*> standLeftFrames; //standRight:flip stand left


    std::vector<ALLEGRO_BITMAP*> walkUpRopeFrames;
    std::vector<ALLEGRO_BITMAP*> walkDownRopeFrames;
    std::vector<ALLEGRO_BITMAP*> walkLeftFrames;
    std::vector<ALLEGRO_BITMAP*> walkRightFrames;

    float animTimer = 0;
    int animFrame = 0;
    enum Direction { UP, DOWN, LEFT, RIGHT } direction;

    void LoadAnimation();

public:
    Player(int x, int y, int speed, int hp, int mp, int atk, int def);
    ~Player();
    void Update(float deltaTime) override;
    void Draw() const override;
};

#endif  // PLAYER_HPP
