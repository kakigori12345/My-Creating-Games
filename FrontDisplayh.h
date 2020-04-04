#pragma once

#include "GameLib/Scene/PrimitiveRenderer.h"
#include "GameLib/Scene/StringRenderer.h"
#include "GameLib/Scene/Font.h"
#include <string>

struct Parameters; //Character.h�ɋL��
struct Parts;
class Weapon;

//��ʂ̍őO�ʂɕ\�����s���N���X�i�V���O���g���j
//���HP�o�[�Ȃǂ̏���\��
class FrontDisplay {
public:
	//�J�[�\����\��������Ƃ��̏ꏊ���w�肷��
	enum Cursor {
		CURSOR_TEXT		//�e�L�X�g�\����ŃJ�[�\����\������Ƃ�
	};
	enum Minimap {
		MAP_MASS_NORMAL,
		MAP_MASS_HOLE,
		MAP_MASS_GOAL,
		MAP_TEAM_PLAYER,
		MAP_TEAM_ENEMY,
	};
	static FrontDisplay* instance();
	static void create();//�g���O��1��Ăяo��
	static void destroy();
	void draw();	//���t���[���̂ǂ�����1�񂱂���ĂԁB�����main.cpp�ŌĂ�ł���
	void drawRect(GameLib::Math::Vector2* p, unsigned c1, unsigned c2);
	void renderString(std::string str, int posX, int posY, unsigned color);
	void renderString(std::string str, int posX, int posY);
	void renderString(int value, int posX, int posY);
	//���b�Z�[�W���o���Bnumber�͈̔͂͂P�`�ő僁�b�Z�[�W���̊�
	void showMessage(int number);
	//���͂𒼐ړn���ĕ\�����Ă��炤
	void showMessage(std::string message, int placeX, int placeY);
	//�J�[�\���\�����o���A����L�[�������ꂽ�猻�݂̃J�[�\���̏ꏊ��Ԃ��B����L�[��������Ă��Ȃ��Ȃ�-1��Ԃ�
	int showCursor(int cursorNum, Cursor place);
	//�p�����[�^��\������
	//�v���C���[�p
	void showParameter(const std::string name, const int maxHP, const int nowHP, const Weapon& weapon);
	//�����X�^�[�p
	void showParameterOfMonsters(int num, const std::string* names, const Parameters* param, const int* nowHPs);
	//�X�e�[�W�������ɊK�w��\��
	void showStageNumber(int stageNum);
	//�~�j�}�b�v�̕\��
	void showMiniMap(int height, int width, Minimap* map);
	//�v���C���[�̃A�N�V�����I�����ɁA�}�X���ɕK�v�ȃp�[�c��\������
	void showNeedParts(Parts need);
	//�l�������p�[�c��\������
	void showGotParts(Parts got);
	//�c��^�[������\��
	void showTurnLimit(int turn);
	//�����Ă���p�[�c��\��
	void showParts(Parts* parts);

	//�f�o�b�O�p
	void showMasses(int** params, int height, int width);
private:
	//�B��
	FrontDisplay();
	~FrontDisplay();
	//���̃N���X���ł����g��Ȃ����́i�ėp�I�ȏ������܂Ƃ߂���j
	void renderMessageFrame(
		float left,
		float right,
		float up,
		float down,
		unsigned int color1,
		unsigned int color2);

	static FrontDisplay* mInstance;
	GameLib::Scene::PrimitiveRenderer mPrim;
	GameLib::Scene::StringRenderer mStr;
	GameLib::Scene::Font mFont_1;
	GameLib::Scene::Font mFont_2;
	GameLib::Scene::Font mFont_3;
	std::string* mMessages;		//�\�����郁�b�Z�[�W�Q���i�[
	int mCursor;				//���݂̃J�[�\���̈ʒu
	int mMessageLineInterval;	//���b�Z�[�W��1�s�̕�
	//�萔�Q�@�t�@�C������ǂݏo��
	//�����ݒ�
	int PRIM_VERTEX_CAPACITY;
	int PRIM_COMMAND_CAPACITY;
	int STR_CHAR_CAPACITY;
	int STR_REQUEST_CAPACITY;
	int FONT_1_SIZE;
	int FONT_2_SIZE;
	int FONT_3_SIZE;
	int WINDOW_WIDTH;
	int WINDOW_HEIGTH;
	//���b�Z�[�W�\���ׂ̍����ݒ�l
	int NUM_OF_MESSAGE;
	float MESSAGE_BACK_UP;
	float MESSAGE_BACK_DOWN;
	float MESSAGE_BACK_LEFT;
	float MESSAGE_BACK_RIGHT;
	float MESSAGE_BACK_FRAME_THICK;
	float MESSAGE_PLACE_X;
	float MESSAGE_PLACE_Y;
	int MESSAGE_PLACE_X_INT;
	int MESSAGE_PLACE_Y_INT;
	int MESSAGE_WORDS_PER_LINE;
	int MESSAGE_LINE_INTERVAL;
	//�J�[�\���\��
	float CURSOR_PLACE_X;
	float CURSOR_PLACE_Y;
	float CURSOR_INTERVAL;
	float CURSOR_HALF_SCALE_X;
	float CURSOR_HALF_SCALE_Y;
	//�p�����[�^�\��
	float PARAM_BACK_UP;
	float PARAM_BACK_DOWN;
	float PARAM_BACK_LEFT;
	float PARAM_BACK_RIGHT;
	float PARAM_BACK_FRAME_THICK;
	float PARAM_PLACE_X;
	float PARAM_PLACE_Y;
	int PARAM_PLACE_X_INT;
	int PARAM_PLACE_Y_INT;
	int PARAM_LINE_INTERVAL;
	float PARAM_PLACE_X_FOR_MONSTERS;
	float PARAM_PLACE_Y_FOR_MONSTERS;
	int PARAM_PLACE_X_INT_FOR_MONSTERS;
	int PARAM_PLACE_Y_INT_FOR_MONSTERS;
	int PARAM_LINE_INTERVAL_FOR_MONSTERS;
	//�~�j�}�b�v�\��
	float MINIMAP_PLACE_RIGHT;
	float MINIMAP_PLACE_DOWN;
	float MINIMAP_LENGTH;
};