//////#pragma once


#include "GameLib/Math/Vector3.h"
#include "GameLib/Scene/Model.h"

/*
�Q�[�����ɕ\�������G�t�F�N�g���̂��́B���g�̏��͂��ׂ�
��ʁi�R���g���[���j�N���X�ł���EffectContoroller�N���X���Ǘ�����
*/

class Effect {
public:
	Effect();
	~Effect();
	void update();
	void draw();
	//�ʒu�A�p�x�A�X�P�[�����w�肵�ĕ`��
	void draw(
		const GameLib::Math::Vector3& pos,
		const GameLib::Math::Vector3& angle,
		const GameLib::Math::Vector3& scale);
	//�������ԂȂǕK�v�ȏ���1��^���āA���Ƃ͏���ɕ`�悵�Ă��炤���A����ɏ����Ă��炤
	void create(
		const GameLib::Scene::Container& container,
		const char* name,
		const GameLib::Math::Vector3& pos,
		const GameLib::Math::Vector3& scale,
		float lifetime);
	//�Ƃ肠�������f����񂾂������Ă���
	void set(const GameLib::Scene::Container& container, const char* name);
	bool isLiving();
	//�U�r���{�[�h�B�J�����̎��_�����ɂ������ď�ɐ��ʂ���������
	void setBillBoard();	//�r���{�[�h���[�hON
	bool billBoard();	//�r���{�[�h���ǂ���
	void setEyeAngle(const float angle); //�r���{�[�h�̂��߂Ɏ��_�����̃Z�b�g
private:
	GameLib::Scene::Model mModel;
	GameLib::Math::Vector3 mPosition;
	GameLib::Math::Vector3 mScale;
	float mAngleY;
	float mLifeTime;	//��������
	bool mBillBoard;	//�r���{�[�h����
};