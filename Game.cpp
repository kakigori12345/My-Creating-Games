#include "GameLib/GameLib.h"
#include "GameLib/Math/Matrix34.h"
#include "GameLib/Math/Matrix44.h"
#include "GameLib/Math/Vector3.h"
#include "Game.h"
#include "Action.h"
#include "ContentReader.h"
#include "MyRandom.h"
#include "FrontDisplayh.h"
#include "Button.h"
#include "EffectController.h"
#include <string>
#include "GameClear.h"
#include "GameOver.h"
#include "SoundGenerator.h"

using namespace GameLib;
using namespace GameLib::Math;
using namespace std;

Game::Game() :
	mFinished(false),
	victory(false),
	battleIsDone(false),
	createNextStageFlag(false),
	attackIsMagic(false),
	mSituation(NORMAL),
	mPreSituation(NORMAL),
	situationCount(0.f),
	StageNumber(1),
	playerID(0),
	messageNum(1),
	IDofFriendly(-1),
	IDofFalled(-1),
	nowGotParts(),
	TrapHoleIsDone(false),
	mStage(nullptr),
	mPC(nullptr),
	mMonCon(nullptr)
{
	//ファイルから定数を読み込む（ContentReaderクラスを使う）
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	Game::FINAL_STAGE_NUMBER = (int)cr->getData("Game", "FINAL_STAGE_NUMBER");
	SAFE_DELETE(cr);//使い終わったら削除
	//プレイキャラクターの作成
	mPC = new PlayCharacter();
	playerID = mPC->getID();
	//モンスター管理クラスの生成
	mMonCon = new MonsterController();
	//その他の全ての生成、初期化を行う
	createNewStage();
	SoundGenerator::instance()->playBgm(SoundGenerator::Bgm::GAME);
	mPC->getWeapon(Weapon::Type::Rod);
}

Game::~Game()
{
	SAFE_DELETE(mStage);
	SAFE_DELETE(mPC);
	SAFE_DELETE(mMonCon);
	mActiveCharacter = nullptr;
}

