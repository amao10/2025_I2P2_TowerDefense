#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/color.h>
#include <allegro5/allegro.h>
#include "Engine/Resources.hpp"

Player::Player(int x, int y, int speed, int hp, int mp, int atk, int def)
    : Engine::Sprite("Player_no_weapon/stand1_0.png", x, y), speed(speed), hp(hp), maxHp(hp),
      mp(mp), maxMp(mp), exp(0), attack(atk), defense(def), direction(LEFT) {
    LoadAnimation();
}

Player::~Player() {
    
}

void Player::LoadAnimation() {

    standLeftFrames = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stand1_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stand1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stand1_2.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stand1_3.png").get()
    };

    walkUpRopeFrames = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_2.png").get()
    };

    walkDownRopeFrames = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/rope_2.png").get()
    };

    walkLeftFrames = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_2.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_3.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_4.png").get()
    };
}


void Player::Update(float deltaTime) {
    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);

    float moveDist = speed * deltaTime;
    bool moved = false;

    if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
        Position.y -= moveDist;
        direction = UP;
        moved = true;
    } else if (al_key_down(&keyState, ALLEGRO_KEY_DOWN)) {
        Position.y += moveDist;
        direction = DOWN;
        moved = true;
    } else if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
        Position.x -= moveDist;
        direction = LEFT;
        moved = true;
    } else if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
        Position.x += moveDist;
        direction = RIGHT;
        moved = true;
    }

    if (moved) {
        animTimer += deltaTime;
        if (animTimer >= 0.3f) {
            animFrame = (animFrame + 1) % 3;
            animTimer = 0;
        }
    } else {
        animFrame = 0;
        animTimer = 0;
    }

    Sprite::Update(deltaTime);
}

void Player::Draw() const {
    ALLEGRO_BITMAP* bmp = nullptr;
    const std::vector<ALLEGRO_BITMAP*>* currentFrames = nullptr;
    bool isFlipped = false;

    // ── 停止移動時 ──
    if (animFrame == 0 && animTimer == 0) {
        switch (direction) {
            case UP:
                bmp = walkUpRopeFrames.empty() ? nullptr : walkUpRopeFrames[0];
                break;
            case DOWN:
                bmp = walkDownRopeFrames.empty() ? nullptr : walkDownRopeFrames[0];
                break;
            case LEFT:
                bmp = walkLeftFrames.empty() ? nullptr : walkLeftFrames[1];
                break;
            case RIGHT:
                bmp = walkLeftFrames.empty() ? nullptr : walkLeftFrames[1];
                isFlipped = true;
                break;
        }
    }
    // ── 有移動動畫時 ──
    else {
        switch (direction) {
            case UP:
                currentFrames = &walkUpRopeFrames;
                break;
            case DOWN:
                currentFrames = &walkDownRopeFrames;
                break;
            case LEFT:
                currentFrames = &walkLeftFrames;
                break;
            case RIGHT:
                currentFrames = &walkLeftFrames;
                isFlipped = true;
                break;
        }

        if (currentFrames && !currentFrames->empty()) {
            bmp = (*currentFrames)[animFrame % currentFrames->size()];
        }
    }

    // ── 畫出圖片 ──
    if (bmp) {
        al_draw_bitmap(bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - al_get_bitmap_height(bmp) / 2,
            isFlipped ? ALLEGRO_FLIP_HORIZONTAL : 0);
    }
}
