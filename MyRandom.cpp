#include "MyRandom.h"
#include "GameLib/Framework.h"

using namespace GameLib;
using namespace GameLib::Math;

namespace {
	int mom = 1000;
}

MyRandom* MyRandom::mInstance = nullptr;

MyRandom::MyRandom() : seed(0){
	mRand = Random::create(seed);
}

MyRandom::~MyRandom() {
	mRand.release();
}

void MyRandom::create() {
	ASSERT(!mInstance);
	mInstance = new MyRandom();
}

void MyRandom::destroy() {
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

MyRandom* MyRandom::instance() {
	return mInstance;
}

void MyRandom::incSeed()
{
	seed++;
	seed %= mom;
}

void MyRandom::resetRand()
{
	mRand.release();
	mRand = Random::create(seed);
}

int MyRandom::getInt(int a, int b)
{
	int x = mRand.getInt(a, b);
	
	return x;
}

float MyRandom::getFloat(float a, float b)
{
	float x = mRand.getFloat(a, b);

	return x;
}

bool MyRandom::fortune(int rate)
{
	int rand = getInt(0, 100);
	if (rand < rate)
		return true;
	
	return false;
}