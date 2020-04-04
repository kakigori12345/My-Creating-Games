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
///Character�N���X�B���̕��Ƀp�[�c�Ɋւ���L�q����////
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
	//�萔�̓ǂݍ���
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
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
	SAFE_DELETE(cr);//�g���I�������폜

	//���f����񂪊i�[����Ă���R���e�i��ǂݍ���
	mCharacterContainer = Container::create("data/model/character.txt");
	while (!mCharacterContainer.isReady()) { ; }

	//HP�o�[
	mModelHPbar = mCharacterContainer.createModel("hp_bar");

	//���̑������ݒ�
	CharacterID++;
	mCharacterID = CharacterID;
	HP = mParam.health;
	mWeapon = Weapon();
	mParts = new Parts();
}

Character::~Character()
{
	mModelCharacter.release();
	//�f�X�g���N�^��ID�����炵�Ă������AID�̔�肪�o�Ă��Ă��܂����߈�x�g����ID�̓Q�[���������g��Ȃ����ƂƂ���
	//CharacterID��int�^�Ȃ̂ŁA�Q�[����int�^�̍ő吔�𒴂��鐔�̃����X�^�[�����Ɗm���ɃG���[���o��
	//�������A�����܂ł̐��̃����X�^�[�𐶐����邱�Ƃ��Ȃ��Ǝv���̂ŁA���͂��̂܂܂ɂ��Ă���
	//CharacterID--;
}

