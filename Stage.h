#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Math/Random.h"
#include "Mass.h"
#include <vector>

/*
Stageクラス
Massクラスを保持する。複数のマスからステージが構成され、それらマスを生成・管理する。
マップ上に存在するCharacterクラスのオブジェクトのIDとマップ座標を管理しておく。
これにより、攻撃の当たり判定などをこのクラス内で処理できるようにする。
マスの座標は、例えば4×4ならこんな感じ　（縦,横）
  --- --- --- ---
 |0,0|1,0|2,0|3,0|
  --- --- --- ---
 |0,1|1,1|2,1|3,1|
  --- --- --- ---
 |0,2|1,2|2,2|3,2|
  --- --- --- ---
 |0,3|1,3|2,3|3,3|
  --- --- --- ---
*/

class Stage {
public:
	//所属。NONEはCharacterの所属に関係ないアクションのときに使う
	enum Team {
		TEAM_PLAYER,	//プレイヤーチーム
		TEAM_ENEMY,		//敵チーム
		NONE
	};
	//Characterが干渉するマス。移動や攻撃の際に有効かどうかを判定するよう。実際の処理はGameクラスがやる
	enum TargetMass {
		TARGET_RIGHT,
		TARGET_LEFT,
		TARGET_UP,
		TARGET_DOWN,
		TARGET_SURROUNDINGS,	//周囲のマス全て
		TARGET_NONE
	};
	struct CombIDandMap {
		int ID;
		int Xmap;
		int Ymap;
		Team team;
		bool exist;
		CombIDandMap() :ID(-1), Xmap(-1), Ymap(-1), team(NONE), exist(false) {}
		void reset() { ID = -1; Xmap = -1; Ymap = -1; team = NONE; exist = false; }
	};
	//区画に関するデータ格納用。各区画ごとに、WIDTHの端2つ、HEIGHTの端2つの座標を持つ
	struct mapDataOfDivision {
		int ROOM_NUMBER;		//部屋番号
		int DISTANCE_TO_GOAL;	//ゴールまでの距離（部屋的に）
		int MapX;
		int MapY;
		int WIDTH_LEFT;
		int WIDTH_RIGHT;
		int HEIGHT_TOP;
		int HEIGHT_LOW;
		mapDataOfDivision() :
			ROOM_NUMBER(-1), DISTANCE_TO_GOAL(-1), MapX(-1), MapY(-1), 
			WIDTH_LEFT(-1), WIDTH_RIGHT(-1), HEIGHT_TOP(-1), HEIGHT_LOW(-1) {}
		~mapDataOfDivision() {}
		bool checkMapExistFromXY(int x, int y) const;
		void setDistanceToGoal(int distance);
		void setMap(int x, int y);
		void getXY(int* x, int* y);
		bool checkRoomNumber(int i , int j) const;
		int getDistanceToGoal() const;
	};
	//通路に関するデータ
	struct corridorData {
		int room1;
		int room2;
		int room1_MapX;
		int room1_MapY;
		int room2_MapX;
		int room2_MapY;
		corridorData() :
			room1(-1), room2(-1), room1_MapX(-1), room1_MapY(-1), room2_MapX(-1), room2_MapY(-1) {}
		corridorData(int room1_, int room2_, int room1_MapX_, int room1_MapY_, int room2_MapX_, int room2_MapY_) :
			room1(room1_), room2(room2_), room1_MapX(room1_MapX_), room1_MapY(room1_MapY_), room2_MapX(room2_MapX_), room2_MapY(room2_MapY_) {}
		~corridorData() {}
		//部屋2つを結ぶ通路かどうか
		bool getCorridor(int room1_, int room2_) const;
		//その座標が通路の上かどうか
		bool isOnCorridor(int x, int y) const;
		//部屋番号から通路端の座標を返す
		void getXYfromRoomNumber(int roomNumber, int*x, int* y) const;
		//部屋2つのインデックスを取得
		void getRoomNumbers(int* room1, int* room2);
	};
	Stage( int stageNumber );
	~Stage();
	void update();
	void draw() const;
	//新規IDを登録する。受け取ったVector3に結果を埋め込む。とりあえず初期位置はランダムで
	void registerNewID(int id, GameLib::Math::Vector3* position, Team team);
	//既存のIDを削除する（キャラクターが死んだときに呼び出す）
	void deleteID(int id);
	//プレイヤーのIDを登録
	void registerPlayerID(int id);
	//直前に踏まれたマスの更新
	void updatePrevFootMass();
	//全てのマスのパラメータを更新する。Game側で毎フレーム更新してもらう
	void updateMass();
	//マスのパラメータを更新する。とりあえず全部リセットして、計算しなおす
	void updateMassParameter();
	//キャラクターの所属チームを変更する（現在所属しているのと違うもう片方のチームへ）
	void changeTeam(int id);
	//ID(座標)からマスを特定し、そのマスを指定のパネルに変更
	void changePanel(int id, Mass::Panel toChange);
	void changePanel(int x, int y, Mass::Panel toChange);
	//IDからマスを特定し、足止め罠のカウントをデクリメントする。０になったらtrueを返す
	bool decCountOfTrapStopper(int id);
	//ミニマップ表示（FrontDisplayを介して）
	void showMiniMap();
	//ターン制限のカウントダウンを進める。現在のカウントを返す
	int incTurnLimit();
	int getTurnLimit();

