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
	//���f������ǂݍ���
	mContainer = Container::create("data/model/effect.txt");
	while (!mContainer.isReady()) { ; }
	//�萔�̓ǂݍ���
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	MAX_EFFECT_NUM			= (int)cr->getData("Effect", "MAX_EFFECT_NUM");
	LIFETIME_EXPLOSION		= cr->getData("Effect", "LIFETIME_EXPLOSION");
	SCALE_EXPLOSION			= cr->getData("Effect", "SCALE_EXPLOSION");
	SCALE_MAGIC_ATTACK		= cr->getData("Effect", "SCALE_MAGIC_ATTACK");
	INC_INTERVAL_ANGLE_MAGIC		= cr->getData("Effect", "INC_INTERVAL_ANGLE_MAGIC");
	INC_INTERVAL_SCALE_MAGIC		= cr->getData("Effect", "INC_INTERVAL_SCALE_MAGIC");
	INC_INTERVAL_POSITION_MAGIC		= cr->getData("Effect", "INC_INTERVAL_POSITION_MAGIC");
	SCALE_MAGIC_TARGET		= cr->getData("Effect", "SCALE_MAGIC_TARGET");
	SAFE_DELETE(cr);//�g���I�������폜
	//��萔��Effect�N���X�̃|�C���^��ێ�
	mEffects = new Effect*[MAX_EFFECT_NUM];
	for (int i = 0; i < MAX_EFFECT_NUM; ++i)
		mEffects[i] = nullptr;//NULL�����Ă���
	//show()�p��Effect����
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
				mEffects[i]->update();//Effect��update���ǂ�ōX�V���Ă���
				if (mEffects[i]->billBoard())
					mEffects[i]->setEyeAngle(mEyeAngle);
				mEffects[i]->draw();
			}
			else //����ł�Ȃ�
				SAFE_DELETE(mEffects[i]);
		}
	}
	if (mOneEffectIsOn) {//ON�̂Ƃ������`��
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
	//��̃|�C���^��T���Ă����ɐ���
	int index = 0;
	for (index = 0; index < MAX_EFFECT_NUM; ++index) {
		if (mEffects[index] == nullptr)
			break;
	}
	if (index == MAX_EFFECT_NUM)
		HALT("Effect create Error");

	//��ނɂ���ăG�t�F�N�g�̐������Ԃ�ς���
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
	//���̒��͑O�ƈႤ�G�t�F�N�g���Ă΂ꂽ�Ƃ��ɂ������s����Ȃ�
	//mOneEffect�Ɋi�[���郂�f�����X�V����
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
	//�S�ẴG�t�F�N�g�ŋ��ʂ��鏈���B�ʂɕK�v�ȏ����͈ȍ~�ōs��
	mOneEffectPosition = pos;
	mOneEffectAngle = angle;
	mOneEffectIsOn = true;

	//���o
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

//Camera�N���X���王�_�������󂯎��p
void EffectController::setEyeAngle(const float angle)
{
	mEyeAngle = angle;
}

//Cammera�N���X����󂯎�������_���A�ʂ̂Ƃ���ɎQ�Ƃ��Ă��炤�B�J�����̓O���[�o���ɎQ�Ƃł��Ȃ����A���̃N���X�̓V���O���g���ŃO���[�o���ɎQ�Ɖ\
float EffectController::getEyeAngle() const
{
	return mEyeAngle;
}

//�O������Ăяo����֐�������̕ϐ���S�ă��Z�b�g����
//��ɁA�G�t�F�N�g�̋������Ǘ����邽�߂̕ϐ������Z�b�g
//�L�����N�^�[�̓��삪���؂�����Ƃ��ɂ��̊֐����ĂԂƂ悢
void EffectController::resetAllParameter()
{
	mIncAngleOfMagicAttack = 0.f;
	mIncScaleOfMagicAttack = 0.f;
	mIncPositionOfMagicAttack = 0.f;
}
