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
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
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
	SAFE_DELETE(cr);//使い終わったら削除
}

Camera::~Camera()
{

}

void Camera::update() const
{
	//透視変換行列を作成する
	Matrix44 pvm;
	pvm.setPerspectiveTransform(
		ANGLE_VIEW,												//画角
		static_cast<float>(Framework::instance().width()),		//画面幅
		static_cast<float>(Framework::instance().height()),		//画面高さ
		NEAR_CLIP,												//ニア―クリップ
		FAR_CLIP												//ファークリップ
	);
	pvm *= mViewMatt;
	//作成した行列をセット。その他グラフィック関連の細かい設定
	Graphics::Manager gm = Graphics::Manager::instance();
	gm.setProjectionViewMatrix(pvm);
	gm.setEyePosition(mEyePosition);
	gm.setLightingMode(Graphics::LIGHTING_PER_PIXEL);
	gm.setLightColor(0, Vector3(0.7f, 0.4f, 0.2f));
	gm.enableDepthTest(true);
	gm.enableDepthWrite(true);
}

void Camera::setViewMatt(
	Mode mode,							//カメラモード選択
	GameLib::Math::Vector3* playerPos,	//プレイヤーの位置
	GameLib::Math::Vector3* enemyPos,	//敵の位置
	float eyeAngleYofPlayer,			//プレイヤーの視点
	float time)							//(0.f～１.f))
{
	//モードの更新
	mMode = mode;

	switch (mMode) {
	//ゲーム中の通常状態：プレイヤーの背後にカメラを設置
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
			Vector3 t(0.f, 1.f, 0.f);	//注視点
			Vector3 p(5.f, 3.f, -500.f);//視点の位置
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
		Vector3 t;	//注視点
		Vector3 p;	//視点の位置
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



/////////このクラス内でしか使わない関数/////////

//カメラが一瞬で移動するのを防ぐときに使用。1フレームでの最大移動可能距離を定義する
void Camera::naturalEyeChange(Vector3& eyePos)
{
	Vector3 dir;
	dir.setSub(eyePos, mEyePosition);
	//処理しやすくするために符号を取り出す。重要でない
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
	//カメラモードごとにカメラの最大移動可能距離を定義
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
	//最高速度より大きいなら最高速度に設定して制限をかける
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

	//最初に取り出した符号を戻すだけ
	dir.x *= x;
	dir.y *= y;
	dir.z *= z;

	mEyePosition.setAdd(mEyePosition, dir);
}

void Camera::naturalTargetChange(Vector3& eyePos)
{
	Vector3 dir;
	//注視点
	dir.setSub(eyePos, mTargetPosition);
	//処理しやすくするために符号を取り出す
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
	//最高速度より大きいなら最高速度に設定
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