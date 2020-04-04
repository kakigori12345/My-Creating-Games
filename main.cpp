#include "GameLib/Framework.h"
#include "GameLib/Math/Vector3.h"
using namespace GameLib;
using namespace GameLib::Math;

#include "Game.h"//test
#include "GameClear.h"//test
#include "GameOver.h"//test
#include "Title.h"
#include "Camera.h"
#include "Button.h"
#include "MonsterFactory.h"
#include "Action.h"
#include "MyRandom.h"
#include "FrontDisplayh.h"
#include "ContentReader.h"
#include "EffectController.h"
#include "SoundGenerator.h"

Sequence* gSequence;	//シーケンス遷移 [1.Title  2.Game]
Camera* gCamera;		//視点を管理するカメラ

void FreeMemory() {
	SAFE_DELETE(gSequence);
	SAFE_DELETE(gCamera);
	Button::destroy();
	Action::destroy();
	MyRandom::destroy();
	FrontDisplay::destroy();
	EffectController::destroy();
	SoundGenerator::destroy();
}

//グローバル変数
bool isFirstFrame = true;

namespace GameLib {
	//画面表示の初期設定
	void Framework::configure(Configuration* c) {
		//設定値の読み込み
		ContentReader* cr = new ContentReader("data/contents.txt");
		cr->readStart();//読み込みスタート
		int width = (int)cr->getData("Main", "WIDTH");
		int height = (int)cr->getData("Main", "HEIGHT");
		const char* title = (cr->getData_str("Main", "TITLE")).c_str();
		SAFE_DELETE(cr);//使い終わったら削除
		c->setWidth(width);
		c->setHeight(height);
		c->enableVSync(true);
		c->setTitle(title);
	}

	//ループ処理 Framework::update() の中がループする
	void Framework::update() {
		//1回だけ処理
		if (isFirstFrame) {
			isFirstFrame = false;
			//必要なものを生成（特にシングルトンのインスタンスを生成しておく）
			Button::create();
			Action::create();
			MyRandom::create();
			FrontDisplay::create();
			EffectController::create();
			SoundGenerator::create();
			gSequence = new Title();
			//gSequence = new Game();
			gCamera = new Camera();
			while (!SoundGenerator::instance()->hasLoaded()) { ; }
		}

		Button* button = Button::instance();
		SoundGenerator* sound = SoundGenerator::instance();

		//以下ループ
		//ゲームの本体
		gSequence->update();
		//BGMやSEのON,OFFを切り替える
		sound->switchBGMandSE();

		//ここでカメラ処理
		//現在稼働しているシーケンスから情報をもらってカメラ操作
		Camera::Mode mode; Vector3 pPos; Vector3 ePos; float angY; float time;//伝達用変数
		gSequence->sendInfoToCamera(&mode, &pPos, &ePos, &angY, &time);
		gCamera->setViewMatt(mode,&pPos,&ePos,angY,time);
		gCamera->update();
		//エフェクト管理クラスにカメラアングルを渡してあげる
		EffectController::instance()->setEyeAngle(gCamera->getCameraAngle());

		//描画
		gSequence->draw();
		EffectController::instance()->draw();
		FrontDisplay::instance()->draw();

		//その他の処理
		//乱数のカウントを進める
		MyRandom::instance()->incSeed();
		//シーン遷移が起こるなら、次のシーケンスを生成
		if (gSequence->isFinished()) {
			Sequence* next = gSequence->createNextSequence();
			SAFE_DELETE(gSequence);
			gSequence = next;
		}

		//終了処理
		if (isEndRequested()) {
			FreeMemory();
		}

	}
}