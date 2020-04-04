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
	//�^���烂���X�^�[�𐶂ݏo�����߂̃R���X�g���N�^
	Monster(std::string name, Parameters& param, ParameterAI& paramAI, GameLib::Scene::Model& model);
	bool mFriendly;	//�v���C���[�̒��Ԃ��ǂ���
};