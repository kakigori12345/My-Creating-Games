#include "SoundGenerator.h"
#include "GameLib/GameLib.h"
#include "GameLib/Sound/Wave.h"
#include "GameLib/Sound/Player.h"
#include <sstream>
#include "Button.h"
using namespace GameLib;

SoundGenerator* SoundGenerator::mInstance = nullptr;

SoundGenerator* SoundGenerator::instance() {
	return mInstance;
}

void SoundGenerator::create() {
	ASSERT(!mInstance);
	mInstance = new SoundGenerator();
}

void SoundGenerator::destroy() {
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

SoundGenerator::SoundGenerator() :
	mSePlayerPos(0),
	mBGMswitchOn(true),
	mSEswitchOn(true),
	mSetBGM(Bgm::BGM_INVALID)
{
	//�t�@�C�������X�g�����[�h���Ă���
	const char* bgmFiles[] = {
		"last-war.wav",		//GAME
		"fanfare.wav",		//GAME_CLEAR
		"carstopcrash.wav",	//GAME_OVER
	};
	const char* seFiles[] = {
		"dram.wav",			//TITLE
		"system1.wav",		//SELECT_CURSOR_MOVE
		"punch5.wav",		//SELECT_FINISH
		"organ.wav",		//MAKE_FRIEND_OCCUR
		"run away.wav",		//MAKE_FRIEND_BAD
		"finding2.wav",		//MAKE_FRIEND_GOOD
		"select2.wav",		//CHARACTER_MOVE
		"beam2.wav",		//CHARACTER_WARP
		"retro1.wav",		//CHARACTER_ATTACK_NORMAL
		"magic2.wav",		//CHARACTER_ATTACK_MAGIC
		"fly.wav",			//CHARACTER_FALL
		"alarm4.wav",		//CHARACTER_CATCH
		"noise.wav",		//STAGE_MASS_STOPPER
		"bomb.wav",			//STAGE_MASS_BOMB
		"system3.wav",		//STAGE_MASS_RECOVERY
		"finding.wav",		//STAGE_MASS_PARTS
	};
	//�t�@�C�����w�肵��Wave���쐬���Ă���
	std::ostringstream oss;
	for (int i = 0; i < BGM_INVALID; ++i) {
		oss.str("");//������
		oss << "data/sound/bgm/" << bgmFiles[i];
		mBgmWaves[i] = Sound::Wave::create(oss.str().c_str());
	}
	for (int i = 0; i < SE_INVALID; ++i) {
		oss.str("");//������
		oss << "data/sound/se/" << seFiles[i];
		mSeWaves[i] = Sound::Wave::create(oss.str().c_str());
	}
}

SoundGenerator::~SoundGenerator() {}

bool SoundGenerator::hasLoaded() {
	//Wave�����ׂď������������ׂ�B��ł��_���Ȃ�false
	bool ret = true;
	for (int i = 0; i < BGM_INVALID; ++i)
		ret = ret && mBgmWaves[i].isReady();
	for (int i = 0; i < SE_INVALID; ++i)
		ret = ret && mSeWaves[i].isReady();

	return ret;
}

void SoundGenerator::playBgm(Bgm bgm) {
	mSetBGM = bgm;
	mBgmPlayer = Sound::Player::create(mBgmWaves[bgm]);
	if (mBGMswitchOn)
		mBgmPlayer.play(true);//���[�v�Đ�
}

void SoundGenerator::stopBgm() {
	if (mBgmPlayer)
		mBgmPlayer.stop();
}

void SoundGenerator::playSe(Se se) {
	if (mSEswitchOn) {
		mSePlayers[mSePlayerPos] = Sound::Player::create(mSeWaves[se]);
		mSePlayers[mSePlayerPos].play();//�Đ�
		++mSePlayerPos;
		//�Ō�܂ŗ�����ŏ��ɖ߂�B�Â����̂�������Ă���
		if (mSePlayerPos == SE_PLAYER_MAX)
			mSePlayerPos = 0;
	}
}

void SoundGenerator::switchBGMandSE()
{
	Button* button = Button::instance();

	if (button->isTriggered(Button::Key::KEY_O))
		switchBGM();
	if (button->isTriggered(Button::Key::KEY_P))
		switchSe();
}

void SoundGenerator::switchBGM()
{
	//�؂�ւ���
	mBGMswitchOn = !mBGMswitchOn;
	//BGM��炷
	if (mBGMswitchOn)
		playBgm(mSetBGM);
	//BGM���Z�b�g����Ă�����~�߂�
	else if (!mBGMswitchOn && mBgmPlayer)
		mBgmPlayer.stop();
}

void SoundGenerator::switchSe()
{
	//�؂�ւ���
	mSEswitchOn = !mSEswitchOn;
}

