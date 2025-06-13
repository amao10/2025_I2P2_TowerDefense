#pragma once
#include <allegro5/allegro.h>        // for ALLEGRO_BITMAP
#include <allegro5/allegro_image.h> 
#include <string>
#include "Engine/IObject.hpp"

struct TeleportPoint {
    int           x;              // 來源格子 X
    int           y;              // 來源格子 Y
    std::string   targetMapFile;  // 目標地圖檔路徑
    std::string   targetObjFile;  // 目標物件檔路徑
    int           targetX;        // 目標格子 X
    int           targetY;        // 目標格子 Y
};

class Player;      // 前置声明，避免循环依赖
class MapSystem;   // 同上

class TeleportTrigger : public Engine::IObject {
public:
    TeleportTrigger(Player* player,
                    MapSystem* mapSystem,
                    const TeleportPoint& cfg);
    ~TeleportTrigger() override;

    void Update(float deltaTime) override;
    void Draw() const override;

    static ALLEGRO_BITMAP* bmp_;

private:
    Player*       player_;
    MapSystem*    mapSystem_;
    TeleportPoint cfg_;
    bool          triggered_;
};
