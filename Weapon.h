#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"
#include <string>


struct Parts;

class Weapon {
public:
	enum Type {
		Soard,	//剣：パワー強化＋周りの4マスに範囲攻撃可能
		Rod,	//杖：遠距離攻撃が可能
		INVALID
	};
	Weapon();
	~Weapon();
	void draw();
	Type getWeaponType() const;
	std::string getWeaponName() const;
	int getWeaponLevel() const;
	//強化に必要なパーツを返す
	static Parts getNeededParts(Type type, int weaponLevel);
	//武器を強化する
	void powerUP();
	//武器によるパワーの増強分を返す
	int getAdditionalPower() const;
	//その武器を使うことで、攻撃が届く最大距離を返す
	int getMaxDistanceOfAttack() const;
	//モデルに関して
	void setModel(Type type, const GameLib::Scene::Container& c, const char* batchname);
	void setPosition(const GameLib::Math::Vector3& pos);
	void setScale(float scale);
	void setAngleY(const float& angle);
private:
	Type mType;			//武器の種類
	int mLevel;			//武器レベル	
	//表示モデル
	GameLib::Scene::Model mModel;	//キャラクターのモデル
	GameLib::Math::Vector3 mPosition;		//現在の位置
	GameLib::Math::Vector3 mScale;			//大きさ
	float mAngleY;							//角度
};