#pragma once

#include <memory>
#include "Engine/IScene.hpp"

struct ALLEGRO_SAMPLE_INSTANCE;

class VictoryScene : public Engine::IScene {
public:
    VictoryScene();
    ~VictoryScene() override;

    void Initialize() override;
    void Terminate() override;

private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    void BackOnClick(int /*unused*/);
};
