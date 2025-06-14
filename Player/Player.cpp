#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/color.h>
#include <allegro5/allegro.h>
#include <algorithm>
#include "Engine/Resources.hpp"
#include "Scene/TestScene.hpp"
#include "Engine/GameEngine.hpp"
#include "Map/MapSystem.hpp"
#include "UI/Component/Label.hpp"
#include "Monster/Monster.hpp"
#include "Engine/AudioHelper.hpp"
#include "Bullet/Cannonball.hpp"

Player::Player(int x, int y, int speed, int level, int hp, int mp, int atk, int def)
    : Engine::Sprite("Player_no_weapon/stand1_0.png", x, y),
      speed(speed),
      level(level),
      exp(0),
      expToLevelUp(100*level),
      prevExpToLevelUp(100*level),
      attack(atk),
      defense(def),
      direction(LEFT),
      currentWeapon(UNARMED),
      proning(false),
      onRope(false),
      coin(0),
      redPotion(0),
      bluePotion(0)
{
    maxHp = 100 * level;
    maxMp = 50 * level;
    this->hp = std::min(hp, maxHp);
    this->mp = std::min(mp, maxMp);
    this->CollisionRadius = GetBitmapWidth() / 2.0f; 
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
    
    auto* scene = dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
    if (!scene) return;

    MapSystem* map = scene->GetMapSystem();
    if (!map) return;
    
    // skillcooldown timer
    if (skillCooldown > 0.0f) {
        skillCooldown -= deltaTime;
        if (skillCooldown < 0.0f) skillCooldown = 0.0f;
    }


    const int tileW = 90;
    const int tileH = 60;
    int gridX = static_cast<int>((Position.x) / tileW);
    int gridYBelow = static_cast<int>((Position.y + Size.y / 2 + 1) / tileH);
    int gridY = static_cast<int>((Position.y) / tileH);

    auto& tileData = map->GetTileData();
    bool onFloor = false;
    if (gridYBelow >= 0 && gridYBelow < tileData.size() && gridX >= 0 && gridX < tileData[0].size()) {
        int tileId = tileData[gridYBelow][gridX];
        if (tileId == 1 || tileId == 2) onFloor = true;
    }

    // invincible timer
    if (invincible) {
        invincibleTimer -= deltaTime;
        if (invincibleTimer <= 0) {
            invincible = false;
            invincibleTimer = 0;
        }
    }

    // test hp mp functions with debounce
    /*
    bool keyZ = al_key_down(&keyState, ALLEGRO_KEY_Z);
    if (keyZ && !prevKeyZ) {
        TakeDamage(10); // 扣血
    }
    prevKeyZ = keyZ;

    bool keyC = al_key_down(&keyState, ALLEGRO_KEY_C);
    if (keyC && !prevKeyC) {
        UseMP(10); // 扣魔
    }
    prevKeyC = keyC;

    bool keyA = al_key_down(&keyState, ALLEGRO_KEY_A);
    if (keyA && !prevKeyA) {
        Heal(20); // 補血
    }
    prevKeyA = keyA;

    bool keyS = al_key_down(&keyState, ALLEGRO_KEY_S);
    if (keyS && !prevKeyS) {
        RecoverMP(10); // 補魔
    }
    prevKeyS = keyS;

    bool keyE = al_key_down(&keyState, ALLEGRO_KEY_E);
    if (keyE && !prevKeyE) {
        GainExp(50); // 增加 50 EXP
    }
    prevKeyE = keyE;
    */

    //+Hp, MP
    bool keyH = al_key_down(&keyState, ALLEGRO_KEY_H);
    if (keyH && !prevKeyH) {
        if (redPotion > 0) {
            redPotion--;
            AudioHelper::PlaySample("drink.ogg");
            Heal(50);
        } else if (coin >= 5) {
            coin -= 5;
            AudioHelper::PlaySample("drink.ogg");
            Heal(50);
        }
    }
    prevKeyH = keyH;

    bool keyM = al_key_down(&keyState, ALLEGRO_KEY_M);
    if (keyM && !prevKeyM) {
        if (bluePotion > 0) {
            bluePotion--;
            AudioHelper::PlaySample("drink.ogg");
            RecoverMP(25);
        } else if (coin >= 5) {
            coin -= 5;
            AudioHelper::PlaySample("drink.ogg");
            RecoverMP(25);
        }
    }
    prevKeyM = keyM;

    // skill jump
    static bool skillPrevPressed = false;
    bool skillPressed = al_key_down(&keyState, ALLEGRO_KEY_C);

    if (skillPressed && !skillPrevPressed && !skillActive && skillCooldown == 0.0f && onGround) {
        if (mp >= 25) {
            skillActive = true;
            skillScale = 4.0f;           
            velocity.y = -600;           
            onGround = false;
            skillUsedInAir = false;      
            skillCooldown = skillCooldownTime; 
            mp -= 25;
            AudioHelper::PlaySample("skill.ogg");           
        } 
    }
    skillPrevPressed = skillPressed;



    // jump&jump down
   
    static bool altPrevPressed = false;
    bool altPressed = al_key_down(&keyState, ALLEGRO_KEY_ALT);
    bool downHeld   = al_key_down(&keyState, ALLEGRO_KEY_DOWN);

    if (onGround && altPressed && !altPrevPressed) {
        int tileId = tileData[gridYBelow][gridX];
        if (downHeld && tileId == 1) {
            // ↓+ALT → 從 tile==1 下跳
            Position.y += 70;     // 跳離當前地板
            velocity.y = 200;     // 下墜起始速度
            onGround = false;
            skipLandingThisFrame = true;
        } else if (!downHeld) {
            // 單按 ALT → 一般跳躍
            velocity.y = -500;
            onGround = false;
        }
    }
    altPrevPressed = altPressed;




    // 2. 重力與 Y 移動
    velocity.y += 1000 * deltaTime;
    Position.y += velocity.y * deltaTime;

    // 避免跳太高出畫面（上邊界）
    if (Position.y - Size.y / 2 < 0) {
        Position.y = Size.y / 2;
        velocity.y = 0;
    }

    // 3. 地面檢查（下方 tile 是地板）
    if (gridYBelow >= 0 && gridYBelow < tileData.size() && gridX >= 0 && gridX < tileData[0].size()) {
        int tileId = tileData[gridYBelow][gridX];
        if (skipLandingThisFrame) {
            skipLandingThisFrame = false;
        } 
        else if ((tileId == 1 || tileId == 2) && velocity.y >= 0) {
            velocity.y = 0;
            onGround = true;
            Position.y = gridYBelow * tileH - Size.y / 2;


            //skill touchground
             if (skillActive && !skillUsedInAir) {
            // 落地時觸發秒殺
            auto* scene = dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
            if (scene) {
                for (auto& obj : scene->MonsterGroup->GetObjects()) {
                    Monster* monster = dynamic_cast<Monster*>(obj);
                    if (monster && monster->Visible) {
                        monster->Hit(500); 
                    }
                }
            }
            skillUsedInAir = true;
        }

        if (skillActive && skillUsedInAir) {
            skillActive = false;
            skillScale = 1.0f; // 變回原來大小
        }
        }
        else {
            onGround = false;
        }
    }


    // 趴下
    proning = al_key_down(&keyState, ALLEGRO_KEY_DOWN) && !onRope;

    // 左右移動（檢查是否撞牆）
    if (!attacking) {
        if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
            int nx = static_cast<int>((Position.x - moveDist - Size.x / 2) / tileW);
            if (nx >= 0 && (tileData[gridY][nx] == 0 || tileData[gridY][nx] == 1 || tileData[gridY][nx] == 2)&& (Position.x - moveDist - Size.x / 2 >= 0)) {
                Position.x -= moveDist;
                direction = LEFT;
                moved = true;
            }
        } else if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
            int nx = static_cast<int>((Position.x + moveDist + Size.x / 2) / tileW);
            if (nx < tileData[0].size() && (tileData[gridY][nx] == 0 || tileData[gridY][nx] == 1 || tileData[gridY][nx] == 2)) {
                Position.x += moveDist;
                direction = RIGHT;
                moved = true;
            }
        }
    }

    // 武器切換（空白鍵）
    static bool spacePressed = false;
    if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
        if (!spacePressed) {
            currentWeapon = static_cast<WeaponType>((currentWeapon + 1) % 3);
            animFrame = 0;
            animTimer = 0;
            spacePressed = true;
        }
    } else {
        spacePressed = false;
    }

    // 攻擊鍵（Ctrl）
    if (!al_key_down(&keyState, ALLEGRO_KEY_LCTRL)) {
        attackKeyPressed = false;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_LCTRL) && !attackKeyPressed && !attacking) {
        attacking = true;
        attackKeyPressed = true;
        attackDashed = false;
        attackTimer = 0;
        animFrame = 0;

        const char* soundName = nullptr;
        switch (currentWeapon) {
            case UNARMED:
                soundName = "no_weapon_attack.ogg";
                break;
            case SWORD:
                soundName = "sword_attack.ogg";
                break;
            case HANDCANNON:
                soundName = "handcannon_attack.ogg";
                if (scene) {
                    float dir = (direction == RIGHT ? 1.0f : -1.0f);
                    float spawnX = Position.x + dir * 30;
                    float spawnY = Position.y;
                    scene->BulletGroup->AddNewObject(new CannonBall(spawnX, spawnY, dir, attack + 20));
                }
                break;



            default:
                soundName = "no_weapon_attack.ogg";
                break;
        }

        if (soundName) {
            AudioHelper::PlayAudio(soundName);
        }
        
    }

    if (attacking) {
        attackTimer += deltaTime;
        int totalFrames = attackAnimations[currentWeapon].size();
        int currentFrame = int(attackTimer / attackDuration * totalFrames);
        animFrame = std::min(currentFrame, totalFrames - 1);
        
        //attack logic
        if ((currentWeapon == UNARMED || currentWeapon == SWORD)) {
            auto* scene = dynamic_cast<TestScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
            if (scene) {
                for (auto& obj : scene->MonsterGroup->GetObjects()) {
                    Monster* monster = dynamic_cast<Monster*>(obj);
                    if (!monster) continue;

                    // 偵測攻擊命中區域：面向右時使用右半邊，面向左用左半邊
                    float attackRange = 30.0f;
                    float attackLeft, attackRight;
                    if (direction == RIGHT) {
                        attackLeft = Position.x;
                        attackRight = Position.x + attackRange;
                    } else {
                        attackLeft = Position.x - attackRange;
                        attackRight = Position.x;
                    }
                    float attackTop = Position.y - Size.y / 2;
                    float attackBottom = Position.y + Size.y / 2;

                    float monsterLeft = monster->Position.x - monster->Size.x / 2;
                    float monsterRight = monster->Position.x + monster->Size.x / 2;
                    float monsterTop = monster->Position.y - monster->Size.y / 2;
                    float monsterBottom = monster->Position.y + monster->Size.y / 2;

                    bool hit = !(attackRight < monsterLeft || attackLeft > monsterRight ||
                                 attackBottom < monsterTop || attackTop > monsterBottom);

                        if (hit) {
                            int damage = attack;
                            if (currentWeapon == UNARMED)
                            {
                                damage = attack;
                            }
                                
                            else if (currentWeapon == SWORD)
                            {
                                damage = attack + 10;
                            }
                            monster->Hit(damage);
                        }
                }
            }
        }

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
        if (animTimer >= 0.2f) {
            animFrame = (animFrame + 1) % 4;
            animTimer = 0;
        }
    } else {
        animFrame = 0;
        animTimer = 0;
    }

    Sprite::Update(deltaTime);

    if (shouldClearLevelUpFlag) {
    shouldClearLevelUpFlag = false;
    } else {
        justLeveledUp = false;
    }

}

