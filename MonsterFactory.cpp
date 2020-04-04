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
	//読み込んでMonsterの型を作っていく。とりあえず親子関係などは無視して値を入れ込んでいく
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		string name = cr->getNameOfDataFrame(i);
		const char* target = name.c_str();
		//モンスターの各種パラメータ
		Parameters temp_params;
		//todo:insert （パラメータを新たに追加するなら編集必要）
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
		//モンスターのAIに関するパラメータ
		ParameterAI temp_paramAI;
		//todo:insert （パラメータを新たに追加するなら編集必要）
		temp_paramAI.attack				= static_cast<int>(cr->getData(target, "attack"));
		temp_paramAI.attackMagic		= static_cast<int>(cr->getData(target, "attackMagic"));
		temp_paramAI.moveToEnemy		= static_cast<int>(cr->getData(target, "moveToEnemy"));
		temp_paramAI.playerLove			= static_cast<int>(cr->getData(target, "playerLove"));
		temp_paramAI.runawayFromEnemy	= static_cast<int>(cr->getData(target, "runawayFromEnemy"));
		temp_paramAI.runawayHP			= static_cast<int>(cr->getData(target, "runawayHP"));
		//代入
		mMonsters[i].setName(name);
		mMonsters[i].setParameter(temp_params);
		mMonsters[i].setParameterAI(temp_paramAI);
	}

	//親子関係の処理
	//値が設定されているかどうか判定するための定数
	string NullDataS = ContentReader::NullDataStr;
	float NullDataF = ContentReader::NullDataFloat;
	int NullDataI = static_cast<int>( NullDataF );
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		//継承するMonsterをparentの値から決める
		Parameters targetParam = *mMonsters[i].getParameter();
		string parentName = targetParam.parent;
		ParameterAI targetParamAI = *mMonsters[i].getParameterAI();
		//parentが設定されていないならデフォルト値で補う
		if (parentName == NullDataS)
			parentName = defaultName;
		//継承するパラメータを探す
		Parameters resetParam;
		ParameterAI resetParamAI;
		for (int j = 0; j < mNumOfMonsters; ++j)
		{
			//決定
			if (mMonsters[j].getName() == parentName) {
				resetParam = *mMonsters[j].getParameter();
				resetParamAI = *mMonsters[j].getParameterAI();
				break;
			}
		}
		//継承が必要なパラメータのみ継承する
		//todo:insert （パラメータを新たに追加するなら編集必要）
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
		//最後に更新
		mMonsters[i].setParameter(targetParam);
		mMonsters[i].setParameterAI(targetParamAI);
	}
	delete cr;
	cr = nullptr;

	//その他初期設定
	for (int i = 0; i < mNumOfMonsters; ++i)
	{
		//モデルをセットしておく
		mMonsters[i].setModel( mMonsters[i].getParameter()->image.c_str() );
		//モデルの大きさも
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
	//まずは指定の名前のMonsterを探す
	int target = 0;
	while(target < mNumOfMonsters){
		if (mMonsters[target].getName() == monster)
			break;
		target++;
	}
	if (target == mNumOfMonsters)
		HALT("File::MonsterFactory.cpp [spawnMonster] (That's monster is not exist) Error");
	//指定された数のMonsterを生成
	Monster** reply;
	reply = new Monster*[num];
	for (int i = 0; i < num; ++i)
	{
		reply[i] = mMonsters[target].spawnMe();
	}

	return reply;
}