#pragma once

#include "GameLib/Math/Random.h"

class MyRandom {
public:
	static MyRandom* instance();
	static void create();
	static void destroy();

	//�@�\
	void incSeed();
	void resetRand();
	int getInt(int a, int b);
	float getFloat(float a, float b);
	//0�`100�̃p�[�Z���e�[�W�ɏ]���āA�m����true��false���Ԃ�
	bool fortune(int rate);
private:
	MyRandom();
	~MyRandom();
	static MyRandom* mInstance;
	GameLib::Math::Random mRand;
	int seed;
};