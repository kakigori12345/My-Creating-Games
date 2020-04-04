#pragma once

#include "GameLib/Math/Vector3.h"

//Character�N���X�̓���i�A�N�V�����j�𐧌䂷��N���X
//�������ɐ��䂷��Ώۂ̃|�C���^���󂯎��
//Character����Ăяo����Ďg����Ƃ��ɂ́A
//�Ώۂ̈ʒu�EAction�ԍ��E�^�C�~���O�i0.f�`1.f�j��n��

class Action {
public:
	//�C���X�^���X���擾�E�����E�폜
	static Action* instance();
	static void create();
	static void destroy();
	//����B�l�X��Character�N���X����Ăяo�����
	void update(
		const GameLib::Math::Vector3& target,	//�ړI�̈ʒu
		GameLib::Math::Vector3& position,		//�s����̈ʒu
		const GameLib::Math::Vector3& prePosition,	//�s����̈ړ��O�ʒu
		Character::Act act,						//�s���̎��
		bool& attackTiming,						//�U�������������u�Ԃ�true�ɂ���
		float time);							//�^�C�~���O 0�`1.f
private:
	Action();
	~Action();
	static Action* mInstance;

	//�萔�Q �t�@�C������ǂݏo��
	float ACT_WARP_HEIGHT;	//���[�v���̍���
	float ACT_ATTACK_NORMAL_TIME_1;
	float ACT_ATTACK_NORMAL_TIME_2;
};