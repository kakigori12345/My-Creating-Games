#pragma once

#include "GameLib/Math/Vector3.h"

//Characterクラスの動作（アクション）を制御するクラス
//生成時に制御する対象のポインタを受け取る
//Characterから呼び出されて使われるときには、
//対象の位置・Action番号・タイミング（0.f～1.f）を渡す

class Action {
public:
	//インスタンスを取得・生成・削除
	static Action* instance();
	static void create();
	static void destroy();
	//動作。様々なCharacterクラスから呼び出される
	void update(
		const GameLib::Math::Vector3& target,	//目的の位置
		GameLib::Math::Vector3& position,		//行動主の位置
		const GameLib::Math::Vector3& prePosition,	//行動主の移動前位置
		Character::Act act,						//行動の種類
		bool& attackTiming,						//攻撃が完了した瞬間にtrueにする
		float time);							//タイミング 0～1.f
private:
	Action();
	~Action();
	static Action* mInstance;

	//定数群 ファイルから読み出す
	float ACT_WARP_HEIGHT;	//ワープ時の高さ
	float ACT_ATTACK_NORMAL_TIME_1;
	float ACT_ATTACK_NORMAL_TIME_2;
};