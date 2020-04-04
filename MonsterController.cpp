#include "GameLib/Framework.h"
#include "GameLib/Math/Vector3.h"
#include "ContentReader.h"
#include "MonsterController.h"
#include "FrontDisplayh.h"
#include "MyRandom.h"
#include <string>

using namespace GameLib;
using namespace GameLib::Math;
using namespace std;

/*
仲間にしたモンスターの処理方法
元々モンスターたちはメンバの配列（mMonsters）に保持されており、ステージが進む際に一斉に解放される。
そのため、モンスターを仲間にしても、ステージが進めば一緒に削除されてしまう。
そこで、仲間になったモンスターは別の配列（mFriendMonsters）に移動させ、移動前のmMonsters内のメモリはnullptrにする。
（※ここで、メモリをdeleteしてはいけない。そうすると、仲間モンスターは消えてしまう）
つまり、敵モンスターを保持している配列(mMonsters)に、空のメモリが混在することになる。
そのため、毎フレームのモンスターの更新や描画の際に、メモリが空でないかどうか判定する必要がある点に注意。
*/

MonsterController::MonsterController():
	nowCreatingPoint(0),
	wasSetMemory(false),
	mNumOfMonsters(0),
	mNumOfFriendMonsters(0),
	nowActiveEnemyIndex(0),
	nowActiveFriendIndex(0),
	nowGettingID(0)
{
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	MAX_NUM_OF_FRIEND_MONSTER = (int)cr->getData("MonsterController", "MAX_NUM_OF_FRIEND_MONSTER");
	SAFE_DELETE(cr);//使い終わったら削除

	//仲間モンスターを格納しておく配列のメモリを確保し、初期化する
	mFriendMonsters = new Monster*[MAX_NUM_OF_FRIEND_MONSTER];
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
		mFriendMonsters[i] = nullptr;
}

MonsterController::~MonsterController()
{
	for (int i = 0; i < mNumOfMonsterType; ++i)
	{
		SAFE_DELETE_ARRAY(mMonsters[i]);
	}

	SAFE_DELETE_ARRAY(mNumOfMonsterList);
	SAFE_DELETE_ARRAY(mFriendMonsters);
}

void MonsterController::update()
{

}

void MonsterController::draw()
{
	//敵
	for (int i = 0; i < mNumOfMonsterType; ++i)
	{
		for (int j = 0; j < mNumOfMonsterList[i]; ++j)
		{
			if (mMonsters[i][j] != nullptr)
				mMonsters[i][j]->draw();
		}
	}
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr)
			mFriendMonsters[i]->draw();
	}
}

void MonsterController::setNumOfMonsterType(int num)
{
	mNumOfMonsterType = num;
	mMonsters = new Monster**[num];
	mNumOfMonsterList = new int[num];
	wasSetMemory = true;
}

void MonsterController::createMonster(const char* monster, int num)
{
	//エラーチェック
	if (nowCreatingPoint == mNumOfMonsterType)
		HALT("File:MonsterController [createMonster] (validation error) Error");
	ASSERT(wasSetMemory);
	//指定の数だけメモリ確保
	mNumOfMonsterList[nowCreatingPoint] = num;
	mMonsters[nowCreatingPoint] = mMonFac.spawnMonster(monster, num);
	
	//メモリを格納する場所を一つ進め、総モンスター数を更新する
	nowCreatingPoint++;
	mNumOfMonsters += num;
}

void MonsterController::setPos(int id, const Vector3& pos)
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				mFriendMonsters[i]->setPosition(pos);
				return;
			}
		}
	}
	//敵
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					mMonsters[i][j]->setPosition(pos);
					return;
				}
			}
		}
	}
	//ここまで来たら、そのIDは存在しないということ
	HALT("File:MonsterControlelr.cpp [setPos()] That's ID is not exist. Error");
}

