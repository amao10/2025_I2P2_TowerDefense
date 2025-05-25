#ifndef WINSCENE_HPP
#define WINSCENE_HPP
#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>

class WinScene final : public Engine::IScene {
private:
    float ticks;
    ALLEGRO_SAMPLE_ID bgmId;

    Engine::Label* nameLabel;
    Engine::Label* inputLabel;
    std::string nameInput;
public:
    explicit WinScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void BackOnClick(int stage);
    void OnKeyDown(int keyCode) override;
};

#endif   // WINSCENE_HPP
