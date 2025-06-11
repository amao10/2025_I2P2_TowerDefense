#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>

// 技能冷卻圖示與百分比
struct CooldownIcon {
    ALLEGRO_BITMAP* icon;      // 圖示資源
    float cooldownPercent;     // 冷卻進度 (0.0 - 1.0)
};

class UIManager {
public:
    // 建構子：初始化 HUD、字型與小地圖
    UIManager(ALLEGRO_DISPLAY* display, MapSystem* mapSystem);
    // 解構子：釋放字型與圖像資源
    ~UIManager();

    // 繪製頭上顯示 (HP、MP、等級、技能冷卻)
    void renderHUD(int hp, int mp, int level, const std::vector<CooldownIcon>& skills);

    // 繪製小地圖，根據玩家座標定位
    void renderMiniMap(int playerX, int playerY);

    // 繪製存檔選單介面
    void renderSaveMenu();

    // 切換存檔選單開關狀態
    void toggleSaveMenu();
    // 查詢存檔選單是否開啟
    bool isSaveMenuOpen() const;

private:
    ALLEGRO_FONT* font;             // UI 字型
    ALLEGRO_BITMAP* hpBarBg;        // HP 條背景
    ALLEGRO_BITMAP* hpBarFg;        // HP 條前景
    ALLEGRO_BITMAP* mpBarBg;        // MP 條背景
    ALLEGRO_BITMAP* mpBarFg;        // MP 條前景
    MapSystem* mapSys;              // 參考地圖系統，用於小地圖生成
    bool saveMenuOpen;              // 存檔選單開關旗標
};