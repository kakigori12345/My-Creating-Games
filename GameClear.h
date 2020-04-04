#pragma once

#include "Sequence.h"

namespace GameLib {
	namespace Math {
		class Matrix34;
		class Vector3;
	}
}

//�Q�[���I�[�o��ʂ̃N���X
//Sequence���p��
//���ɐ�������V�[�P���X��Title�N���X
class GameClear : public Sequence {
public:
	GameClear();
	~GameClear();
	virtual void update();
	virtual void draw();
	virtual void sendInfoToCamera(
		Camera::Mode* mode,
		GameLib::Math::Vector3* playerPos,
		GameLib::Math::Vector3* enemyPos,
		float* eyeAngleYofPlayer,
		float* time
	) const;
	virtual bool isFinished() const;
	virtual Sequence* createNextSequence();
private:
	bool mFinished;
	float mCount;
};