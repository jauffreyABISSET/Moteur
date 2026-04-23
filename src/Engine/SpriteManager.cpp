#include "pch.h"
//#include "SpriteManager.h"
//#include <fstream>
//#include <sstream>
//#include <string>
//
//namespace spr {
//
//	SpriteManager::SpriteManager()
//	{
//
//	}
//
//	SpriteManager::~SpriteManager()
//	{
//	}
//
//	std::vector<Sprite*> SpriteManager::GetSprites() const
//	{
//		return std::vector<Sprite*>();
//	}
//
//	Sprite* SpriteManager::CreateSprite(std::string path)
//	{
//		std::string file = Utils::ReadAll(path + ".sprite");
//		if (file.empty()) {
//			assert(false && "Failed to read sprite file");
//			return nullptr;
//		}
//
//		Sprite* sprite = new Sprite(0, "");
//		std::ifstream fileStream(path);
//
//
//		return nullptr;
//	}
//
//}
