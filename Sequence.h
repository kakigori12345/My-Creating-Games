#pragma once

#include "Camera.h"

namespace GameLib {
	namespace Math {
		class Matrix34;
		class Vector3;
	}
}

//シーンの基底クラス（抽象クラス）
class Sequence {
public:
	virtual ~Sequence() {}
	//毎フレームの処理
	virtual void update() = 0;
	//描画
	virtual void draw() = 0;
	//対象の位置・角度などをカメラに渡すよう
	virtual void sendInfoToCamera(
		Camera::Mode* mode,
		GameLib::Math::Vector3* playerPos,
		GameLib::Math::Vector3* enemyPos,
		float* eyeAngleYofPlayer,
		float* time
	) const = 0;
	//終了したかどうか
	virtual bool isFinished() const = 0;
	//次のシーンを生成
	virtual Sequence* createNextSequence() = 0;
};