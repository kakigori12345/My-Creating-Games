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
	//対象の位置・角度からViewMattを計算する。カメラの位置と視点を決めているだけ
	void setViewMatt(
		Mode mode,							//カメラモード選択
		GameLib::Math::Vector3* playerPos,	//プレイヤーの位置
		GameLib::Math::Vector3* enemyPos,	//敵の位置
		float eyeAngleYofPlayer,			//プレイヤーの視点
		float time							//特別なカメラの動きのために、その動きの中でどの時点かを渡してもらう(0～１)
	);
	//カメラアングルを渡してあげる
	float getCameraAngle() const;
private:
	GameLib::Math::Matrix34 mViewMatt;		//ビュー変換行列を持っておく
	GameLib::Math::Vector3 mEyePosition;	//カメラの位置
	GameLib::Math::Vector3 mTargetPosition;	//注視点の位置
	Mode mMode;	//現在のカメラモード

	//このクラスのみで使う関数
	// なめらかなカメラ移動を実現
	void naturalEyeChange(GameLib::Math::Vector3& eyePos);	//視点
	void naturalTargetChange(GameLib::Math::Vector3& targetPos);//注視点

	//定数群　ファイルから読み出す
	float ANGLE_VIEW;			//画角
	float NEAR_CLIP;			//ニア―クリップ
	float FAR_CLIP;				//ファークリップ
	//カメラワーク用定数
	//テスト用
	float MODE_TEST_X;
	float MODE_TEST_Y;
	float MODE_TEST_Z;
	//一人称
	float MODE_NORMAL_TARGET_DISTANCE_Z;
	float MODE_NORMAL_CAMERA_DISTANCE_Z;
	float MODE_NORMAL_CAMERA_DISTANCE_Y;
	float MODE_NORMAL_CAMERA_SPEED_NORMAL;
	float MODE_NORMAL_TARGET_SPEED_NORMAL;
};