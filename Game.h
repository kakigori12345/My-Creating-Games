#pragma once

#include "Sequence.h"
#include "Stage.h"
#include "PlayCharacter.h"
#include "MonsterController.h"

namespace GameLib {
	namespace Math {
		class Vector3;
		class Matrix34;
		class Matrix44;
	}
}

//�Q�[���N���X
//�Q�[�����̃I�u�W�F�N�g�E�����̊Ǘ������̃N���X���s��

class Game : public Sequence{
public:
	//�Q�[���̏��
	enum Situation {
		NORMAL,				//�ʏ�
		MENU,				//�s���I��
		MAKE_FRIEND,		//�G�𒇊Ԃɂ��邩�ǂ����I��
		TURN_LIMIT,			//�^�[�������Ɉ�����������
		SHOW_STAGE_NUM,		//�X�e�[�W�̊K�w��\�����B��莞�ԂŎ��ɐi��
		ANNOUNCEMENT,		//������\������
		VICTORY,			//�Q�[���N���A
		LOSE,				//�Q�[���I�[�o

		INVALID
	};

	Game();
	~Game();
	virtual void update();
	virtual void draw();
	//�J�����֐��ɏ���`���Ă��������ɉ��o���Ă��炤�B������main.cpp���ōs����
	virtual void sendInfoToCamera(
		Camera::Mode* mode,					//�J�����̃��[�h��I������
		GameLib::Math::Vector3* playerPos,	//�v���C���[�̈ʒu���W
		GameLib::Math::Vector3* enemyPos,	//����L�����N�^�[�i�G�Ȃǁj�̈ʒu���W
		float* eyeAngleYofPlayer,			//�v���C���[�̎�������
		float* time							//�^�C�~���O����( 0.f �` 1.f )
	) const;
	//�Q�[�����I���������Ƃ��ǂ����m��
	virtual bool isFinished() const;
	virtual Sequence* createNextSequence();
	//�O���ɏ���n���֐�
	Situation getSituation();
private:
	//���̃N���X���ł����g��Ȃ��֐��B��ɏ����̒���������̂�ʂɂ܂Ƃ߂�����
	void menu();
	//�A�N�e�B�u�L�����N�^�[�̃`�F���W�Ǘ�
	void checkActiveCharacterChange();
	//�v���C���[�ȊO�������Ȃ����A�X�e�[�W�̊K�w��i�߂�
	void createNewStage();
	//�v���C���[���K�w��i�߂邩�ǂ����m�F�B�����Ȃ玟�̃X�e�[�W��
	bool playerOnTheGoal();
	//�K�w��i�߂鏈���B�ŏI�K�w�Ȃ�N���A
	void gotoNextStage();
	//�퓬�����B�G�����ԂɂȂ�����A�Ώۂ������������蕡�G�Ȃ̂ŁA�ʂɈڐ�
	void battle(bool& enemyWantToBeFriend);
	//�퓬�ȊO�ł̃_���[�W����
	void setDamage(bool passiveIsPlayer, int id);
	//�����X�^�[���E���Ƃ��ɌĂяo���������܂Ƃ߂�����
	void killMonster(int targetID, bool partsWillBeGot);

	//�t���O�Ǘ�
	bool battleIsDone;			//�U�����s���Ă��邩�ǂ���
	bool attackIsMagic;			//�������U�����ǂ���
	bool mFinished;				//�Q�[���̏I��
	bool createNextStageFlag;	//�V�����X�e�[�W�𐶐�����Ƃ���true��

	//���̑��A�֗��Ɏg�����߂̕ϐ�
	bool victory;			//�I���������_�ŁA�Q�[���N���A�Ȃ̂�
	Situation mSituation;	//���݂̃V�`���G�[�V������ۑ�
	Situation mPreSituation;//��O�̃V�`���G�[�V������ۑ�
	float situationCount;	//0.f�`1.f�̊ԂŃ^�C�~���O��ۑ�
	int StageNumber;		//���݂ǂ̃X�e�[�W�i�K�w�j����ێ�����B���̐���Stage�N���X�ɓn���ĊY���X�e�[�W�𐶐����Ă��炤
	int playerID;			//�v���C�L�����N�^�[��ID��ێ����Ă���
	int messageNum;			//�����̃��b�Z�[�W�����ʂŕ\������Ƃ��ɁA���݉����ڂ�
	int IDofFriendly;		//���ԂɂȂ肽�����ȃ����X�^�[��ID
	bool TrapHoleIsDone;	//���Ƃ����������������ǂ����B�A�N�e�B�u�L�����N�^�[��ύX��A
							//���O�ɃA�N�e�B�u�L�����N�^�[���������́i���Ƃ����ɂ͂܂������́j���E��
	int IDofFalled;			//���Ƃ����ɂ͂܂��������X�^�[��ID
	bool TrapCatchIsDone;	//�ߊl㩂������������ǂ����B�������A�N�e�B�u�L�����N�^�[��ύX��A
							//�ߊl㩂ɂ͂܂��������X�^�[�𒇊Ԃɂł��邩�ǂ����𔻒肵�āA�ł���Ȃ璇�Ԃ�
	int IDofCatched;		//�ߊl㩂ɂ͂܂��������X�^�[��ID
	bool TrapBombIsDone;	//�n���������������ǂ����B�������A�N�e�B�u�L�����N�^�[��ύX��i�ύX��łȂ��ƃL�����ύX�Ńo�O��j�A
							//�n���𓥂񂾃����X�^�[�Ƀ_���[�W��^����B���񂾂炻�̂܂܎E��
	int IDofBombed;			//�n���𓥂񂾃����X�^�[��ID
	Parts nowGotParts;		//�v���C���[���Q�b�g����p�[�c�B�\���p

	//�����o�Ƃ��ĕێ����Ă������N���X�̃|�C���^
	Character* mActiveCharacter;	//���݁A�œ_���������Ă���i�^�[��������Ă����j�L�����N�^�[�̃|�C���^��ێ�����
	Character* mPassiveCharacter;	//�A�N�e�B�u�L�����N�^�[����̊��ɂ��A�������瓮�삳����󂯐g�̃L�����N�^�[�̃|�C���^��ێ�����
	Stage* mStage;					//�X�e�[�W
	PlayCharacter* mPC;				//�v���C�L�����N�^�[
	MonsterController* mMonCon;		//�����X�^�[�R���g���[���[

	//�萔�Q�@�t�@�C������ǂݏo��
	int		FINAL_STAGE_NUMBER;		//�ŏI�X�e�[�W�̃i���o�[
};