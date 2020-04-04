#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"
#include "MyRandom.h"
#include "Weapon.h"
#include <string>

class Action;
struct Parts;	//パーツに関しては、下の方に記述してある

//キャラクターがもつパラメータ群の型。ファイルから読み出していく
//todo:insert （パラメータを新たに追加するなら編集必要）
//FrontDisplayにもこのパラメータに準拠した処理があるので、変更するならそちらも	
struct Parameters {
	std::string parent;
	int health;
	int power;
	int speed;
	int defence;
	int capture;
	std::string weapon;
	int parts;
	float scale;
	std::string image;
	//コンストラクタにて、デフォルト値として適当に値をセットしておく
	Parameters() :
		parent(""), health(100), power(100), speed(100), defence(100), capture(100), weapon("NULL"), parts(0), scale(1.f), image("") {}
	~Parameters() {}
};

//AIが行動を決める際に参考にするbool値の集合。キャラクターごとに自身の状況を把握する
struct Information {
	bool youAreEnemyTeam;	//自分が敵側であるか知る
	bool playerIsNearMass;	//プレイヤーが近くにいる
	bool EnemyIsNextMass;	//敵が隣にいる
	bool EnemyIsNearMass;	//敵が近くにいる
	bool CanRunAway;		//逃げるマスがある
	int NearTargetID;		//遠距離攻撃用　ターゲットID
	int direction;			//ターゲットの方向（1:上　2:右　3:下　4:左）
	int directionForNear;	//近くの敵を追いかける
	int directionForPlayer;	//プレイヤーを追いかける
	int directionForRun;	//逃げるよう方向
	//適当なコンストラクタ
	Information() :
		youAreEnemyTeam(false), playerIsNearMass(false), EnemyIsNextMass(false), EnemyIsNearMass(false), CanRunAway(false), NearTargetID(-1), 
		direction(1), directionForNear(1), directionForPlayer(1), directionForRun(1){}
	~Information() {}
};

//AIが行動を評価するときの値
struct ParameterAI {
	int attack;				//攻撃する度合い
	int attackMagic;		//魔法攻撃する度合い
	int moveToEnemy;		//敵に近づく度合い
	int playerLove;			//プレイヤーに近づく度合い（仲間モンスターが）
	int runawayFromEnemy;	//敵から逃げる度合い
	int runawayHP;			//このHPいかになったら逃げる判定が有効なる

	ParameterAI() :
		attack(0), attackMagic(0), moveToEnemy(0),  playerLove(0), runawayFromEnemy(0), runawayHP(30) {}
	~ParameterAI() {}
	void set(int attack_, int attackMagic_, int moveToEnemy_, int playerLove_, int runawayFromEnemy_, int runawayHP_) {
		attack = attack_;
		attackMagic = attackMagic_;
		moveToEnemy = moveToEnemy_;
		playerLove = playerLove_;
		runawayFromEnemy = runawayFromEnemy_;
		runawayHP = runawayHP_;
	}
};

//プレイヤAI用の、情報のひと塊。メモリ効率は落ちるが、色々な情報をまとめてもらう
struct PlayerAIinfo {
	int directionForGoal;			//ゴールまでの方向（1:上　2:右　3:下　4:左）
	bool destinationArrived;		//目的地に着いた。そこがゴールなら終了

	PlayerAIinfo() :
		directionForGoal(1), destinationArrived(false) {}
	~PlayerAIinfo() {}
};


