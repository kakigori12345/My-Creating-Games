#include "Weapon.h"
#include "Character.h"
#include "GameLib/Framework.h"

using namespace GameLib;
using namespace GameLib::Math;

Weapon::Weapon():
	mType(INVALID),
	mLevel(1)
{
	
}

Weapon::~Weapon()
{
	mModel.release();
}

void Weapon::draw()
{
	mModel.setPosition(mPosition);
	mModel.setAngle(Vector3(0.f, mAngleY, 0.f));
	mModel.setScale(mScale);
	mModel.draw();
}

Weapon::Type Weapon::getWeaponType() const
{
	return mType;
}

std::string Weapon::getWeaponName() const
{
	switch (mType) {
	case Type::Soard:
		return "Soard";
	case Type::Rod:
		return "Rod";
	case Type::INVALID:
		return "Nothing";
	default:
		HALT("File: Weapon.cpp [getWeaponName()] switch Error");
	}

	return "";
}

int Weapon::getWeaponLevel() const
{
	return mLevel;
}

//���݂̕��탌�x���̈��̃��x���ŕK�v�ƂȂ�p�[�c��Ԃ�
Parts Weapon::getNeededParts(Type type, int weaponLevel)
{
	Parts needParts = Parts();
	
	switch (type) {
	case Soard:
		needParts.Parts_Screw = 4;
		needParts.Parts_Hammer = 1;
		needParts.Parts_Net = 0;
		needParts.Parts_Stone = 4;
		break;
	case Rod:
		needParts.Parts_Screw = 2;
		needParts.Parts_Hammer = 1;
		needParts.Parts_Net = 4;
		needParts.Parts_Stone = 1;
		break;
	default:
		HALT("File:Weapon.cpp [getNeededParts()] switch Error");
	}
	

	//���x���ɉ����ĕK�v�p�[�c�𑝂₷
	needParts.mul(weaponLevel + 1);

	return needParts;
}

void Weapon::powerUP()
{
	mLevel++;
}

int Weapon::getAdditionalPower() const
{
	float basePower = 0;

	switch (mType) {
	case Type::Soard:
		basePower = 30.f;
		break;
	case Type::Rod:
		basePower = 10.f;
		break;
	case Type::INVALID:
		break;
	}

	//���탌�x���ɂ���đ������ς��
	float amplifier = 0.5f * (float)(1+mLevel);
	basePower *= amplifier;

	return (int)basePower;
}

//���킲�Ƃ̍U���˒������B���͊�{1�}�X�����A��Ȃ�2�}�X�ȏ�̋������͂��悤�ɂ���i���x���ɉ����āj
int Weapon::getMaxDistanceOfAttack() const
{
	int distance = 0;

	switch (mType) {
	case Type::Soard:
		distance = 1;
		break;
	case Type::Rod:
		distance = 1 + mLevel;
		break;
	case Type::INVALID:
		break;
	}

	return distance;
}

void Weapon::setModel(Type type, const GameLib::Scene::Container& c, const char* batchname)
{
	mType = type;
	mModel = c.createModel(batchname);
}

void Weapon::setPosition(const Vector3& pos)
{
	mPosition = pos;
}

void Weapon::setAngleY(const float& angle)
{
	mAngleY = angle;
}

void Weapon::setScale(float scale)
{
	mScale = Vector3(scale, scale, scale);
}
