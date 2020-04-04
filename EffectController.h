#pragma once


#include "GameLib/Scene/Container.h"
#include "Effect.h"

/*
�G�t�F�N�g���Ǘ�����N���X
����Effect�N���X�̃|�C���^��ێ����Ă���
*/
class EffectController {
public:
	enum Type {
		TITLE,			//�^�C�g�����
		GAME_CLEAR,		//�N���A���
		GAME_OVER,		//�Q�[���I�[�o���
		EXPLOSION,		//����
		MAGIC_ATTACK,	//���@�U��
		MAGIC_TARGET,	//���@�U�����̑I���ʒu
		NOTHING
	};
	static EffectController* instance();
	static void create();
	static void destroy();

	void draw();
	//�G�t�F�N�g�̎�ނƍ��W���󂯎���Đ�������B��莞�ԕ\�����ď���ɏ�����
	void createEffect(Type type, const GameLib::Math::Vector3& position, bool isBillBoard);
	//����̃G�t�F�N�g��\������B���̊֐����Ă΂�Ă���Ԃ����L���BpatternPhase�̓G�t�F�N�g�̉��o�Ɋւ������(1�`)
	void show(Type type, const GameLib::Math::Vector3& position, const GameLib::Math::Vector3& angle, int patternPhase);
	void setEyeAngle(const float angle);//�J�������王�_�������󂯎�
	float getEyeAngle() const;
	//���X�̕ϐ������Z�b�g����
	void resetAllParameter();
private:
	EffectController();
	~EffectController();
	static EffectController* mInstance;

	GameLib::Scene::Container mContainer;	//�e��G�t�F�N�g�̃��f�����i�[���ꂽ�R���e�i
	Effect** mEffects;	//������Effect�N���X��ێ����Ă����B�������͈�莞�Ԃŏ���ɏ�����

	//����ɏ����Ȃ��G�t�F�N�g�Bshow()�֐����Ă΂��t���[���ł̂ݕ\�������B�ő�P��
	Effect* mOneEffect;		//����ɏ����Ȃ��Bshow()���ĂԊԕ\����������
	Type mNowOneEffect;		//mOneEffect���������Ă���G�t�F�N�g���ǂꂩ�ۑ�
	bool mOneEffectIsOn;	//mOneEffect���L�����ǂ����B���t���[��OFF�ɂ���Bshow()���Ă΂ꂽ��ON�ɂȂ�
	float mEyeAngle;	//�r���{�[�h�p���_����(Y���̂�)
	GameLib::Math::Vector3 mOneEffectPosition;
	GameLib::Math::Vector3 mOneEffectAngle;
	GameLib::Math::Vector3 mOneEffectScale;

	//���̑��֗��Ɏg���ϐ��BresetAllParameter()�Ń��Z�b�g�����
	float mIncAngleOfMagicAttack;	//���@�U���͉�]���������B���t���[�����Ƃɒl�𑝂₵�Ă������Ƃŉ�]������
	float mIncScaleOfMagicAttack;	//���@�U���̍Ō�ŊO�Ɍ������Ă͂�����悤�ȉ��o�ɂ�����
	float mIncPositionOfMagicAttack;//���@�U���̈ʒu�𒲐�

	//�萔�Q�@�t�@�C������ǂݏo��
	int MAX_EFFECT_NUM;			//��x�ɕێ��ł���G�t�F�N�g�̍ő吔
	float LIFETIME_EXPLOSION;	//EXPLOSION�̐�������
	float SCALE_EXPLOSION;		//EXPLOSION�̃X�P�[��
	float SCALE_MAGIC_ATTACK;	//MAGIC_ATTACK�̃X�P�[��
	float INC_INTERVAL_ANGLE_MAGIC;		//���@�U���̉��o�F��]
	float INC_INTERVAL_SCALE_MAGIC;		//		;;		�F�傫��
	float INC_INTERVAL_POSITION_MAGIC;	//		;;		�F�ʒu
	float SCALE_MAGIC_TARGET;	//MAGIC_TARGET�̃X�P�[��
};