////////////////////////////////////////////////////////////////
//プレイヤー、敵のキャラクターの基底クラス
//PlayerクラスとEnemyクラスはこのクラスを継承する
//それぞれに共通する変数・関数はここですべて定義する
/////////////////////////////////////////////////////////////////
class Character {
public:
	//意思決定の方法（プレイヤー用の入力型と、それ以外に大別される）
	enum Mind {
		MIND_INPUT,		//入力を受け付ける。基本的にプレイキャラクター用
		MIND_PLAYER_AI,	//プレイヤーのAI。デモ動作などで使用
		MIND_AI_1,		//AIで意思決定。パターン①
	};
	//実際に行動する際の種類を定義
	enum Act {
		ACT_WAIT,
		ACT_MOVE,			//移動。セットするときはDestinationも一緒にセットする（それぞれセットするために、別々の関数が用意してある）
		ACT_WARP,			//ワープ。同じく
		ACT_ATTACK_NORMAL,	//通常攻撃
		ACT_ATTACK_MAGIC,	//遠距離魔法攻撃
		ACT_FALL,			//落下。モンスター専用
		ACT_CATCH,			//捕獲される。モンスター専用
		ACT_NONE
	};
	//行動選択時にゲームマスターに意思を伝えるために利用
	enum Reply {
		REPLY_WANT_INFORMATION,	//情報が欲しい。主にAI用
		REPLY_MOVE,				//移動。方向はDirectionを参照
		REPLY_ATTACK_NORMAL,	//攻撃（通常）
		REPLY_ATTACK_MAGIC,		//杖を使った遠距離攻撃
		REPLY_MASS_TRAP_HOLE,	//落とし穴を作る
		REPLY_MASS_TRAP_CATCH,	//捕獲罠を作る
		REPLY_MASS_TRAP_STOPPER,//足止めを作る
		REPLY_MASS_TRAP_BOMB,	//地雷を作る
		REPLY_MASS_MAKE,		//穴を作る

		//プレイヤーAI用
		//AI_GOAL
		REPLY_GOAL_PATH_PLANNNING,	//新しく経路探索を行ってもらう
		REPLY_GOAL_ROOT,			//経路(方向)を教えてもらう

		REPLY_NONE				//何もしない
	};
	//キャラクターの向いている方向を定義。とりあえず斜めなしの4方向
	enum Direction {
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		INVALID
	};
	//自身の状態を管理。主にselectAction関数用のパラメータ
	enum State {
		STATE_WAIT,				//通常時
		STATE_THINKING,			//行動選択時。主にAI用
		STATE_ACTION_SELECT,	//移動以外の動作を選択する
		STATE_ATTACK_SELECT,	//どの攻撃を使うかを選択
		STATE_MAGIC_TARGET_SELECT,	//魔法攻撃の対象を選択する
		STATE_MASS_ACTION_SELECT,	//マスに対するどのような処理をしたいか選択
		STATE_MASS_SELECT,		//四方どのマスを選択するか（攻撃も、マスへの干渉も同様にこの状態に移行）
		STATE_WEAPON_POWERUP,	//武器強化。今持っている武器を強化する
		STATE_ANNOUNCE,			//注意書きを表示
		STATE_NONE
	};
	//プレイヤーAI用の大きな方針
	enum PlayerAI_Strategy {
		AI_GOAL,		//とにかくゴールへ
		AI_BATTLE,		//近くの敵と戦闘
		AI_RUNAWAY,		//とにかく逃げる

		AI_NONE
	};
	//プレイヤーAI用の方針に対する具体的な行動指針
	enum PlayerAI_Behavior {
		//AI_GOAL
		GOAL_PATH_PLANNING,		//経路探索
		GOAL_RECEIVE_ROOT,		//経路を教えてもらう
		GOAL_GO_ALONG_PATH,		//経路に沿って移動
		//AI_BATTLE
		BATTLE_ATTACK_NEXT,		//隣の敵を攻撃
		BATTLE_ATTACK_NEAR,		//近くの敵を攻撃
		BATTLE_THINK_TRAP,		//罠の設置を考える
		BATTLE_PUT_TRAP,		//罠を張る
		BATTLE_DECOY,			//敵を罠に誘導
		//AI_RUNAWAY
		RUNAWAY_GO,				//ただただ逃げる

		BEHAVE_NONE
	};

	friend class Action;
	Character();
	virtual ~Character();
	//毎フレームの更新
	virtual void update();
	//自分のターンが回ってきたときにどう行動するか決定する。決定したらtrueを返すようにする
	virtual bool selectAction();
	virtual void draw();
	//それぞれの行動ごとに、行動するときに合わせて行う処理
	virtual void actEffect(Reply rep);

