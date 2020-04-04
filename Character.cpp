#include "GameLib/GameLib.h"
#include "Character.h"
#include "ContentReader.h"
#include "Action.h"
#include "EffectController.h"
#include "Button.h"
#include "FrontDisplayh.h"
#include "SoundGenerator.h"

using namespace GameLib;
using namespace GameLib::Scene;
using namespace GameLib::Math;
using namespace std;

int Character::CharacterID = 0;
int Parts::numParts = 4;


//////////////////////////////////////////////////////
///Characterクラス。下の方にパーツに関する記述あり////
//////////////////////////////////////////////////////

Character::Character() :
	mPosition(0.f, 0.f, 0.f),
	mDestination(0.f, 0.f, 0.f),
	mAngleY(0.f),
	mMind(MIND_AI_1),
	mAct(ACT_WAIT),
	mActTime(0.f),
	mReply(REPLY_NONE),
	mDirection(Direction::DIR_UP),
	mState(STATE_WAIT),
	mParts(nullptr),
	HP(0),
	attackIsDone(false),
	haveWeapon(false),
	magicTargetID(-1),
	announceMessageNumber(0),
	backFromAnnounce(STATE_NONE),
	moveWasFault(false),
	pathPlanningWasFinished(false),
	mAIstrategy(PlayerAI_Strategy::AI_GOAL),
	mAIbehavior(PlayerAI_Behavior::GOAL_PATH_PLANNING)
{
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	ROTATE_SPEED = cr->getData("Character", "ROTATE_SPEED");
	WARP_ROTATE_SPEED				= cr->getData("Character", "WARP_ROTATE_SPEED");
	COUNT_INTERVAL_ACT_MOVE			= cr->getData("Character", "COUNT_INTERVAL_ACT_MOVE");
	COUNT_INTERVAL_ACT_WARP			= cr->getData("Character", "COUNT_INTERVAL_ACT_WARP");
	COUNT_INTERVAL_ACT_ATTACK_NORMAL= cr->getData("Character", "COUNT_INTERVAL_ACT_ATTACK_NORMAL");
	COUNT_INTERVAL_ACT_ATTACK_MAGIC	= cr->getData("Character", "COUNT_INTERVAL_ACT_ATTACK_MAGIC");
	COUNT_INTERVAL_ACT_FALL			= cr->getData("Character", "COUNT_INTERVAL_ACT_FALL");
	COUNT_INTERVAL_ACT_CATCH		= cr->getData("Character", "COUNT_INTERVAL_ACT_CATCH");
	HP_BAR_SCALE_X					= cr->getData("Character", "HP_BAR_SCALE_X");
	HP_BAR_SCALE_Y					= cr->getData("Character", "HP_BAR_SCALE_Y");
	HP_BAR_PLACE_Y					= cr->getData("Character", "HP_BAR_PLACE_Y");
	HP_BAR_PINCH					= cr->getData("Character", "HP_BAR_PINCH");
	MASS_LENGTH						= cr->getData("Stage", "MASS_LENGTH");
	SAFE_DELETE(cr);//使い終わったら削除

	//モデル情報が格納されているコンテナを読み込む
	mCharacterContainer = Container::create("data/model/character.txt");
	while (!mCharacterContainer.isReady()) { ; }

	//HPバー
	mModelHPbar = mCharacterContainer.createModel("hp_bar");

	//その他初期設定
	CharacterID++;
	mCharacterID = CharacterID;
	HP = mParam.health;
	mWeapon = Weapon();
	mParts = new Parts();
}

Character::~Character()
{
	mModelCharacter.release();
	//デストラクタでIDも減らしていたが、IDの被りが出てきてしまうため一度使ったIDはゲーム中もう使わないこととする
	//CharacterIDはint型なので、ゲーム中int型の最大数を超える数のモンスターを作ると確実にエラーが出る
	//しかし、そこまでの数のモンスターを生成することがないと思うので、今はこのままにしておく
	//CharacterID--;
}

