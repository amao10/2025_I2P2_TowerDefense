#ifndef SCOREBOARDSCENE_HPP
#define SCOREBOARDSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>
#include "Engine/IScene.hpp"

class ScoreBoardScene final : public Engine::IScene {
private:
	std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
public:
	static int isloading;
	explicit ScoreBoardScene() = default;
	void Initialize() override;
	void Terminate() override;

	void BackOnClick(int stage);
	void DisplayPage(int page);
	void NextPage();
	void PrevPage();
	///to private?
	void ClearLabels();
	std::vector<std::shared_ptr<Engine::Label>> labels; // 用于存储当前页面的所有文字标签
};

#endif // SCOREBOARDSCENE_HPP
