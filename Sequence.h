#pragma once

#include "Camera.h"

namespace GameLib {
	namespace Math {
		class Matrix34;
		class Vector3;
	}
}

//�V�[���̊��N���X�i���ۃN���X�j
class Sequence {
public:
	virtual ~Sequence() {}
	//���t���[���̏���
	virtual void update() = 0;
	//�`��
	virtual void draw() = 0;
	//�Ώۂ̈ʒu�E�p�x�Ȃǂ��J�����ɓn���悤
	virtual void sendInfoToCamera(
		Camera::Mode* mode,
		GameLib::Math::Vector3* playerPos,
		GameLib::Math::Vector3* enemyPos,
		float* eyeAngleYofPlayer,
		float* time
	) const = 0;
	//�I���������ǂ���
	virtual bool isFinished() const = 0;
	//���̃V�[���𐶐�
	virtual Sequence* createNextSequence() = 0;
};