void Game::update()
{
	FrontDisplay* front = FrontDisplay::instance();
	Button* button = Button::instance();

	//定型処理
	if (!mPC->isLiving())
		mSituation = Situation::LOSE;

	//アクティブキャラと、存在するならパッシブキャラの更新
	if (mActiveCharacter == nullptr)
		HALT("File: Game.cpp [update()] mActiveCharacter is nullptr. Error");
	mActiveCharacter->update();
	if (mPassiveCharacter != nullptr)
		mPassiveCharacter->update();
	//ステージの更新
	mStage->update();
	mStage->updateMassParameter();
	mStage->showMiniMap();
	//情報を出す
	if (mSituation != Situation::SHOW_STAGE_NUM) {
		front->showParameter(mPC->getName(), mPC->getParameter()->health, mPC->getHP(), mPC->weapon());
		mMonCon->showFriendMonstersParameter();
		front->showParts(&mPC->getParts());
		front->showTurnLimit(mStage->getTurnLimit());
	}

	switch (mSituation) {
	//移動や攻撃などの通常処理
	case NORMAL: {
		mPreSituation = Situation::NORMAL;
		//戦闘処理
		if (battleIsDone) {
			//アクティブキャラクターが攻撃のタイミングになったら戦闘
			if (mActiveCharacter->getAttackIsDone()) {
				bool friendly = false;	//戦闘後の仲間判定用
				battle(friendly);		//戦闘
				battleIsDone = false;
				//仲間判定
				if (friendly)
					mSituation = MAKE_FRIEND;
			}
		}
		//アクティブキャラの交代チェック。交代が起こるときSituationがMENUに遷移
		//ついでにこの関数内で、次のステージへの遷移判定も行っている
		//行動が終わったかどうか
		if (mActiveCharacter->actFinished()) {
			//アクティブキャラの行動が終了したときにエフェクトの演出用パラメータをリセット
			EffectController::instance()->resetAllParameter();
			//同じくアクティブキャラの特定パラメータもリセット
			mActiveCharacter->resetAllParameters();
			//アクティブキャラの変更
			checkActiveCharacterChange();
		}
		break;
	}
	//キャラクターの行動選択フェーズ
	case MENU: {
		mPreSituation = Situation::MENU;
		//アクティブキャラに行動選択させる
		if (mActiveCharacter->selectAction())
			//コードが長くなったので別にまとめた
			menu();
		break;
	}
	//敵モンスターを仲間にするかどうか選択する
	case MAKE_FRIEND: {
		//仲間イベント発生のSEを鳴らす
		if (mPreSituation != Situation::MAKE_FRIEND)
			SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_OCCUR);
		mPreSituation = Situation::MAKE_FRIEND;

		//メッセージ表示
		switch (messageNum) {
		case 1:
			//モンスターが起き上がったことを知らせる
			front->showMessage(8);
			if (button->isTriggered(Button::KEY_SPACE))
				messageNum = 2;
			break;
		case 2: {
			//仲間にするかどうかを選択してもらう
			front->showMessage(9);
			int cursor = front->showCursor(2, FrontDisplay::Cursor::CURSOR_TEXT);
			if (button->isTriggered(Button::Key::KEY_SPACE)) {
				if (cursor == 1) {
					//仲間にする
					messageNum = 3;
					SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_GOOD);
				}
				else {
					//仲間にしない
					messageNum = 4;
					SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_BAD);
				}
			}
			break;
		}
		case 3:
			//仲間にする
			front->showMessage(10);
			if (button->isTriggered(Button::KEY_SPACE)) {
				//とりあえず、仲間上限に達していて仲間にできないなら殺す
				if (!mMonCon->makeFriend(IDofFriendly)) {
					killMonster(IDofFriendly, false);
				}
				else
					mStage->changeTeam(IDofFriendly);
				IDofFriendly = -1;
				messageNum = 1;
				mSituation = NORMAL;
			}
			break;
		case 4:
			//仲間にしないなら殺す
			front->showMessage(11);
			if (button->isTriggered(Button::KEY_SPACE)) {
				killMonster(IDofFriendly, true);
				IDofFriendly = -1;
				messageNum = 1;
				mSituation = NORMAL;
			}
			break;
		}
		break;
	}
	//ターン制限に引っかかった
	case TURN_LIMIT: {
		front->showMessage(14);
		if (button->isTriggered(Button::KEY_SPACE)) {
			mPC->turnLimitAction();
			mSituation = mPreSituation;
		}
		break;
	}
	//現在のステージ階層を表示する。階層を移動した際に数秒間だけ表示される
	case SHOW_STAGE_NUM: {
		mPreSituation = Situation::SHOW_STAGE_NUM;
		front->showStageNumber(StageNumber);
		situationCount += 0.05f;
		if (situationCount > 1.f) {
			situationCount = 0.f;
			mSituation = Situation::MENU;
		}
		if (situationCount > 0.9f && createNextStageFlag) {
			createNextStageFlag = false;
			createNewStage();
		}
		break;
	}
	//何かしらのメッセージを出す。直前のSituationに戻る
	case ANNOUNCEMENT: {
		front->showGotParts(nowGotParts);
		if (Button::instance()->isTriggered(Button::Key::KEY_SPACE))
			mSituation = mPreSituation;
		break;
	}
	case VICTORY: {
		victory = true;
		front->showMessage(18);
		if (Button::instance()->isTriggered(Button::Key::KEY_SPACE))
			mFinished = true;
		break;
	}
	case LOSE: {
		victory = false;
		front->showMessage(19);
		if (Button::instance()->isTriggered(Button::Key::KEY_SPACE))
			mFinished = true;
		break;
	}
	default:
		HALT("File::Game.cpp [update] Error");
		break;
	}
}

void Game::draw()
{
	mPC->draw();
	mStage->draw();
	mMonCon->draw();
}

void Game::sendInfoToCamera(
	Camera::Mode* mode,
	GameLib::Math::Vector3* playerPos,
	GameLib::Math::Vector3* enemyPos,
	float* eyeAngleYofPlayer,
	float* time) const
{
	switch (mSituation) {
	case Situation::NORMAL:
		//*mode = Camera::MODE_GAME_NORMAL;
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::MENU:
		//*mode = Camera::MODE_GAME_NORMAL;
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::MAKE_FRIEND:
		//*mode = Camera::MODE_GAME_NORMAL;
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::TURN_LIMIT:
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::SHOW_STAGE_NUM:
		//*mode = Camera::MODE_GAME_NORMAL;
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::ANNOUNCEMENT:
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::VICTORY:
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	case Situation::LOSE:
		*mode = Camera::TEST;
		*playerPos = *mPC->position();
		*enemyPos = Vector3(0.f, 0.f, 0.f);//todo
		*eyeAngleYofPlayer = mPC->angle();
		*time = 0.f;
		break;
	default:
		HALT("FILE:Game.cpp [sendInfoToCamera] Error");
		break;
	}
}

bool Game::isFinished() const
{
	return mFinished;
}

Sequence* Game::createNextSequence()
{
	//BGMを終了
	SoundGenerator::instance()->stopBgm();
	//勝利しているかどうかで分岐
	if (victory)
		return new GameClear();
	else
		return new GameOver();
}


