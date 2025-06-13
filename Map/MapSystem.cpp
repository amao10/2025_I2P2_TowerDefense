// MapSystem.cpp
#include "MapSystem.hpp"
#include <queue>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Teleport.hpp"

using namespace Engine;

//————— MapTile 物件 —————  
// 继承自 IObject，只负责在它的 Position 处画一张单独的 Bitmap
// MapSystem.cpp (或单独 MapTile.hpp/.cpp 中)
class MapTile : public IObject {
    ALLEGRO_BITMAP* bmp_;
    float bmpW_, bmpH_;    // 原始位图尺寸
public:
    // 接受目标宽高
    MapTile(ALLEGRO_BITMAP* bmp,
            float x, float y,
            float width, float height)
      : IObject(x, y, width, height, 0, 0)
      , bmp_(bmp)
    {
        // 记录原图大小，用来缩放
        bmpW_ = (float)al_get_bitmap_width(bmp_);
        bmpH_ = (float)al_get_bitmap_height(bmp_);
    }

    ~MapTile() override {
        if (bmp_) al_destroy_bitmap(bmp_);
    }

    void Draw() const override {
        // 把整张 bmp_ 从 (0,0)-(bmpW_,bmpH_) 区域 缩放到 Size.x×Size.y
        al_draw_scaled_bitmap(
            bmp_,
            0, 0,         // 源图起点
            bmpW_, bmpH_, // 源图宽高
            Position.x, Position.y,
            Size.x, Size.y,
            0
        );
    }

    void Update(float) override {
        // 瓦片不需要动画逻辑
    }
};


//——— MapSystem 方法实现 ———

MapSystem::MapSystem(ALLEGRO_DISPLAY* display)
  : display_(display)
  , mapWidth(0), mapHeight(0)
  , tileWidth(90), tileHeight(60)
  , cameraX(0), cameraY(0)
  , tileGroup_(nullptr)
{
    screenWidth  = al_get_display_width(display_);
    screenHeight = al_get_display_height(display_);
}

MapSystem::~MapSystem() {
    unloadMap();
}

void MapSystem::unloadMap() {
    // 先删掉瓦片组
    if (tileGroup_) {
        delete tileGroup_;
        tileGroup_ = nullptr;
    }
    // 清空旧数据
    tileData.clear();
    ropes.clear();
    teleports.clear();
}

bool MapSystem::loadMap(const std::string& mapFile,
                        const std::string& objectFile)
{
    std::cerr << "[DEBUG] loadMap: map=" << mapFile
              << " obj=" << objectFile << "\n";
    unloadMap();
    parseTileData(mapFile);
    parseObjectData(objectFile);

    // 1. 新建一个 Group
    tileGroup_ = new Group();

    // 2. 对每个 tileData[y][x]，如果 id>0，就载入对应的单张图
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            int id = tileData[y][x];
            if (id <= 0) continue;

            std::string p = pathForTileId(id);
            ALLEGRO_BITMAP* bmp = al_load_bitmap(p.c_str());
            if (!bmp) {
                throw std::ios_base::failure("cannot load tile image " + p);
            }
            // 物件座標 = 網格坐標 * tileSize
            float px = x * tileWidth;
            float py = y * tileHeight;
            tileGroup_->AddNewObject(new MapTile(bmp, px, py, tileWidth, tileHeight));
        }
    }
    return true;
}

void MapSystem::parseTileData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::ios_base::failure("cannot load map: " + filename);
    }
    tileData.clear();
    std::vector<std::vector<int>> tmp;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::vector<int> row;
        int v;
        while (ss >> v) row.push_back(v);
        tmp.push_back(row);
    }
    mapHeight = tmp.size();
    mapWidth  = mapHeight>0? tmp[0].size(): 0;
    tileData = std::move(tmp);
}

void MapSystem::parseObjectData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "cannot open: " << filename << "\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        char type; ss >> type;
        if (type == 'R') {
            Rope r; ss >> r.x >> r.bottomY >> r.topY;
            ropes.push_back(r);
        }
        else if (type == 'T') {
            TeleportPoint tp;
            ss
            >> tp.x
            >> tp.y
            >> tp.targetMapFile
            >> tp.targetObjFile
            >> tp.targetX
            >> tp.targetY;
            teleports.push_back(tp);
            std::cerr << "[DEBUG] load T at ("<<tp.x<<","<<tp.y<<") -> "
              << tp.targetMapFile << "\n";
        }
    }
}

std::string MapSystem::pathForTileId(int id) {
    // TODO: 按自己的檔名規則擴充
    switch (id) {
        case 1: return "Resource/images/tiles/floor.png";
        case 2: return "Resource/images/tiles/basefloor.png";
        // 更多 case…
        default:
            throw std::invalid_argument("unknown tile id: " + std::to_string(id));
    }
}

void MapSystem::update(float /*dt*/, int playerX, int playerY) {
    // 让摄像机中心对准玩家
    cameraX = playerX - screenWidth  / 2;
    cameraY = playerY - screenHeight / 2;
    // 计算最大可移动范围
    float maxX = mapWidth  * tileWidth  - screenWidth;
    float maxY = mapHeight * tileHeight - screenHeight;
    // 限制在 [0, max]
    cameraX = std::max(0.0f, std::min(cameraX, maxX));
    cameraY = std::max(0.0f, std::min(cameraY, maxY));
}


// void MapSystem::update(float dt, int playerX, int playerY) {
//     // TestScene 相机固定 (0,0)，这里可先不动
//     (void)dt; (void)playerX; (void)playerY;
// }

// MapSystem.cpp
void MapSystem::render(ALLEGRO_BITMAP* buffer) {
    // 切到 backbuffer 或 指定 buffer
    if (buffer) al_set_target_bitmap(buffer);
    else        al_set_target_backbuffer(display_);

    // 构造摄像机变换
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_translate_transform(&trans, -cameraX, -cameraY);
    al_use_transform(&trans);

    // 绘制瓦片
    if (tileGroup_) tileGroup_->Draw();

    // if (TeleportTrigger::bmp_) {
    //     for (const auto& tp : teleports) {
    //         float px = tp.x * tileWidth;
    //         float py = tp.y * tileHeight;
    //         al_draw_scaled_bitmap(
    //             TeleportTrigger::bmp_,         // 由 TeleportTrigger 预加载的贴图
    //             0, 0,
    //             al_get_bitmap_width(TeleportTrigger::bmp_),
    //             al_get_bitmap_height(TeleportTrigger::bmp_),
    //             px, py,
    //             tileWidth, tileHeight,
    //             0
    //         );
    //     }
    // }

    // 重置变换（后面绘制不跟随相机的内容时要用）
    al_identity_transform(&trans);
    al_use_transform(&trans);
}


// void MapSystem::render(ALLEGRO_BITMAP* buffer) {
//     // 先切到 backbuffer 或 指定的 buffer
//     if (buffer) al_set_target_bitmap(buffer);
//     else        al_set_target_backbuffer(display_);

//     // 因为 TestScene 锁在 (0,0)，直接 draw 整个组
//     if (tileGroup_) tileGroup_->Draw();

//     // TODO: 绘制 ropes/teleports…
// }

// checkTeleport 同原本不变…
