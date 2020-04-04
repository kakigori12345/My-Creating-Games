#include "GameOver.h"
#include "Title.h"
#include "Button.h"
#include "SoundGenerator.h"

GameOver::GameOver():
	mFinished(false),
	mCount(0.f)
{
	SoundGenerator::instance()->playBgm(SoundGenerator::Bgm::GAME_OVER);
}

GameOver::~GameOver()
{

}

void GameOver::update()
{
	mCount += 0.01f;

	if (Button::instance()->isTriggered(Button::Key::KEY_SPACE))
		mFinished = true;
}

void GameOver::draw()
{

}

void GameOver::sendInfoToCamera(
	Camera::Mode* mode,
	GameLib::Math::Vector3* playerPos,
	GameLib::Math::Vector3* enemyPos,
	float* eyeAngleYofPlayer,
	float* time
) const
{
	*mode = Camera::Mode::MODE_GAME_OVER;
	*time = mCount;
}

bool GameOver::isFinished() const
{
	return mFinished;
}

Sequence* GameOver::createNextSequence()
{
	return new Title();
}