	//メンバ変数に情報をセットする関数いろいろ
	virtual void setModel(const char* batchname);
	virtual void setPosition(const GameLib::Math::Vector3& pos);
	virtual void setAngleY(const float& angle);
	virtual void setScale(float scale);
	//行動するときのターゲットとするものの座標（攻撃位置・移動先など）
	virtual void setDestination(const GameLib::Math::Vector3& pos);
	//行動を起こさせるためにアクション選択結果を教える。内部で諸々のパラメータを初期化している
	virtual void setAction(Act act);
	virtual void setID(int id);
	virtual void setInformation(Information& st);
	virtual void setPlayerAIinfo(PlayerAIinfo& info);
	virtual void setState(State state);
	virtual void resetActTime();
	virtual void setBeingStopped();		//足止め罠にかかったとき。モンスター専用
	virtual void resetBeingStopped();	//足止め状態を解除
	virtual void setMagicTargetID(int target);
	virtual void pathPlanningWasFinish();

	//メンバ変数の情報を渡す関数いろいろ
	virtual const GameLib::Math::Vector3* position() const;
	virtual const float angle() const;
	virtual const float getActTime() const;
	virtual const Act getAct() const;
	virtual bool actFinished() const;
	virtual const Reply getSelect() const;
	virtual const int getCharacterIDs() const;	//現在のインスタンスの数と同じ
	virtual const int getID() const;			//こっちはあるインスタンスのID
	virtual const Direction getDirection() const;
	virtual const State getState() const;
	virtual const bool getAttackIsDone() const;
	virtual const bool getBeStopped() const;
	virtual const int getMagicTargetID() const;	//遠距離魔法攻撃時の対象ID
	virtual const void getMagicTargetPlace(int* x, int* y) const;	//魔法攻撃時の対象マス：プレイヤー専用

	//パラメータ関連
	//値をセット
	virtual void setParameter(Parameters& param);
	virtual void setName(std::string name);
	virtual void setParameterAI(ParameterAI& paramAI);
	//値を返す
	virtual const std::string getName() const;
	virtual const Parameters* getParameter() const;
	virtual const ParameterAI* getParameterAI() const;
	//HP、ダメージ計算など
	virtual void damage(int receivePower);
	virtual void recovery(int plus);
	virtual void subHP(int num);
	virtual int getHP() const;
	virtual bool isLiving() const;
	//パーツを渡す
	virtual Parts getParts() const;

	//武器関連
	//武器をもらう
	virtual void getWeapon(Weapon::Type type);
	//自身の最大射程距離
	virtual int canAttackToNear() const;
	//武器情報
	virtual const Weapon& weapon() const;

	//その他
	void resetAllParameters();
	void turnLimitAction();		//プレイヤー専用。ターン制限のペナルティ
	void moveFault();			//移動が拒否されたときに1回呼び出す
	void resetAIparameter();	//AIのパラメータをリセット
protected:
	//モデルの表示に関する値
	GameLib::Scene::Container mCharacterContainer;	//プレイヤー、敵両方のモデル情報が格納されているコンテナ
	GameLib::Scene::Model mModelCharacter;			//キャラクターのモデル
	GameLib::Scene::Model mModelHPbar;				//HPバーのモデル
	GameLib::Math::Vector3 mPosition;				//現在の位置
	GameLib::Math::Vector3 mPrevPosition;			//位置が変わる前の位置
	GameLib::Math::Vector3 mDestination;			//目的地。これから移動するべき場所
	float mAngleY;//角度

	//enumで定義したパラメータを使用するものたち
	Mind mMind;
	Act mAct;
	float mActTime;		//0.f～1.fの間でタイミングを管理する
	Reply mReply;
	Direction mDirection;
	State mState;
	Information mInformation;
	//パラメータ関連
	Parameters mParam;
	mutable Parameters mReturnParam;
	std::string mName;
	ParameterAI mParamAI;
	int HP;//現在のHP。パラメータのhealthは最大HP
	Parts* mParts;
	Weapon mWeapon;
	//プレイヤーのAI用
	PlayerAIinfo	  mAIinfo;
	PlayerAI_Strategy mAIstrategy;
	PlayerAI_Behavior mAIbehavior;

