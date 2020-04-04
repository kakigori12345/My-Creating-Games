#include "GameLib/GameLib.h"
#include "GameLib/Scene/Container.h"
#include "GameLib/Math/Matrix34.h"
#include "EffectController.h"
#include "ContentReader.h"

using namespace GameLib;
using namespace GameLib::Scene;
using namespace GameLib::Math;

EffectController* EffectController::mInstance = nullptr;

EffectController::EffectController() :
	mNowOneEffect(NOTHING),
	mOneEffectPosition(0.f, 0.f, 0.f),
	mOneEffectAngle(0.f, 0.f, 0.f),
	mOneEffectScale(1.f, 1.f, 1.f),
	mEyeAngle(0.f),
	mIncAngleOfMagicAttack(0.f),
	mIncScaleOfMagicAttack(0.f),
	mIncPositionOfMagicAttack(0.f)
{
	//モデル情報を読み込む
	mContainer = Container::create("data/model/effect.txt");
	while (!mContainer.isReady()) { ; }
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	MAX_EFFECT_NUM			= (int)cr->getData("Effect", "MAX_EFFECT_NUM");
	LIFETIME_EXPLOSION		= cr->getData("Effect", "LIFETIME_EXPLOSION");
	SCALE_EXPLOSION			= cr->getData("Effect", "SCALE_EXPLOSION");
	SCALE_MAGIC_ATTACK		= cr->getData("Effect", "SCALE_MAGIC_ATTACK");
	INC_INTERVAL_ANGLE_MAGIC		= cr->getData("Effect", "INC_INTERVAL_ANGLE_MAGIC");
	INC_INTERVAL_SCALE_MAGIC		= cr->getData("Effect", "INC_INTERVAL_SCALE_MAGIC");
	INC_INTERVAL_POSITION_MAGIC		= cr->getData("Effect", "INC_INTERVAL_POSITION_MAGIC");
	SCALE_MAGIC_TARGET		= cr->getData("Effect", "SCALE_MAGIC_TARGET");
	SAFE_DELETE(cr);//使い終わったら削除
	//一定数のEffectクラスのポインタを保持
	mEffects = new Effect*[MAX_EFFECT_NUM];
	for (int i = 0; i < MAX_EFFECT_NUM; ++i)
		mEffects[i] = nullptr;//NULLを入れておく
	//show()用のEffect生成
	mOneEffect = new Effect();
}

EffectController::~EffectController()
{
	for (int i = 0; i < MAX_EFFECT_NUM; ++i)
		SAFE_DELETE(mEffects[i]);
	SAFE_DELETE(mOneEffect);
}

void EffectController::create()
{
	ASSERT(!mInstance);
	mInstance = new EffectController();
}