	//情報を返す関数
	//座標を返す
	const GameLib::Math::Vector3* returnPlaceFromID(int id) const;
	//座標とターゲットから座標を返す
	const GameLib::Math::Vector3* returnPlaceFromIDandTarget(int id, TargetMass target) const;
	//キャラクター同士が一定範囲内にあるかを判定。基本的な使い方としては、
	//敵がプレイヤーと近いかどうか判定し、アクティブにするかを決定する（遠いところにいる敵は動かない）
	bool judgeActive(int judgeId, int baseId) const;
	//移動可能かの判定。可能ならその座標を返し、IDとマップの更新もここで行う
	bool judgeMove(int id, TargetMass mass, GameLib::Math::Vector3* rePos, int* passiveID);
	//移動可能かだけ返す
	bool judgeMove(int x, int y, TargetMass target, int id) const;
	//ワープ可能化の判定。やることは上のjudgeMoveと同様
	bool judgeWarp(int id, GameLib::Math::Vector3* rePos);
	//マスへの干渉の有効性を判定
	bool judgeMassChange(int id, TargetMass mass, Mass::Panel toChange);
	//特定マスにいるキャラクターのIDを渡す。複数キャラを想定してIDは配列で受け取って返す
	//いまのところ複数はとりあえずなしで todo:
	void getIDs(int id, TargetMass mass, int* IDs, int* IDnum);
	//キャラクターがどの種類のマスにいるかを返す。
	//用途としては、プレイヤーが階段の上に立っている場合を判定して、階層を進めるなど
	Mass::Panel getPanelFromID(int id) const;
	Mass::Panel getPanelFromIDandTarget(int id, TargetMass target) const;
	//キャラクターに自身の周りの状況を把握させるための関数
	//敵が隣にいるか、もしくは近くにいるか
	//隣でなく近くの敵をターゲットにするなら、その索敵範囲をマスの長さで指定する。0か1を指定したら遠距離は索敵しないことにする
	//※プレイヤーの場所も索敵できるが、敵が周りに全くいない場合しか探索は行っていない
	void information(int id, bool& playerIsNear, bool& enemyIsNext, bool& enemyIsNear, bool& canRunAway,
		TargetMass& targetNext, TargetMass& targetNear, TargetMass& targetPlayer, TargetMass& targetRunAway, int searchDistance, int& nearTargetID) const;
	//IDから座標を取得
	const GameLib::Math::Vector3* getPositionFromID(int id) const;
	//idとX方向Y方向の差分を受け取ってそこにいるキャラクターのIDを返す。いないなら-1を返す
	int getIDFromIDandMassDiff(int id, int xDiff, int yDiff) const;
	//ステージがボス部屋かどうか
	bool thisStageIsBossRoom() const;
	//敵が全滅しているかどうか。ボス部屋の場合に勝利判定用として使う
	bool AllEnemyIsDied() const;

	//プレイヤーのAI用関数群
	//経路探索。ある程度汎用的に作る
	void doPathPlanning(int id, Mass::Panel destination);
	//経路を返す
	void getPath(TargetMass& target);
	//XY座標から部屋番号を返す
	int getRoomNumberFromXY(int x, int y) const;
	//現在部屋と隣に位置する部屋のうちから、ゴールに近い部屋番を取得
	int nextRoomEvaluation(int nowRoomNumber);
	//ある座標が通路の上かどうか。またその通路のインデックスを返却
	bool isOnCorridor(int x, int y, int* corridorResult);
	//部屋２つから通路のインデックスを取得
	int getCorridorIndex(int room1, int room2) const;
	//ある部屋の、ランダムな場所を返す
	void getRandomMassOfRoom(int roomNumber, int* x, int* y) const;

	//デバッグ用
	void showMassParameter();
private:
	Mass *mMasses;	//ステージを構成する一つ一つのマス
	//モデルやモデルの情報が記されているコンテナ
	GameLib::Scene::Container mStageContainer;//ステージのモデル情報
	GameLib::Scene::Container mBackContainer;	//背景のモデル情報
	GameLib::Scene::Model mModelBackUp;	//背景モデル（上半分）
	GameLib::Scene::Model mModelBackDown;	//背景モデル（下半分）
	float backgroundAngle;
	//その他便利に使う
	int NumOfRegisterIDs; //現在登録しているIDの数
	CombIDandMap* mIDandMap; //キャラクターのIDとマップ情報を併せ持つ
	int playerID;			//プレイヤーのIDだけ別に保存
	int* warpMassList;		//ワープマスになっているマップを格納する
	Mass* prevFootMass;		//直前に踏まれたマスを格納。Brokenマスなどのために使用
	int mTurnLimit;			//ターン制限

