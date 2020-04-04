#pragma once


#include "GameLib/Scene/Container.h"
#include "Effect.h"

/*
エフェクトを管理するクラス
中にEffectクラスのポインタを保持しておく
*/
class EffectController {
public:
	enum Type {
		TITLE,			//タイトル画面
		GAME_CLEAR,		//クリア画面
		GAME_OVER,		//ゲームオーバ画面
		EXPLOSION,		//爆発
		MAGIC_ATTACK,	//魔法攻撃
		MAGIC_TARGET,	//魔法攻撃時の選択位置
		NOTHING
	};
	static EffectController* instance();
	static void create();
	static void destroy();

	void draw();
	//エフェクトの種類と座標を受け取って生成する。一定時間表示して勝手に消える
	void createEffect(Type type, const GameLib::Math::Vector3& position, bool isBillBoard);
	//特定のエフェクトを表示する。この関数が呼ばれている間だけ有効。patternPhaseはエフェクトの演出に関するもの(1～)
	void show(Type type, const GameLib::Math::Vector3& position, const GameLib::Math::Vector3& angle, int patternPhase);
	void setEyeAngle(const float angle);//カメラから視点方向を受け取
	float getEyeAngle() const;
	//諸々の変数をリセットする
	void resetAllParameter();
private:
	EffectController();
	~EffectController();
	static EffectController* mInstance;

	GameLib::Scene::Container mContainer;	//各種エフェクトのモデルが格納されたコンテナ
	Effect** mEffects;	//内部にEffectクラスを保持しておく。こっちは一定時間で勝手に消える

	//勝手に消えないエフェクト。show()関数が呼ばれるフレームでのみ表示される。最大１つ
	Effect* mOneEffect;		//勝手に消えない。show()を呼ぶ間表示し続ける
	Type mNowOneEffect;		//mOneEffectが今持っているエフェクトがどれか保存
	bool mOneEffectIsOn;	//mOneEffectが有効かどうか。毎フレームOFFにする。show()が呼ばれたらONになる
	float mEyeAngle;	//ビルボード用視点方向(Y軸のみ)
	GameLib::Math::Vector3 mOneEffectPosition;
	GameLib::Math::Vector3 mOneEffectAngle;
	GameLib::Math::Vector3 mOneEffectScale;

	//その他便利に使う変数。resetAllParameter()でリセットされる
	float mIncAngleOfMagicAttack;	//魔法攻撃は回転させたい。毎フレームごとに値を増やしていくことで回転させる
	float mIncScaleOfMagicAttack;	//魔法攻撃の最後で外に向かってはじけるような演出にしたい
	float mIncPositionOfMagicAttack;//魔法攻撃の位置を調整

	//定数群　ファイルから読み出す
	int MAX_EFFECT_NUM;			//一度に保持できるエフェクトの最大数
	float LIFETIME_EXPLOSION;	//EXPLOSIONの生存時間
	float SCALE_EXPLOSION;		//EXPLOSIONのスケール
	float SCALE_MAGIC_ATTACK;	//MAGIC_ATTACKのスケール
	float INC_INTERVAL_ANGLE_MAGIC;		//魔法攻撃の演出：回転
	float INC_INTERVAL_SCALE_MAGIC;		//		;;		：大きさ
	float INC_INTERVAL_POSITION_MAGIC;	//		;;		：位置
	float SCALE_MAGIC_TARGET;	//MAGIC_TARGETのスケール
};