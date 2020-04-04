#include "GameLib/Framework.h"
#include "MonsterFactory.h"
#include "Action.h"
#include "ContentReader.h"
#include <string>
using namespace std;
using namespace GameLib;

const char* MonsterFactory::filename = "data/monster_book.txt";
const char* MonsterFactory::defaultName = "Default";

MonsterFactory::MonsterFactory()
{
	ContentReader* cr = new ContentReader(filename);
	cr->readStart();
	mNumOfMonsters = cr->getNumOfDataFrames();
	mMonsters = new Monster[mNumOfMonsters];
	//�ǂݍ����Monster�̌^������Ă����B�Ƃ肠�����e�q�֌W�Ȃǂ͖������Ēl����ꍞ��ł���
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		string name = cr->getNameOfDataFrame(i);
		const char* target = name.c_str();
		//�����X�^�[�̊e��p�����[�^
		Parameters temp_params;
		//todo:insert �i�p�����[�^��V���ɒǉ�����Ȃ�ҏW�K�v�j
		temp_params.parent	=					cr->getData_str(target, "parent");
		temp_params.health	= static_cast<int>(cr->getData(target, "health"));
		temp_params.power	= static_cast<int>(cr->getData(target, "power"));
		temp_params.speed	= static_cast<int>(cr->getData(target, "speed"));
		temp_params.defence = static_cast<int>(cr->getData(target, "defence"));
		temp_params.capture = static_cast<int>(cr->getData(target, "capture"));
		temp_params.weapon	=					cr->getData_str(target, "weapon");
		temp_params.parts	= static_cast<int>(cr->getData(target, "parts"));
		temp_params.scale	=					cr->getData(target, "scale");
		temp_params.image	=					cr->getData_str(target, "image");
		//�����X�^�[��AI�Ɋւ���p�����[�^
		ParameterAI temp_paramAI;
		//todo:insert �i�p�����[�^��V���ɒǉ�����Ȃ�ҏW�K�v�j
		temp_paramAI.attack				= static_cast<int>(cr->getData(target, "attack"));
		temp_paramAI.attackMagic		= static_cast<int>(cr->getData(target, "attackMagic"));
		temp_paramAI.moveToEnemy		= static_cast<int>(cr->getData(target, "moveToEnemy"));
		temp_paramAI.playerLove			= static_cast<int>(cr->getData(target, "playerLove"));
		temp_paramAI.runawayFromEnemy	= static_cast<int>(cr->getData(target, "runawayFromEnemy"));
		temp_paramAI.runawayHP			= static_cast<int>(cr->getData(target, "runawayHP"));
		//���
		mMonsters[i].setName(name);
		mMonsters[i].setParameter(temp_params);
		mMonsters[i].setParameterAI(temp_paramAI);
	}

	//�e�q�֌W�̏���
	//�l���ݒ肳��Ă��邩�ǂ������肷�邽�߂̒萔
	string NullDataS = ContentReader::NullDataStr;
	float NullDataF = ContentReader::NullDataFloat;
	int NullDataI = static_cast<int>( NullDataF );
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		//�p������Monster��parent�̒l���猈�߂�
		Parameters targetParam = *mMonsters[i].getParameter();
		string parentName = targetParam.parent;
		ParameterAI targetParamAI = *mMonsters[i].getParameterAI();
		//parent���ݒ肳��Ă��Ȃ��Ȃ�f�t�H���g�l�ŕ₤
		if (parentName == NullDataS)
			parentName = defaultName;
		//�p������p�����[�^��T��
		Parameters resetParam;
		ParameterAI resetParamAI;
		for (int j = 0; j < mNumOfMonsters; ++j)
		{
			//����
			if (mMonsters[j].getName() == parentName) {
				resetParam = *mMonsters[j].getParameter();
				resetParamAI = *mMonsters[j].getParameterAI();
				break;
			}
		}
		//�p�����K�v�ȃp�����[�^�̂݌p������
		//todo:insert �i�p�����[�^��V���ɒǉ�����Ȃ�ҏW�K�v�j
		if (targetParam.health == NullDataI)
			targetParam.health = resetParam.health;
		if (targetParam.power == NullDataI)
			targetParam.power = resetParam.power;
		if (targetParam.speed == NullDataI)
			targetParam.speed = resetParam.speed;
		if (targetParam.defence == NullDataI)
			targetParam.defence = resetParam.defence;
		if (targetParam.capture == NullDataI)
			targetParam.capture = resetParam.capture;
		if (targetParam.weapon == NullDataS)
			targetParam.weapon = resetParam.weapon;
		if (targetParam.parts == NullDataI)
			targetParam.parts = resetParam.parts;
		if (targetParam.scale == NullDataF)
			targetParam.scale = resetParam.scale;
		if (targetParam.image == NullDataS)
			targetParam.image = resetParam.image;
		if (targetParamAI.attack == NullDataI)
			targetParamAI.attack = resetParamAI.attack;
		if (targetParamAI.attackMagic == NullDataI)
			targetParamAI.attackMagic = resetParamAI.attackMagic;
		if (targetParamAI.moveToEnemy == NullDataI)
			targetParamAI.moveToEnemy = resetParamAI.moveToEnemy;
		if (targetParamAI.playerLove == NullDataI)
			targetParamAI.playerLove = resetParamAI.playerLove;
		if (targetParamAI.runawayFromEnemy == NullDataI)
			targetParamAI.runawayFromEnemy = resetParamAI.runawayFromEnemy;
		if (targetParamAI.runawayHP == NullDataI)
			targetParamAI.runawayHP = resetParamAI.runawayHP;
		//�Ō�ɍX�V
		mMonsters[i].setParameter(targetParam);
		mMonsters[i].setParameterAI(targetParamAI);
	}
	delete cr;
	cr = nullptr;

	//���̑������ݒ�
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		//���f�����Z�b�g���Ă���
		mMonsters[i].setModel( mMonsters[i].getParameter()->image.c_str() );
		//���f���̑傫����
		mMonsters[i].setScale(mMonsters[i].getParameter()->scale);
	}
}

MonsterFactory::~MonsterFactory()
{
	delete[] mMonsters;
	mMonsters = nullptr;
}

Monster** MonsterFactory::spawnMonster(const char* monster, int num)
{
	//�܂��͎w��̖��O��Monster��T��
	int target = 0;
	while(target < mNumOfMonsters){
		if (mMonsters[target].getName() == monster)
			break;
		target++;
	}
	if (target == mNumOfMonsters)
		HALT("File::MonsterFactory.cpp [spawnMonster] (That's monster is not exist) Error");
	//�w�肳�ꂽ����Monster�𐶐�
	Monster** reply;
	reply = new Monster*[num];
	for (int i = 0; i < num; ++i)
	{
		reply[i] = mMonsters[target].spawnMe();
	}

	return reply;
}