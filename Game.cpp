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
	//�t�@�C������萔��ǂݍ��ށiContentReader�N���X���g���j
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	Game::FINAL_STAGE_NUMBER = (int)cr->getData("Game", "FINAL_STAGE_NUMBER");
	SAFE_DELETE(cr);//�g���I�������폜
	//�v���C�L�����N�^�[�̍쐬
	mPC = new PlayCharacter();
	playerID = mPC->getID();
	//�����X�^�[�Ǘ��N���X�̐���
	mMonCon = new MonsterController();
	//���̑��̑S�Ă̐����A���������s��
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

	//��^����
	if (!mPC->isLiving())
		mSituation = Situation::LOSE;

	//�A�N�e�B�u�L�����ƁA���݂���Ȃ�p�b�V�u�L�����̍X�V
	if (mActiveCharacter == nullptr)
		HALT("File: Game.cpp [update()] mActiveCharacter is nullptr. Error");
	mActiveCharacter->update();
	if (mPassiveCharacter != nullptr)
		mPassiveCharacter->update();
	//�X�e�[�W�̍X�V
	mStage->update();
	mStage->updateMassParameter();
	mStage->showMiniMap();
	//�����o��
	if (mSituation != Situation::SHOW_STAGE_NUM) {
		front->showParameter(mPC->getName(), mPC->getParameter()->health, mPC->getHP(), mPC->weapon());
		mMonCon->showFriendMonstersParameter();
		front->showParts(&mPC->getParts());
		front->showTurnLimit(mStage->getTurnLimit());
	}

	switch (mSituation) {
	//�ړ���U���Ȃǂ̒ʏ폈��
	case NORMAL: {
		mPreSituation = Situation::NORMAL;
		//�퓬����
		if (battleIsDone) {
			//�A�N�e�B�u�L�����N�^�[���U���̃^�C�~���O�ɂȂ�����퓬
			if (mActiveCharacter->getAttackIsDone()) {
				bool friendly = false;	//�퓬��̒��Ԕ���p
				battle(friendly);		//�퓬
				battleIsDone = false;
				//���Ԕ���
				if (friendly)
					mSituation = MAKE_FRIEND;
			}
		}
		//�A�N�e�B�u�L�����̌��`�F�b�N�B��オ�N����Ƃ�Situation��MENU�ɑJ��
		//���łɂ��̊֐����ŁA���̃X�e�[�W�ւ̑J�ڔ�����s���Ă���
		//�s�����I��������ǂ���
		if (mActiveCharacter->actFinished()) {
			//�A�N�e�B�u�L�����̍s�����I�������Ƃ��ɃG�t�F�N�g�̉��o�p�p�����[�^�����Z�b�g
			EffectController::instance()->resetAllParameter();
			//�������A�N�e�B�u�L�����̓���p�����[�^�����Z�b�g
			mActiveCharacter->resetAllParameters();
			//�A�N�e�B�u�L�����̕ύX
			checkActiveCharacterChange();
		}
		break;
	}
	//�L�����N�^�[�̍s���I���t�F�[�Y
	case MENU: {
		mPreSituation = Situation::MENU;
		//�A�N�e�B�u�L�����ɍs���I��������
		if (mActiveCharacter->selectAction())
			//�R�[�h�������Ȃ����̂ŕʂɂ܂Ƃ߂�
			menu();
		break;
	}
	//�G�����X�^�[�𒇊Ԃɂ��邩�ǂ����I������
	case MAKE_FRIEND: {
		//���ԃC�x���g������SE��炷
		if (mPreSituation != Situation::MAKE_FRIEND)
			SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_OCCUR);
		mPreSituation = Situation::MAKE_FRIEND;

		//���b�Z�[�W�\��
		switch (messageNum) {
		case 1:
			//�����X�^�[���N���オ�������Ƃ�m�点��
			front->showMessage(8);
			if (button->isTriggered(Button::KEY_SPACE))
				messageNum = 2;
			break;
		case 2: {
			//���Ԃɂ��邩�ǂ�����I�����Ă��炤
			front->showMessage(9);
			int cursor = front->showCursor(2, FrontDisplay::Cursor::CURSOR_TEXT);
			if (button->isTriggered(Button::Key::KEY_SPACE)) {
				if (cursor == 1) {
					//���Ԃɂ���
					messageNum = 3;
					SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_GOOD);
				}
				else {
					//���Ԃɂ��Ȃ�
					messageNum = 4;
					SoundGenerator::instance()->playSe(SoundGenerator::Se::MAKE_FRIEND_BAD);
				}
			}
			break;
		}
		case 3:
			//���Ԃɂ���
			front->showMessage(10);
			if (button->isTriggered(Button::KEY_SPACE)) {
				//�Ƃ肠�����A���ԏ���ɒB���Ă��Ē��Ԃɂł��Ȃ��Ȃ�E��
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
			//���Ԃɂ��Ȃ��Ȃ�E��
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
	//�^�[�������Ɉ�����������
	case TURN_LIMIT: {
		front->showMessage(14);
		if (button->isTriggered(Button::KEY_SPACE)) {
			mPC->turnLimitAction();
			mSituation = mPreSituation;
		}
		break;
	}
	//���݂̃X�e�[�W�K�w��\������B�K�w���ړ������ۂɐ��b�Ԃ����\�������
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
	//��������̃��b�Z�[�W���o���B���O��Situation�ɖ߂�
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
	//BGM���I��
	SoundGenerator::instance()->stopBgm();
	//�������Ă��邩�ǂ����ŕ���
	if (victory)
		return new GameClear();
	else
		return new GameOver();
}


Game::Situation Game::getSituation()
{
	return mSituation;
}



/////////////////////���̃N���X���ł����g��Ȃ��֐�/////////////////////

void Game::createNewStage()
{
	//������
	SAFE_DELETE(mStage);
	mActiveCharacter = nullptr;
	mPassiveCharacter = nullptr;
	mMonCon->resetEnemyMonster();

	//������������
	MyRandom::instance()->resetRand();

	//�X�e�[�W�ԍ���i�߂�

	//�t�@�C������K�v�ȏ���ǂݍ��ށiContentReader�N���X���g���j
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	//�X�e�[�W�̊K�w
	string stName = cr->getData_str("StageNumber", to_string(StageNumber).c_str());
	const char* chStName = stName.c_str();
	//�����X�^�[�̎��
	int NUM_OF_MONSTER_TYPE = (int)cr->getData(chStName, "NUM_OF_MONSTER_TYPE");
	//�����X�^�[�̎�ނɉ����āA���O�Ɛ��̔z��𐶐�
	string* nameList = new string[NUM_OF_MONSTER_TYPE];
	int* numList = new int[NUM_OF_MONSTER_TYPE];
	//�ēx�ǂݍ���ł���
	for (int i = 0; i < NUM_OF_MONSTER_TYPE; ++i)
	{
		//�������p�ӁB�Ⴆ�΁A�����X�^�[�̎�ނ��R��ނ��Ƃ�����A���̂悤�Ȋ���
		//nameList = [ "NAME_OF_MONSTER1", "NAME_OF_MONSTER2", "NAME_OF_MONSTER3"];
		//numList = ["NUM_OF_MONSTER1", "NUM_OF_MONSTER2", "NUM_OF_MONSTER3"];
		//�����̒萔�̓e�L�X�g�t�@�C�����ɏ����Ă���
		string name = "NAME_OF_MONSTER" + to_string(i + 1);
		string num = "NUM_OF_MONSTER" + to_string(i + 1);
		//���ۂɃt�@�C������ǂݏo��
		nameList[i] = cr->getData_str(chStName, name.c_str());
		numList[i] = (int)cr->getData(chStName, num.c_str());
	}
	SAFE_DELETE(cr);//�g���I�������폜


	//�����B�������炪�{���̏���
	mStage = new Stage(StageNumber);
	//�v���C���[��ID��Stage�ɓo�^���č��W���Z�b�g����
	{
		Vector3 pos;
		mStage->registerNewID(mPC->getID(), &pos, Stage::TEAM_PLAYER);
		mStage->registerPlayerID(mPC->getID());
		mPC->setPosition(pos);
	}
	
	//�����X�^�[�𐶐����Ă����B��Ńt�@�C������ǂݍ��񂾒l���g��
	mMonCon->setNumOfMonsterType(NUM_OF_MONSTER_TYPE);
	for (int i = 0; i < NUM_OF_MONSTER_TYPE; ++i) {
		mMonCon->createMonster(nameList[i].c_str(), numList[i]);
	}
	//�������������X�^�[��ID��Stage�ɓo�^���A���W���Z�b�g����
	bool temp_first = true;
	int temp_id = 0;
	bool beFriend = false;
	while (true)
	{
		//1��ڂ���
		if (temp_first) {
			temp_first = false;
			temp_id = mMonCon->getID(true, &beFriend);
		}
		else
			temp_id = mMonCon->getID(false, &beFriend);
		//ID�T�����Ō�܂œ��B������-1���Ԃ��Ă���͂�
		if (temp_id == -1)
			break;
		//Stage�N���X��ID��o�^���A���W�����炤�B����������X�^�[�ɃZ�b�g
		Vector3 pos;
		Stage::Team team;
		if (beFriend)
			team = Stage::Team::TEAM_PLAYER;
		else
			team = Stage::Team::TEAM_ENEMY;
		mStage->registerNewID(temp_id, &pos, team);
		mMonCon->setPos(temp_id, pos);
	}
	//���̑������ݒ�
	mActiveCharacter = mPC;
	mSituation = Situation::SHOW_STAGE_NUM;
	mStage->updateMass();
}


void Game::checkActiveCharacterChange()
{
	SoundGenerator* sound = SoundGenerator::instance();
	/////////////�}�X�Ɏ~�܂������̃C�x���g����/////////////
	//�G���[�`�F�b�N
	ASSERT(mSituation == Situation::NORMAL);
	//���O�ɓ��܂�Ă����}�X�̍X�V
	mStage->updatePrevFootMass();
	//�v���C���[���K�i��ɂ���Ȃ玟�̃X�e�[�W��
	if (playerOnTheGoal())
		return;
	//�{�X�����œG���S�ł��Ă���Ȃ玟�̃X�e�[�W��
	if (mStage->thisStageIsBossRoom() && mStage->AllEnemyIsDied()) {
		gotoNextStage();
		return;
	}
	//�A�N�e�B�u�L�����N�^�[������}�X�Ɏ~�܂������ɃC�x���g�����荞�ݏ���
	switch (mStage->getPanelFromID(mActiveCharacter->getID())) {
	case Mass::Panel::WARP: {
		//�ړ���̃��[�v�ȊO�͔F�߂Ȃ��i���[�v��Ƀ��[�v�}�X�Ɏ~�܂�Ȃǂ͔F�߂Ȃ��Ƃ������Ɓj
		if (mActiveCharacter->getAct() != Character::ACT_MOVE)
			break;
		//�ړI�n��ݒ�
		Vector3 pos;
		if (!mStage->judgeWarp(mActiveCharacter->getID(), &pos))
			break;
		mActiveCharacter->setDestination(pos);
		mActiveCharacter->setAction(Character::Act::ACT_WARP);
		//AI�̃p�����[�^�����Z�b�g
		mActiveCharacter->resetAIparameter();
		return;
	}
	//�g���b�v�Ɋւ��āB�g���b�v�͊�{�I�Ƀv���C���[�⒇�ԃ����X�^�[�ɂ͖���
	case Mass::Panel::TRAP_HOLE: {
		//�v���C���[�ɂ͖���
		if (mActiveCharacter->getID() == playerID)
			break;
		//���Ԃɂ�����
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		//����if�����Ȃ��Ɩ����ɗ���������
		if (mActiveCharacter->getAct() == Character::ACT_FALL)
			break;
		mActiveCharacter->setAction(Character::Act::ACT_FALL);
		IDofFalled = mActiveCharacter->getID();
		TrapHoleIsDone = true;
		mStage->changePanel(IDofFalled, Mass::Panel::HOLE);
		return;
	}
	case Mass::Panel::TRAP_CATCH: {
		//�v���C���[�ɂ͖���
		if (mActiveCharacter->getID() == playerID)
			break;
		//���Ԃɂ�����
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		//����if�����Ȃ��Ɩ���
		if (mActiveCharacter->getAct() == Character::ACT_CATCH) {
			break;
		}
		mActiveCharacter->setAction(Character::Act::ACT_CATCH);
		IDofCatched = mActiveCharacter->getID();
		TrapCatchIsDone = true;
		return;
	}
	case Mass::Panel::TRAP_STOPPER: {
		//�v���C���[�ɂ͖���
		if (mActiveCharacter->getID() == playerID)
			break;
		//���Ԃɂ�����
		else if (mMonCon->getFriendlyByID(mActiveCharacter->getID()))
			break;
		mActiveCharacter->setBeingStopped();
		SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_STOPPER);
		break;
	}
	case Mass::Panel::TRAP_BOMB: {
		//�v���C���[�ɂ͖���
		if (mActiveCharacter->getID() == playerID)
			break;
		//���Ԃɂ�����
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
		//�v���C���[�̂ݗL��
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
	//mPassiveCharacter�����
	if (mPassiveCharacter) {
		mPassiveCharacter->setAction(Character::Act::ACT_WAIT);
		mPassiveCharacter = nullptr;
	}

	/////////////�A�N�e�B�u�L�����N�^�[�̕ύX/////////////
	//Menu�ւ̑J��
	mSituation = Situation::MENU;
	//------�A�N�e�B�u�L�����̍X�V�̗���------
	//�^�[���̈ڂ�ς��F�v���C���[�@���@���������X�^�[�@���@�G�����X�^�[
	//�P�F�܂��v���C���[���A�N�e�B�u���������̊m�F
	//�Q�F����Monster�̔Ԃ��̔���B�����łȂ��Ȃ�3��
	//�R�F�GMonster�̔Ԃ��̔���B�����łȂ��Ȃ�4��
	//�S�F�v���C���[���A�N�e�B�u�L�����N�^�[�Ƃ��ăZ�b�g����
	//------�ȉ��P�`�S�̃��[�v�i�A�N�e�B�u�`�F�b�N���ʂ����烋�[�v�I���j------
	while (true) {
		bool last = false;
		//1�@���O�ɓ������̂��v���C���[��
		if (mActiveCharacter == mPC) {
			//�X�e�[�W�̃^�[�������Ɉ��������邩
			if (mStage->incTurnLimit() == 0) {
				mPreSituation = Situation::MENU;
				mSituation = Situation::TURN_LIMIT;
			}
			//�����X�^�[�̍s���������Z�b�g
			mMonCon->resetActiveCounter();
		}
		//2�@���������X�^�[�̔ԁB�S�����s���ς݂��ǂ���
		mActiveCharacter = mMonCon->returnActiveFriendPointer(&last);
		if (!last) {
			//3�@�G�����X�^�[�̔ԁB�S�����s���ς݂��ǂ���
			mActiveCharacter = mMonCon->returnActiveEnemyPointer(&last);
			if (!last)
				//4�@�v���C���[�̔�
				mActiveCharacter = mPC;
		}
		//�A�N�e�B�u�`�F�b�N
		//�A�N�e�B�u�L�����N�^�[�����~�ߏ�ԂȂ�^�[�����΂�
		if (mActiveCharacter->getBeStopped()) {
			SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_STOPPER);
			//���~�߃J�E���g��i�߂ăJ�E���g���O�ɂȂ����瑫�~�ߏ�Ԃ������B������������͎̂��̃^�[������
			if (mStage->decCountOfTrapStopper(mActiveCharacter->getID())) {
				mActiveCharacter->resetBeingStopped();
				//㩁F�X�g�b�p�[���P�����Ă���
				mStage->changePanel(mActiveCharacter->getID(), Mass::Panel::NORMAL);
			}
			continue;
		}
		//�����܂ł�����A�N�e�B�u�`�F�b�N���ʂ����Ƃ������ƁB���[�v���甲����
		break;
	}

	//�Ō�ɁB�A�N�e�B�u�L�����N�^�[��ύX������ōs���ׂ��������ς܂���
	//���Ƃ�������
	if (TrapHoleIsDone)
	{
		TrapHoleIsDone = false;
		killMonster(IDofFalled, false);
	}
	//�ߊl㩔���
	else if (TrapCatchIsDone)
	{
		TrapCatchIsDone = false;
		//�ߊl���삪�I�������ɕߊl㩂�P�����āA�m�[�}���}�X�ɂ���
		mStage->changePanel(IDofCatched, Mass::Panel::NORMAL);
		//�Ώۂ𒇊Ԃɂ��邩�I������
		IDofFriendly = IDofCatched;
		mSituation = Situation::MAKE_FRIEND;
	}
	//�n������
	else if (TrapBombIsDone)
	{
		TrapBombIsDone = false;
		//SE
		SoundGenerator::instance()->playSe(SoundGenerator::Se::STAGE_MASS_BOMB);
		//�n���P��
		mStage->changePanel(IDofBombed, Mass::Panel::NORMAL);
		//�G�t�F�N�g�\��
		Vector3 effectPos = *mStage->getPositionFromID(IDofBombed);
		EffectController::instance()->createEffect(EffectController::Type::EXPLOSION, effectPos, true);
		//�_���[�W������������
		setDamage(false, IDofBombed);
	}
}

void Game::menu()
{
	Vector3 pos;
	Character::Reply rep = mActiveCharacter->getSelect();
	//Character�̎w��}�X�ƁAStage�̃^�[�Q�b�g�}�X�����v������B�����Ӗ���ϊ����Ă��邾���ŏd�v�łȂ�
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
	//�A�N�V�����B�I�����ꂽ�A�N�V�����������Ȃ�A�I�������蒼��������
	switch (rep) {
	//���v��
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

		//�X�e�[�W���������������A�N�e�B�u�L�����N�^�[�ɓn���p�ɍ�蒼��
		Information info;
		//�G���ǂ���
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
		//�A�N�e�B�u�L�����N�^�[�ɓn��
		mActiveCharacter->setInformation(info);
		mActiveCharacter->setState(Character::State::STATE_THINKING);
		break;
	}
	//�}�X�ړ�
	case Character::Reply::REPLY_MOVE: {
		//Stage�N���X�Ɉړ�������s���Ă��炤�B�L���Ȃ�ړ�����W��Ԃ��Ă��炤
		//Stage��ID�ƍ��W�́Aif�����̊֐��ŏ�������Ă���
		int passiveID = -1;
		if (mStage->judgeMove(mActiveCharacter->getID(), targetmass, &pos, &passiveID))
		{
			//�ړI�n��ݒ�
			mActiveCharacter->actEffect(Character::Reply::REPLY_MOVE);
			mActiveCharacter->setDestination(pos);
			mActiveCharacter->setAction(Character::ACT_MOVE);
			//�}�X�̓���ւ����N����Ȃ�
			if (passiveID != -1) {
				mPassiveCharacter = mMonCon->getMonsterFromID(passiveID);
				mPassiveCharacter->actEffect(Character::Reply::REPLY_MOVE);
				mPassiveCharacter->setDestination(*mActiveCharacter->position());
				mPassiveCharacter->setAction(Character::ACT_MOVE);
			}
			//�Q�[���̃V�`���G�[�V�����ω�
			mSituation = Game::Situation::NORMAL;
		}
		else {
			mActiveCharacter->moveFault();
		}
		break;
	}
	//�ʏ�U��
	case Character::Reply::REPLY_ATTACK_NORMAL: {
		battleIsDone = true;
		mActiveCharacter->setDestination(*mStage->returnPlaceFromIDandTarget(mActiveCharacter->getID(), targetmass));
		mActiveCharacter->setAction(Character::ACT_ATTACK_NORMAL);
		mSituation = Game::Situation::NORMAL;
		break;
	}
	//���������@�U��
	case Character::Reply::REPLY_ATTACK_MAGIC: {
		//�v���C���[�̏ꍇ�A���肪�K�v
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
		//�ʏ�U���Ƃ���Ă��邱�Ƃ͂���Ȃɕς��Ȃ�
		battleIsDone = true;
		attackIsMagic = true;
		mActiveCharacter->setDestination(*mStage->returnPlaceFromID(mActiveCharacter->getMagicTargetID()));
		mActiveCharacter->setAction(Character::ACT_ATTACK_MAGIC);
		mSituation = Game::Situation::NORMAL;
		break;
	}
	//�X�e�[�W��ɗ��Ƃ���������i�v���C���[��p�j
	case Character::Reply::REPLY_MASS_TRAP_HOLE: {
		//Stage�N���X�Ƀ}�X�ւ̊����\���𔻒肵�Ă��炤
		//�p�[�c������Ă��邩�m�F
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
	//�X�e�[�W��ɕߊl㩂�����i�v���C���[��p�j
	case Character::Reply::REPLY_MASS_TRAP_CATCH: {
		//��̃P�[�X�Ɠ��l
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
	//�X�e�[�W��ɑ��~��㩂�����i�v���C���[��p�j
	case Character::Reply::REPLY_MASS_TRAP_STOPPER: {
		//��̃P�[�X�Ɠ��l
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
	//�X�e�[�W��ɒn��������i�v���C���[��p�j
	case Character::Reply::REPLY_MASS_TRAP_BOMB: {
		//��̃P�[�X�Ɠ��l
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
	//�������܂��ɐV�����}�X���쐬����i�v���C���[��p�j
	case Character::Reply::REPLY_MASS_MAKE: {
		//��̃P�[�X�Ɠ��l
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
	//�������Ȃ�
	case Character::Reply::REPLY_NONE: {
		mSituation = Game::Situation::NORMAL;
		break;
	}
	///////��������̓v���C���[��AI�p�̂���////////////
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
		//�Z�b�g
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
	//�v���C���[���K�i�̏�ɂ���Ȃ�A���̃X�e�[�W��
	//�����ŁA�K�i�̂����ɂ���Ȃ�ⓚ���p�ŊK�w��i�߂Ă��邪�A
	//�v���C���[���������I���ł���悤�ɂ����� todo:
	if (mStage->getPanelFromID(mPC->getID()) == Mass::GOAL) {
		gotoNextStage();
		return true;
	}

	return false;
}

void Game::gotoNextStage()
{
	//�K�v�ȏ�����
	mActiveCharacter->setAction(Character::Act::ACT_WAIT);
	//�ŏI�K�w�Ȃ炱��ŃN���A
	if (StageNumber == FINAL_STAGE_NUMBER) {
		//�Ƃ肠�����I��
		mSituation = Situation::VICTORY;
		return;
	}
	//���̃X�e�[�W�𐶐�
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
	//Stage�N���X�ɍU���}�X�ɂ���L�����N�^�[��ID�������Ă��炤�B���Ȃ��Ȃ�-1���Ԃ��Ă���͂�
	//�{���A�U���Ώۂ͕����ɂȂ邱�Ƃ�����͂��B���܂͂Ƃ肠�����Ώۂ͈�l�Ƃ��Ă��邪�A
	//������ǂ��ĕ����ΏۂɑΉ����� todo:
	int targetID(0), targetIDnum(0);
	//�אڂ���}�X�ւ̍U��
	if (!attackIsMagic) {
		mStage->getIDs(mActiveCharacter->getID(), targetmass, &targetID, &targetIDnum);
	}
	//�������U��
	else {
		targetID = mActiveCharacter->getMagicTargetID();
		targetIDnum = 1;
		attackIsMagic = false;
	}
	//�_���[�W�����B�v���C���[�Ƒ��ŏ����𕪂��Ă���i�v���C�L�����N�^�[��ID�͕ʂɕۑ����Ă���j
	int powerOfAttacker = mActiveCharacter->getParameter()->power;
	for (int i = 0; i < targetIDnum; ++i)
	{
		if (targetID == -1)
			continue;
		//��U���Ώۂ��v���C���[
		if (targetID == playerID) {
			mPC->damage(powerOfAttacker);
		}
		//��U���Ώۂ������X�^�[
		else {
			//�����X�^�[�ւ̃_���[�W�����B���񂾂�ID�����Z�b�g����
			bool attackerIsPlayer = (mActiveCharacter->getID() == playerID);
			bool died = false;
			bool beFriend = false;
			mMonCon->setDamage(targetID, powerOfAttacker, attackerIsPlayer, died, beFriend);
			//�����X�^�[������
			if (died) {
				//���ԂɂȂ肽������
				if (beFriend) {
					enemyWantToBeFriend = true;
					IDofFriendly = targetID;
				}
				//���Ԃɂ͂Ȃ�Ȃ��A�������͌��X���Ԃ���������
				else {
					bool flag = !mMonCon->getFriendlyByID(targetID);
					killMonster(targetID, flag);
				}
			}
		}
	}
}

//�퓬�ȊO�ł̃_���[�W�����B�G��|���Ă����Ԕ���͍s���Ȃ�
void Game::setDamage(bool passiveIsPlayer, int id)
{
	if (passiveIsPlayer)
	{
		//�Ƃ肠�����p���[�P�O�O�̃_���[�W��H��킹��
		mPC->damage(100);
	}
	else
	{
		bool died = false;		//���񂾂�
		bool beFriend = false;	//���ԂɂȂ邩�B�����ł͎g��Ȃ�
		mMonCon->setDamage(id, 100, false, died, beFriend);
		//�����X�^�[�����񂾂炱�̏ꍇ���̂܂܎E��
		if (died) {
			killMonster(id, false);
		}
	}
}

//�����X�^�[���E���Ƃ��̏������܂Ƃ߂�����
void Game::killMonster(int target, bool partsWillBeGot)
{
	//�G����p�[�c���Q�b�g�ł��邩�ǂ���
	if (partsWillBeGot) {
		nowGotParts = mMonCon->getParts(target);
		if (nowGotParts.hasSomethingParts()) {
			mPC->addParts(nowGotParts);
			mSituation = Situation::ANNOUNCEMENT;
		}
	}
	//�����X�^�[�R���g���[���[������̏���
	mMonCon->killMonster(target);
	//�X�e�[�W�ɓo�^���Ă��郊�X�g����̏���
	mStage->deleteID(target);
}