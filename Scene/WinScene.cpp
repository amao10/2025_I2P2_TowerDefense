#include <functional>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <filesystem>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "WinScene.hpp"

void WinScene::Initialize() {
    ticks = 0;
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("You Win!", "pirulen.ttf", 48, halfW, halfH / 4 - 10, 255, 255, 255, 255, 0.5, 0.5));

    nameInput = "";
    inputLabel = new Engine::Label(nameInput, "pirulen.ttf", 48, halfW, halfH, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(inputLabel);

    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("win/dirt.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&WinScene::BackOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));
    bgmId = AudioHelper::PlayAudio("win.wav");
}
void WinScene::Terminate() {
    IScene::Terminate();
    AudioHelper::StopBGM(bgmId);
}
void WinScene::Update(float deltaTime) {
    ticks += deltaTime;
    if (ticks > 4 && ticks < 100 &&
        dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"))->MapId == 2) {
        ticks = 100;
        bgmId = AudioHelper::PlayBGM("happy.ogg");
    }
}
void WinScene::BackOnClick(int stage) {
    // Change to select scene.
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void WinScene::OnKeyDown(int keyCode) {
	IScene::OnKeyDown(keyCode);
	std::cout<<keyCode<<"winscene\n";
	if (keyCode == ALLEGRO_KEY_ENTER) {
		//get the time
    	std::time_t now = std::time(nullptr);
		std::tm* localTime = std::localtime(&now);
		std::cout << (localTime->tm_year + 1900) << '/' 
				<< (localTime->tm_mon + 1) << '/' 
				<< localTime->tm_mday << '-' 
				<< localTime->tm_hour << ':' 
				<< localTime->tm_min << ':' 
				<< localTime->tm_sec 
				<< std::endl;
        // Write name and score to the file.//and time
        std::ofstream ofs("../2025_I2P2_TowerDefense/Resource/scoreboard.txt", std::ios::app);
        std::cout << "Current working dir: " << std::filesystem::current_path() << std::endl;
        ofs << nameInput << " " << PlayScene::ScoreForWinner << " " << (localTime->tm_year + 1900) << '/' 
				<< (localTime->tm_mon + 1) << '/' 
				<< localTime->tm_mday << '-' 
				<< localTime->tm_hour << ':' 
				<< localTime->tm_min << ':' 
				<< localTime->tm_sec 
				<< "\n";
        ofs.close();

		//for debug
        std::cout << "Name: " << nameInput << ", Score: " << PlayScene::ScoreForWinner << std::endl;
        nameInput = "";
    } else if (keyCode == ALLEGRO_KEY_BACKSPACE) {
        if (!nameInput.empty()) {
            nameInput.pop_back();
        }
    } else if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
        nameInput += static_cast<char>('A' + (keyCode - ALLEGRO_KEY_A));
    } else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        nameInput += static_cast<char>('0' + (keyCode - ALLEGRO_KEY_0));
    }
    inputLabel->Text = nameInput;
}