void Character::update()
{
	//Action�N���X�̃C���X�^���X���擾�B�L�����N�^�[�̓����͑��Action�N���X���S��
	//���̊֐����ŕύX���Ă���̂͂��������L�����N�^�[�̌����Ă���������炢
	Action* action = Action::instance();

	switch (mAct) {
	case ACT_WAIT: {
		mActTime = 1.1f;
		//�����ύX�B�i�`��������
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
		//�����ύX�B�������͈�u�ŕω�
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
		//�����ύX�B�i�`��������
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
	//�L�����N�^�[
	mModelCharacter.setAngle( Vector3(0.f, mAngleY, 0) );
	mModelCharacter.setPosition(mPosition);
	mModelCharacter.draw();
	//HP�o�[
	//��芄���ȉ��ɂȂ�ƃs���`�F�ɂȂ�
	float hpRange = (float)HP / (float)mParam.health;
	if ( hpRange < HP_BAR_PINCH)
		mModelHPbar.setColor(Vector3(1.f, 0.5f, 0.5f));
	else
		mModelHPbar.setColor(Vector3(1.f, 1.f, 1.f));
	//HP�o�[�̑傫���E�ʒu�𒲐�
	float cameraAngle = EffectController::instance()->getEyeAngle();
	mModelHPbar.setAngle( Vector3(0.f, cameraAngle, 0.f) );
	Vector3 hpPos;
	hpPos.set(mPosition.x, mPosition.y + HP_BAR_PLACE_Y * mParam.scale, mPosition.z);
	mModelHPbar.setPosition(hpPos);
	mModelHPbar.setScale(Vector3(HP_BAR_SCALE_X * hpRange, HP_BAR_SCALE_Y, 1.f));
	mModelHPbar.draw();
	//����
	//�ʒu�B�L�����N�^�[�̂�����Ɖ��ɕ\��
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
		//�P������
		this->recovery(1);
		break;
	default:
		HALT("File: Character.cpp [actEffect()] switch Error");
	}
}

//�s�������肵�Ȃ��Ԃ�false��Ԃ�������Btrue��Ԃ�����s���ӎv�����܂����Ƃ������ƁB
//�s�������܂����Ƃ��Ă����ꂪGame�N���X���ŋ��ۂ����ꍇ������B
//���̏ꍇ�A������x���̊֐����Ă΂�đI�����Ȃ���
bool Character::selectAction()
{
	switch (mMind) {
	//�L�[�{�[�h����̓���
	case Mind::MIND_INPUT: {
		//�����̂ŕʂ̊֐��Ɉڐ�
		return selectOfInput();
	}
	//�v���C���[��AI
	case Mind::MIND_PLAYER_AI: {
		//�����̂ŕʂ̊֐��Ɉڐ�
		return selectOfPlayerAI();
	}
	//AI�̃p�^�[���P
	case Mind::MIND_AI_1: {
		//�����̂ŕʂ̊֐��Ɉڐ�
		return selectOfMind1();
	}
	//����̓G���[
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
	//�A�N�V�����^�C�������Z�b�g
	mActTime = 0.f;
	//���łɈړ��O�ʒu���i�[���Ă���
	mPrevPosition = mPosition;
	//���̑����������K�v�ȃp�����[�^�����������Ă���
	attackIsDone = false;

	//SE�������Ŗ炵�Ă���
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
	//�A�N�V�����^�C������莞�ԂɂȂ�����Finish
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

//�p�����[�^�֘A

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

	//����ɂ���Ĕ\�͒l���ω�
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

//����̃p���[���󂯎���āA�_���[�W�v�Z���s��
void Character::damage(int receivePower)
{
	int result_damage = 0;
	//�h���N�G11���Q�l�ɂ���
	float power = (float)receivePower / 2.f;
	float defence = (float)mParam.defence / 4.f;
	float base_damage = power - defence;
	//��{�_���[�W�̐�������B���Ȃ�Ƃ肠����1�_���[�W�Œ�
	if (base_damage > 0) {
		int rand = (int)(base_damage / 16.f);
		int rand_damage = rand;
		if (rand != 0)
			int rand_damage = MyRandom::instance()->getInt(-rand, rand);
		result_damage = (int)base_damage + rand_damage;
	}
	else
		result_damage = 1;
	//�Ō�Ɏ��g��HP���猋�ʂ�����
	HP -= result_damage;
	if (HP < 0)
		HP = 0;

	//�󂯂��_���[�W���L�����Ă����A����HP�ȉ��̂Ƃ��͐퓬�������
	mParamAI.runawayHP = result_damage;
}

void Character::recovery(int plus)
{
	//�P���ɑ���
	HP += plus;
	//�ő�HP�𒴂��Ƃ��͍ő�HP��
	if (HP > mParam.health)
		HP = mParam.health;
}

//HP���炻�̂܂܈���
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
	//����������Ă���Ȃ炻��ɉ������˒�����
	if (haveWeapon)
		return mWeapon.getMaxDistanceOfAttack();
	//���킪�Ȃ��Ȃ�˒������͒ʏ�P
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
	//�����\��
	EffectController::instance()->createEffect(EffectController::Type::EXPLOSION, mPosition, true);
	//������
	SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_BOMB);
	//HP��1�ɂ���
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

/////////////////////////���̃N���X�ł����g��Ȃ��֐�///////////////////////////

void Character::naturalAngleChange(float targetAngle)
{
	while (true)
	{
		if (mAngleY < 360.f)
			break;
		mAngleY -= 360.f;
	}
	/*
	���̊֐������������Ƃ́A���R�ȉ�]���������邱�Ƃł���B
	��������u�ŕω�����̂�h���A�����Âω�������B���̍ہA
	�ړI�̊p�x�Ǝ��g�̊p�x�̍������A������180�x�ȏ゠�����Ƃ��Ă��A
	�����̂܂܉�]���Ă��܂��A�s���R�ƂȂ�B���̂��߁A
	180�x�ȏ�̂Ƃ��ɂ͉E��]�ƂȂ�悤�ɏ������s���B
	*/
	//�܂����g�̊p�x�ƖړI�̊p�x�̍��������
	float diff = targetAngle - mAngleY;
	if (diff < 0.f)
		diff += 360.f;
	if (diff > 180.f)
		diff = -1 * (360.f - diff);
	//���������l�ȉ��Ȃ�ړI�̊p�x�ɃZ�b�g���ďI��
	if ((diff >= 0 && diff <= 10.f) ||	//��
		(diff <= 0 && diff >= -10.f)) {	//��
		mAngleY = targetAngle;
		return;
	}
	//������180�x�ȏ゠�邩�ǂ����ŕ���
	if (diff > 0.f)
		mAngleY += ROTATE_SPEED;
	else
		mAngleY -= ROTATE_SPEED;
}

//update�̒��g���ꕔ�ڐ݁i�������ĉǐ����Ⴍ�Ȃ������߁j
bool Character::selectOfInput()
{
	Button* button = Button::instance();
	EffectController* effect = EffectController::instance();
	FrontDisplay* front = FrontDisplay::instance();

	//[r]�ōŏ��ɖ߂�
	if (button->isTriggered(Button::Key::KEY_R))
		mState = State::STATE_WAIT;

	switch (mState) {
	case State::STATE_WAIT: {
		//asdw�ňړ�
		// A �L�[
		if (button->isTriggered(Button::KEY_A)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_LEFT;
			return true;
		}
		// D �L�[
		else if (button->isTriggered(Button::KEY_D)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_RIGHT;
			return true;
		}
		// W �L�[
		else if (button->isTriggered(Button::KEY_W)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_UP;
			return true;
		}
		// S �L�[
		else if (button->isTriggered(Button::KEY_S)) {
			mReply = Character::REPLY_MOVE;
			mDirection = Character::DIR_DOWN;
			return true;
		}
		// SPACE�L�[ �ڂ̑O�̓G�ɒʏ�U��
		else if (button->isTriggered(Button::KEY_SPACE)) {
			mReply = Character::REPLY_ATTACK_NORMAL;
			return true;
		}
		// SHIFT�L�[ ���̑��̃A�N�V����
		else if (button->isTriggered(Button::KEY_SHIFT)) {
			mState = State::STATE_ACTION_SELECT;
			return false;
		}
		break;
	}
	case State::STATE_ACTION_SELECT: {
		//�܂��͑�܂��ɉ��������������߂�i�U���������̂��A�}�X�Ɋ��������̂��j
		FrontDisplay::instance()->showMessage(4);
		int cursor = front->showCursor(3, FrontDisplay::Cursor::CURSOR_TEXT);
		if(button->isTriggered(Button::Key::KEY_SPACE)){
			switch (cursor) {
			case 1:
				//�U��
				mState = Character::STATE_ATTACK_SELECT;
				break;
			case 2:
				//�}�X�ւ̊���
				mState = Character::STATE_MASS_ACTION_SELECT;
				mReply = Character::Reply::REPLY_MASS_TRAP_HOLE; //����1�s�͖������Ă悢
				break;
			case 3:
				//����̃p���[�A�b�v
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
			//�ǂ̕����ɍU�����邩����
			mState = Character::STATE_MASS_SELECT;

			switch (cursor) {
			case 1:
				//�ʏ�U��
				mReply = Character::REPLY_ATTACK_NORMAL;
				break;
			case 2:
				//�X�y�V�����U��
				if (mWeapon.getWeaponType() == Weapon::Type::Rod) {
					mState = State::STATE_MAGIC_TARGET_SELECT;
					mReply = Character::REPLY_ATTACK_MAGIC;
				}
				else {
					//����
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
		// A �L�[
		if (button->isTriggered(Button::KEY_A))
			x = 1;
		// D �L�[
		else if (button->isTriggered(Button::KEY_D))
			x = -1;
		// W �L�[
		else if (button->isTriggered(Button::KEY_W))
			y = 1;
		// S �L�[
		else if (button->isTriggered(Button::KEY_S))
			y = -1;
		//���݂̑I���ʒu�͎���2�̕ϐ��Ɋi�[����Ă���
		magicTargetX += x;
		magicTargetY += y;
		//�I���}�X�Ǝ����̈ʒu�Ƃ̃}�X�I�ȋ����𑪂�
		int distance = ((magicTargetX >= 0) ? magicTargetX : -magicTargetX) + ((magicTargetY >= 0) ? magicTargetY : -magicTargetY);
		//�ő�˒��͈͈ȏ��I������̂̓_��
		if (distance > this->canAttackToNear()) {
			magicTargetX -= x;
			magicTargetY -= y;
		}
		//�I�𒆂ɃJ�[�\���I�Ȃ��̂�\��
		Vector3 pos = mPosition;
		pos.x += MASS_LENGTH * (float)magicTargetX;
		pos.z += MASS_LENGTH * (float)magicTargetY;
		effect->show(EffectController::Type::MAGIC_TARGET, pos, Vector3(0.f, 0.f, 0.f), 1);
		//����
		if (button->isTriggered(Button::KEY_SPACE)) {
			//�v�������ۂ��ꂽ�Ƃ��悤�ɃZ�b�g
			toAnnouce(13, State::STATE_MAGIC_TARGET_SELECT);
			mState = Character::STATE_WAIT;
			mReply = Character::REPLY_ATTACK_MAGIC;
			return true;
		}
		break;
	}
	case State::STATE_MASS_ACTION_SELECT: {
		//�}�X�ւ̊��B�������邩���߂�i����������A���𖄂߂�Ȃǁj
		front->showMessage(6);
		//�K�v�ȃp�[�c��\��
		front->showNeedParts(Parts::neededParts(mReply));
		int cursor = front->showCursor(5, FrontDisplay::Cursor::CURSOR_TEXT);
		switch (cursor) {
		case 1:
			//���Ƃ���
			mReply = Character::REPLY_MASS_TRAP_HOLE;
			break;
		case 2:
			//�ߊl�
			mReply = Character::REPLY_MASS_TRAP_CATCH;
			break;
		case 3:
			//���~��
			mReply = Character::REPLY_MASS_TRAP_STOPPER;
			break;
		case 4:
			//�n��
			mReply = Character::REPLY_MASS_TRAP_BOMB;
			break;
		case 5:
			//�V�����}�X�̐���
			mReply = Character::REPLY_MASS_MAKE;
			break;
		default:
			HALT("File: PlayCharacter.cpp [selectAction()] STATE_MASS_ACTION_SELECT's switch is Error");
			break;
		}
		//�X�y�[�X�L�[�Ŏ���
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
		//Space�L�[�ŕ�������
		if (button->isTriggered(Button::KEY_SPACE)) {
			mState = Character::STATE_WAIT;
			return true;
		}
		//A�L�[
		if (button->isTriggered(Button::KEY_A))
			mDirection = Character::DIR_LEFT;
		// D �L�[
		else if (button->isTriggered(Button::KEY_D))
			mDirection = Character::DIR_RIGHT;
		// W �L�[
		else if (button->isTriggered(Button::KEY_W))
			mDirection = Character::DIR_UP;
		// S �L�[
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
				//����Ȃ�
				if (!mParts->judge(need))
					toAnnouce(15, State::STATE_ACTION_SELECT);
				//�����B�p���[�A�b�v
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
		//Space�L�[�Ŗ߂�
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

		//����̏󋵂��݂ĕ��j������
		if (mAIinfo.destinationArrived) {
			pathPlanningWasFinished = false;
			mAIinfo.destinationArrived = false;
		}
		//�߂��ɓG������Ȃ�퓬
		if (selectOfMind1()) {
			//�������Ȃ�
			//�S�[����ڎw��
			if (mReply != Reply::REPLY_NONE) {
				resetAIparameter();
				return true;
			}
		}
		
		/*
		�EPlayerAI_Strategy��PlayerAI_Behavior�ɉ����čs����I�����Ă���
		�EGame�N���X��Stage�N���X������𓾂�Ƃ��́A���̎|��Reply�ɂ���ē`����iGame,Stage�ɂ��̏������L�q�j

		PlayerAI_Strategy�ɂ���
		�@�EAI_GOAL
		�@ �P�D�o�H�T����Stage�N���X�Ɉ˗�����i�T���ς݃t���O�������Ă����炵�Ȃ��j
		  �@�@ ��GOAL_PATH_PLANNING
		   �Q�D�o�H�T���ς݃t���O�𗧂Ă�
		   �R�DStage�N���X�Ɍo�H�������Ă��炤
			   ��GOAL_RECEIVE_ROOT
		   �S�D�ړ�����
		   �@�@��GOAL_GO_ALONG_PATH
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
		//�o�H�T���̕K�v�����邩
		if (pathPlanningWasFinished)
			//�o�H���󂯎��ɂ���
			mAIbehavior = GOAL_RECEIVE_ROOT;
		else {
			//�o�H�T�����˗�
			mReply = Reply::REPLY_GOAL_PATH_PLANNNING;
			return true;
		}
		break;
	}
	case PlayerAI_Behavior::GOAL_RECEIVE_ROOT: {
		//�o�H�����炤
		mReply = Reply::REPLY_GOAL_ROOT;
		mAIbehavior = GOAL_GO_ALONG_PATH;
		return true;
	}
	case PlayerAI_Behavior::GOAL_GO_ALONG_PATH: {
		//�ړ�
		mReply = Reply::REPLY_MOVE;
		mAIbehavior = GOAL_PATH_PLANNING;
		//�������킹
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
	//�Ȃɂ����Ȃ�
	return true;
}

bool Character::selectOfMind1()
{
	/*
	1�F�^�[��������Ă�����A�܂��������������Ƃ����̂�Game�N���X�ɓ`����
	2�FGame�N���X����A�אڂ���}�X�Ɋւ��Ă̏������炤�isetStrategy�֐������ł��炤�j
	   ��������setState��STATE_THINKING��ԂɃZ�b�g���Ă��炤
	3�F�󂯎�����������Ƃɍs��������

	*/
	
	switch (mState) {
	//�P
	case State::STATE_WAIT: {
		mReply = Reply::REPLY_WANT_INFORMATION;
		return true;
	}
	//�R
	case State::STATE_THINKING: {
		//�ړ������ۂ��ꂽ�Ƃ��ɂ͉������Ȃ����ƂƂ���
		if (moveWasFault) {
			moveWasFault = false;
			pathPlanningWasFinished = false;
			mReply = Reply::REPLY_NONE;
			mState = State::STATE_WAIT;
			//HALT("File: Character.cpp MOVEwasFault ");
			return true;
		}

		//�󂯎�����������Ƃɕ]���l���v�Z����
		//�s�����Ƃ̌v�Z���ʂ��i�[����B�s���̉s�͊֌W�Ȃ��Ƃ肠�����v�Z����
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

		//�������ƂɁA�s���ł��Ȃ��I�����������Ă���
		//�G���ׂɂ��Ȃ�
		if (!mInformation.EnemyIsNextMass)
			Value_Attack_Normal = 0;
		//�G���߂��ɂ��Ȃ�
		if (!mInformation.EnemyIsNearMass) {
			Value_MoveToEnemy = 0;
			Value_RunAway = 0;
		}
		//�������U�����g���Ȃ�
		if (mInformation.NearTargetID == -1)
			Value_Attack_Magic = 0;
		//�v���C���[���߂��ɂ��Ȃ�
		if (!mInformation.playerIsNearMass)
			Value_MoveToPlayer = 0;
		//�������󋵂łȂ�
		if (HP > mParamAI.runawayHP || !mInformation.CanRunAway)
			Value_RunAway = 0;
		//�G
		if (mInformation.youAreEnemyTeam)
			Value_MoveToPlayer = 0;

		//����������
		//�G���ׂɂ��āA�U���ł���̂Ɉړ����悤�Ƃ���ꍇ
		if (Value_Attack_Normal > 0 && Value_MoveToEnemy > 0)
			Value_MoveToEnemy = 0;

		//�ŏI�I�ɁA�]���l�����Ƃɍs�������肷��
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
		
		//���肵���I���ɉ������s����Ԃ�
		switch (index) {
		case 0:
			//�ʏ�U��
			mReply = Reply::REPLY_ATTACK_NORMAL;
			//�������킹
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
			//���@�U��
			magicTargetID = mInformation.NearTargetID;
			mReply = Reply::REPLY_ATTACK_MAGIC;
			break;
		case 2:
			//�G�ɋ߂Â�
			mReply = Reply::REPLY_MOVE;
			//�������킹
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
			//�v���C���[�ɋ߂Â�
			mReply = Reply::REPLY_MOVE;
			//�������킹
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
			//������
			mReply = Reply::REPLY_MOVE;
			//�������킹
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
			//�������Ȃ�
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



//���̃N���X���̂�
void Character::toAnnouce(int messageNumber, State backTo)
{
	announceMessageNumber = messageNumber;
	backFromAnnounce = backTo;
	mState = State::STATE_ANNOUNCE;
}






//////////////////////////////////////////
/////////////�p�[�c�֘A///////////////////
//////////////////////////////////////////

Parts Parts::neededParts(Character::Reply rep)
{
	Parts result;

	switch (rep)
	{
		//���Ƃ���
	case Character::Reply::REPLY_MASS_TRAP_HOLE:
		result.Parts_Screw	= 2;
		result.Parts_Hammer = 1;
		result.Parts_Net	= 2;
		result.Parts_Stone	= 1;
		break;
		//�ߊl�
	case Character::Reply::REPLY_MASS_TRAP_CATCH:
		result.Parts_Screw	= 4;
		result.Parts_Hammer = 2;
		result.Parts_Net	= 4;
		result.Parts_Stone	= 2;
		break;
		//���~��
	case Character::Reply::REPLY_MASS_TRAP_STOPPER:
		result.Parts_Screw	= 0;
		result.Parts_Hammer = 2;
		result.Parts_Net	= 0;
		result.Parts_Stone	= 2;
		break;
		//�n��
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

//����
void Parts::add(Parts parts) {
	this->Parts_Screw += parts.Parts_Screw;
	this->Parts_Hammer += parts.Parts_Hammer;
	this->Parts_Net += parts.Parts_Net;
	this->Parts_Stone += parts.Parts_Stone;
}

//����
void Parts::sub(Parts parts) {
	this->Parts_Screw -= parts.Parts_Screw;
	this->Parts_Hammer -= parts.Parts_Hammer;
	this->Parts_Net -= parts.Parts_Net;
	this->Parts_Stone -= parts.Parts_Stone;
}

//���g�𐔔{����
void Parts::mul(int num) {
	this->Parts_Screw *= num;
	this->Parts_Hammer *= num;
	this->Parts_Net *= num;
	this->Parts_Stone *= num;
}

//�K�v������Ă��邩����
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

//�����X�^�[�p�B�����_����num�̐��̃p�[�c����������
void Parts::setRandomParts(int num) {
	for (int i = 0; i < num; ++i) {
		int rand = MyRandom::instance()->getInt(0, 100);
		//���ꂼ��̃p�[�c�̏o�����͓K���Ɍ��߂Ă���
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