Game::Situation Game::getSituation()
{
	return mSituation;
}



/////////////////////このクラス内でしか使わない関数/////////////////////

void Game::createNewStage()
{
	//初期化
	SAFE_DELETE(mStage);
	mActiveCharacter = nullptr;
	mPassiveCharacter = nullptr;
	mMonCon->resetEnemyMonster();

	//乱数を初期化
	MyRandom::instance()->resetRand();

	//ステージ番号を進める

	//ファイルから必要な情報を読み込む（ContentReaderクラスを使う）
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	//ステージの階層
	string stName = cr->getData_str("StageNumber", to_string(StageNumber).c_str());
	const char* chStName = stName.c_str();
	//モンスターの種類
	int NUM_OF_MONSTER_TYPE = (int)cr->getData(chStName, "NUM_OF_MONSTER_TYPE");
	//モンスターの種類に応じて、名前と数の配列を生成
	string* nameList = new string[NUM_OF_MONSTER_TYPE];
	int* numList = new int[NUM_OF_MONSTER_TYPE];
	//再度読み込んでいく
	for (int i = 0; i < NUM_OF_MONSTER_TYPE; ++i)
	{
		//文字列を用意。例えば、モンスターの種類が３種類だとしたら、次のような感じ
		//nameList = [ "NAME_OF_MONSTER1", "NAME_OF_MONSTER2", "NAME_OF_MONSTER3"];
		//numList = ["NUM_OF_MONSTER1", "NUM_OF_MONSTER2", "NUM_OF_MONSTER3"];
		//※中の定数はテキストファイル内に書いている
		string name = "NAME_OF_MONSTER" + to_string(i + 1);
		string num = "NUM_OF_MONSTER" + to_string(i + 1);
		//実際にファイルから読み出す
		nameList[i] = cr->getData_str(chStName, name.c_str());
		numList[i] = (int)cr->getData(chStName, num.c_str());
	}
	SAFE_DELETE(cr);//使い終わったら削除


	//生成。ここからが本命の処理
	mStage = new Stage(StageNumber);
	//プレイヤーのIDをStageに登録して座標をセットする
	{
		Vector3 pos;
		mStage->registerNewID(mPC->getID(), &pos, Stage::TEAM_PLAYER);
		mStage->registerPlayerID(mPC->getID());
		mPC->setPosition(pos);
	}
	
	//モンスターを生成していく。上でファイルから読み込んだ値を使う
	mMonCon->setNumOfMonsterType(NUM_OF_MONSTER_TYPE);
	for (int i = 0; i < NUM_OF_MONSTER_TYPE; ++i) {
		mMonCon->createMonster(nameList[i].c_str(), numList[i]);
	}
	//生成したモンスターのIDをStageに登録し、座標をセットする
	bool temp_first = true;
	int temp_id = 0;
	bool beFriend = false;
	while (true)
	{
		//1回目だけ
		if (temp_first) {
			temp_first = false;
			temp_id = mMonCon->getID(true, &beFriend);
		}
		else
			temp_id = mMonCon->getID(false, &beFriend);
		//ID探索が最後まで到達したら-1が返ってくるはず
		if (temp_id == -1)
			break;
		//StageクラスにIDを登録し、座標をもらう。それをモンスターにセット
		Vector3 pos;
		Stage::Team team;
		if (beFriend)
			team = Stage::Team::TEAM_PLAYER;
		else
			team = Stage::Team::TEAM_ENEMY;
		mStage->registerNewID(temp_id, &pos, team);
		mMonCon->setPos(temp_id, pos);
	}
	//その他初期設定
	mActiveCharacter = mPC;
	mSituation = Situation::SHOW_STAGE_NUM;
	mStage->updateMass();
}


