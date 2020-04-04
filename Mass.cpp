#include "GameLib/GameLib.h"
#include "GameLib/Scene/Container.h"
#include "GameLib/Math/Vector3.h"
#include "ContentReader.h"
#include "Mass.h"

using namespace GameLib;
using namespace GameLib::Scene;
using namespace GameLib::Math;

Mass::Mass() :
	mXmap(0),
	mYmap(0),
	mCount(0),
	mStopCount(0),
	mPosition(0.f, 0.f, 0.f),
	mScale(0.f, 0.f, 0.f),
	mPanel(NORMAL),
	mFoot(NONE)
{
	//�萔�̓ǂݍ���
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	TRAP_COUNT_STOP = (int)cr->getData("Mass", "TRAP_COUNT_STOP");
	SAFE_DELETE(cr);//�g���I�������폜
}

Mass::~Mass()
{
	mModel.release();
}

void Mass::draw()
{
	//���܂�Ă���Ƃ��F��ς���
	if (mFoot == Foot::FOOT_PLAYER)
		mModel.setColor(Vector3(0.1f, 0.1f, 1.0f));
	else if (mFoot == Foot::FOOT_ENEMY)
		mModel.setColor(Vector3(1.f, 0.1f, 0.1f));
	else if (mFoot == Foot::NONE)
		mModel.setColor(Vector3(1.f, 1.f, 1.f));
	//�������Ȃ�`�悵�Ȃ�
	if (mPanel != HOLE)
		mModel.draw();
}

void Mass::createModel(const Container& container)
{
	mModel = container.createModel("normal");
	mModel.setAngle(Vector3(0.f, 180.f, 0.f));
}


void Mass::setPos(const Vector3& pos)
{
	mPosition = pos;
	mModel.setPosition(mPosition);
}

void Mass::setScale(float scale)
{
	mScale = scale;
	mModel.setScale(mScale);
}

void Mass::setMap(int x, int y)
{
	mXmap = x;
	mYmap = y;
}

void Mass::setPanel(const Container& container, Panel panel)
{
	//�����p�l�����Z�b�g���悤�Ƃ��Ă��Ȃ���
	if (mPanel == panel)
		return;
	//HOLE������O
	if (panel == Mass::Panel::HOLE) {
		mPanel = Mass::Panel::HOLE;
		return;
	}
	//���f����ύX
	mModel.release();
	switch (panel) {
	case NORMAL:
		mPanel = NORMAL;
		mModel = container.createModel("normal");
		break;
	case CORRIDOR:
		mPanel = CORRIDOR;
		mModel = container.createModel("corridor");
		break;
	case WARP:
		mPanel = WARP;
		mModel = container.createModel("warp");
		break;
	case TRAP_HOLE:
		mPanel = TRAP_HOLE;
		mModel = container.createModel("broken");
		break;
	case TRAP_CATCH:
		mPanel = TRAP_CATCH;
		mModel = container.createModel("trap_catch");
		break;
	case TRAP_STOPPER:
		//���~��㩂����Ƃ��ɁA�J�E���^���Z�b�g���Ă���
		mStopCount = TRAP_COUNT_STOP;
		mPanel = TRAP_STOPPER;
		mModel = container.createModel("trap_stopper");
		break;
	case TRAP_BOMB:
		mPanel = TRAP_BOMB;
		mModel = container.createModel("trap_bomb");
		break;
	case GOAL:
		mPanel = GOAL;
		mModel = container.createModel("goal");
		break;
	case RECOVERY:
		mPanel = RECOVERY;
		mModel = container.createModel("recovery");
		break;
	case PARTS:
		mPanel = PARTS;
		mModel = container.createModel("parts");
		break;
	default:
		HALT("File:Mass.cpp [setPanel] switch Error");
		break;
	}
	//�����Z�b�g���Ȃ���
	mCount = 0;
	mModel.setPosition(mPosition);
	mModel.setScale(mScale);
	mModel.setAngle(Vector3(0.f, 180.f, 0.f));
}

void Mass::setPanelToHole()
{
	mPanel = HOLE;
}

void Mass::setFoot(Foot foot)
{
	mFoot = foot;
}

//�}�X�̍��W���󂯎�����|�C���^�ɖ��ߍ���œn��
void Mass::getMap(int* reX, int* reY) const
{
	*reX = mXmap;
	*reY = mYmap;
}

//�}�X�̏�Ԃ�Ԃ�
Mass::Panel Mass::getPanel() const 
{
	return mPanel;
}

Mass::Foot Mass::getFoot() const
{
	return mFoot;
}

const Vector3* Mass::getPosition() const
{
	return &mPosition;
}

void Mass::updateFoot()
{
	if (mPanel == Mass::Panel::TRAP_HOLE)
		this->setPanelToHole();
}

bool Mass::decStopCount()
{
	mStopCount--;
	if (mStopCount == 0)
		return true;

	return false;
}


//�}�X�̃p�����[�^�֘A
void Mass::resetMassParam()
{
	mMassParam.reset();
}

//change�Ƃ����������Ă���
void Mass::changeMassParam(Mass::MassParamName name, int value)
{
	//�������܂��Ȃ疳��
	if (mPanel == Mass::Panel::HOLE)
		return;
	//�w�肳�ꂽ�p�����[�^�ɁA������value�����̂܂ܑ���
	switch (name)
	{
	case MassParamName::MP_TEAM_PLAYER:
		mMassParam.TeamPlayer += value;
		break;
	case MassParamName::MP_TEAM_ENEMY:
		mMassParam.TeamEnemy += value;
		break;
	default:
		HALT("File:Mass.cpp [changeMassParam] Error");
		break;
	}
}

const Mass::MassParam* Mass::getMassParam() const
{
	return &mMassParam;
}