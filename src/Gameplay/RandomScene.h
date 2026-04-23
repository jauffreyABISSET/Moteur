#pragma once

class RandomScene : public Scene
{
	bool m_test = false;
public:
	RandomScene() = default;
	virtual ~RandomScene();
	void Init() override;
	void Update(const GameTimer& gt) override;
	void Draw(const GameTimer& gt) override;
};

