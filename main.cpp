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

Sequence* gSequence;	//�V�[�P���X�J�� [1.Title  2.Game]
Camera* gCamera;		//���_���Ǘ�����J����

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

//�O���[�o���ϐ�
bool isFirstFrame = true;

namespace GameLib {
	//��ʕ\���̏����ݒ�
	void Framework::configure(Configuration* c) {
		//�ݒ�l�̓ǂݍ���
		ContentReader* cr = new ContentReader("data/contents.txt");
		cr->readStart();//�ǂݍ��݃X�^�[�g
		int width = (int)cr->getData("Main", "WIDTH");
		int height = (int)cr->getData("Main", "HEIGHT");
		const char* title = (cr->getData_str("Main", "TITLE")).c_str();
		SAFE_DELETE(cr);//�g���I�������폜
		c->setWidth(width);
		c->setHeight(height);
		c->enableVSync(true);
		c->setTitle(title);
	}

	//���[�v���� Framework::update() �̒������[�v����
	void Framework::update() {
		//1�񂾂�����
		if (isFirstFrame) {
			isFirstFrame = false;
			//�K�v�Ȃ��̂𐶐��i���ɃV���O���g���̃C���X�^���X�𐶐����Ă����j
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

		//�ȉ����[�v
		//�Q�[���̖{��
		gSequence->update();
		//BGM��SE��ON,OFF��؂�ւ���
		sound->switchBGMandSE();

		//�����ŃJ��������
		//���݉ғ����Ă���V�[�P���X�������������ăJ��������
		Camera::Mode mode; Vector3 pPos; Vector3 ePos; float angY; float time;//�`�B�p�ϐ�
		gSequence->sendInfoToCamera(&mode, &pPos, &ePos, &angY, &time);
		gCamera->setViewMatt(mode,&pPos,&ePos,angY,time);
		gCamera->update();
		//�G�t�F�N�g�Ǘ��N���X�ɃJ�����A���O����n���Ă�����
		EffectController::instance()->setEyeAngle(gCamera->getCameraAngle());

		//�`��
		gSequence->draw();
		EffectController::instance()->draw();
		FrontDisplay::instance()->draw();

		//���̑��̏���
		//�����̃J�E���g��i�߂�
		MyRandom::instance()->incSeed();
		//�V�[���J�ڂ��N����Ȃ�A���̃V�[�P���X�𐶐�
		if (gSequence->isFinished()) {
			Sequence* next = gSequence->createNextSequence();
			SAFE_DELETE(gSequence);
			gSequence = next;
		}

		//�I������
		if (isEndRequested()) {
			FreeMemory();
		}

	}
}