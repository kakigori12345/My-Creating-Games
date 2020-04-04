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
���Ԃɂ��������X�^�[�̏������@
���X�����X�^�[�����̓����o�̔z��imMonsters�j�ɕێ�����Ă���A�X�e�[�W���i�ލۂɈ�Ăɉ�������B
���̂��߁A�����X�^�[�𒇊Ԃɂ��Ă��A�X�e�[�W���i�߂Έꏏ�ɍ폜����Ă��܂��B
�����ŁA���ԂɂȂ��������X�^�[�͕ʂ̔z��imFriendMonsters�j�Ɉړ������A�ړ��O��mMonsters���̃�������nullptr�ɂ���B
�i�������ŁA��������delete���Ă͂����Ȃ��B��������ƁA���ԃ����X�^�[�͏����Ă��܂��j
�܂�A�G�����X�^�[��ێ����Ă���z��(mMonsters)�ɁA��̃����������݂��邱�ƂɂȂ�B
���̂��߁A���t���[���̃����X�^�[�̍X�V��`��̍ۂɁA����������łȂ����ǂ������肷��K�v������_�ɒ��ӁB
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
	//�萔�̓ǂݍ���
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	MAX_NUM_OF_FRIEND_MONSTER = (int)cr->getData("MonsterController", "MAX_NUM_OF_FRIEND_MONSTER");
	SAFE_DELETE(cr);//�g���I�������폜

	//���ԃ����X�^�[���i�[���Ă����z��̃��������m�ۂ��A����������
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
	//�G
	for (int i = 0; i < mNumOfMonsterType; ++i)
	{
		for (int j = 0; j < mNumOfMonsterList[i]; ++j)
		{
			if (mMonsters[i][j] != nullptr)
				mMonsters[i][j]->draw();
		}
	}
	//����
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
	//�G���[�`�F�b�N
	if (nowCreatingPoint == mNumOfMonsterType)
		HALT("File:MonsterController [createMonster] (validation error) Error");
	ASSERT(wasSetMemory);
	//�w��̐������������m��
	mNumOfMonsterList[nowCreatingPoint] = num;
	mMonsters[nowCreatingPoint] = mMonFac.spawnMonster(monster, num);
	
	//���������i�[����ꏊ����i�߁A�������X�^�[�����X�V����
	nowCreatingPoint++;
	mNumOfMonsters += num;
}

void MonsterController::setPos(int id, const Vector3& pos)
{
	//����
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				mFriendMonsters[i]->setPosition(pos);
				return;
			}
		}
	}
	//�G
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
	//�����܂ŗ�����A����ID�͑��݂��Ȃ��Ƃ�������
	HALT("File:MonsterControlelr.cpp [setPos()] That's ID is not exist. Error");
}

void MonsterController::setDamage(int id, int receiveDamage, bool attackerIsPlayer, bool& died, bool& beFriend)
{
	//����
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				//�_���[�W��^����
				mFriendMonsters[i]->damage(receiveDamage);
				//�����Ă��邩�H
				if (!mFriendMonsters[i]->isLiving()) {
					//����ł���
					died = true;
				}
				return;
			}
		}
	}
	//�G
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				if (mMonsters[i][j]->getID() == id) {
					//�_���[�W��^����
					mMonsters[i][j]->damage(receiveDamage);
					//�����Ă��邩�H
					if (!mMonsters[i][j]->isLiving()) {
						//����ł���
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
	//�����܂ŗ�����A����ID�͑��݂��Ȃ��Ƃ�������
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");
}

void MonsterController::killMonster(int id)
{
	//����
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
	//�G
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
	//�����܂ŗ�����A����ID�͑��݂��Ȃ��Ƃ�������
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");
}

void MonsterController::resetEnemyMonster()
{
	//�p�����[�^��������
	nowCreatingPoint = 0;
	nowActiveEnemyIndex = 0;
	nowActiveFriendIndex = 0;
	nowGettingID = 0;
	wasSetMemory = false;
	mNumOfMonsters = 0;
	mNumOfMonsterType = 0;
	//�������J��
	//�������Œ��ԃ����X�^�[�̃��������J�����Ȃ�����
	for (int i = 0; i < mNumOfMonsterType; ++i)
		SAFE_DELETE_ARRAY(mMonsters[i]);
	SAFE_DELETE_ARRAY(mNumOfMonsterList);
}

bool MonsterController::makeFriend(int id)
{
	//����ɒB���Ă����疳��
	if (mNumOfFriendMonsters == MAX_NUM_OF_FRIEND_MONSTER)
		return false;
	//ID��T��
	for (int i = 0; i < mNumOfMonsterType; i++) {
		for (int j = 0; j < mNumOfMonsterList[i]; j++) {
			if (mMonsters[i][j] != nullptr) {
				//����������
				if (mMonsters[i][j]->getID() == id) {
					//�|�C���^���ڂ�
					for (int k = 0; k < MAX_NUM_OF_FRIEND_MONSTER; ++k){
						if (mFriendMonsters[k] == nullptr) {
							Monster* temp;
							temp = mMonsters[i][j];
							mMonsters[i][j] = nullptr;
							SAFE_DELETE(mMonsters[i][j]);
							mMonsters[i][j] = nullptr;
							mFriendMonsters[k] = temp;
							mFriendMonsters[k]->beFriend();
							//�ړ��O�̃������͍폜���Ȃ��B�폜����ƃ����X�^�[�͏����Ă��܂�
							//mMonsters[i][j] = nullptr;
							mNumOfFriendMonsters++;
							return true;
						}
					}
					//��̃��������Ȃ��B����̓G���[
					HALT("File:MonsterController.cpp [makeFriend()] Error");
					return false;
				}
			}
		}
	}

	//�Y��ID��������Ȃ��B������G���[
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


//���胂���X�^�[�̏���Ԃ�
//�����X�^�[���̂̃|�C���^
Monster* MonsterController::getMonsterFromID(int id)
{
	//����
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return mFriendMonsters[i];
			}
		}
	}
	//�G
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
	//�����܂ŗ�����A����ID�͑��݂��Ȃ��Ƃ�������
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");

	return nullptr;
}

//�����X�^�[�������Ă���p�[�c	
Parts MonsterController::getParts(int id)
{
	//����
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return mFriendMonsters[i]->getParts();
			}
		}
	}
	//�G
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
	//�����܂ŗ�����A����ID�͑��݂��Ȃ��Ƃ�������
	HALT("File:MonsterControlelr.cpp [setDamage()] That's ID is not exist. Error");

	return Parts::Parts();
}

