//////#pragma once


#include "GameLib/Math/Vector3.h"
#include "GameLib/Scene/Model.h"

/*
ゲーム中に表示されるエフェクトそのもの。中身の情報はすべて
上位（コントローラ）クラスであるEffectContorollerクラスが管理する
*/

class Effect {
public:
	Effect();
	~Effect();
	void update();
	void draw();
	//位置、角度、スケールを指定して描画
	void draw(
		const GameLib::Math::Vector3& pos,
		const GameLib::Math::Vector3& angle,
		const GameLib::Math::Vector3& scale);
	//生存時間など必要な情報を1回与えて、あとは勝手に描画してもらうし、勝手に消えてもらう
	void create(
		const GameLib::Scene::Container& container,
		const char* name,
		const GameLib::Math::Vector3& pos,
		const GameLib::Math::Vector3& scale,
		float lifetime);
	//とりあえずモデル情報だけ持っておく
	void set(const GameLib::Scene::Container& container, const char* name);
	bool isLiving();
	//偽ビルボード。カメラの視点方向にたいして常に正面を向くだけ
	void setBillBoard();	//ビルボードモードON
	bool billBoard();	//ビルボードかどうか
	void setEyeAngle(const float angle); //ビルボードのために視点方向のセット
private:
	GameLib::Scene::Model mModel;
	GameLib::Math::Vector3 mPosition;
	GameLib::Math::Vector3 mScale;
	float mAngleY;
	float mLifeTime;	//生存時間
	bool mBillBoard;	//ビルボード判定
};