#pragma once

#include "Character.h"
#include <string>

class Monster :public Character{
public:

	Monster();
	~Monster();
	Monster* spawnMe();
	void beFriend();
	bool getFriendly() const;
private:
	//型からモンスターを生み出すためのコンストラクタ
	Monster(std::string name, Parameters& param, ParameterAI& paramAI, GameLib::Scene::Model& model);
	bool mFriendly;	//プレイヤーの仲間かどうか
};