void Player::GainExp(int amount) {
    exp += amount;
    while (exp >= expToLevelUp) {
        prevExpToLevelUp = expToLevelUp;
        exp -= expToLevelUp;
        LevelUp();
        justLeveledUp = true;
        shouldClearLevelUpFlag = true; // 延遲一幀清除
    }
}



void Player::LevelUp() {
    level++;
    AudioHelper::PlaySample("level_up.ogg");
    expToLevelUp = 100 *level;

    maxHp += 20;
    hp = maxHp;
    maxMp += 10;
    mp = maxMp;
    attack += 5;
    defense += 2;

}


void Player::Draw() const {
    ALLEGRO_BITMAP* bmp = nullptr;
    const std::vector<ALLEGRO_BITMAP*>* currentFrames = nullptr;

    if (attacking && !attackAnimations[currentWeapon].empty()) {
        bmp = attackAnimations[currentWeapon][animFrame % attackAnimations[currentWeapon].size()];
    } else if (!onGround && !jumpAnimations[currentWeapon].empty()) {
        bmp = jumpAnimations[currentWeapon][animFrame % jumpAnimations[currentWeapon].size()];
    } else if (proning && !proneAnimations[currentWeapon].empty()) {
        bmp = proneAnimations[currentWeapon][0];
    } else {
        if (animFrame == 0 && animTimer == 0) {
            bmp = standAnimations[currentWeapon].empty() ? nullptr : standAnimations[currentWeapon][0];
        } else {
            currentFrames = &walkAnimations[currentWeapon];
            if (currentFrames && !currentFrames->empty()) {
                bmp = (*currentFrames)[animFrame % currentFrames->size()];
            }
        }
    }

    if (bmp) {
        al_draw_scaled_bitmap(
        bmp,
        0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
        Position.x - al_get_bitmap_width(bmp) * skillScale / 2,
        proning ? Position.y - 5 : Position.y - al_get_bitmap_height(bmp) * skillScale / 2,
        al_get_bitmap_width(bmp) * skillScale,
        al_get_bitmap_height(bmp) * skillScale,
        direction == RIGHT ? ALLEGRO_FLIP_HORIZONTAL : 0);
    }

    // ====== 畫上方血條、魔力條、經驗條 ======
    float bmpW = static_cast<float>(al_get_bitmap_width(bmp));
    float bmpH = static_cast<float>(al_get_bitmap_height(bmp));
    float barW = 80.0f, barH = 6.0f, spacing = 4.0f;
    float barX = Position.x - barW / 2;
    float totalBarHeight = 3 * barH + 2 * spacing;
    float barYTop = Position.y - bmpH / 2 - 10 - totalBarHeight;
    float hpY = barYTop;
    float mpY = hpY + barH + spacing;
    float expY = mpY + barH + spacing;

    al_draw_filled_rectangle(barX, hpY,  barX + barW, hpY + barH,  al_map_rgb(100, 100, 100));
    al_draw_filled_rectangle(barX, mpY,  barX + barW, mpY + barH,  al_map_rgb(100, 100, 100));
    al_draw_filled_rectangle(barX, expY, barX + barW, expY + barH, al_map_rgb(100, 100, 100));

    float hpRatio  = static_cast<float>(hp) / maxHp;
    float mpRatio  = static_cast<float>(mp) / maxMp;
    float expRatio;
    if (justLeveledUp) {
        // 升級當幀：畫滿格（即使 exp = 0）
        expRatio = 1.0f;
    } else {
        expRatio = static_cast<float>(exp) / expToLevelUp;
        expRatio = std::min(expRatio, 1.0f);
    }



    al_draw_filled_rectangle(barX, hpY,  barX + barW * hpRatio,  hpY + barH,  al_map_rgb(255, 0, 0));
    al_draw_filled_rectangle(barX, mpY,  barX + barW * mpRatio,  mpY + barH,  al_map_rgb(0, 0, 255));
    al_draw_filled_rectangle(barX, expY, barX + barW * expRatio, expY + barH, al_map_rgb(255, 255, 0));

    // 顯示 LV. 等級文字（角色下方)
    float textW = 50.0f;
    float textH = 20.0f;
    float textX = Position.x - textW / 2;
    float textY = Position.y + bmpH / 2 + 5;
    al_draw_filled_rectangle(textX, textY, textX + textW, textY + textH,
        al_map_rgba(50, 50, 50, 180));

    std::string levelText = "LV. " + std::to_string(level);
    Engine::Label label(levelText, "pirulen.ttf", 12,
        Position.x, Position.y + bmpH / 2 + 5, 255, 255, 255, 255);
    label.Anchor = Engine::Point(0.5, 0);
    label.Draw();
}



int Player::GetLevel() const {
    return level;
}

int Player::GetExp() const {
    return exp;
}

int Player::GetExpToLevelUp() const {
    return expToLevelUp;
}

void Player::TakeDamage(int dmg) {
    if (invincible) return;

    hp -= std::max(dmg - defense, 0);
    hp = std::max(hp, 0);

    invincible = true;
    invincibleTimer = invincibleDuration;

}


bool Player::UseMP(int MPcost) {
    if (mp >= MPcost) {
        mp -= MPcost;
        return true;
    }
    return false;
}

void Player::Heal(int amount) {
    hp = std::min(hp + amount, maxHp);
}

void Player::RecoverMP(int amount) {
    mp = std::min(mp + amount, maxMp);
}
