#include "Title.h"
#include "GameLib/Framework.h"
#include "GameLib/Math/Vector3.h"
#include "Button.h"
#include "FrontDisplayh.h"
#include "SoundGenerator.h"
#include "Game.h"

using namespace GameLib;
using namespace GameLib::Math;
using namespace GameLib::Scene;

Title::Title():
	mFinished(false),
	mCount(0.f)
{
	//���f����񂪊i�[����Ă���R���e�i��ǂݍ���
	mContainer = Container::create("data/model/background.txt");
	while (!mContainer.isReady()) { ; }
	mModelBack = mContainer.createModel("back");
	mModelStage = mContainer.createModel("stage");
	mPC = new PlayCharacter();
	//�����ݒ�
	mModelBack.setScale(Vector3(1000.f, 1000.f, 1000.f));
	mModelStage.setScale(Vector3(1200.f, 1200.f, 1200.f));
	mPC->setPosition(Vector3(3.f, 0.f, 0.f));
	mPC->subHP(mPC->getHP());	//HP�o�[���ז��Ȃ̂ŏ����Ă��邾��
	SoundGenerator::instance()->stopBgm();
	SoundGenerator::instance()->playSe(SoundGenerator::Se::TITLE);
	Button::instance()->resetDemo();
}

Title::~Title() {
	mContainer.release();
	mModelBack.release();
	mModelStage.release();
	SAFE_DELETE(mPC);
}

void Title::update()
{
	mCount += 0.0031f;

	//��莞�Ԃ̊Ԃ̓^�C�g����ʂ��y����ł��炤
	if (mCount < 1.f)
		return;

	FrontDisplay* front = FrontDisplay::instance();
	Button* button = Button::instance();


	//�X�y�[�X�L�[�Ői��
	if (!toSelect && button->isTriggered(Button::Key::KEY_SPACE)) {
		toSelect = true;
		return;
	}

	//test
	if (button->demo()) {
		mFinished = true;
		return;
	}

	if(toSelect){
		front->showMessage(20);
		int input = front->showCursor(2, FrontDisplay::CURSOR_TEXT);
		if (button->isTriggered(Button::Key::KEY_SPACE)) {
			mFinished = true;
			if (input == 2) {
				Button::instance()->thisGameIsDemo();
			}
		}
	}
}

void Title::draw()
{
	mModelBack.draw();
	mModelStage.draw();
	mPC->draw();
}

void Title::sendInfoToCamera(
	Camera::Mode* mode,
	Vector3* playerPos,
	Vector3* enemyPos,
	float* eyeAngleYofPlayer,
	float* time
) const
{
	*mode = Camera::Mode::MODE_TITLE;
	playerPos->set(0.f, 0.f, 0.f);
	enemyPos->set(0.f, 0.f, 0.f);
	*eyeAngleYofPlayer = 0.f;
	*time = mCount;
}

bool Title::isFinished() const
{
	return mFinished;
}

Sequence* Title::createNextSequence()
{
	return new Game();
}