	//その他便利に使うための諸々の変数
	static int CharacterID;	//キャラクターを識別するID。Stageクラス内で、このIDとマスの座標を紐づけておく
	int mCharacterID;		//Characterのインスタンス生成時に静的変数CharacterIDの値を格納。個々のモンスターを識別するためのもの
	bool attackIsDone;		//アクションの中で、攻撃がした瞬間にtrueにする（任意のタイミングで攻撃を入れることができる）
	bool beStopped;			//罠：あしどめを食らっている状態。アクティブキャラクター変更のときに参照して、trueならターンを飛ばす
	bool haveWeapon;		//武器持ってるか
	int magicTargetID;		//遠距離攻撃時の対象ID
	int announceMessageNumber;	//注意を行うときにどのメッセージを表示するか
	State backFromAnnounce;		//注意から戻る先
	bool moveWasFault;		//移動が拒否された
	bool pathPlanningWasFinished;	//経路探索は済んでいる

	//resetAllParameter()でリセットする変数
	int magicTargetX;	//魔法攻撃時に対象を選択するときのX方向
	int magicTargetY;	//			;;					　Y方向

	//このクラス群でしか使わない関数
	//キャラクターの角度を変更するときに自然な感じで変化するように調整するためのもの
	void naturalAngleChange(float targetAngle);
	//プレイヤーが入力してキャラクターの行動を選択する
	bool selectOfInput();
	//自動で行動選択するパターン1。基本的にはモンスター専用
	bool selectOfMind1();
	//プレイヤのAI
	bool selectOfPlayerAI();
	bool selectOfPlayerAI_GOAL();
	bool selectOfPLayerAI_BATTLE();

	//定数群　ファイルから読み出す
	float ROTATE_SPEED;				//回転速度
	float WARP_ROTATE_SPEED;		//ワープ時の回転速度
	float COUNT_INTERVAL_ACT_MOVE;	//COUNT_INTERVALは、mActTimeのカウントの進むスピード（/frame）
	float COUNT_INTERVAL_ACT_WARP;
	float COUNT_INTERVAL_ACT_ATTACK_NORMAL;
	float COUNT_INTERVAL_ACT_ATTACK_MAGIC;
	float COUNT_INTERVAL_ACT_FALL;
	float COUNT_INTERVAL_ACT_CATCH;
	//HPバーについて
	float HP_BAR_SCALE_X;	//HPバーの横の長さ
	float HP_BAR_SCALE_Y;	//　""    縦の長さ
	float HP_BAR_PLACE_Y;	//キャラモデルのいくつ上に表示するか
	float HP_BAR_PINCH;		//ピンチ判定がいくつからか
	//その他
	float MASS_LENGTH;		//1マスの長さ
private:
	//このクラス内でしか使わない関数
	void toAnnouce(int messageNumber, State backTo);
};



//罠を生成するときに必要となるパーツ
struct Parts {
public:
	static int numParts;		//パーツの種類
	int Parts_Screw;	//ねじ
	int Parts_Hammer;	//ハンマー
	int Parts_Net;		//網
	int Parts_Stone;	//石
	//コンストラクタ
	Parts() :Parts_Screw(0), Parts_Hammer(0), Parts_Net(0), Parts_Stone(0) {}
	Parts(int screw, int hammer, int net, int stone) : 
		Parts_Screw(screw), Parts_Hammer(hammer), Parts_Net(net), Parts_Stone(stone) {}
	//要求に対して必要なパーツを返す
	static Parts neededParts(Character::Reply rep);
	//名前を教える。連番で
	std::string getNameOfParts(int number);
	//持っているパーツの数を教える。連番で
	int getNumOfParts(int number);
	//何かしらパーツを持っているか
	bool hasSomethingParts();
	//足す
	void add(Parts parts);
	//引く
	void sub(Parts parts);
	//自身を数倍する
	void mul(int num);
	//必要数足りているか判定
	bool judge(Parts parts);
	//モンスター用。ランダムにnumの数のパーツを持たせる
	void setRandomParts(int num);
};
