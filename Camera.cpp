#include "GameLib/Framework.h"
#include "GameLib/Graphics/Manager.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Functions.h"
#include "Camera.h"
#include "ContentReader.h"
#include "Button.h" //test
#include "EffectController.h"
using namespace GameLib;
using namespace GameLib::Math;

Camera::Camera():
	mEyePosition(40.f, 60.f, 0.f),
	mTargetPosition(20.f, 0.f, 20.f),
	mMode(MODE_GAME_NORMAL)
{
	//�萔�̓ǂݍ���
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	Camera::ANGLE_VIEW			= cr->getData("Camera", "ANGLE_VIEW");
	Camera::NEAR_CLIP			= cr->getData("Camera", "NEAR_CLIP");
	Camera::FAR_CLIP			= cr->getData("Camera", "FAR_CLIP");
	Camera::MODE_TEST_X			= cr->getData("Camera", "MODE_TEST_X");
	Camera::MODE_TEST_Y			= cr->getData("Camera", "MODE_TEST_Y");
	Camera::MODE_TEST_Z			= cr->getData("Camera", "MODE_TEST_Z");
	Camera::MODE_NORMAL_TARGET_DISTANCE_Z	= cr->getData("Camera", "MODE_NORMAL_TARGET_DISTANCE_Z");
	Camera::MODE_NORMAL_CAMERA_DISTANCE_Z	= cr->getData("Camera", "MODE_NORMAL_CAMERA_DISTANCE_Z");
	Camera::MODE_NORMAL_CAMERA_DISTANCE_Y	= cr->getData("Camera", "MODE_NORMAL_CAMERA_DISTANCE_Y");
	Camera::MODE_NORMAL_CAMERA_SPEED_NORMAL = cr->getData("Camera", "MODE_NORMAL_CAMERA_SPEED");
	Camera::MODE_NORMAL_TARGET_SPEED_NORMAL = cr->getData("Camera", "MODE_NORMAL_TARGET_SPEED");
	SAFE_DELETE(cr);//�g���I�������폜
}

Camera::~Camera()
{

}

void Camera::update() const
{
	//�����ϊ��s����쐬����
	Matrix44 pvm;
	pvm.setPerspectiveTransform(
		ANGLE_VIEW,												//��p
		static_cast<float>(Framework::instance().width()),		//��ʕ�
		static_cast<float>(Framework::instance().height()),		//��ʍ���
		NEAR_CLIP,												//�j�A�\�N���b�v
		FAR_CLIP												//�t�@�[�N���b�v
	);
	pvm *= mViewMatt;
	//�쐬�����s����Z�b�g�B���̑��O���t�B�b�N�֘A�ׂ̍����ݒ�
	Graphics::Manager gm = Graphics::Manager::instance();
	gm.setProjectionViewMatrix(pvm);
	gm.setEyePosition(mEyePosition);
	gm.setLightingMode(Graphics::LIGHTING_PER_PIXEL);
	gm.setLightColor(0, Vector3(0.7f, 0.4f, 0.2f));
	gm.enableDepthTest(true);
	gm.enableDepthWrite(true);
}

void Camera::setViewMatt(
	Mode mode,							//�J�������[�h�I��
	GameLib::Math::Vector3* playerPos,	//�v���C���[�̈ʒu
	GameLib::Math::Vector3* enemyPos,	//�G�̈ʒu
	float eyeAngleYofPlayer,			//�v���C���[�̎��_
	float time)							//(0.f�`�P.f))
{
	//���[�h�̍X�V
	mMode = mode;

	switch (mMode) {
	//�Q�[�����̒ʏ��ԁF�v���C���[�̔w��ɃJ������ݒu
	case TEST: {
		/*
		Button* button = Button::instance();
		if (button->isOn(Button::KEY_W))
			mEyePosition.z += 3.f;
		if (button->isOn(Button::KEY_S))
			mEyePosition.z -= 3.f;
		if (button->isOn(Button::KEY_A))
			mEyePosition.x += 3.f;
		if (button->isOn(Button::KEY_D))
			mEyePosition.x -= 3.f;

		mEyePosition.set(MODE_TEST_X, MODE_TEST_Y, MODE_TEST_Z);
		mTargetPosition.set(20.f, 0.f, 50.f);
		*/

		Vector3 ad(MODE_TEST_X, MODE_TEST_Y, MODE_TEST_Z);
		mEyePosition.setAdd(*playerPos, ad);
		mTargetPosition = *playerPos;
		mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));

		break;
	}
	case MODE_TITLE: {
		if (time < 1.f) {
			Vector3 t(0.f, 1.f, 0.f);	//�����_
			Vector3 p(5.f, 3.f, -500.f);//���_�̈ʒu
			p.z += 506.f * time;
			mEyePosition = p;
			mTargetPosition = t;
		}
		else {
			Vector3 menuPos(0.14f, -0.9f, 1.1f);
			Matrix34 menuMatt;
			menuMatt.setRotationY(getCameraAngle());
			menuMatt.mul(&menuPos, menuPos);
			Vector3 tmp;
			tmp.setMul(menuPos, 2.f);
			tmp += mEyePosition;
			EffectController::instance()->show(EffectController::TITLE, tmp, Vector3(0.f, getCameraAngle(), 0.f), 1);
		}
		mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));
		break;
	}
	case MODE_GAME_NORMAL: {
		Vector3 d(0.f, 0.f, 1.f);
		Matrix34 m;
		m.setRotationY(eyeAngleYofPlayer);
		m.mul(&d, d);
		Vector3 t;	//�����_
		Vector3 p;	//���_�̈ʒu
		t.setMul(d, MODE_NORMAL_TARGET_DISTANCE_Z);
		p.setMul(d, -MODE_NORMAL_CAMERA_DISTANCE_Z);
		p.y += MODE_NORMAL_CAMERA_DISTANCE_Y;
		t += *playerPos;
		p += *playerPos;
		naturalEyeChange(p);
		naturalTargetChange(t);
		mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));
		break;
	}
	case MODE_GAME_CLEAR: {
		EffectController::instance()->show(EffectController::GAME_CLEAR, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), 1);
		mTargetPosition.set(0.f, 0.6f, 0.f);
		mEyePosition.set(1.3f * Math::sin(180.f * time), 0.6f, 1.3f * Math::cos(180.f * time));
		mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));
		break;
	}
	case MODE_GAME_OVER: {
		EffectController::instance()->show(EffectController::GAME_OVER, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), 1);
		mTargetPosition.set(0.f, 0.6f, 0.f);
		mEyePosition.set(1.3f * Math::sin(180.f * time), 0.6f, 1.3f * Math::cos(180.f * time));
		mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));
		break;
	}
	default: {
		HALT("FILE:Camera.cpp [setViewMatt] switch Error");
		break;
	}
	}
	mViewMatt.setViewTransform(mEyePosition, mTargetPosition, Vector3(0.f, 1.f, 0.f));
}


