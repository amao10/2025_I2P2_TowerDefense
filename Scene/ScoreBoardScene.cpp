#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>//
#include <sstream>//
#include <iomanip>



#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "PlayScene.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "UI/Component/Slider.hpp"
#include "ScoreBoardScene.hpp"

//to print scoretxt 
#include <vector>//

struct ScoreEntry {
    std::string name;
    int score;
    std::string timestamp;
};
std::vector<ScoreEntry> rawScoreboardData;
std::vector<std::string> scoreboardData;
int page=0;
const int itemsPerPage=6;
int ScoreBoardScene::isloading=0;

void ScoreBoardScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    Engine::ImageButton* btn;

    AddNewObject(new Engine::Label("Score Board", "pirulen.ttf", 90, halfW, halfH / 5, 10, 255, 255, 255, 0.5, 0.5));
    
    // Read txt and store data
    if(!isloading){
        std::string fname = "../2025_I2P2_TowerDefense/Resource/scoreboard.txt";
        std::ifstream fin(fname);
        scoreboardData.clear();
        ///debug///
        std::cout << "[DEBUG] Try open file: " << fname << "\n";
        if (!fin.is_open()) {
            std::cerr << "[ERROR] Cannot open file!\n";
        } else {
            std::cout << "[DEBUG] File opened successfully.\n";
            std::string pplname,pplscore,tmstamp;
            rawScoreboardData.clear();

            while(fin >> pplname >> pplscore >> tmstamp){
                ScoreEntry entry;
                entry.name = pplname;
                entry.score = std::stoi(pplscore);
                entry.timestamp = tmstamp;
                rawScoreboardData.push_back(entry);
            }
            fin.close();

            std::sort(rawScoreboardData.begin(),rawScoreboardData.end(),[](const ScoreEntry& a,const ScoreEntry& b){
                if (a.score != b.score)
                    return a.score > b.score;              // 分數高的排前面
                return a.name < b.name; //再排名字
            });
            scoreboardData.clear();
            int rank = 1;
            std::stringstream formattedLine;
            for(const auto& entry : rawScoreboardData){
                formattedLine.str("");
                formattedLine.clear();
                formattedLine << std::right << std::setw(5) << rank
                              << std::right << std::setw(15) << entry.name
                              << std::setw(8)  << entry.score
                              << std::setw(30) << entry.timestamp;
                scoreboardData.push_back(formattedLine.str());
                rank++;
            }
            isloading = 1;
        }
        ///debug///

        // while (fin >> pplname >> pplscore >> tmstamp) {
        //     std::cout << "[DEBUG] Read: " << pplname << ", " << pplscore << ", " << tmstamp << "\n";
        //     formattedLine.str("");
        //     formattedLine.clear(); 
        //     formattedLine << std::right << std::setw(5) << rank 
        //                     <<std::right << std::setw(15) << pplname 
        //                     <<std::right << std::setw(8) << pplscore 
        //                     <<std::right << std::setw(30) << tmstamp ;
        //     scoreboardData.push_back(formattedLine.str());
        //     rank++;
        // }
        // fin.close();
        // isloading=1;
    }
    
    //print
    std::stringstream topicformatted;
    topicformatted.str("");
    topicformatted.clear(); 
    topicformatted << std::right << std::setw(5) << "order" 
                    << std::right << std::setw(15) << "Player" 
                    << std::right << std::setw(8) << "Score" 
                    << std::right << std::setw(30) << "Record Time" ;
    AddNewObject(new Engine::Label(topicformatted.str(), "pirulen.ttf", 40, halfW, halfH / 5 + 100, 10, 255, 255, 255, 0.5, 0.5));
    int first_data_height = halfH / 5 + 150;
    int space_for_datas = 40;
    int startIdx = page * itemsPerPage;
    int endIdx = std::min(startIdx + itemsPerPage, (int)scoreboardData.size());
    for (int i = startIdx; i < endIdx; i++) {
        AddNewObject(new Engine::Label(scoreboardData[i], "pirulen.ttf", 40, halfW, first_data_height, 10, 255, 255, 255, 0.5, 0.5));
        first_data_height += space_for_datas;
    }

    //Next 按鈕
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW *3/2, halfH * 7 / 4 - 50, 200, 100);
    btn->SetOnClickCallback(std::bind(&ScoreBoardScene::NextPage, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Next", "pirulen.ttf", 48, halfW *3/2+100, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));

    //Prev 按鈕
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW *1/2-200, halfH * 7 / 4 - 50, 200, 100);
    btn->SetOnClickCallback(std::bind(&ScoreBoardScene::PrevPage, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Prev", "pirulen.ttf", 48, halfW *1/2-100, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&ScoreBoardScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));

    
    // (Not safe if release resource while playing, however we only free while change scene, so it's fine.)
	bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);
}



void ScoreBoardScene::NextPage() {
    if ((page + 1) * itemsPerPage < scoreboardData.size()){
        page++;
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard-scene");
    } 
    
}

void ScoreBoardScene::PrevPage() {
    if (page>0){
        page--;
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard-scene");
    } 
    
}

////////////////////////////////////////////////



void ScoreBoardScene::Terminate() {
	AudioHelper::StopSample(bgmInstance);
	bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
	IScene::Terminate();
}
void ScoreBoardScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

