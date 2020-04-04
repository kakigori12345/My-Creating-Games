#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"
#include <string>


struct Parts;

class Weapon {
public:
	enum Type {
		Soard,	//���F�p���[�����{�����4�}�X�ɔ͈͍U���\
		Rod,	//��F�������U�����\
		INVALID
	};
	Weapon();
	~Weapon();
	void draw();
	Type getWeaponType() const;
	std::string getWeaponName() const;
	int getWeaponLevel() const;
	//�����ɕK�v�ȃp�[�c��Ԃ�
	static Parts getNeededParts(Type type, int weaponLevel);
	//�������������
	void powerUP();
	//����ɂ��p���[�̑�������Ԃ�
	int getAdditionalPower() const;
	//���̕�����g�����ƂŁA�U�����͂��ő勗����Ԃ�
	int getMaxDistanceOfAttack() const;
	//���f���Ɋւ���
	void setModel(Type type, const GameLib::Scene::Container& c, const char* batchname);
	void setPosition(const GameLib::Math::Vector3& pos);
	void setScale(float scale);
	void setAngleY(const float& angle);
private:
	Type mType;			//����̎��
	int mLevel;			//���탌�x��	
	//�\�����f��
	GameLib::Scene::Model mModel;	//�L�����N�^�[�̃��f��
	GameLib::Math::Vector3 mPosition;		//���݂̈ʒu
	GameLib::Math::Vector3 mScale;			//�傫��
	float mAngleY;							//�p�x
};