void Character::update()
{
	//Actionクラスのインスタンスを取得。キャラクターの動きは大体Actionクラスが担う
	//この関数内で変更しているのはせいぜいキャラクターの向いている方向くらい
	Action* action = Action::instance();

	switch (mAct) {
	case ACT_WAIT: {
		mActTime = 1.1f;
		//向き変更。ナチュラルに
		switch (mDirection) {
		case DIR_UP:
			naturalAngleChange(0.f);
			break;
		case DIR_DOWN:
			naturalAngleChange(180.f);
			break;
		case DIR_LEFT:
			naturalAngleChange(90.f);
			break;
		case DIR_RIGHT:
			naturalAngleChange(270.f);
			break;
		}
		break;
	}
	case ACT_MOVE: {
		mActTime += COUNT_INTERVAL_ACT_MOVE;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_MOVE, attackIsDone, mActTime);
		//向き変更。こっちは一瞬で変化
		switch (mDirection) {
		case DIR_UP:
			mAngleY = 0.f;
			break;
		case DIR_DOWN:
			mAngleY = 180.f;
			break;
		case DIR_LEFT:
			mAngleY = 90.f;
			break;
		case DIR_RIGHT:
			mAngleY = 270.f;
			break;
		}
		break;
	}
	case ACT_WARP: {
		mActTime += COUNT_INTERVAL_ACT_WARP;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_WARP, attackIsDone, mActTime);
		mAngleY += WARP_ROTATE_SPEED;
		break;
	}
	case ACT_ATTACK_NORMAL: {
		mActTime += COUNT_INTERVAL_ACT_ATTACK_NORMAL;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_ATTACK_NORMAL, attackIsDone, mActTime);
		//向き変更。ナチュラルに
		switch (mDirection) {
		case DIR_UP:
			naturalAngleChange(0.f);
			break;
		case DIR_DOWN:
			naturalAngleChange(180.f);
			break;
		case DIR_LEFT:
			naturalAngleChange(90.f);
			break;
		case DIR_RIGHT:
			naturalAngleChange(270.f);
			break;
		}
		break;
	}
	case ACT_ATTACK_MAGIC: {
		mActTime += COUNT_INTERVAL_ACT_ATTACK_MAGIC;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_ATTACK_MAGIC, attackIsDone, mActTime);
		break;
	}
	case ACT_FALL: {
		mActTime += COUNT_INTERVAL_ACT_FALL;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_FALL, attackIsDone, mActTime);
		break;
	}
	case ACT_CATCH: {
		mActTime += COUNT_INTERVAL_ACT_CATCH;
		if (!actFinished())
			action->update(mDestination, mPosition, mPrevPosition, ACT_CATCH, attackIsDone, mActTime);
		break;
	}
	case ACT_NONE: {

	}
	default:
		HALT("File:PlayCharacter.cpp [update] Error");
	}
}

void Character::draw() {
	//キャラクター
	mModelCharacter.setAngle( Vector3(0.f, mAngleY, 0) );
	mModelCharacter.setPosition(mPosition);
	mModelCharacter.draw();
	//HPバー
	//一定割合以下になるとピンチ色になる
	float hpRange = (float)HP / (float)mParam.health;
	if ( hpRange < HP_BAR_PINCH)
		mModelHPbar.setColor(Vector3(1.f, 0.5f, 0.5f));
	else
		mModelHPbar.setColor(Vector3(1.f, 1.f, 1.f));
	//HPバーの大きさ・位置を調整
	float cameraAngle = EffectController::instance()->getEyeAngle();
	mModelHPbar.setAngle( Vector3(0.f, cameraAngle, 0.f) );
	Vector3 hpPos;
	hpPos.set(mPosition.x, mPosition.y + HP_BAR_PLACE_Y * mParam.scale, mPosition.z);
	mModelHPbar.setPosition(hpPos);
	mModelHPbar.setScale(Vector3(HP_BAR_SCALE_X * hpRange, HP_BAR_SCALE_Y, 1.f));
	mModelHPbar.draw();
	//武器
	//位置。キャラクターのちょっと横に表示
	if (haveWeapon) {
		Vector3 wepPos;
		wepPos.set(mPosition.x+1.8f, mPosition.y + 1.f, mPosition.z);
		mWeapon.setPosition(wepPos);
		mWeapon.setAngleY(EffectController::instance()->getEyeAngle());
		mWeapon.setScale(5.f);
		mWeapon.draw();
	}
}

//
void Character::actEffect(Reply rep)
{
	switch (rep) {
	case Reply::REPLY_MOVE:
		//１だけ回復
		this->recovery(1);
		break;
	default:
		HALT("File: Character.cpp [actEffect()] switch Error");
	}
}

//行動が決定しない間はfalseを返し続ける。trueを返したら行動意思が決まったということ。
//行動が決まったとしてもそれがGameクラス側で拒否される場合もある。
//その場合、もう一度この関数が呼ばれて選択しなおす
bool Character::selectAction()
{
	switch (mMind) {
	//キーボードからの入力
	case Mind::MIND_INPUT: {
		//長いので別の関数に移設
		return selectOfInput();
	}
	//プレイヤーのAI
	case Mind::MIND_PLAYER_AI: {
		//長いので別の関数に移設
		return selectOfPlayerAI();
	}
	//AIのパターン１
	case Mind::MIND_AI_1: {
		//長いので別の関数に移設
		return selectOfMind1();
	}
	//これはエラー
	default:
		HALT("File:Character.cpp [selectAction()] Error");
		break;
	}

	return false;
}

void Character::setModel(const char* batchname)
{
	mModelCharacter = mCharacterContainer.createModel(batchname);
}

void Character::setPosition(const Vector3& pos)
{
	mPosition = pos;
}

void Character::setAngleY(const float& angle)
{
	mAngleY = angle;
}

void Character::setScale(float scale)
{
	mModelCharacter.setScale(Vector3(scale, scale, scale));
}

void Character::setDestination(const Vector3& pos)
{
	mDestination = pos;
}