float Camera::getCameraAngle() const
{
	Vector3 dir;
	dir.setSub(mTargetPosition, mEyePosition);
	float angle = GameLib::Math::atan2(dir.x, dir.z);

	return angle;
}



/////////���̃N���X���ł����g��Ȃ��֐�/////////

//�J��������u�ňړ�����̂�h���Ƃ��Ɏg�p�B1�t���[���ł̍ő�ړ��\�������`����
void Camera::naturalEyeChange(Vector3& eyePos)
{
	Vector3 dir;
	dir.setSub(eyePos, mEyePosition);
	//�������₷�����邽�߂ɕ��������o���B�d�v�łȂ�
	int x, y, z;
	if (dir.x >= 0) {
		x = 1;
	}
	else {
		dir.x *= -1;
		x = -1;
	}
	if (dir.y >= 0) {
		y = 1;
	}
	else {
		dir.y *= -1;
		y = -1;
	}
	if (dir.z >= 0) {
		z = 1;
	}
	else {
		dir.z *= -1;
		z = -1;
	}
	//�J�������[�h���ƂɃJ�����̍ő�ړ��\�������`
	float maxSpeed;
	switch (mMode) {
	case MODE_GAME_NORMAL:
		maxSpeed = MODE_NORMAL_CAMERA_SPEED_NORMAL;
		break;
	default:
		HALT("FILE:Camera.cpp [naturalEyeChange] Error");
		break;
	}

	//x
	//�ō����x���傫���Ȃ�ō����x�ɐݒ肵�Đ�����������
	if (dir.x > maxSpeed) {
		dir.x = maxSpeed;
	}
	//y
	if (dir.y > maxSpeed) {
		dir.y = maxSpeed;
	}
	//z
	if (dir.z > maxSpeed) {
		dir.z = maxSpeed;
	}

	//�ŏ��Ɏ��o����������߂�����
	dir.x *= x;
	dir.y *= y;
	dir.z *= z;

	mEyePosition.setAdd(mEyePosition, dir);
}

void Camera::naturalTargetChange(Vector3& eyePos)
{
	Vector3 dir;
	//�����_
	dir.setSub(eyePos, mTargetPosition);
	//�������₷�����邽�߂ɕ��������o��
	int x, y, z;
	if (dir.x >= 0) {
		x = 1;
	}
	else {
		dir.x *= -1;
		x = -1;
	}
	if (dir.y >= 0) {
		y = 1;
	}
	else {
		dir.y *= -1;
		y = -1;
	}
	if (dir.z >= 0) {
		z = 1;
	}
	else {
		dir.z *= -1;
		z = -1;
	}

	float maxSpeed;
	switch (mMode) {
	case MODE_GAME_NORMAL:
		maxSpeed = MODE_NORMAL_TARGET_SPEED_NORMAL;
		break;
	default:
		HALT("FILE:Cameracpp [naturalTargetChange] ERROR");
		break;
	}

	//x
	//�ō����x���傫���Ȃ�ō����x�ɐݒ�
	if (dir.x > maxSpeed) {
		dir.x = maxSpeed;
	}
	//y
	if (dir.y > maxSpeed) {
		dir.y = maxSpeed;
	}
	//z
	if (dir.z > maxSpeed) {
		dir.z = maxSpeed;
	}

	dir.x *= x;
	dir.y *= y;
	dir.z *= z;

	mTargetPosition.setAdd(mTargetPosition, dir);
}