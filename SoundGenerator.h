#pragma once

#include "GameLib/Sound/Wave.h"
#include "GameLib/Sound/Player.h"

//BGM�ASE��炷���̑S�ʂ�S���B�V���O���g��
class SoundGenerator {
public:
	static SoundGenerator* instance();
	static void create();//����炷�O��1��K���Ă�
	static void destroy();
	bool hasLoaded();//���[�h���������Ă��邩�m�F

	enum Bgm {
		GAME,			//�Q�[����
		GAME_CLEAR,		//�Q�[���N���A
		GAME_OVER,		//�Q�[���I�[�o

		BGM_INVALID
	};
	enum Se {
		//�V�[�P���X���Ƃ̉��y
		TITLE,			//�^�C�g�����
		//�Q�[�����̌��ʉ�
		//��{�I�Ȍ��ʉ��B�Ⴆ�ΑI�����Ȃ�
		SELECT_CURSOR_MOVE,		//�I�𒆂ɃJ�[�\���𓮂�����
		SELECT_FINISH,			//�I���Ō��肵��
		//�C�x���g���Ƃ̓���SE
		MAKE_FRIEND_OCCUR,		//���ԃC�x���g����
		MAKE_FRIEND_BAD,		//���Ԃɂ��邩�ǂ��������Ȃ�
		MAKE_FRIEND_GOOD,		//���Ԃɂ��邩�ǂ���������
		//�L�����N�^�[�̓���Ɋւ���SE
		CHARACTER_MOVE,			//�ړ�
		CHARACTER_WARP,			//���[�v
		CHARACTER_ATTACK_NORMAL,//�ʏ�U��
		CHARACTER_ATTACK_MAGIC,	//�������U��
		CHARACTER_FALL,			//���Ƃ���
		CHARACTER_CATCH,		//�ߊl�
		//�X�e�[�W�̃}�X�̌��ʂɊւ���SE
		STAGE_MASS_STOPPER,		//���~��㩔���
		STAGE_MASS_BOMB,		//�n������
		STAGE_MASS_RECOVERY,	//��
		STAGE_MASS_PARTS,		//�p�[�c�Q�b�g

		SE_INVALID
	};
	//BGM��炷�B���[�v�Đ�
	void playBgm(Bgm);
	void stopBgm();
	//SE��炷�B
	void playSe(Se);
	//���y�̃X�C�b�`�̐؂�ւ�
	void switchBGMandSE();
	void switchBGM();	//���stopBgm�Ƃ̈Ⴂ�Ƃ��āA��͌��݂Ȃ��Ă���BGM���~�߂�̂ɑ΂��A�������͂�����BGM���Ȃ�Ȃ�
	void switchSe();	//����������l�B������SE���Ȃ�Ȃ��Ȃ�
private:
	SoundGenerator();
	~SoundGenerator();

	//Wave�͂͂��߂Ƀ��[�h
	GameLib::Sound::Wave mBgmWaves[BGM_INVALID];
	GameLib::Sound::Wave mSeWaves[SE_INVALID];
	//Player�͍Đ����ɐ���
	GameLib::Sound::Player mBgmPlayer;
	static const int SE_PLAYER_MAX = 4;
	GameLib::Sound::Player mSePlayers[SE_PLAYER_MAX];
	int mSePlayerPos;//����SE������ꏊ
	bool mSEswitchOn;	//SE��ON,OFF
	bool mBGMswitchOn;	//BGM��ON,OFF
	Bgm mSetBGM;		//���݃Z�b�g����Ă���BGM

	static SoundGenerator* mInstance;
};