void Character::setAction(Act act)
{
	//アクションタイムもリセット
	mActTime = 0.f;
	//ついでに移動前位置も格納しておく
	mPrevPosition = mPosition;
	//その他初期化が必要なパラメータも初期化しておく
	attackIsDone = false;

	//SEもここで鳴らしておく
	SoundGenerator* sound = SoundGenerator::instance();
	switch (act) {
	case Character::Act::ACT_WAIT:
		break;
	case Character::Act::ACT_MOVE:
		sound->playSe(SoundGenerator::Se::CHARACTER_MOVE);
		break;
	case Character::Act::ACT_WARP:
		sound->playSe(SoundGenerator::Se::CHARACTER_WARP);
		break;
	case Character::Act::ACT_ATTACK_NORMAL:
		sound->playSe(SoundGenerator::Se::CHARACTER_ATTACK_NORMAL);
		break;
	case Character::Act::ACT_ATTACK_MAGIC:
		sound->playSe(SoundGenerator::Se::CHARACTER_ATTACK_MAGIC);
		break;
	case Character::Act::ACT_FALL:
		sound->playSe(SoundGenerator::Se::CHARACTER_FALL);
		break;
	case Character::Act::ACT_CATCH:
		sound->playSe(SoundGenerator::Se::CHARACTER_CATCH);
		break;
	default:
		HALT("File:Character.cpp [setAction()] switch sound Error");
	}

	mAct = act;
}

void Character::setID(int id)
{
	mCharacterID = id;
}

void Character::setInformation(Information& info)
{
	mInformation = info;
}

void Character::setPlayerAIinfo(PlayerAIinfo& info)
{
	mAIinfo = info;
}

void Character::setState(State state)
{
	mState = state;
}

void Character::resetActTime()
{
	mActTime = 0.f;
}

void Character::setBeingStopped()
{
	beStopped = true;
}

void Character::resetBeingStopped()
{
	beStopped = false;
}

void Character::setMagicTargetID(int id)
{
	magicTargetID = id;
}

void Character::pathPlanningWasFinish()
{
	pathPlanningWasFinished = true;
}

const Vector3* Character::position() const
{
	return &mPosition;
}

const float Character::angle() const
{
	return mAngleY;
}

const float Character::getActTime() const
{
	return mActTime;
}

const Character::Act Character::getAct() const
{
	return mAct;
}

bool Character::actFinished() const
{
	//アクションタイムが一定時間になったらFinish
	return mActTime >= 1.f;
}

const Character::Reply Character::getSelect() const
{
	return mReply;
}

const int Character::getCharacterIDs() const
{
	return CharacterID;
}

const int Character::getID() const
{
	return mCharacterID;
}

const Character::Direction Character::getDirection() const
{
	return mDirection;
}

const Character::State Character::getState() const
{
	return mState;
}

const bool Character::getAttackIsDone() const
{
	return attackIsDone;
}

const bool Character::getBeStopped() const
{
	return beStopped;
}

const int Character::getMagicTargetID() const
{
	return magicTargetID;
}

const void Character::getMagicTargetPlace(int* x, int* y) const
{
	*x = magicTargetX;
	*y = magicTargetY;
}

//パラメータ関連

void Character::setParameter(Parameters& param)
{
	mParam = param;
}

void Character::setName(std::string name)
{
	mName = name;
}

void Character::setParameterAI(ParameterAI& paramAI)
{
	mParamAI = paramAI;
}

const string Character::getName() const
{
	return mName;
}

const Parameters* Character::getParameter() const
{
	mReturnParam = mParam;

	//武器によって能力値が変化
	if (haveWeapon) {
		int addPow = mWeapon.getAdditionalPower();
		mReturnParam.power += addPow;
	}
	return &mReturnParam;
}

const ParameterAI* Character::getParameterAI() const
{
	return &mParamAI;
}

//相手のパワーを受け取って、ダメージ計算を行う
void Character::damage(int receivePower)
{
	int result_damage = 0;
	//ドラクエ11を参考にする
	float power = (float)receivePower / 2.f;
	float defence = (float)mParam.defence / 4.f;
	float base_damage = power - defence;
	//基本ダメージの正負判定。負ならとりあえず1ダメージ固定
	if (base_damage > 0) {
		int rand = (int)(base_damage / 16.f);
		int rand_damage = rand;
		if (rand != 0)
			int rand_damage = MyRandom::instance()->getInt(-rand, rand);
		result_damage = (int)base_damage + rand_damage;
	}
	else
		result_damage = 1;
	//最後に自身のHPから結果を引く
	HP -= result_damage;
	if (HP < 0)
		HP = 0;

	//受けたダメージを記憶しておき、このHP以下のときは戦闘を避ける
	mParamAI.runawayHP = result_damage;
}

void Character::recovery(int plus)
{
	//単純に足す
	HP += plus;
	//最大HPを超すときは最大HPに
	if (HP > mParam.health)
		HP = mParam.health;
}

//HPからそのまま引く
void Character::subHP(int num)
{
	HP -= num;
	if (HP <= 0)
		HP = 0;
}

int Character::getHP() const
{
	return HP;
}

bool Character::isLiving() const
{
	return HP > 0;
}

Parts Character::getParts() const
{
	return *mParts;
}

