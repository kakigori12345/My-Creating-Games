#include "GameClear.h"
#include "Title.h"
#include "Button.h"
#include "SoundGenerator.h"

GameClear::GameClear() :
	mFinished(false),
	mCount(0.f)
{
	SoundGenerator::instance()->playBgm(SoundGenerator::Bgm::GAME_CLEAR);
}

GameClear::~GameClear()
{

}

void GameClear::update()
{
	mCount += 0.01f;

	if (Button::instance()->isTriggered(Button::Key::KEY_SPACE))
		mFinished = true;
}

void GameClear::draw()
{

}

void GameClear::sendInfoToCamera(
	Camera::Mode* mode,
	GameLib::Math::Vector3* playerPos,
	GameLib::Math::Vector3* enemyPos,
	float* eyeAngleYofPlayer,
	float* time
) const
{
	*mode = Camera::Mode::MODE_GAME_CLEAR;
	*time = mCount;
}

bool GameClear::isFinished() const
{
	return mFinished;
}

Sequence* GameClear::createNextSequence()
{
	return new Title();
}