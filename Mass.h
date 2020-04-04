#pragma once

#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"

class Mass {
public:
	//�p�l���̎�ށBTRAP��Monster�ɂ����|����Ȃ�
	enum Panel {
		NORMAL,			//�ʏ�		�G�Ȃɂ��Ȃ�
		CORRIDOR,		//�ʘH		�F�Ȃɂ��Ȃ����A㩂��u���Ȃ����������ɂ��Ȃ�Ȃ�
		HOLE,			//������	�F�ʂ�Ȃ�
		TRAP_HOLE,		//���Ƃ���	�F���񂾂玀�ʁB���̌セ�̃}�X��HOLE�ɂȂ�
		TRAP_CATCH,		//�ߊl��	�F�߂܂���
		TRAP_STOPPER,	//���~��	�F�~�܂�
		TRAP_BOMB,		//�n��		�F�������ă_���[�W��^����
		WARP,			//���[�v�@�@�F�ʂ̃��[�v�}�X�ɔ��(�����_��)
		GOAL,			//�K�i		�F���̃X�e�[�W��
		RECOVERY,		//��		�F�񕜂ł���
		PARTS,			//�p�[�c	�F�p�[�c�E����
		INVALID
	};
	//�ǂ���̃`�[���ɓ��܂�Ă��邩�B���܂�Ă���Ƃ��F���ς��
	enum Foot {
		FOOT_PLAYER,
		FOOT_ENEMY,
		NONE
	};
	//�}�X�Ƀp�����[�^�t�^�B�GAI���s������Ƃ��Ɏ���̏󋵂�c�����邽�߂�
	struct MassParam {
		int TeamPlayer;		//�v���C���[�`�[�����߂��ɂ�����x������
		int TeamEnemy;		//�G�l�~�[�`�[��
		//�R���X�g���N�^�B�Ƃ肠�����S��0�ŏ�����
		MassParam() : TeamPlayer(0), TeamEnemy(0) {};
		//���Z�b�g�p�֐�
		void reset() {
			TeamPlayer = 0;
			TeamEnemy = 0;
		}
	};
	//���MassParam��ύX����ۂɁA�֐��Ɏw�肷�邽�߂̖��O��`
	enum MassParamName {
		MP_TEAM_PLAYER,
		MP_TEAM_ENEMY
	};
	Mass();
	~Mass();
	void draw();
	//���f�����̏����ꂽ�R���e�i���󂯎��A�������烂�f�������
	void createModel(const GameLib::Scene::Container& container);
	//�������ꂽ���ƂɈʒu���Ƒ傫���A���W���󂯎��
	void setPos(const GameLib::Math::Vector3& position);
	void setScale(float scale);
	void setMap(int x, int y);
	//�p�l����ʂ̎�ނɒ���ւ���
	void setPanel(const GameLib::Scene::Container& container, Panel panel);
	//�������}�X�ɂ���
	void setPanelToHole();
	void setFoot(Foot foot);
	//�����o�ϐ��̏���n���֐����낢��
	void getMap(int* reX, int* reY)const;
	Panel getPanel() const;
	Foot getFoot() const;
	const GameLib::Math::Vector3* getPosition() const;

	//���̑�
	//�N�ɓ��܂�Ă��邩���X�V����
	void updateFoot();
	//���~��㩂̎c��^�[�������炷�B�O�ɂȂ�����true��Ԃ�
	bool decStopCount();

	//�}�X�̃p�����[�^�֘A
	void resetMassParam();
	//change�Ƃ����������Ă���
	void changeMassParam(MassParamName select, int value);
	const MassParam* getMassParam() const;
private:
	int mXmap;		//�����W
	int mYmap;		//�c���W
	int mCount;
	int mStopCount;	//���~��㩂̎c��^�[����
	Panel mPanel;
	Foot mFoot;
	MassParam mMassParam;
	GameLib::Scene::Model mModel;//���f��
	GameLib::Math::Vector3 mPosition;//�ʒu
	GameLib::Math::Vector3 mScale;	//�傫��

	//�萔�Q�@�t�@�C������ǂ݂���
	int TRAP_COUNT_STOP;		//���~��㩂̃^�[����
};