#pragma once

#include "Monster.h"

class MonsterFactory {
public:
	MonsterFactory();
	~MonsterFactory();
	//�w��̃����X�^�[�𐶐����ĕԂ�
	Monster** spawnMonster(const char* monster, int num);
private:
	Monster* mMonsters;
	int mNumOfMonsters;
	//
	static const char* filename;	//Monster�̏ڍׂ��L�q����Ă���t�@�C���̃p�X
	static const char* defaultName;	//Monster�̃p�����[�^�̃f�t�H���g�l�B�p�����[�^�������ݒ肳��Ă��Ȃ��Ȃ炱�̖��O�̃f�[�^���ݒ肳���
};