#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Scene/Model.h"
#include "PlayCharacter.h"
#include "Sequence.h"

namespace GameLib {
	namespace Math {
		class Matrix34;
		class Vector3;
	}
}

//タイトル画面のクラス
//Sequenceを継承
//次に生成するシーケンスはGameクラス
class Title : public Sequence{
public:
	Title();
	~Title();
	virtual void update();
	virtual void draw();
	virtual void sendInfoToCamera(
		Camera::Mode* mode,
		GameLib::Math::Vector3* playerPos,
		GameLib::Math::Vector3* enemyPos,
		float* eyeAngleYofPlayer,
		float* time
	) const;
	virtual bool isFinished() const;
	virtual Sequence* createNextSequence();
private:
	bool mFinished;
	bool toSelect;
	float mCount;
	GameLib::Scene::Container mContainer;
	GameLib::Scene::Model mModelBack;
	GameLib::Scene::Model mModelStage;
	PlayCharacter* mPC;
};