void Game::checkActiveCharacterChange()
{
	SoundGenerator* sound = SoundGenerator::instance();
	/////////////マスに止まった時のイベント処理/////////////
	//エラーチェック
	ASSERT(mSituation == Situation::NORMAL);
	//直前に踏まれていたマスの更新
	mStage->updatePrevFootMass();
	//プレイヤーが階段上にいるなら次のステージへ
	if (playerOnTheGoal())
		return;
	//ボス部屋で敵が全滅しているなら次のステージへ
	if (mStage->thisStageIsBossRoom() && mStage->AllEnemyIsDied()) {
		gotoNextStage();
		return;
	}
	//アクティブキャラクターが特定マスに止まった時にイベントを割り込み処理
	switch (mStage->getPanelFromID(mActiveCharacter->getID())) {
	case Mass::Panel::WARP: {
		//移動後のワープ以外は認めない（ワープ後にワープマスに止まるなどは認めないということ）
		if (mActiveCharacter->getAct() != Character::ACT_MOVE)
			break;
		//目的地を設定
		Vector3 pos;
		if (!mStage->judgeWarp(mActiveCharacter->getID(), &pos))
			break;
		mActiveCharacter->setDestination(pos);
		mActiveCharacter->setAction(Character::Act::ACT_WARP);
		//AIのパラメータもリセット
		mActiveCharacter->resetAIparameter();
		return;
	}
	//トラップに関して。トラップは基本的にプレイヤーや仲間モンスターには無効
	case Mass::Panel::TRAP_HOLE: {
		//プレイヤーには無効
		if (mActiveCharacter->getID() == playerID)
			break;
		//仲間にも無効
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		//このif文がないと無限に落ち続ける
		if (mActiveCharacter->getAct() == Character::ACT_FALL)
			break;
		mActiveCharacter->setAction(Character::Act::ACT_FALL);
		IDofFalled = mActiveCharacter->getID();
		TrapHoleIsDone = true;
		mStage->changePanel(IDofFalled, Mass::Panel::HOLE);
		return;
	}
	case Mass::Panel::TRAP_CATCH: {
		//プレイヤーには無効
		if (mActiveCharacter->getID() == playerID)
			break;
		//仲間にも無効
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		//このif文がないと無限
		if (mActiveCharacter->getAct() == Character::ACT_CATCH) {
			break;
		}
		mActiveCharacter->setAction(Character::Act::ACT_CATCH);
		IDofCatched = mActiveCharacter->getID();
		TrapCatchIsDone = true;
		return;
	}
	case Mass::Panel::TRAP_STOPPER: {
		//プレイヤーには無効
		if (mActiveCharacter->getID() == playerID)
			break;
		//仲間にも無効
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		mActiveCharacter->setBeingStopped();
		SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_STOPPER);
		break;
	}
	case Mass::Panel::TRAP_BOMB: {
		//プレイヤーには無効
		if (mActiveCharacter->getID() == playerID)
			break;
		//仲間にも無効
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		IDofBombed = mActiveCharacter->getID();
		TrapBombIsDone = true;
		break;
	}
	case Mass::Panel::RECOVERY: {
		sound->playSe(SoundGenerator::Se::STAGE_MASS_RECOVERY);
		mActiveCharacter->recovery(20);
		mStage->changePanel(mActiveCharacter->getID(), Mass::Panel::NORMAL);
		break;
	}
	case Mass::Panel::PARTS: {
		//プレイヤーのみ有効
		if (mActiveCharacter->getID() != playerID)
			break;
		sound->playSe(SoundGenerator::Se::STAGE_MASS_PARTS);
		nowGotParts = Parts(5, 5, 5, 5);
		mPC->addParts(nowGotParts);
		mSituation = Situation::ANNOUNCEMENT;
		mStage->changePanel(mActiveCharacter->getID(), Mass::Panel::NORMAL);
		break;
	}
	}

	mActiveCharacter->setAction(Character::Act::ACT_WAIT);
	//mPassiveCharacterを空に
	if (mPassiveCharacter) {
		mPassiveCharacter->setAction(Character::Act::ACT_WAIT);
		mPassiveCharacter = nullptr;
	}

	/////////////アクティブキャラクターの変更/////////////
	//Menuへの遷移
	mSituation = Situation::MENU;
	//------アクティブキャラの更新の流れ------
	//ターンの移り変わり：プレイヤー　→　味方モンスター　→　敵モンスター
	//１：まずプレイヤーがアクティブだったかの確認
	//２：味方Monsterの番かの判定。そうでないなら3へ
	//３：敵Monsterの番かの判定。そうでないなら4へ
	//４：プレイヤーをアクティブキャラクターとしてセットする
	//------以下１～４のループ（アクティブチェックが通ったらループ終了）------
	while (true) {
		bool last = false;
		//1　直前に動いたのがプレイヤーか
		if (mActiveCharacter == mPC) {
			//ステージのターン制限に引っかかるか
			if (mStage->incTurnLimit() == 0) {
				mPreSituation = Situation::MENU;
				mSituation = Situation::TURN_LIMIT;
			}
			//モンスターの行動順をリセット
			mMonCon->resetActiveCounter();
		}
		//2　味方モンスターの番。全員が行動済みかどうか
		mActiveCharacter = mMonCon->returnActiveFriendPointer(&last);
		if (!last) {
			//3　敵モンスターの番。全員が行動済みかどうか
			mActiveCharacter = mMonCon->returnActiveEnemyPointer(&last);
			if (!last)
				//4　プレイヤーの番
				mActiveCharacter = mPC;
		}
		//アクティブチェック
		//アクティブキャラクターが足止め状態ならターンを飛ばす
		if (mActiveCharacter->getBeStopped()) {
			SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_STOPPER);
			//足止めカウントを進めてカウントが０になったら足止め状態を解除。ただし動けるのは次のターンから
			if (mStage->decCountOfTrapStopper(mActiveCharacter->getID())) {
				mActiveCharacter->resetBeingStopped();
				//罠：ストッパーも撤去しておく
				mStage->changePanel(mActiveCharacter->getID(), Mass::Panel::NORMAL);
			}
			continue;
		}
		//ここまできたらアクティブチェックが通ったということ。ループから抜ける
		break;
	}

	//最後に。アクティブキャラクターを変更した後で行うべき処理を済ませる
	//落とし穴判定
	if (TrapHoleIsDone)
	{
		TrapHoleIsDone = false;
		killMonster(IDofFalled, false);
	}
	//捕獲罠判定
	else if (TrapCatchIsDone)
	{
		TrapCatchIsDone = false;
		//捕獲動作が終わった後に捕獲罠を撤去して、ノーマルマスにする
		mStage->changePanel(IDofCatched, Mass::Panel::NORMAL);
		//対象を仲間にするか選択する
		IDofFriendly = IDofCatched;
		mSituation = Situation::MAKE_FRIEND;
	}
	//地雷判定
	else if (TrapBombIsDone)
	{
		TrapBombIsDone = false;
		//SE
		SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_BOMB);
		//地雷撤去
		mStage->changePanel(IDofBombed, Mass::Panel::NORMAL);
		//エフェクト表示
		Vector3 effectPos = *mStage->getPositionFromID(IDofBombed);
		EffectController::instance()->createEffect(EffectController::Type::EXPLOSION, effectPos, true);
		//ダメージ処理を加える
		setDamage(false, IDofBombed);
	}
}