void Character::getWeapon(Weapon::Type type)
{
	haveWeapon = true;

	switch (type) {
	case Weapon::Type::Soard:
		mWeapon.setModel(Weapon::Type::Soard, mCharacterContainer, "soard");
		break;
	case Weapon::Type::Rod:
		mWeapon.setModel(Weapon::Type::Rod, mCharacterContainer, "rod");
		break;
	default:
		HALT("FIle: Character.cpp [getWeapon()] switch Error");
	}
}

int Character::canAttackToNear() const
{
	//武器を持っているならそれに応じた射程距離
	if (haveWeapon)
		return mWeapon.getMaxDistanceOfAttack();
	//武器がないなら射程距離は通常１
	return 1;
}

const Weapon& Character::weapon() const
{
	return mWeapon;
}

void Character::resetAllParameters()
{
	magicTargetX = 0;
	magicTargetY = 0;
}

void Character::turnLimitAction()
{
	//爆発表示
	EffectController::instance()->createEffect(EffectController::Type::EXPLOSION, mPosition, true);
	//爆発音
	SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_BOMB);
	//HPを1にする
	this->subHP(getHP() - 1);
}

void Character::moveFault()
{
	moveWasFault = true;
}

void Character::resetAIparameter()
{
	pathPlanningWasFinished = false;
}

/////////////////////////このクラスでしか使わない関数///////////////////////////

void Character::naturalAngleChange(float targetAngle)
{
	while (true)
	{
		if (mAngleY < 360.f)
			break;
		mAngleY -= 360.f;
	}
	/*
	この関数がしたいことは、自然な回転を実現することである。
	向きが一瞬で変化するのを防ぎ、少しづつ変化させる。その際、
	目的の角度と自身の角度の差分が、左回りで180度以上あったとしても、
	左回りのまま回転してしまい、不自然となる。そのため、
	180度以上のときには右回転となるように処理を行う。
	*/
	//まず自身の角度と目的の角度の差分を取る
	float diff = targetAngle - mAngleY;
	if (diff < 0.f)
		diff += 360.f;
	if (diff > 180.f)
		diff = -1 * (360.f - diff);
	//差分が一定値以下なら目的の角度にセットして終了
	if ((diff >= 0 && diff <= 10.f) ||	//正
		(diff <= 0 && diff >= -10.f)) {	//負
		mAngleY = targetAngle;
		return;
	}
	//差分が180度以上あるかどうかで分岐
	if (diff > 0.f)
		mAngleY += ROTATE_SPEED;
	else
		mAngleY -= ROTATE_SPEED;
}

