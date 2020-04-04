#include "GameLib/GameLib.h"
#include "GameLib/Scene/Container.h"
#include "Effect.h"

using namespace GameLib;
using namespace GameLib::Scene;
using namespace GameLib::Math;

Effect::Effect() :
	mPosition(0.f, 0.f, 0.f),
	mScale(0.f, 0.f, 0.f),
	mAngleY(0.f),
	mLifeTime(0.f)
{

}

Effect::~Effect() {
	mModel.release();
}

void Effect::update()
{
	--mLifeTime;
}

void Effect::draw()
{
	mModel.setScale(mScale);
	mModel.setAngle(Vector3(0.f, mAngleY, 0.f));
	//地面に描くエフェクトはちょっと浮かせる
	Vector3 position(mPosition.x, mPosition.y + 0.1f, mPosition.z);
	mModel.setPosition(position);
	mModel.draw();
}

void Effect::draw(const Vector3& pos, const Vector3& angle, const Vector3& scale)
{
	mModel.setScale(scale);
	mModel.setAngle(angle);
	//地面に描くエフェクトはちょっと浮かせる
	Vector3 position(pos.x, pos.y + 0.1f, pos.z);
	mModel.setPosition(position);
	mModel.draw();
}

void Effect::create(const Container& c, const char* name, const Vector3& pos, const Vector3& scale, float lifetime)
{
	//モデルをセット
	set(c, name);
	//初期値設定
	mPosition = pos;
	mScale = scale;
	mLifeTime = lifetime;
}

void Effect::set(const Container& c, const char* name)
{
	//タイプごとにモデルの生成
	mModel = c.createModel(name);
}

bool Effect::isLiving()
{
	return mLifeTime > 0.f;
}

void Effect::setBillBoard()
{
	mBillBoard = true;
}

bool Effect::billBoard()
{
	return mBillBoard;
}

void Effect::setEyeAngle(const float angle)
{
	mAngleY = angle;
}