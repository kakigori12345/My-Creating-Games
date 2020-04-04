#pragma once

#include "Monster.h"
#include "MonsterFactory.h"

/*
Monsterをゲーム中に管理するクラス
生成自体はMonsterFactoryに任せて、その結果を受け取り管理する
基本的にフレームごとのupdate（更新）や描画を担当する
敵モンスターを仲間にすることもできるようにする
*/
class MonsterController {
public:
	MonsterController();
	~MonsterController();
	void update();
	void draw();

	//生成するモンスターの種類を指定。その分のメモリを確保
	void setNumOfMonsterType(int num);
	//生成するモンスターとその数を指定して生成。生成自体はMonstarFactoryに任せる
	void createMonster(const char* monster, int num);
	//位置を変更する
	void setPos(int id, const GameLib::Math::Vector3& pos);
	//ダメージを与える。アタッカーがプレイヤのときに、モンスターがダメージで倒れたら仲間判定がされる
	void setDamage(int id, int receivePower, bool attackerIsPlayer, bool& died, bool& beFriend);
	//モンスターを殺す
	void killMonster(int id);
	//敵モンスターを仲間モンスターにする
	bool makeFriend(int id);
	//敵モンスターを全て削除する。仲間モンスターは残す
	void resetEnemyMonster();
	//仲間モンスターのパラメータを表示させる（実際に表示するのはFrontDisplayの役目）
	void showFriendMonstersParameter();

	//特定モンスターの情報を返す
	//IDから、そのモンスター自体のポインタを
	Monster* getMonsterFromID(int id);
	//そのモンスターが持っているパーツ
	Parts getParts(int id);
	//IDから、そのモンスターが仲間かどうか返す
	bool getFriendlyByID(int id) const;

	//情報を返す関数
	//敵の総数を返す
	int getNumOfMonsters() const;
	//保持しているモンスターのIDを返す。使われ方は限定的で、GameクラスがStageクラスに
	//一つ一つのキャラクターのIDをセットするときにしか使われない（このクラスからはStageクラスに
	//アクセスできないため）。引数にtrueをセットすると、最初からリセット。
	//最後まで到達したら-1を返す。ついでにプレイヤーの仲間かどうかも返す
	int getID( bool reset , bool* frinedly);
	//アクティブキャラクター（これから動くモンスター）のポインタを返す
	//敵モンスター
	Monster* returnActiveEnemyPointer(bool* last);
	//味方モンスター
	Monster* returnActiveFriendPointer(bool* last);
	//アクティブキャラクターを参照するための変数を最初にリセット
	void resetActiveCounter();
private:
	MonsterFactory mMonFac;
	Monster*** mMonsters;		//ステージに生成するモンスターたち。2次元配列かつポインタで保持するために3次ポインタになった
	Monster** mFriendMonsters;	//プレイヤーの仲間になったモンスターを保持する
	int mNumOfMonsterType;		//生成する予定のモンスターの種類の数
	int* mNumOfMonsterList;		//モンスター1種類ごとの生成数
	int mNumOfMonsters;			//生成したモンスターの総数
	int mNumOfFriendMonsters;	//仲間になったモンスターの数
	int nowCreatingPoint;		//createMonsterで次に使う配列のインデックスs
	bool wasSetMemory;			//メモリがセットされているかどうか
	int nowActiveEnemyIndex;	//アクティブキャラクター（敵）を返す時の連番 　0～最大モンスター数-1　まで
	int nowActiveFriendIndex;	//上と同様だが、味方モンスター用
	int nowGettingID;			//getIDを使うときに使う。それ以外では使わない

	//このクラス内でしか使わない関数
	//受け取った数字番目のインスタンスのMonsterのインデックスを返す
	void fromNumberToMonstersIndex(int num, int* x, int* y) const;

	//定数群
	int MAX_NUM_OF_FRIEND_MONSTER;		//仲間モンスターの最大数
};