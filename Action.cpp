#include "GameLib/Framework.h"
#include "GameLib/Math/Functions.h"
#include "Character.h"
#include "Action.h"
#include "ContentReader.h"
#include "EffectController.h"

using namespace GameLib;
using namespace GameLib::Math;

Action* Action::mInstance = nullptr;

Action::Action()
{
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	ACT_WARP_HEIGHT				= cr->getData("Action", "ACT_WARP_HEIGHT");
	ACT_ATTACK_NORMAL_TIME_1	= cr->getData("Action", "ACT_ATTACK_NORMAL_TIME_1");
	ACT_ATTACK_NORMAL_TIME_2	= cr->getData("Action", "ACT_ATTACK_NORMAL_TIME_2");
	SAFE_DELETE(cr);//使い終わったら削除
}

Action::~Action()
{

}

void Action::create() {
	ASSERT(!mInstance);
	mInstance = new Action();
}

void Action::destroy() {
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

Action* Action::instance()
{
	return mInstance;
}

//受け取ったpositionを変更する
void Action::update(const Vector3& target, Vector3& position, const Vector3& prePos, Character::Act act, bool& attackTiming, float time)
{
	//エフェクト
	EffectController* effect = EffectController::instance();

	switch (act) {
	case Character::ACT_MOVE: {
		Vector3 diff;
		diff.setSub( target, prePos );
		diff *= time;
		Vector3 result;
		result.setAdd(prePos, diff);
		position = result;
		break;
	}
	case Character::ACT_WARP: {
		//x,z平面は２地点の差分を取って、ACT_MOVEと同様に処理する
		//y方向（高さ）はsin関数を使う
		Vector3 diff;
		diff.setSub(target, prePos);
		diff *= time;
		Vector3 result;
		result.setAdd(prePos, diff);
		if (time < 1.f)
			result.y = ACT_WARP_HEIGHT * GameLib::Math::sin(180.f * time);
		else
			result.y = 0.f;
		position = result;
		break;
	}
	case Character::ACT_ATTACK_NORMAL: {
		//方向を変えて、対象の方に頭突きしてから元の位置に戻る。targetは元居た場所
		if (time < ACT_ATTACK_NORMAL_TIME_1) {
			//何もしない
		}
		else if (time < ACT_ATTACK_NORMAL_TIME_2) {
			//攻撃対象に対して、ジャンプしながらタックル
			Vector3 diff;
			float timeRatio = (time - ACT_ATTACK_NORMAL_TIME_1) / (ACT_ATTACK_NORMAL_TIME_2 - ACT_ATTACK_NORMAL_TIME_1);
			diff.setSub(target, prePos);
			diff *= timeRatio;
			Vector3 result;
			result.setAdd(prePos, diff);
			result.y = 4.f * GameLib::Math::sin(180.f * timeRatio);
			position = result;
		}
		else {
			attackTiming = true;
			//元の位置に戻る
			Vector3 diff;
			float timeRatio = (time - ACT_ATTACK_NORMAL_TIME_2) / (1.f - ACT_ATTACK_NORMAL_TIME_2);
			diff.setSub(prePos, target);
			diff *= timeRatio;
			Vector3 result;
			result.setAdd(target, diff);
			position = result;
		}
		break;
	}
	case Character::ACT_ATTACK_MAGIC: {
		//方向を変えて、対象の方に頭突きしてから元の位置に戻る。targetは元居た場所
		if (time < ACT_ATTACK_NORMAL_TIME_1) {
			//何もしない
		}
		else if (time < ACT_ATTACK_NORMAL_TIME_2) {
			//攻撃対象に対して、ジャンプしながらタックル
			Vector3 diff;
			float timeRatio = (time - ACT_ATTACK_NORMAL_TIME_1) / (ACT_ATTACK_NORMAL_TIME_2 - ACT_ATTACK_NORMAL_TIME_1);
			diff.setSub(target, prePos);
			diff *= timeRatio;
			Vector3 result;
			result.setAdd(prePos, diff);
			//result.y = 2.f * GameLib::Math::sin(720.f * timeRatio);
			Vector3 angle;
			angle.y = effect->getEyeAngle();
			effect->show(EffectController::Type::MAGIC_ATTACK, result, angle, 1);
		}
		else {
			attackTiming = true;
			//対象に当たった時にはじける
			Vector3 angle;
			angle.y = effect->getEyeAngle();
			effect->show(EffectController::Type::MAGIC_ATTACK, target, angle, 2);
		}
		break;
	}
	case Character::ACT_FALL: {
		//ただ落下するだけ
		position.y -= 10.f * time;
		break;
	}
	case Character::ACT_CATCH: {
		//じたばたする
		position.y = 1.f * GameLib::Math::sin(1080 * time);
		break;
	}
	default:
		HALT("File Action.cpp [update] Error");
	}
}