//updateの中身を一部移設（長すぎて可読性が低くなったため）
bool Character::selectOfInput()
{
	Button* button = Button::instance();
	EffectController* effect = EffectController::instance();
	FrontDisplay* front = FrontDisplay::instance();

	//[r]で最初に戻る
	if (button->isTriggered(Button::Key::KEY_R))
		mState = State::STATE_WAIT;

	switch (mState) {
	case State::STATE_WAIT: {
		//asdwで移動
		// A キー
		if (button->isTriggered(Button::KEY_A)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_LEFT;
			return true;
		}
		// D キー
		else if (button->isTriggered(Button::KEY_D)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_RIGHT;
			return true;
		}
		// W キー
		else if (button->isTriggered(Button::KEY_W)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_UP;
			return true;
		}
		// S キー
		else if (button->isTriggered(Button::KEY_S)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_DOWN;
			return true;
		}
		// SPACEキー 目の前の敵に通常攻撃
		else if (button->isTriggered(Button::KEY_SPACE)) {
			mReply = Character::REPLY_ATTACK_NORMAL;
			return true;
		}
		// SHIFTキー その他のアクション
		else if (button->isTriggered(Button::KEY_SHIFT)) {
			mState = State::STATE_ACTION_SELECT;
			return false;
		}
		break;
	}
	case State::STATE_ACTION_SELECT: {
		//まずは大まかに何がしたいか決める（攻撃したいのか、マスに干渉したいのか）
		FrontDisplay::instance()->showMessage(4);
		int cursor = front->showCursor(3, FrontDisplay::Cursor::CURSOR_TEXT);
		if(button->isTriggered(Button::Key::KEY_SPACE)){
			switch (cursor) {
			case 1:
				//攻撃
				mState = Character::STATE_ATTACK_SELECT;
				break;
			case 2:
				//マスへの干渉
				mState = Character::STATE_MASS_ACTION_SELECT;
				mReply = Character::Reply::REPLY_MASS_TRAP_HOLE; //この1行は無視してよい
				break;
			case 3:
				//武器のパワーアップ
				mState = State::STATE_WEAPON_POWERUP;
				break;
			default:
				HALT("File: PlayCharacter.cpp [selectAction()] STATE_ACTION_SELECT's switch is Error");
				break;
			}
		}
		break;
	}
	case State::STATE_ATTACK_SELECT: {
		front->showMessage(5);
		int cursor =front->showCursor(2, FrontDisplay::Cursor::CURSOR_TEXT);
		if (button->isTriggered(Button::Key::KEY_SPACE)) {
			//どの方向に攻撃するか決定
			mState = Character::STATE_MASS_SELECT;

			switch (cursor) {
			case 1:
				//通常攻撃
				mReply = Character::REPLY_ATTACK_NORMAL;
				break;
			case 2:
				//スペシャル攻撃
				if (mWeapon.getWeaponType() == Weapon::Type::Rod) {
					mState = State::STATE_MAGIC_TARGET_SELECT;
					mReply = Character::REPLY_ATTACK_MAGIC;
				}
				else {
					//注意
					toAnnouce(12, State::STATE_ATTACK_SELECT);
				}
				break;
			default:
				HALT("File: PlayCharacter.cpp [selectAction()] STATE_MASS_ACTION_SELECT's switch is Error");
				break;
			}

		}
		break;
	}
	case State::STATE_MAGIC_TARGET_SELECT: {
		int x(0), y(0);
		// A キー
		if (button->isTriggered(Button::KEY_A))
			x = 1;
		// D キー
		else if (button->isTriggered(Button::KEY_D))
			x = -1;
		// W キー
		else if (button->isTriggered(Button::KEY_W))
			y = 1;
		// S キー
		else if (button->isTriggered(Button::KEY_S))
			y = -1;
		//現在の選択位置は次の2つの変数に格納されている
		magicTargetX += x;
		magicTargetY += y;
		//選択マスと自分の位置とのマス的な距離を測る
		int distance = ((magicTargetX >= 0) ? magicTargetX : -magicTargetX) + ((magicTargetY >= 0) ? magicTargetY : -magicTargetY);
		//最大射程範囲以上を選択するのはダメ
		if (distance > this->canAttackToNear()) {
			magicTargetX -= x;
			magicTargetY -= y;
		}
		//選択中にカーソル的なものを表示
		Vector3 pos = mPosition;
		pos.x += MASS_LENGTH * (float)magicTargetX;
		pos.z += MASS_LENGTH * (float)magicTargetY;
		effect->show(EffectController::Type::MAGIC_TARGET, pos, Vector3(0.f, 0.f, 0.f), 1);
		//決定
		if (button->isTriggered(Button::KEY_SPACE)) {
			//要求が拒否されたときようにセット
			toAnnouce(13, State::STATE_MAGIC_TARGET_SELECT);
			mState = Character::STATE_WAIT;
			mReply = Character::REPLY_ATTACK_MAGIC;
			return true;
		}
		break;
	}
	case State::STATE_MASS_ACTION_SELECT: {
		//マスへの干渉。何をするか決める（穴をあける、穴を埋めるなど）
		front->showMessage(6);
		//必要なパーツを表示
		front->showNeedParts(Parts::neededParts(mReply));
		int cursor = front->showCursor(5, FrontDisplay::Cursor::CURSOR_TEXT);
		switch (cursor) {
		case 1:
			//落とし穴
			mReply = Character::REPLY_MASS_TRAP_HOLE;
			break;
		case 2:
			//捕獲罠
			mReply = Character::REPLY_MASS_TRAP_CATCH;
			break;
		case 3:
			//足止め
			mReply = Character::REPLY_MASS_TRAP_STOPPER;
			break;
		case 4:
			//地雷
			mReply = Character::REPLY_MASS_TRAP_BOMB;
			break;
		case 5:
			//新しいマスの生成
			mReply = Character::REPLY_MASS_MAKE;
			break;
		default:
			HALT("File: PlayCharacter.cpp [selectAction()] STATE_MASS_ACTION_SELECT's switch is Error");
			break;
		}
		//スペースキーで次へ
		if (button->isTriggered(Button::Key::KEY_SPACE)) {
			if (!mParts->judge(Parts::neededParts(mReply))) {
				toAnnouce(15, STATE_MASS_ACTION_SELECT);
				break;
			}
			mState = STATE_MASS_SELECT;
		}
		break;
	}
	case State::STATE_MASS_SELECT: {
		front->showMessage(7);
		//Spaceキーで方向決定
		if (button->isTriggered(Button::KEY_SPACE)) {
			mState = Character::STATE_WAIT;
			return true;
		}
		//Aキー
		if (button->isTriggered(Button::KEY_A))
			mDirection = Character::DIR_LEFT;
		// D キー
		else if (button->isTriggered(Button::KEY_D))
			mDirection = Character::DIR_RIGHT;
		// W キー
		else if (button->isTriggered(Button::KEY_W))
			mDirection = Character::DIR_UP;
		// S キー
		else if (button->isTriggered(Button::KEY_S))
			mDirection = Character::DIR_DOWN;

		break;
	}
	case State::STATE_WEAPON_POWERUP: {
		Parts need = Weapon::getNeededParts(mWeapon.getWeaponType(), mWeapon.getWeaponLevel());
		front->showMessage(16);
		front->showNeedParts(need);
		int cursor = front->showCursor(2, FrontDisplay::Cursor::CURSOR_TEXT);
		if (button->isTriggered(Button::Key::KEY_SPACE)) {
			switch (cursor) {
			case 1:
				//足りない
				if (!mParts->judge(need))
					toAnnouce(15, State::STATE_ACTION_SELECT);
				//足りる。パワーアップ
				else {
					mParts->sub(need);
					mWeapon.powerUP();
					toAnnouce(17, State::STATE_WAIT);
				}
				break;
			case 2:
				mState = State::STATE_WAIT;
				break;
			}
		}
		break;
	}
	case State::STATE_ANNOUNCE: {
		front->showMessage(announceMessageNumber);
		//Spaceキーで戻る
		if (button->isTriggered(Button::KEY_SPACE))
			mState = backFromAnnounce;
		break;
	}
	case State::STATE_NONE: {

		break;
	}
	default:
		HALT("File:PlayCharacter.cpp [selectAction()] Error");
		break;
	}

	return false;
}