bool MonsterController::getFriendlyByID(int id) const
{
	//����
	for (int i = 0; i < MAX_NUM_OF_FRIEND_MONSTER; ++i)
	{
		if (mFriendMonsters[i] != nullptr) {
			if (mFriendMonsters[i]->getID() == id) {
				ASSERT(mFriendMonsters[i]);
				return true;
			}
		}
	}

	//�����܂ł����炻�̃����X�^�[�͒��Ԃ���Ȃ��Ƃ�������
	return false;
}



int MonsterController::getNumOfMonsters() const
{
	return mNumOfMonsters;
}

int MonsterController::getID(bool reset, bool* friendly)
{
	//������true�Ȃ�ŏ���1��ڂƂ������ƂȂ̂ŁA�J�E���g�����Z�b�g
	if (reset)
		nowGettingID = 0;
	//false�Ȃ�A�J�E���g�����ɐi�߂�
	else
		nowGettingID++;
	//���݂̃J�E���g���G�����X�^�[���������Ȃ�A�G�����X�^�[���Q��
	//����ȏ�Ȃ�A���ԃ����X�^�[���Q��
	if (nowGettingID < mNumOfMonsters)
	{
		while (true) {
			int x(0), y(0);
			fromNumberToMonstersIndex(nowGettingID, &x, &y);
			if (mMonsters[y][x] != nullptr) {
				*friendly = mMonsters[y][x]->getFriendly();
				return mMonsters[y][x]->getID();
			}
			//�Y������������Ȃ�C���f�b�N�X��i�߂�
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
			//�Y������������Ȃ�C���f�b�N�X��i�߂�
			else
				nowGettingID++;
		}
	}

	//�����܂ŗ����Ȃ�A�Ō�܂œ��B�����Ƃ������ƂȂ̂ŁA-1��Ԃ�
	return -1;
}

Monster* MonsterController::returnActiveEnemyPointer(bool* last)
{
	//�C���f�b�N�X�Ԗڂ̃C���X�^���X��Monster��Ԃ�
	while (true) {
		//�C���f�b�N�X���Ō�ɓ��B���Ă���Ȃ�false
		if (nowActiveEnemyIndex >= mNumOfMonsters) {
			*last = false;
			return nullptr;
		}
		int x(0), y(0);
		fromNumberToMonstersIndex(nowActiveEnemyIndex, &x, &y);
		if (mMonsters[y][x] != nullptr) {
			//�C���f�b�N�X��i�߂�
			Monster* test = mMonsters[y][x];
			nowActiveEnemyIndex++;
			*last = true;
			return test;
		}
		//�Y������������Ȃ�C���f�b�N�X��i�߂�
		else
			nowActiveEnemyIndex++;
	}

	HALT("File:MonsterConstroller.cpp [returnActiveEnemyPointer] Error");
}

Monster* MonsterController::returnActiveFriendPointer(bool* last)
{
	//�C���f�b�N�X�Ԗڂ̃C���X�^���X��Monster��Ԃ�
	while (true) {
		//�C���f�b�N�X���Ō�ɓ��B���Ă���Ȃ�false
		if (nowActiveFriendIndex >= MAX_NUM_OF_FRIEND_MONSTER) {
			*last = false;
			return nullptr;
		}
		if (mFriendMonsters[nowActiveFriendIndex] != nullptr) {
			//�C���f�b�N�X��i�߂�
			int temp = nowActiveFriendIndex;
			nowActiveFriendIndex++;
			*last = true;
			return mFriendMonsters[temp];
		}
		//�Y������������Ȃ�C���f�b�N�X��i�߂�
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


////////////////////���̃N���X���ł����g��Ȃ��֐�//////////////////////////

//�G�����X�^�[�̔z���T�����邽�߂̊֐�
void MonsterController::fromNumberToMonstersIndex(int num, int* x, int* y) const
{
	//�ꉞ���Z�b�g
	*x = 0;
	*y = 0;
	//num�Ԗځi0����j�̃C���X�^���X�������i2�����z��Ȃ̂ŏ�������╡�G�j
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