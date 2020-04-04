#include "GameLib/GameLib.h"
#include "GameLib/Math/Vector3.h"
#include "GameLib/Math/Matrix34.h"
#include "PlayCharacter.h"
#include "Action.h"
#include "Button.h"
#include "FrontDisplayh.h"
#include "MyRandom.h"

using namespace GameLib;
using namespace GameLib::Math;

PlayCharacter::PlayCharacter()
{
	this->setModel("playcharacter");
	this->setName("PLAYER");
	this->mMind = Character::Mind::MIND_INPUT;
	//ƒfƒ‚ƒvƒŒƒC‚©
	if(Button::instance()->demo()) {
		this->mMind = Character::Mind::MIND_PLAYER_AI;
		mParamAI.set(80, 90, 70, 0, 100, 25);
	}
}

PlayCharacter::~PlayCharacter()
{

}

void PlayCharacter::addParts(Parts parts)
{
	mParts->add(parts);
}

void PlayCharacter::subParts(Parts parts)
{
	mParts->sub(parts);
}

bool PlayCharacter::judgeParts(Parts parts)
{
	return mParts->judge(parts);
}
