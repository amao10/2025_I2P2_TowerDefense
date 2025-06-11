#pragma once
#include <string>
#include "GameState.h"  // 定義 GameState 結構與序列化接口

class SaveManager {
public:
    // 儲存遊戲狀態到指定路徑
    // filepath: 存檔檔案位置
    // state: 要儲存的遊戲狀態資料
    // 回傳是否成功
    static bool saveGame(const std::string& filepath, const GameState& state);

    // 從指定路徑讀取遊戲狀態
    // filepath: 存檔檔案位置
    // outState: 回填讀出的遊戲狀態
    // 回傳是否成功
    static bool loadGame(const std::string& filepath, GameState& outState);
};