void MonsterController::setDamage(int id, int receiveDamage, bool attackerIsPlayer, bool& died, bool& beFriend)
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				//ダメージを与える
				mFriendMonsters[i]->damage(receiveDamage);
				//生きているか？
				if (!mFriendMonsters[i]->isLiving()) {
					//死んでいる
					died = true;
				}
				return;
			}
		}
	}
	//敵
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					//ダメージを与える
					mMonsters[i][j]->damage(receiveDamage);
					//生きているか？
					if (!mMonsters[i][j]->isLiving()) {
						//死んでいる
						died = true;
						if (attackerIsPlayer) {
							int rate = mMonsters[i][j]->getParameter()->capture;
							if (MyRandom::instance()->fortune(rate))
								beFriend = true;

						}
					}
					return;
				}
			}
		}
	}
	//ここまで来たら、そのIDは存在しないということ
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");
}

void MonsterController::killMonster(int id)
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				SAFE_DELETE(mFriendMonsters[i]);
				mNumOfFriendMonsters--;
				return;
			}
		}
	}
	//敵
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					ASSERT(mMonsters[i][j]);
					SAFE_DELETE(mMonsters[i][j]);
					return;
				}
			}
		}
	}
	//ここまで来たら、そのIDは存在しないということ
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");
}

void MonsterController::resetEnemyMonster()
{
	//パラメータを初期化
	nowCreatingPoint = 0;
	nowActiveEnemyIndex = 0;
	nowActiveFriendIndex = 0;
	nowGettingID = 0;
	wasSetMemory = false;
	mNumOfMonsters = 0;
	mNumOfMonsterType = 0;
	//メモリ開放
	//※ここで仲間モンスターのメモリを開放しないこと
	for (int i = 0; i < mNumOfMonsterType; ++i)
		SAFE_DELETE_ARRAY(mMonsters[i]);
	SAFE_DELETE_ARRAY(mNumOfMonsterList);
}

bool MonsterController::makeFriend(int id)
{
	//上限に達していたら無効
	if (mNumOfFriendMonsters == MAX_NUM_OF_FRIEND_MONSTER)
		return false;
	//IDを探索
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				//見つかったか
				if (mMonsters[i][j]->getID() == id) {
					//ポインタを移す
					for (int k = 0; k < MAX_NUM_OF_FRIEND_MONSTER; ++k){
						if (mFriendMonsters[k] == nullptr) {
							Monster* temp;
							temp = mMonsters[i][j];
							mMonsters[i][j] = nullptr;
							SAFE_DELETE(mMonsters[i][j]);
							mMonsters[i][j] = nullptr;
							mFriendMonsters[k] = temp;
							mFriendMonsters[k]->beFriend();
							//移動前のメモリは削除しない。削除するとモンスターは消えてしまう
							//mMonsters[i][j] = nullptr;
							mNumOfFriendMonsters++;
							return true;
						}
					}
					//空のメモリがない。これはエラー
					HALT("File:MonsterController.cpp [makeFriend()] Error");
					return false;
				}
			}
		}
	}

	//該当IDが見つからない。これもエラー
	HALT("File:MonsterController.cpp [makeFriend()] Error");
	return false;
}

void MonsterController::showFriendMonstersParameter()
{
	string* names = new string[mNumOfFriendMonsters];
	Parameters* params = new Parameters[mNumOfFriendMonsters];
	int* nowHPs = new int[mNumOfFriendMonsters];

	int count = 0;
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr)
		{
			names[count] = mFriendMonsters[i]->getName();
			params[count] = *mFriendMonsters[i]->getParameter();
			nowHPs[count] = mFriendMonsters[i]->getHP();
			count++;
		}
		if (count == mNumOfFriendMonsters)
			break;
	}

	if (count > 0) {
		ASSERT(names);
		ASSERT(params);
		ASSERT(nowHPs);
		FrontDisplay::instance()->showParameterOfMonsters(mNumOfFriendMonsters, names, params, nowHPs);
	}

	SAFE_DELETE_ARRAY(names);
	SAFE_DELETE_ARRAY(params);
	SAFE_DELETE_ARRAY(nowHPs);
}


//特定モンスターの情報を返す
//モンスター自体のポインタ
Monster* MonsterController::getMonsterFromID(int id)
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return mFriendMonsters[i];
			}
		}
	}
	//敵
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					ASSERT(mMonsters[i][j]);
					return mMonsters[i][j];
				}
			}
		}
	}
	//ここまで来たら、そのIDは存在しないということ
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");

	return nullptr;
}

