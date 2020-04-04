#pragma once

#include "Monster.h"
#include "MonsterFactory.h"

/*
Monster���Q�[�����ɊǗ�����N���X
�������̂�MonsterFactory�ɔC���āA���̌��ʂ��󂯎��Ǘ�����
��{�I�Ƀt���[�����Ƃ�update�i�X�V�j��`���S������
�G�����X�^�[�𒇊Ԃɂ��邱�Ƃ��ł���悤�ɂ���
*/
class MonsterController {
public:
	MonsterController();
	~MonsterController();
	void update();
	void draw();

	//�������郂���X�^�[�̎�ނ��w��B���̕��̃��������m��
	void setNumOfMonsterType(int num);
	//�������郂���X�^�[�Ƃ��̐����w�肵�Đ����B�������̂�MonstarFactory�ɔC����
	void createMonster(const char* monster, int num);
	//�ʒu��ύX����
	void setPos(int id, const GameLib::Math::Vector3& pos);
	//�_���[�W��^����B�A�^�b�J�[���v���C���̂Ƃ��ɁA�����X�^�[���_���[�W�œ|�ꂽ�璇�Ԕ��肪�����
	void setDamage(int id, int receivePower, bool attackerIsPlayer, bool& died, bool& beFriend);
	//�����X�^�[���E��
	void killMonster(int id);
	//�G�����X�^�[�𒇊ԃ����X�^�[�ɂ���
	bool makeFriend(int id);
	//�G�����X�^�[��S�č폜����B���ԃ����X�^�[�͎c��
	void resetEnemyMonster();
	//���ԃ����X�^�[�̃p�����[�^��\��������i���ۂɕ\������̂�FrontDisplay�̖�ځj
	void showFriendMonstersParameter();

	//���胂���X�^�[�̏���Ԃ�
	//ID����A���̃����X�^�[���̂̃|�C���^��
	Monster* getMonsterFromID(int id);
	//���̃����X�^�[�������Ă���p�[�c
	Parts getParts(int id);
	//ID����A���̃����X�^�[�����Ԃ��ǂ����Ԃ�
	bool getFriendlyByID(int id) const;

	//����Ԃ��֐�
	//�G�̑�����Ԃ�
	int getNumOfMonsters() const;
	//�ێ����Ă��郂���X�^�[��ID��Ԃ��B�g�����͌���I�ŁAGame�N���X��Stage�N���X��
	//���̃L�����N�^�[��ID���Z�b�g����Ƃ��ɂ����g���Ȃ��i���̃N���X�����Stage�N���X��
	//�A�N�Z�X�ł��Ȃ����߁j�B������true���Z�b�g����ƁA�ŏ����烊�Z�b�g�B
	//�Ō�܂œ��B������-1��Ԃ��B���łɃv���C���[�̒��Ԃ��ǂ������Ԃ�
	int getID( bool reset , bool* frinedly);
	//�A�N�e�B�u�L�����N�^�[�i���ꂩ�瓮�������X�^�[�j�̃|�C���^��Ԃ�
	//�G�����X�^�[
	Monster* returnActiveEnemyPointer(bool* last);
	//���������X�^�[
	Monster* returnActiveFriendPointer(bool* last);
	//�A�N�e�B�u�L�����N�^�[���Q�Ƃ��邽�߂̕ϐ����ŏ��Ƀ��Z�b�g
	void resetActiveCounter();
private:
	MonsterFactory mMonFac;
	Monster*** mMonsters;		//�X�e�[�W�ɐ������郂���X�^�[�����B2�����z�񂩂|�C���^�ŕێ����邽�߂�3���|�C���^�ɂȂ���
	Monster** mFriendMonsters;	//�v���C���[�̒��ԂɂȂ��������X�^�[��ێ�����
	int mNumOfMonsterType;		//��������\��̃����X�^�[�̎�ނ̐�
	int* mNumOfMonsterList;		//�����X�^�[1��ނ��Ƃ̐�����
	int mNumOfMonsters;			//�������������X�^�[�̑���
	int mNumOfFriendMonsters;	//���ԂɂȂ��������X�^�[�̐�
	int nowCreatingPoint;		//createMonster�Ŏ��Ɏg���z��̃C���f�b�N�Xs
	bool wasSetMemory;			//���������Z�b�g����Ă��邩�ǂ���
	int nowActiveEnemyIndex;	//�A�N�e�B�u�L�����N�^�[�i�G�j��Ԃ����̘A�� �@0�`�ő僂���X�^�[��-1�@�܂�
	int nowActiveFriendIndex;	//��Ɠ��l�����A���������X�^�[�p
	int nowGettingID;			//getID���g���Ƃ��Ɏg���B����ȊO�ł͎g��Ȃ�

	//���̃N���X���ł����g��Ȃ��֐�
	//�󂯎���������Ԗڂ̃C���X�^���X��Monster�̃C���f�b�N�X��Ԃ�
	void fromNumberToMonstersIndex(int num, int* x, int* y) const;

	//�萔�Q
	int MAX_NUM_OF_FRIEND_MONSTER;		//���ԃ����X�^�[�̍ő吔
};