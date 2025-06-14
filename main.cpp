// [main.cpp]
// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include <allegro5/allegro_image.h>

#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"
#include "Scene/LoseScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"
#include "Scene/StartScene.hpp"
#include "Scene/SettingsScene.hpp"
#include "Scene/ScoreBoardScene.hpp"
#include "Scene/TestScene.hpp"
#include "Scene/EndScene.hpp"
#include "Scene/VictoryScene.hpp"

int main(int argc, char **argv) {
	Engine::LOG::SetConfig(true);
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();

	al_init_image_addon(); 

    // TODO HACKATHON-2 (2/3): Register Scenes here
	game.AddNewScene("start", new Engine::StartScene());
	game.AddNewScene("settings", new SettingsScene());
    game.AddNewScene("stage-select", new StageSelectScene());
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("lose", new LoseScene());
	game.AddNewScene("win", new WinScene());
	game.AddNewScene("scoreboard-scene",new ScoreBoardScene());
	game.AddNewScene("test", new TestScene());
	game.AddNewScene("end", new EndScene());
	game.AddNewScene("victory", new VictoryScene());


    // TODO HACKATHON-1 (1/1): Change the start scene
	game.Start("start", 60, 1080, 720);
	return 0;
}

//測試一下pull request
//測試一下更新