//モンスターが持っているパーツ	
Parts MonsterController::getParts(int id)
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return mFriendMonsters[i]->getParts();
			}
		}
	}
	//敵
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					ASSERT(mMonsters[i][j]);
					return mMonsters[i][j]->getParts();
				}
			}
		}
	}
	//ここまで来たら、そのIDは存在しないということ
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");

	return Parts::Parts();
}

bool MonsterController::getFriendlyByID(int id) const
{
	//仲間
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return true;
			}
		}
	}

	//ここまできたらそのモンスターは仲間じゃないということ
	return false;
}



int MonsterController::getNumOfMonsters() const
{
	return mNumOfMonsters;
}

int MonsterController::getID(bool reset, bool* friendly)
{
	//引数がtrueなら最初の1回目ということなので、カウントをリセット
	if (reset)
		nowGettingID = 0;
	//falseなら、カウントを次に進める
	else
		nowGettingID++;
	//現在のカウントが敵モンスター総数未満なら、敵モンスターを参照
	//それ以上なら、仲間モンスターを参照
	if (nowGettingID < mNumOfMonsters)
	{
		while (true) {
			int x(0), y(0);
			fromNumberToMonstersIndex(nowGettingID, &x, &y);
			if (mMonsters[y][x] != nullptr) {
				*friendly = mMonsters[y][x]->getFriendly();
				return mMonsters[y][x]->getID();
			}
			//該当メモリが空ならインデックスを進める
			else
				nowGettingID++;
		}
	}
	else
	{
		while (true) {
			int x = nowGettingID - mNumOfMonsters;
			if (x >= MAX_NUM_OF_FRIEND_MONSTER)
				break;
			if (mFriendMonsters[x] != nullptr) {
				*friendly = mFriendMonsters[x]->getFriendly();
				return mFriendMonsters[x]->getID();
			}
			//該当メモリが空ならインデックスを進める
			else
				nowGettingID++;
		}
	}

	//ここまで来たなら、最後まで到達したということなので、-1を返す
	return -1;
}

Monster* MonsterController::returnActiveEnemyPointer(bool* last)
{
	//インデックス番目のインスタンスのMonsterを返す
	while (true) {
		//インデックスが最後に到達しているならfalse
		if (nowActiveEnemyIndex >= mNumOfMonsters) {
			*last = false;
			return nullptr;
		}
		int x(0), y(0);
		fromNumberToMonstersIndex(nowActiveEnemyIndex, &x, &y);
		if (mMonsters[y][x] != nullptr) {
			//インデックスを進める
			Monster* test = mMonsters[y][x];
			nowActiveEnemyIndex++;
			*last = true;
			return test;
		}
		//該当メモリが空ならインデックスを進める
		else
			nowActiveEnemyIndex++;
	}

	HALT("File:MonsterConstroller.cpp [returnActiveEnemyPointer] Error");
}

Monster* MonsterController::returnActiveFriendPointer(bool* last)
{
	//インデックス番目のインスタンスのMonsterを返す
	while (true) {
		//インデックスが最後に到達しているならfalse
		if (nowActiveFriendIndex >= MAX_NUM_OF_FRIEND_MONSTER) {
			*last = false;
			return nullptr;
		}
		if (mFriendMonsters[nowActiveFriendIndex] != nullptr) {
			//インデックスを進める
			int temp = nowActiveFriendIndex;
			nowActiveFriendIndex++;
			*last = true;
			return mFriendMonsters[temp];
		}
		//該当メモリが空ならインデックスを進める
		else
			nowActiveFriendIndex++;
	}

	HALT("File:MonsterConstroller.cpp [returnActiveFriendPointer] Error");
}

void MonsterController::resetActiveCounter()
{
	nowActiveEnemyIndex = 0;
	nowActiveFriendIndex = 0;
}


////////////////////このクラス内でしか使わない関数//////////////////////////

//敵モンスターの配列を探索するための関数
void MonsterController::fromNumberToMonstersIndex(int num, int* x, int* y) const
{
	//一応リセット
	*x = 0;
	*y = 0;
	//num番目（0から）のインスタンスを検索（2次元配列なので処理がやや複雑）
	while (true) {
		if (mNumOfMonsterList[*y] > num) {
			*x = num;
			break;
		}
		else {
			num -= mNumOfMonsterList[*y];
			*y += 1;
		}
	}
}