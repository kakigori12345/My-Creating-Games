#pragma once

#include "Monster.h"

class MonsterFactory {
public:
	MonsterFactory();
	~MonsterFactory();
	//指定のモンスターを生成して返す
	Monster** spawnMonster(const char* monster, int num);
private:
	Monster* mMonsters;
	int mNumOfMonsters;
	//
	static const char* filename;	//Monsterの詳細が記述されているファイルのパス
	static const char* defaultName;	//Monsterのパラメータのデフォルト値。パラメータが何も設定されていないならこの名前のデータが設定される
};