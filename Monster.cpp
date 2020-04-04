#include "GameLib/Framework.h"
#include "Monster.h"
#include "Action.h"
#include "MyRandom.h"

using namespace GameLib;
using namespace GameLib::Math;
using namespace GameLib::Scene;


Monster::Monster() : 
	mFriendly(false)
{
	
}

Monster::Monster(string name, Parameters& param, ParameterAI& paramAI, Model& model)
{
	//�^����p�����[�^�Ȃǂ��p��
	mName = name;
	mParam = param;
	mParamAI = paramAI;
	mModelCharacter = model;
	HP = mParam.health;
	//�����X�^�[�������Ƀ����_���ȃp�[�c����������
	mParts->setRandomParts(mParam.parts);
	//��������
	if (mParam.weapon != "NULL") {
		if (mParam.weapon == "Soard")
			this->getWeapon(Weapon::Type::Soard);
		else if (mParam.weapon == "Rod")
			this->getWeapon(Weapon::Type::Rod);
	}
}

Monster::~Monster()
{

}

Monster* Monster::spawnMe()
{
	Monster* reply = new Monster(this->mName, this->mParam, this->mParamAI, this->mModelCharacter);

	return reply;
}

void Monster::beFriend()
{
	mFriendly = true;
	//���ԂɂȂ�Ƃ���HP��S�J�ɂ���
	HP = mParam.health;
}

bool Monster::getFriendly() const
{
	return mFriendly;
}