	//このクラス内でしか使わない関数
	//ID同士でのマスの被りなどをチェック
	bool idenfifyID(const CombIDandMap id) const;
	bool identifyID(int x, int y) const;
	//mIDandMapリストから、指定されたIDのインデックスを返す
	int getIndexFromID(int id) const;
	//x座標とy座標からマップの連番を返す。マップの生成法によって処理を変える必要がある
	//引数のx,yがエリア外かどうか判定を行い、エリア外なら-1を返す
	int getMapFromXY(int x, int y) const;
	//マップからx座標とy座標を返す
	void getXYfromMap(int map, int* x, int* y) const;
	//IDからマップの連番を返す
	int getMapFromID(int id) const;
	int getMapFromIDandTarget(int id, TargetMass target) const;
	//IDからx座標とy座標を返す
	void getXYfromID(int id, int* x, int* y) const;
	void getXYfromIDandTarget(int id, TargetMass target, int* x, int* y) const;
	void getXYfromXYandTarget(int x, int y, TargetMass, int* reX, int* reY) const;
	Team getTeamFromID(int id) const;
	//IDを探す
	int getIDfromMap(int map) const;
	//該当マップのマス上に、キャラクターが存在するかどうか調べる
	bool characterOnTheMap(int map) const;
	//該当マップのマス上に、敵が存在するかどうか調べる
	bool enemyOnTheMap(int map, Team myTeam) const;
	//部屋番号を返す
	int getRoomIndex(int i, int j);
	//パネルを返す
	Mass::Panel getPanelFromXY(int x, int y) const;
	//2地点の座標の距離を測る
	int getDistance(int x1, int y1, int x2, int y2) const;

	//プレイヤーのAI用にあらたなパラメータを追加
	mapDataOfDivision* mapData;
	std::vector<corridorData*> corridors;
	int ROOM_NUMBER_OF_GOAL;			//ゴールがある部屋番号
	int GoalMapX;						//ゴールの座標
	int GoalMapY;						//ゴールの座標
	TargetMass* targetQue;				//経路を格納
	int targetQueIndex;					//次の経路を示す
	static const int MAX_TARGET_QUE;	//経路キューの最大数

	//定数群　コンストラクタの処理時にファイルから読み出す
	//マス自体のパラメータ
	int		MASS_HEIGHT;		//縦マス数		ちなみにマスの座標は0からスタートなので、
	int		MASS_WIDTH;			//横マス数　	縦も横も最大マス目の座標はこの数値を-1した値になる
	int		MASS_NUM;			//全てのマス数（＝MASS_HEIGHT×MASS_WIDTH）
	float	MASS_SCALE;			//マスの大きさ
	float	MASS_LENGTH;		//1マスの長さ（正方形）
	float	MASS_HALF_LENGTH;	//1マスの半分の長さ (MASS_LENGTH / 2.f)
	//マスの特別な効果に関して
	int		MASS_EFFECT_NUM_OF_HOLE;	//マス「HOLE（穴あき）」の数
	int		MASS_EFFECT_NUM_OF_WARP;	//マス「WARP（ワープ）」の数
	int		MASS_EFFECT_NUM_OF_BROKEN;	//マス「BROKEN（壊れかけ）」の数
	int		MASS_EFFECT_NUM_OF_RECOVERY;//マス「RECOVERY（回復）」の数
	int		MASS_EFFECT_NUM_OF_PARTS;	//マス「PARTS（パーツ入手）」の数
	//ダンジョン生成時に必要となるパラメータ
	int		ROOM_NUM_WIDTH;						//部屋の横の数
	int		ROOM_NUM_HEIGHT;					//部屋の縦の数
	int		ROOM_NUM;							//部屋の総数　上の2つの積
	int		MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM;	//削るマス数の最小値
	int		MASSNUM_OF_DIVIDING_TO_ROOM_MAX;		//削るマス数の最大値
	int		ROOM_MIN_RANGE;							//部屋は最低でもこの幅を保証する
	bool	ROOM_IS_BOSS;							//ボス部屋かどうか

	//その他
	int		MAX_NUM_OF_ID;			//IDリストの最大数
	int		ACTIVE_AREA_FROM_PC;	//モンスターが相手を索敵する範囲。プレイヤ―チームからこの範囲内にある敵がアクティブになりうる。それ以上離れているなら基本何もしない
	int		TURN_LIMIT;				//一つのステージで、このターン数が経過するとペナルティ
};