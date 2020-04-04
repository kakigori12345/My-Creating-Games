#pragma once

#include "Sequence.h"
#include "Stage.h"
#include "PlayCharacter.h"
#include "MonsterController.h"

namespace GameLib {
	namespace Math {
		class Vector3;
		class Matrix34;
		class Matrix44;
	}
}

//ゲームクラス
//ゲーム内のオブジェクト・処理の管理をこのクラスが行う

class Game : public Sequence{
public:
	//ゲームの状態
	enum Situation {
		NORMAL,				//通常
		MENU,				//行動選択
		MAKE_FRIEND,		//敵を仲間にするかどうか選択
		TURN_LIMIT,			//ターン制限に引っかかった
		SHOW_STAGE_NUM,		//ステージの階層を表示中。一定時間で次に進む
		ANNOUNCEMENT,		//何かを表示する
		VICTORY,			//ゲームクリア
		LOSE,				//ゲームオーバ

		INVALID
	};

	Game();
	~Game();
	virtual void update();
	virtual void draw();
	//カメラ関数に情報を伝えていい感じに演出してもらう。やり取りはmain.cpp内で行われる
	virtual void sendInfoToCamera(
		Camera::Mode* mode,					//カメラのモードを選択する
		GameLib::Math::Vector3* playerPos,	//プレイヤーの位置座標
		GameLib::Math::Vector3* enemyPos,	//特定キャラクター（敵など）の位置座標
		float* eyeAngleYofPlayer,			//プレイヤーの視線方向
		float* time							//タイミング時間( 0.f ～ 1.f )
	) const;
	//ゲームが終了したことかどうか知る
	virtual bool isFinished() const;
	virtual Sequence* createNextSequence();
	//外部に情報を渡す関数
	Situation getSituation();
private:
	//このクラス内でしか使わない関数。主に処理の長すぎるものを別にまとめただけ
	void menu();
	//アクティブキャラクターのチェンジ管理
	void checkActiveCharacterChange();
	//プレイヤー以外生成しなおし、ステージの階層を進める
	void createNewStage();
	//プレイヤーが階層を進めるかどうか確認。そうなら次のステージへ
	bool playerOnTheGoal();
	//階層を進める処理。最終階層ならクリア
	void gotoNextStage();
	//戦闘処理。敵が仲間になったり、対象が複数だったり複雑なので、別に移設
	void battle(bool& enemyWantToBeFriend);
	//戦闘以外でのダメージ処理
	void setDamage(bool passiveIsPlayer, int id);
	//モンスターを殺すときに呼び出す処理をまとめたもの
	void killMonster(int targetID, bool partsWillBeGot);

	//フラグ管理
	bool battleIsDone;			//攻撃が行われているかどうか
	bool attackIsMagic;			//遠距離攻撃かどうか
	bool mFinished;				//ゲームの終了
	bool createNextStageFlag;	//新しいステージを生成するときにtrueに

	//その他、便利に使うための変数
	bool victory;			//終了した時点で、ゲームクリアなのか
	Situation mSituation;	//現在のシチュエーションを保存
	Situation mPreSituation;//一個前のシチュエーションを保存
	float situationCount;	//0.f～1.fの間でタイミングを保存
	int StageNumber;		//現在どのステージ（階層）かを保持する。この数をStageクラスに渡して該当ステージを生成してもらう
	int playerID;			//プレイキャラクターのIDを保持しておく
	int messageNum;			//複数のメッセージを一場面で表示するときに、現在何枚目か
	int IDofFriendly;		//仲間になりたそうなモンスターのID
	bool TrapHoleIsDone;	//落とし穴が発動したかどうか。アクティブキャラクターを変更後、
							//直前にアクティブキャラクターだったもの（落とし穴にはまったもの）を殺す
	int IDofFalled;			//落とし穴にはまったモンスターのID
	bool TrapCatchIsDone;	//捕獲罠が発動したかどうか。同じくアクティブキャラクターを変更後、
							//捕獲罠にはまったモンスターを仲間にできるかどうかを判定して、できるなら仲間に
	int IDofCatched;		//捕獲罠にはまったモンスターのID
	bool TrapBombIsDone;	//地雷が発動したかどうか。同じくアクティブキャラクターを変更後（変更後でないとキャラ変更でバグる）、
							//地雷を踏んだモンスターにダメージを与える。死んだらそのまま殺す
	int IDofBombed;			//地雷を踏んだモンスターのID
	Parts nowGotParts;		//プレイヤーがゲットするパーツ。表示用

	//メンバとして保持しておく他クラスのポインタ
	Character* mActiveCharacter;	//現在、焦点が当たっている（ターンが回ってきた）キャラクターのポインタを保持する
	Character* mPassiveCharacter;	//アクティブキャラクターからの干渉により、何かしら動作させる受け身のキャラクターのポインタを保持する
	Stage* mStage;					//ステージ
	PlayCharacter* mPC;				//プレイキャラクター
	MonsterController* mMonCon;		//モンスターコントローラー

	//定数群　ファイルから読み出す
	int		FINAL_STAGE_NUMBER;		//最終ステージのナンバー
};