bool Character::selectOfPlayerAI()
{
	switch (mState) {
	case State::STATE_WAIT: {
		mReply = Reply::REPLY_WANT_INFORMATION;
		return true;
	}
	case State::STATE_THINKING: {

		//周りの状況をみて方針を決定
		if (mAIinfo.destinationArrived) {
			pathPlanningWasFinished = false;
			mAIinfo.destinationArrived = false;
		}
		//近くに敵がいるなら戦闘
		if (selectOfMind1()) {
			//何もしない
			//ゴールを目指す
			if (mReply != Reply::REPLY_NONE) {
				resetAIparameter();
				return true;
			}
		}
		
		/*
		・PlayerAI_StrategyとPlayerAI_Behaviorに沿って行動を選択していく
		・GameクラスやStageクラスから情報を得るときは、その旨をReplyによって伝える（Game,Stageにその処理を記述）

		PlayerAI_Strategyについて
		　・AI_GOAL
		　 １．経路探索をStageクラスに依頼する（探索済みフラグが立っていたらしない）
		  　　 →GOAL_PATH_PLANNING
		   ２．経路探索済みフラグを立てる
		   ３．Stageクラスに経路を教えてもらう
			   →GOAL_RECEIVE_ROOT
		   ４．移動する
		   　　→GOAL_GO_ALONG_PATH
		*/


		switch (mAIstrategy) {
		case PlayerAI_Strategy::AI_GOAL: {
			if (!pathPlanningWasFinished)
				mAIbehavior = PlayerAI_Behavior::GOAL_PATH_PLANNING;
			return selectOfPlayerAI_GOAL();
			break;
		}
		case PlayerAI_Strategy::AI_BATTLE: {
			return selectOfPLayerAI_BATTLE();
			break;
		}
		case PlayerAI_Strategy::AI_RUNAWAY: {
			HALT("File:Character.cpp debugError AI_RUNAWAY");
			break;
		}
		default:
			HALT("File:Character.cpp selectOfPlayerAI() mAIstrategy's switch Error");
		}
		break;
	}
	default:	
		HALT("File:Character.cpp selectOfPlayerAI() mState's switch Error");
	}
	return true;
}

bool Character::selectOfPlayerAI_GOAL()
{
	switch (mAIbehavior) {
	case PlayerAI_Behavior::GOAL_PATH_PLANNING: {
		//経路探索の必要があるか
		if (pathPlanningWasFinished)
			//経路を受け取りにいく
			mAIbehavior = GOAL_RECEIVE_ROOT;
		else {
			//経路探索を依頼
			mReply = Reply::REPLY_GOAL_PATH_PLANNNING;
			return true;
		}
		break;
	}
	case PlayerAI_Behavior::GOAL_RECEIVE_ROOT: {
		//経路をもらう
		mReply = Reply::REPLY_GOAL_ROOT;
		mAIbehavior = GOAL_GO_ALONG_PATH;
		return true;
	}
	case PlayerAI_Behavior::GOAL_GO_ALONG_PATH: {
		//移動
		mReply = Reply::REPLY_MOVE;
		mAIbehavior = GOAL_PATH_PLANNING;
		//方向合わせ
		switch (mAIinfo.directionForGoal) {
		case 1:
			mDirection = Direction::DIR_UP;
			break;
		case 2:
			mDirection = Direction::DIR_RIGHT;
			break;
		case 3:
			mDirection = Direction::DIR_DOWN;
			break;
		case 4:
			mDirection = Direction::DIR_LEFT;
			break;
		default:
			HALT("File: Character.cpp [selectOfMind1()] switch Error");
		}
		return true;
	}
	default:
		HALT("File:Character.cpp selectOfPlayerAI_GOAL() switchError");
	}

	return false;
}

bool Character::selectOfPLayerAI_BATTLE()
{
	//なにもしない
	return true;
}

