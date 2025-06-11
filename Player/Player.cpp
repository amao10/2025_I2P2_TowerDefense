#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/color.h>
#include <allegro5/allegro.h>
#include <algorithm>
#include "Engine/Resources.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/GameEngine.hpp"

Player::Player(int x, int y, int speed, int hp, int mp, int atk, int def)
    : Engine::Sprite("Player_no_weapon/stand1_0.png", x, y), speed(speed), hp(hp), maxHp(hp),
      mp(mp), maxMp(mp), exp(0), attack(atk), defense(def), direction(LEFT), currentWeapon(UNARMED), proning(false), onRope(false) {
    LoadAnimation();
}

Player::~Player() {}

void Player::LoadAnimation() {
    // Stand Animations
    standAnimations[UNARMED] = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stand1_1.png").get(),
    };
    standAnimations[SWORD] = {
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stand1_1.png").get(),
    };
    standAnimations[HANDCANNON] = {
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/stand1_1.png").get(),
    };

    // Walk Animations
    walkAnimations[UNARMED] = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_2.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_3.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/walk1_2.png").get(),
    };
    walkAnimations[SWORD] = {
        Engine::Resources::GetInstance().GetBitmap("Player_sword/walk1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/walk1_2.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/walk1_3.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/walk1_2.png").get(),
    };
    walkAnimations[HANDCANNON] = {
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/walk1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/walk1_2.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/walk1_3.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/walk1_2.png").get(),
    };
    //Jump Animations
    jumpAnimations[UNARMED] = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/jump_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/jump_1.png").get(),
    };

    jumpAnimations[SWORD] = {
        Engine::Resources::GetInstance().GetBitmap("Player_sword/jump_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/jump_1.png").get(),
    };

    jumpAnimations[HANDCANNON] = {
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/jump_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/jump_1.png").get(),
    };

    proneAnimations[UNARMED] = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/prone_0.png").get(),
    };

    proneAnimations[SWORD] = {
        Engine::Resources::GetInstance().GetBitmap("Player_sword/prone_0.png").get(),
    };

    proneAnimations[HANDCANNON] = {
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/prone_0.png").get(),
    };
    // Attack Animations
    attackAnimations[UNARMED] = {
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stabO2_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stabO2_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stabO2_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_no_weapon/stabO2_1.png").get()
    };
    attackAnimations[SWORD] = {
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_1.png").get()
        //Engine::Resources::GetInstance().GetBitmap("Player_sword/stabO1_2.png").get()
    };
    attackAnimations[HANDCANNON] = {
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/shootF_0.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/shootF_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/shootF_1.png").get(),
        Engine::Resources::GetInstance().GetBitmap("Player_handcannon/shootF_1.png").get()
    };
}

void Player::Update(float deltaTime) {
    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);

    float moveDist = speed * deltaTime / 2;
    bool moved = false;

    auto* scene = dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
    int gridX = floor(Position.x / PlayScene::BlockSize);
    int gridY = floor((Position.y + Size.y / 2 + 1) / PlayScene::BlockSize);

    bool touchingFloor = false;
    if (scene && gridX >= 0 && gridX < PlayScene::MapWidth && gridY >= 0 && gridY < PlayScene::MapHeight) {
        if (!scene->mapState[gridY][gridX]) {
            touchingFloor = true;
        }
    }

    if (onGround && al_key_down(&keyState, ALLEGRO_KEY_ALT)) {
        velocity.y = -400;
        onGround = false;
    }

    //prone
    proning = al_key_down(&keyState, ALLEGRO_KEY_DOWN) && !onRope;
    
    if (!onGround) {
        velocity.y += 1000 * deltaTime;
        Position.y += velocity.y * deltaTime;
    }

    if (touchingFloor && velocity.y >= 0) {
        onGround = true;
        velocity.y = 0;
        Position.y = gridY * PlayScene::BlockSize - Size.y / 2;
    } else {
        onGround = false;
    }

    // 武器切換邏輯（按 SPACE）
    static bool spacePressed = false;
    if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
        if (!spacePressed) {
            // 切換武器：循環 UNARMED -> SWORD -> HANDCANNON -> UNARMED
            currentWeapon = static_cast<WeaponType>((currentWeapon + 1) % 3);
            animFrame = 0; // 切換時重置動畫
            animTimer = 0;
            spacePressed = true;
        }
    } else {
        spacePressed = false; // 放開鍵後才能再次觸發
    }


    int screenW = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int screenH = Engine::GameEngine::GetInstance().GetScreenSize().y;
    Position.x = std::max(Size.x / 2.0f, std::min(Position.x, screenW - Size.x / 2.0f));
    Position.y = std::min(Position.y, screenH - Size.y / 2.0f);

    if (!al_key_down(&keyState, ALLEGRO_KEY_LCTRL)) {
        attackKeyPressed = false;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL) && !attackKeyPressed && !attacking) {
        attacking = true;
        attackKeyPressed = true;
        attackDashed = false;
        attackTimer = 0;
        animFrame = 0;
    }

    if (!attacking) {
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
    }

    if (attacking) {
        attackTimer += deltaTime;
        int totalFrames = attackAnimations[currentWeapon].size();
        int currentFrame = int(attackTimer / attackDuration * totalFrames);
        animFrame = std::min(currentFrame, totalFrames - 1);

        if ((animFrame == 0 || animFrame == 1) && !attackDashed) {
            float dashAmount = 300 * deltaTime;
            Position.x += (direction == RIGHT ? dashAmount : -dashAmount);
            attackDashed = true;
        }

        if (attackTimer >= attackDuration) {
            attacking = false;
            animFrame = 0;
        }
    } else if (moved) {
        animTimer += deltaTime;
        if (animTimer >= 0.3f) {
            animFrame = (animFrame + 1) % 4;
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

    if (attacking && !attackAnimations[currentWeapon].empty()) {
        bmp = attackAnimations[currentWeapon][animFrame % attackAnimations[currentWeapon].size()];
        al_draw_bitmap(bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - al_get_bitmap_height(bmp) / 2,
            direction == RIGHT ? ALLEGRO_FLIP_HORIZONTAL : 0);
        return;
    }

    if (!onGround && !jumpAnimations[currentWeapon].empty()) {
        bmp = jumpAnimations[currentWeapon][animFrame % jumpAnimations[currentWeapon].size()];
        al_draw_bitmap(bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - al_get_bitmap_height(bmp) / 2,
            direction == RIGHT ? ALLEGRO_FLIP_HORIZONTAL : 0);
        return;
    }

    if (proning && !proneAnimations[currentWeapon].empty()) {
        bmp = proneAnimations[currentWeapon][0];
        al_draw_bitmap(bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - 5,
            direction == RIGHT ? ALLEGRO_FLIP_HORIZONTAL : 0);
        return;
    }
    //Position.y - al_get_bitmap_height(bmp) / 2,
    if (animFrame == 0 && animTimer == 0) {
        bmp = standAnimations[currentWeapon].empty() ? nullptr : standAnimations[currentWeapon][0];
    } else {
        currentFrames = &walkAnimations[currentWeapon];
        if (currentFrames && !currentFrames->empty()) {
            bmp = (*currentFrames)[animFrame % currentFrames->size()];
        }
    }

    if (bmp) {
        al_draw_bitmap(bmp,
            Position.x - al_get_bitmap_width(bmp) / 2,
            Position.y - al_get_bitmap_height(bmp) / 2,
            direction == RIGHT ? ALLEGRO_FLIP_HORIZONTAL : 0);
    }
}