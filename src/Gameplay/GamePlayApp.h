#include "../Engine/AppManager.h"

#pragma once

class GamePlayApp : public AppManager
{
	SceneManager* m_sm;
	EntityManager* m_em;
    Camera* m_pCamera = nullptr;
    XMFLOAT2 mLastMousePos = { 0.f, 0.f };
    Window& m_win;
	bool m_showCursor = false;
	bool m_cursorLocked = true;
public:
    GamePlayApp(Window& win);

    void Init();
    void Update(const GameTimer& gt);

	void CreatePrefabs();
	void CreatePrefabRails(MaterialConstants* material);
    void CreatePrefabWagon(MaterialConstants* material);
    void CreatePrefabStreetLamp(MaterialConstants* material);
    void CreatePrefabStreetLight(MaterialConstants* material);
    void CreatePrefabSpotLight(MaterialConstants* material);
	void TextureRegister();
	void FontRegister();
	void CreatePrefabBullet();

};