bool Character::selectOfMind1()
{
	/*
	1：ターンが回ってきたら、まず情報をくださいというのをGameクラスに伝える
	2：Gameクラスから、隣接するマスに関しての情報をもらう（setStrategy関数をよんでもらう）
	   ※同時にsetStateでSTATE_THINKING状態にセットしてもらう
	3：受け取った情報をもとに行動を決定

	*/
	
	switch (mState) {
	//１
	case State::STATE_WAIT: {
		mReply = Reply::REPLY_WANT_INFORMATION;
		return true;
	}
	//３
	case State::STATE_THINKING: {
		//移動が拒否されたときには何もしないこととする
		if (moveWasFault) {
			moveWasFault = false;
			pathPlanningWasFinished = false;
			mReply = Reply::REPLY_NONE;
			mState = State::STATE_WAIT;
			//HALT("File: Character.cpp MOVEwasFault ");
			return true;
		}

		//受け取った情報をもとに評価値を計算する
		//行動ごとの計算結果を格納する。行動の可不可は関係なくとりあえず計算する
		int Value_Attack_Normal(0), Value_Attack_Magic(0), Value_MoveToEnemy(0), Value_MoveToPlayer(0), Value_RunAway(0), Value_None(0);
		//ATTACK_NORMAL
		Value_Attack_Normal = mParamAI.attack;
		//ATTACK_MAGIC
		Value_Attack_Magic = mParamAI.attackMagic;
		//MoveToEnemy
		Value_MoveToEnemy = mParamAI.moveToEnemy;
		//MoveToPlayer
		Value_MoveToPlayer = mParamAI.playerLove;
		//RunAway
		Value_RunAway = mParamAI.runawayFromEnemy;
		//None
		Value_None = 1;

		//情報をもとに、行動できない選択肢を消していく
		//敵が隣にいない
		if (!mInformation.EnemyIsNextMass)
			Value_Attack_Normal = 0;
		//敵が近くにいない
		if (!mInformation.EnemyIsNearMass) {
			Value_MoveToEnemy = 0;
			Value_RunAway = 0;
		}
		//遠距離攻撃が使えない
		if (mInformation.NearTargetID == -1)
			Value_Attack_Magic = 0;
		//プレイヤーが近くにいない
		if (!mInformation.playerIsNearMass)
			Value_MoveToPlayer = 0;
		//逃げれる状況でない
		if (HP > mParamAI.runawayHP || !mInformation.CanRunAway)
			Value_RunAway = 0;
		//敵
		if (mInformation.youAreEnemyTeam)
			Value_MoveToPlayer = 0;

		//矛盾を解消
		//敵が隣にいて、攻撃できるのに移動しようとする場合
		if (Value_Attack_Normal > 0 && Value_MoveToEnemy > 0)
			Value_MoveToEnemy = 0;

		//最終的に、評価値をもとに行動を決定する
		int num = 6;
		int* tempArray = new int[num];
		tempArray[0] = Value_Attack_Normal;
		tempArray[1] = Value_Attack_Magic;
		tempArray[2] = Value_MoveToEnemy;
		tempArray[3] = Value_MoveToPlayer;
		tempArray[4] = Value_RunAway;
		tempArray[5] = Value_None;
		int max = -1000;
		int index = -1;
		for (int i = 0; i < num; i++)
		{
			if (max < tempArray[i]) {
				max = tempArray[i];
				index = i;
			}
		}
		
		//決定した選択に応じた行動を返す
		switch (index) {
		case 0:
			//通常攻撃
			mReply = Reply::REPLY_ATTACK_NORMAL;
			//方向合わせ
			switch (mInformation.direction) {
			case 1:
				mDirection = Direction::DIR_UP;
				break;
			case 2:
				mDirection = Direction::DIR_RIGHT;
				break;
			case 3:
				mDirection = Direction::DIR_DOWN;
				break;
			case 4:
				mDirection = Direction::DIR_LEFT;
				break;
			default:
				;
				//HALT("File: Character.cpp [selectOfMind1()] switch Error");
			}
			break;
		case 1:
			//魔法攻撃
			magicTargetID = mInformation.NearTargetID;
			mReply = Reply::REPLY_ATTACK_MAGIC;
			break;
		case 2:
			//敵に近づく
			mReply = Reply::REPLY_MOVE;
			//方向合わせ
			switch (mInformation.directionForNear) {
			case 1:
				mDirection = Direction::DIR_UP;
				break;
			case 2:
				mDirection = Direction::DIR_RIGHT;
				break;
			case 3:
				mDirection = Direction::DIR_DOWN;
				break;
			case 4:
				mDirection = Direction::DIR_LEFT;
				break;
			default:
				;
				//HALT("File: Character.cpp [selectOfMind1()] switch Error");
			}
			break;
			break;
		case 3:
			//プレイヤーに近づく
			mReply = Reply::REPLY_MOVE;
			//方向合わせ
			switch (mInformation.directionForPlayer) {
			case 1:
				mDirection = Direction::DIR_UP;
				break;
			case 2:
				mDirection = Direction::DIR_RIGHT;
				break;
			case 3:
				mDirection = Direction::DIR_DOWN;
				break;
			case 4:
				mDirection = Direction::DIR_LEFT;
				break;
			default:
				;
				//HALT("File: Character.cpp [selectOfMind1()] switch Error");
			}
			break;
			break;
		case 4:
			//逃げる
			mReply = Reply::REPLY_MOVE;
			//方向合わせ
			switch (mInformation.directionForRun) {
			case 1:
				mDirection = Direction::DIR_UP;
				break;
			case 2:
				mDirection = Direction::DIR_RIGHT;
				break;
			case 3:
				mDirection = Direction::DIR_DOWN;
				break;
			case 4:
				mDirection = Direction::DIR_LEFT;
				break;
			default:
				;
				//HALT("File: Character.cpp [selectOfMind1()] switch Error");
			}
			break;
		case 5:
			//何もしない
			mReply = Reply::REPLY_NONE;
			break;
		default:
			HALT("File: Character.cpp [selectOfMind1()] switch Error");
		}

		delete[] tempArray;
		tempArray = nullptr;

		mState = State::STATE_WAIT;
		return true;
	}
	default:
		HALT("File: Character.cpp [selectOfMind1()] switch Error");
		break;
	}

	return false;
}



