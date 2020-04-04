#pragma once

#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"

class Mass {
public:
	//パネルの種類。TRAPはMonsterにしか掛からない
	enum Panel {
		NORMAL,			//通常		；なにもない
		CORRIDOR,		//通路		：なにもないが、罠も置けないし穴あきにもならない
		HOLE,			//穴あき	：通れない
		TRAP_HOLE,		//落とし穴	：踏んだら死ぬ。その後そのマスはHOLEになる
		TRAP_CATCH,		//捕獲網	：捕まえる
		TRAP_STOPPER,	//足止め	：止まる
		TRAP_BOMB,		//地雷		：爆発してダメージを与える
		WARP,			//ワープ　　：別のワープマスに飛ぶ(ランダム)
		GOAL,			//階段		：次のステージへ
		RECOVERY,		//回復		：回復できる
		PARTS,			//パーツ	：パーツ拾える
		INVALID
	};
	//どちらのチームに踏まれているか。踏まれているとき色が変わる
	enum Foot {
		FOOT_PLAYER,
		FOOT_ENEMY,
		NONE
	};
	//マスにパラメータ付与。敵AIが行動するときに周りの状況を把握するために
	struct MassParam {
		int TeamPlayer;		//プレイヤーチームが近くにいる程度を示す
		int TeamEnemy;		//エネミーチーム
		//コンストラクタ。とりあえず全部0で初期化
		MassParam() : TeamPlayer(0), TeamEnemy(0) {};
		//リセット用関数
		void reset() {
			TeamPlayer = 0;
			TeamEnemy = 0;
		}
	};
	//上のMassParamを変更する際に、関数に指定するための名前定義
	enum MassParamName {
		MP_TEAM_PLAYER,
		MP_TEAM_ENEMY
	};
	Mass();
	~Mass();
	void draw();
	//モデル情報の書かれたコンテナを受け取り、そこからモデルを作る
	void createModel(const GameLib::Scene::Container& container);
	//生成されたあとに位置情報と大きさ、座標を受け取る
	void setPos(const GameLib::Math::Vector3& position);
	void setScale(float scale);
	void setMap(int x, int y);
	//パネルを別の種類に張り替える
	void setPanel(const GameLib::Scene::Container& container, Panel panel);
	//穴あきマスにする
	void setPanelToHole();
	void setFoot(Foot foot);
	//メンバ変数の情報を渡す関数いろいろ
	void getMap(int* reX, int* reY)const;
	Panel getPanel() const;
	Foot getFoot() const;
	const GameLib::Math::Vector3* getPosition() const;

	//その他
	//誰に踏まれているかを更新する
	void updateFoot();
	//足止め罠の残りターンを減らす。０になったらtrueを返す
	bool decStopCount();

	//マスのパラメータ関連
	void resetMassParam();
	//changeというか足していく
	void changeMassParam(MassParamName select, int value);
	const MassParam* getMassParam() const;
private:
	int mXmap;		//横座標
	int mYmap;		//縦座標
	int mCount;
	int mStopCount;	//足止め罠の残りターン数
	Panel mPanel;
	Foot mFoot;
	MassParam mMassParam;
	GameLib::Scene::Model mModel;//モデル
	GameLib::Math::Vector3 mPosition;//位置
	GameLib::Math::Vector3 mScale;	//大きさ

	//定数群　ファイルから読みだす
	int TRAP_COUNT_STOP;		//足止め罠のターン数
};