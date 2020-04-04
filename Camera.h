#pragma once

#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix34.h"

class Camera {
public:
	enum Mode {
		TEST,
		MODE_TITLE,
		MODE_GAME_NORMAL,
		MODE_GAME_CLEAR,
		MODE_GAME_OVER,
		INVALID
	};
	Camera();
	~Camera();
	void update() const;
	//�Ώۂ̈ʒu�E�p�x����ViewMatt���v�Z����B�J�����̈ʒu�Ǝ��_�����߂Ă��邾��
	void setViewMatt(
		Mode mode,							//�J�������[�h�I��
		GameLib::Math::Vector3* playerPos,	//�v���C���[�̈ʒu
		GameLib::Math::Vector3* enemyPos,	//�G�̈ʒu
		float eyeAngleYofPlayer,			//�v���C���[�̎��_
		float time							//���ʂȃJ�����̓����̂��߂ɁA���̓����̒��łǂ̎��_����n���Ă��炤(0�`�P)
	);
	//�J�����A���O����n���Ă�����
	float getCameraAngle() const;
private:
	GameLib::Math::Matrix34 mViewMatt;		//�r���[�ϊ��s��������Ă���
	GameLib::Math::Vector3 mEyePosition;	//�J�����̈ʒu
	GameLib::Math::Vector3 mTargetPosition;	//�����_�̈ʒu
	Mode mMode;	//���݂̃J�������[�h

	//���̃N���X�݂̂Ŏg���֐�
	// �Ȃ߂炩�ȃJ�����ړ�������
	void naturalEyeChange(GameLib::Math::Vector3& eyePos);	//���_
	void naturalTargetChange(GameLib::Math::Vector3& targetPos);//�����_

	//�萔�Q�@�t�@�C������ǂݏo��
	float ANGLE_VIEW;			//��p
	float NEAR_CLIP;			//�j�A�\�N���b�v
	float FAR_CLIP;				//�t�@�[�N���b�v
	//�J�������[�N�p�萔
	//�e�X�g�p
	float MODE_TEST_X;
	float MODE_TEST_Y;
	float MODE_TEST_Z;
	//��l��
	float MODE_NORMAL_TARGET_DISTANCE_Z;
	float MODE_NORMAL_CAMERA_DISTANCE_Z;
	float MODE_NORMAL_CAMERA_DISTANCE_Y;
	float MODE_NORMAL_CAMERA_SPEED_NORMAL;
	float MODE_NORMAL_TARGET_SPEED_NORMAL;
};