void Game::menu()
{
	Vector3 pos;
	Character::Reply rep = mActiveCharacter->getSelect();
	//Characterの指定マスと、Stageのターゲットマスを合致させる。同じ意味を変換しているだけで重要でない
	Stage::TargetMass targetmass;
	switch (mActiveCharacter->getDirection()) {
	case Character::Direction::DIR_RIGHT:
		targetmass = Stage::TargetMass::TARGET_RIGHT;
		break;
	case Character::Direction::DIR_LEFT:
		targetmass = Stage::TargetMass::TARGET_LEFT;
		break;
	case Character::Direction::DIR_UP:
		targetmass = Stage::TargetMass::TARGET_UP;
		break;
	case Character::Direction::DIR_DOWN:
		targetmass = Stage::TargetMass::TARGET_DOWN;
		break;
	}
	//アクション。選択されたアクションが無効なら、選択からやり直しさせる
	switch (rep) {
	//情報要求
	case Character::Reply::REPLY_WANT_INFORMATION: {
		bool playerIsNear = false;
		bool enemyIsNext = false;
		bool enemyIsNear = false;
		bool canRunAway = false;
		int nearTargetID = -1;
		int searchDistance = mActiveCharacter->canAttackToNear();
		Stage::TargetMass targetNext = Stage::TargetMass::TARGET_UP;
		Stage::TargetMass targetNear = Stage::TargetMass::TARGET_UP;
		Stage::TargetMass targetPlayer = Stage::TargetMass::TARGET_UP;
		Stage::TargetMass targetRunAway = Stage::TargetMass::TARGET_UP;
		mStage->information(mActiveCharacter->getID(), playerIsNear, enemyIsNext, enemyIsNear, canRunAway, 
			targetNext, targetNear, targetPlayer, targetRunAway, searchDistance, nearTargetID);

		//ステージからもらった情報をアクティブキャラクターに渡す用に作り直す
		Information info;
		//敵かどうか
		if (!mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			info.youAreEnemyTeam = true;
		else
			info.youAreEnemyTeam = false;
		info.playerIsNearMass = playerIsNear;
		info.EnemyIsNextMass = enemyIsNext;
		info.EnemyIsNearMass = enemyIsNear;
		info.CanRunAway = canRunAway;
		info.NearTargetID = nearTargetID;
		switch (targetNext) {
		case Stage::TargetMass::TARGET_UP:
			info.direction = 1;
			break;
		case Stage::TargetMass::TARGET_RIGHT:
			info.direction = 2;
			break;
		case Stage::TargetMass::TARGET_DOWN:
			info.direction = 3;
			break;
		case Stage::TargetMass::TARGET_LEFT:
			info.direction = 4;
			break;
		default:
			HALT("File:Game.cpp [update()] case Character::Reply::REPLY_WANT_INFORMATION: switch Error");
			break;
		}
		switch (targetNear) {
		case Stage::TargetMass::TARGET_UP:
			info.directionForNear = 1;
			break;
		case Stage::TargetMass::TARGET_RIGHT:
			info.directionForNear = 2;
			break;
		case Stage::TargetMass::TARGET_DOWN:
			info.directionForNear = 3;
			break;
		case Stage::TargetMass::TARGET_LEFT:
			info.directionForNear = 4;
			break;
		default:
			HALT("File:Game.cpp [update()] case Character::Reply::REPLY_WANT_INFORMATION: switch Error");
			break;
		}
		switch (targetPlayer) {
		case Stage::TargetMass::TARGET_UP:
			info.directionForPlayer = 1;
			break;
		case Stage::TargetMass::TARGET_RIGHT:
			info.directionForPlayer = 2;
			break;
		case Stage::TargetMass::TARGET_DOWN:
			info.directionForPlayer = 3;
			break;
		case Stage::TargetMass::TARGET_LEFT:
			info.directionForPlayer = 4;
			break;
		default:
			HALT("File:Game.cpp [update()] case Character::Reply::REPLY_WANT_INFORMATION: switch Error");
			break;
		}
		switch (targetRunAway) {
		case Stage::TargetMass::TARGET_UP:
			info.directionForRun = 1;
			break;
		case Stage::TargetMass::TARGET_RIGHT:
			info.directionForRun = 2;
			break;
		case Stage::TargetMass::TARGET_DOWN:
			info.directionForRun = 3;
			break;
		case Stage::TargetMass::TARGET_LEFT:
			info.directionForRun = 4;
			break;
		default:
			HALT("File:Game.cpp [update()] case Character::Reply::REPLY_WANT_INFORMATION: switch Error");
			break;
		}
		//アクティブキャラクターに渡す
		mActiveCharacter->setInformation(info);
		mActiveCharacter->setState(Character::State::STATE_THINKING);
		break;
	}
	//マス移動
	case Character::Reply::REPLY_MOVE: {
		//Stageクラスに移動判定を行ってもらう。有効なら移動先座標を返してもらう
		//StageのIDと座標は、if文内の関数で処理されている
		int passiveID = -1;
		if (mStage->judgeMove(mActiveCharacter->getID(), targetmass, &pos, &passiveID))
		{
			//目的地を設定
			mActiveCharacter->actEffect(Character::Reply::REPLY_MOVE);
			mActiveCharacter->setDestination(pos);
			mActiveCharacter->setAction(Character::ACT_MOVE);
			//マスの入れ替えが起こるなら
			if (passiveID != -1) {
				mPassiveCharacter = mMonCon->getMonsterFromID(passiveID);
				mPassiveCharacter->actEffect(Character::Reply::REPLY_MOVE);
				mPassiveCharacter->setDestination(*mActiveCharacter->position());
				mPassiveCharacter->setAction(Character::ACT_MOVE);
			}
			//ゲームのシチュエーション変化
			mSituation = Game::Situation::NORMAL;
		}
		else {
			mActiveCharacter->moveFault();
		}
		break;
	}
	//通常攻撃
	case Character::Reply::REPLY_ATTACK_NORMAL: {
		battleIsDone = true;
		mActiveCharacter->setDestination(*mStage->returnPlaceFromIDandTarget(mActiveCharacter->getID(), targetmass));
		mActiveCharacter->setAction(Character::ACT_ATTACK_NORMAL);
		mSituation = Game::Situation::NORMAL;
		break;
	}
	//遠距離魔法攻撃
	case Character::Reply::REPLY_ATTACK_MAGIC: {
		//プレイヤーの場合、判定が必要
		if (mActiveCharacter->getID() == playerID && !Button::instance()->demo()) {
			int x(0), y(0);
			mActiveCharacter->getMagicTargetPlace(&x, &y);
			int magicTarget = mStage->getIDFromIDandMassDiff(playerID, x, y);
			if (magicTarget == -1 || magicTarget == playerID) {
				mActiveCharacter->setState(Character::State::STATE_ANNOUNCE);
				return;
			}
			mActiveCharacter->setMagicTargetID(magicTarget);
		}
		//通常攻撃とやっていることはそんなに変わらない
		battleIsDone = true;
		attackIsMagic = true;
		mActiveCharacter->setDestination(*mStage->returnPlaceFromID(mActiveCharacter->getMagicTargetID()));
		mActiveCharacter->setAction(Character::ACT_ATTACK_MAGIC);
		mSituation = Game::Situation::NORMAL;
		break;
	}
	//ステージ上に落とし穴をつくる（プレイヤー専用）
	case Character::Reply::REPLY_MASS_TRAP_HOLE: {
		//Stageクラスにマスへの干渉が可能かを判定してもらう
		//パーツが足りているか確認
		Parts needed = Parts::neededParts(Character::REPLY_MASS_TRAP_HOLE);
		if (!mPC->judgeParts(needed))
			break;
		if (mStage->judgeMassChange(mActiveCharacter->getID(), targetmass, Mass::Panel::TRAP_HOLE))
		{
			mPC->subParts(needed);
			mActiveCharacter->setAction(Character::ACT_WAIT);
			mSituation = Game::Situation::NORMAL;
		}
		break;
	}
	//ステージ上に捕獲罠をつくる（プレイヤー専用）
	case Character::Reply::REPLY_MASS_TRAP_CATCH: {
		//上のケースと同様
		Parts needed = Parts::neededParts(Character::REPLY_MASS_TRAP_CATCH);
		if (!mPC->judgeParts(needed))
			break;
		if (mStage->judgeMassChange(mActiveCharacter->getID(), targetmass, Mass::Panel::TRAP_CATCH))
		{
			mPC->subParts(needed);
			mActiveCharacter->setAction(Character::ACT_WAIT);
			mSituation = Game::Situation::NORMAL;
		}
		break;
	}
	//ステージ上に足止め罠をつくる（プレイヤー専用）
	case Character::Reply::REPLY_MASS_TRAP_STOPPER: {
		//上のケースと同様
		Parts needed = Parts::neededParts(Character::REPLY_MASS_TRAP_STOPPER);
		if (!mPC->judgeParts(needed))
			break;
		if (mStage->judgeMassChange(mActiveCharacter->getID(), targetmass, Mass::Panel::TRAP_STOPPER))
		{
			mPC->subParts(needed);
			mActiveCharacter->setAction(Character::ACT_WAIT);
			mSituation = Game::Situation::NORMAL;
		}
		break;
	}
	//ステージ上に地雷をつくる（プレイヤー専用）
	case Character::Reply::REPLY_MASS_TRAP_BOMB: {
		//上のケースと同様
		Parts needed = Parts::neededParts(Character::REPLY_MASS_TRAP_BOMB);
		if (!mPC->judgeParts(needed))
			break;
		if (mStage->judgeMassChange(mActiveCharacter->getID(), targetmass, Mass::Panel::TRAP_BOMB))
		{
			mPC->subParts(needed);
			mActiveCharacter->setAction(Character::ACT_WAIT);
			mSituation = Game::Situation::NORMAL;
		}
		break;
	}
	//穴あきますに新しいマスを作成する（プレイヤー専用）
	case Character::Reply::REPLY_MASS_MAKE: {
		//上のケースと同様
		Parts needed = Parts::neededParts(Character::REPLY_MASS_MAKE);
		if (!mPC->judgeParts(needed))
			break;
		if (mStage->judgeMassChange(mActiveCharacter->getID(), targetmass, Mass::Panel::NORMAL))
		{
			mPC->subParts(needed);
			mActiveCharacter->setAction(Character::ACT_WAIT);
			mSituation = Game::Situation::NORMAL;
		}
		break;
	}
	//何もしない
	case Character::Reply::REPLY_NONE: {
		mSituation = Game::Situation::NORMAL;
		break;
	}
	///////ここからはプレイヤーのAI用のもの////////////
	case Character::Reply::REPLY_GOAL_PATH_PLANNNING: {
		mStage->doPathPlanning(mActiveCharacter->getID(), Mass::Panel::GOAL);
		mActiveCharacter->pathPlanningWasFinish();
		break;
	}
	case Character::Reply::REPLY_GOAL_ROOT: {
		PlayerAIinfo AIinfo;
		Stage::TargetMass target;
		mStage->getPath(target);
		switch (target) {
		case Stage::TargetMass::TARGET_UP:
			AIinfo.directionForGoal = 1;
			break;
		case Stage::TargetMass::TARGET_RIGHT:
			AIinfo.directionForGoal = 2;
			break;
		case Stage::TargetMass::TARGET_DOWN:
			AIinfo.directionForGoal = 3;
			break;
		case Stage::TargetMass::TARGET_LEFT:
			AIinfo.directionForGoal = 4;
			break;
		case Stage::TargetMass::TARGET_NONE:
			AIinfo.destinationArrived = true;
			break;
		default:
			HALT("File:Game.cpp [update()] case Character::Reply::GOAL_ROOT: switch Error");
			break;
		}
		//セット
		mActiveCharacter->setPlayerAIinfo(AIinfo);
		break;
	}

	default:
		HALT("File:Game.cpp [update()] case:MENU switch Error");
		break;
	}
}

bool Game::playerOnTheGoal()
{
	//プレイヤーが階段の上にいるなら、次のステージへ
	//ここで、階段のうえにいるなら問答無用で階層を進めているが、
	//プレイヤー側がそれを選択できるようにしたい todo:
	if (mStage->getPanelFromID(mPC->getID()) == Mass::GOAL) {
		gotoNextStage();
		return true;
	}

	return false;
}

void Game::gotoNextStage()
{
	//必要な初期化
	mActiveCharacter->setAction(Character::Act::ACT_WAIT);
	//最終階層ならこれでクリア
	if (StageNumber == FINAL_STAGE_NUMBER) {
		//とりあえず終了
		mSituation = Situation::VICTORY;
		return;
	}
	//次のステージを生成
	createNextStageFlag = true;
	StageNumber++;
	mSituation = Situation::SHOW_STAGE_NUM;
	mPC->resetAIparameter();
}

void Game::battle(bool& enemyWantToBeFriend)
{
	Stage::TargetMass targetmass;
	switch (mActiveCharacter->getDirection()) {
	case Character::Direction::DIR_RIGHT:
		targetmass = Stage::TargetMass::TARGET_RIGHT;
		break;
	case Character::Direction::DIR_LEFT:
		targetmass = Stage::TargetMass::TARGET_LEFT;
		break;
	case Character::Direction::DIR_UP:
		targetmass = Stage::TargetMass::TARGET_UP;
		break;
	case Character::Direction::DIR_DOWN:
		targetmass = Stage::TargetMass::TARGET_DOWN;
		break;
	}
	//Stageクラスに攻撃マスにいるキャラクターのIDを教えてもらう。いないなら-1が返ってくるはず
	//本来、攻撃対象は複数になることもあるはず。いまはとりあえず対象は一人としているが、
	//今後改良して複数対象に対応する todo:
	int targetID(0), targetIDnum(0);
	//隣接するマスへの攻撃
	if (!attackIsMagic) {
		mStage->getIDs(mActiveCharacter->getID(), targetmass, &targetID, &targetIDnum);
	}
	//遠距離攻撃
	else {
		targetID = mActiveCharacter->getMagicTargetID();
		targetIDnum = 1;
		attackIsMagic = false;
	}
	//ダメージ処理。プレイヤーと他で処理を分けている（プレイキャラクターのIDは別に保存してある）
	int powerOfAttacker = mActiveCharacter->getParameter()->power;
	for (int i = 0; i < targetIDnum; ++i)
	{
		if (targetID == -1)
			continue;
		//被攻撃対象がプレイヤー
		if (targetID == playerID) {
			mPC->damage(powerOfAttacker);
		}
		//被攻撃対象がモンスター
		else {
			//モンスターへのダメージ処理。死んだらIDもリセットする
			bool attackerIsPlayer = (mActiveCharacter->getID() == playerID);
			bool died = false;
			bool beFriend = false;
			mMonCon->setDamage(targetID, powerOfAttacker, attackerIsPlayer, died, beFriend);
			//モンスターが死んだ
			if (died) {
				//仲間になりたそうだ
				if (beFriend) {
					enemyWantToBeFriend = true;
					IDofFriendly = targetID;
				}
				//仲間にはならない、もしくは元々仲間だったもの
				else {
					bool flag = !mMonCon->getFriendlyByID(targetID);
					killMonster(targetID, flag);
				}
			}
		}
	}
}

//戦闘以外でのダメージ処理。敵を倒しても仲間判定は行われない
void Game::setDamage(bool passiveIsPlayer, int id)
{
	if (passiveIsPlayer)
	{
		//とりあえずパワー１００のダメージを食らわせる
		mPC->damage(100);
	}
	else
	{
		bool died = false;		//死んだか
		bool beFriend = false;	//仲間になるか。ここでは使わない
		mMonCon->setDamage(id, 100, false, died, beFriend);
		//モンスターが死んだらこの場合そのまま殺す
		if (died) {
			killMonster(id, false);
		}
	}
}

//モンスターを殺すときの処理をまとめたもの
void Game::killMonster(int target, bool partsWillBeGot)
{
	//敵からパーツをゲットできるかどうか
	if (partsWillBeGot) {
		nowGotParts = mMonCon->getParts(target);
		if (nowGotParts.hasSomethingParts()) {
			mPC->addParts(nowGotParts);
			mSituation = Situation::ANNOUNCEMENT;
		}
	}
	//モンスターコントローラー内からの消去
	mMonCon->killMonster(target);
	//ステージに登録してあるリストからの消去
	mStage->deleteID(target);
}