void EffectController::destroy()
{
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

EffectController* EffectController::instance()
{
	return mInstance;
}

void EffectController::draw()
{
	for (int i = 0; i < MAX_EFFECT_NUM; ++i) {
		if (mEffects[i] != nullptr) {
			if (mEffects[i]->isLiving()) {
				mEffects[i]->update();//Effectのupdateも読んで更新しておく
				if (mEffects[i]->billBoard())
					mEffects[i]->setEyeAngle(mEyeAngle);
				mEffects[i]->draw();
			}
			else //死んでるなら
				SAFE_DELETE(mEffects[i]);
		}
	}
	if (mOneEffectIsOn) {//ONのときだけ描画
		switch (mNowOneEffect) {
		case Type::TITLE:
			break;
		case Type::GAME_CLEAR:
			break;
		case Type::GAME_OVER:
			break;
		case EffectController::Type::EXPLOSION:
			break;
		case EffectController::Type::MAGIC_ATTACK: {
			mOneEffectAngle.z += mIncAngleOfMagicAttack;
			float tempScale = SCALE_MAGIC_ATTACK + mIncScaleOfMagicAttack;
			mOneEffectScale.set(tempScale, tempScale, tempScale);
			mOneEffectPosition.x -= mIncPositionOfMagicAttack;
			break;
		}
		case EffectController::Type::MAGIC_TARGET:
			break;
		default:
			HALT("File: EffectController.cpp [draw()] switch Error");
		}
		mOneEffect->draw(mOneEffectPosition, mOneEffectAngle, mOneEffectScale);
	}
	mOneEffectIsOn = false;
}

void EffectController::createEffect(Type type, const Vector3& position, bool billboard)
{
	//空のポインタを探してそこに生成
	int index = 0;
	for (index = 0; index < MAX_EFFECT_NUM; ++index) {
		if (mEffects[index] == nullptr)
			break;
	}
	if (index == MAX_EFFECT_NUM)
		HALT("Effect create Error");

	//種類によってエフェクトの生存時間を変える
	const char* name = "";
	Vector3 scale;
	float lifetime;
	switch (type) {
	case EXPLOSION:
		name = "explosion";
		scale.set(SCALE_EXPLOSION, SCALE_EXPLOSION, SCALE_EXPLOSION);
		lifetime = LIFETIME_EXPLOSION;
		break;
	default:
		HALT("Effect create Error. That's type is not exist");
		break;
	}

	mEffects[index] = new Effect();
	mEffects[index]->create(mContainer,  name, position, scale, lifetime);
	if (billboard)
		mEffects[index]->setBillBoard();
}

void EffectController::show(Type type, const Vector3& pos, const Vector3& angle, int patternPhase)
{
	//この中は前と違うエフェクトが呼ばれたときにしか実行されない
	//mOneEffectに格納するモデルを更新する
	if (type != mNowOneEffect) {
		switch (type) {
		case TITLE:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "title");
			mOneEffectScale.set(2.f, 2.f, 2.f);
			mNowOneEffect = TITLE;
			break;
		case GAME_CLEAR:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "game_clear");
			mOneEffectScale.set(1.f, 1.f, 1.f);
			mNowOneEffect = GAME_CLEAR;
			break;
		case GAME_OVER:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "game_over");
			mOneEffectScale.set(1.f, 1.f, 1.f);
			mNowOneEffect = GAME_OVER;
			break;
		case EXPLOSION:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "explosion");
			mOneEffectScale.set(SCALE_EXPLOSION, SCALE_EXPLOSION, SCALE_EXPLOSION);
			mNowOneEffect = EXPLOSION;
			break;
		case MAGIC_ATTACK:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "magic_attack");
			mOneEffectScale.set(SCALE_MAGIC_ATTACK, SCALE_MAGIC_ATTACK, SCALE_MAGIC_ATTACK);
			mNowOneEffect = MAGIC_ATTACK;
			break;
		case MAGIC_TARGET:
			SAFE_DELETE(mOneEffect);
			mOneEffect = new Effect();
			mOneEffect->set(mContainer, "magic_target");
			mOneEffectScale.set(SCALE_MAGIC_TARGET, SCALE_MAGIC_TARGET, SCALE_MAGIC_TARGET);
			mNowOneEffect = MAGIC_TARGET;
			break;
		case NOTHING:
			HALT("EffectController show() ERROR");
			break;
		default:
			HALT("File: EffectController.cpp [show()] switch1 Error");
		}
	}
	//全てのエフェクトで共通する処理。個別に必要な処理は以降で行う
	mOneEffectPosition = pos;
	mOneEffectAngle = angle;
	mOneEffectIsOn = true;

	//演出
	switch (mNowOneEffect) {
	case Type::TITLE:
		break;
	case Type::GAME_CLEAR:
		break;
	case Type::GAME_OVER:
		break;
	case Type::EXPLOSION:
		break;
	case Type::MAGIC_ATTACK:
		if (patternPhase == 1) {
			mIncAngleOfMagicAttack += INC_INTERVAL_ANGLE_MAGIC;
		}
		else if (patternPhase == 2) {
			mIncScaleOfMagicAttack += INC_INTERVAL_SCALE_MAGIC;
			mIncPositionOfMagicAttack += INC_INTERVAL_POSITION_MAGIC;
		}
		break;
	case Type::MAGIC_TARGET:
		mOneEffectPosition.z += 1.f;
		break;
	default:
		HALT("File: EffectController.cpp [show()] switch2 Error");
	}
}

//Cameraクラスから視点方向を受け取る用
void EffectController::setEyeAngle(const float angle)
{
	mEyeAngle = angle;
}

//Cammeraクラスから受け取った視点を、別のところに参照してもらう。カメラはグローバルに参照できないが、このクラスはシングルトンでグローバルに参照可能
float EffectController::getEyeAngle() const
{
	return mEyeAngle;
}

//外部から呼び出せる関数かつ特定の変数を全てリセットする
//主に、エフェクトの挙動を管理するための変数をリセット
//キャラクターの動作が一区切りついたときにこの関数を呼ぶとよい
void EffectController::resetAllParameter()
{
	mIncAngleOfMagicAttack = 0.f;
	mIncScaleOfMagicAttack = 0.f;
	mIncPositionOfMagicAttack = 0.f;
}