//このクラス内のみ
void Character::toAnnouce(int messageNumber, State backTo)
{
	announceMessageNumber = messageNumber;
	backFromAnnounce = backTo;
	mState = State::STATE_ANNOUNCE;
}






//////////////////////////////////////////
/////////////パーツ関連///////////////////
//////////////////////////////////////////

Parts Parts::neededParts(Character::Reply rep)
{
	Parts result;

	switch (rep)
	{
		//落とし穴
	case Character::Reply::REPLY_MASS_TRAP_HOLE:
		result.Parts_Screw	= 2;
		result.Parts_Hammer = 1;
		result.Parts_Net	= 2;
		result.Parts_Stone	= 1;
		break;
		//捕獲罠
	case Character::Reply::REPLY_MASS_TRAP_CATCH:
		result.Parts_Screw	= 4;
		result.Parts_Hammer = 2;
		result.Parts_Net	= 4;
		result.Parts_Stone	= 2;
		break;
		//足止め
	case Character::Reply::REPLY_MASS_TRAP_STOPPER:
		result.Parts_Screw	= 0;
		result.Parts_Hammer = 2;
		result.Parts_Net	= 0;
		result.Parts_Stone	= 2;
		break;
		//地雷
	case Character::Reply::REPLY_MASS_TRAP_BOMB:
		result.Parts_Screw	= 3;
		result.Parts_Hammer	= 0;
		result.Parts_Net	= 0;
		result.Parts_Stone	= 4;
		break;
	case Character::Reply::REPLY_MASS_MAKE:
		result.Parts_Screw	= 1;
		result.Parts_Hammer = 1;
		result.Parts_Net	= 1;
		result.Parts_Stone	= 1;
		break;
	default:
		HALT("File:Game.cpp [neededParts] Error");
	}

	return result;
}

std::string Parts::getNameOfParts(int number) {
	switch (number) {
	case 0:
		return "Screw";
	case 1:
		return "Hammer";
	case 2:
		return "Net";
	case 3:
		return "Stone";
	default:
		return "Nothing";
	}
}

int Parts::getNumOfParts(int number) {
	switch (number) {
	case 0:
		return this->Parts_Screw;
	case 1:
		return this->Parts_Hammer;
	case 2:
		return this->Parts_Net;
	case 3:
		return this->Parts_Stone;
	default:
		return 0;
	}
}

bool Parts::hasSomethingParts() {
	for (int i = 0; i < numParts; ++i) {
		if (getNumOfParts(i) != 0)
			return true;
	}
	return false;
}

//足す
void Parts::add(Parts parts) {
	this->Parts_Screw += parts.Parts_Screw;
	this->Parts_Hammer += parts.Parts_Hammer;
	this->Parts_Net += parts.Parts_Net;
	this->Parts_Stone += parts.Parts_Stone;
}

//引く
void Parts::sub(Parts parts) {
	this->Parts_Screw -= parts.Parts_Screw;
	this->Parts_Hammer -= parts.Parts_Hammer;
	this->Parts_Net -= parts.Parts_Net;
	this->Parts_Stone -= parts.Parts_Stone;
}

//自身を数倍する
void Parts::mul(int num) {
	this->Parts_Screw *= num;
	this->Parts_Hammer *= num;
	this->Parts_Net *= num;
	this->Parts_Stone *= num;
}

//必要数足りているか判定
bool Parts::judge(Parts parts) {
	if (this->Parts_Screw < parts.Parts_Screw)
		return false;
	if (this->Parts_Hammer < parts.Parts_Hammer)
		return false;
	if (this->Parts_Net < parts.Parts_Net)
		return false;
	if (this->Parts_Stone < parts.Parts_Stone)
		return false;

	return true;
}

//モンスター用。ランダムにnumの数のパーツを持たせる
void Parts::setRandomParts(int num) {
	for (int i = 0; i < num; ++i) {
		int rand = MyRandom::instance()->getInt(0, 100);
		//それぞれのパーツの出現数は適当に決めている
		if (rand < 40)
			this->Parts_Screw += 1;
		else if (rand < 50)
			this->Parts_Hammer += 1;
		else if (rand < 70)
			this->Parts_Net += 1;
		else
			this->Parts_Stone += 1;
	}
}