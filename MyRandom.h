#pragma once

#include "GameLib/Math/Random.h"

class MyRandom {
public:
	static MyRandom* instance();
	static void create();
	static void destroy();

	//機能
	void incSeed();
	void resetRand();
	int getInt(int a, int b);
	float getFloat(float a, float b);
	//0～100のパーセンテージに従って、確率でtrueかfalseか返す
	bool fortune(int rate);
private:
	MyRandom();
	~MyRandom();
	static MyRandom* mInstance;
	GameLib::Math::Random mRand;
	int seed;
};