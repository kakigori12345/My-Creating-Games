#pragma once

#include "GameLib/Scene/PrimitiveRenderer.h"
#include "GameLib/Scene/StringRenderer.h"
#include "GameLib/Scene/Font.h"
#include <string>

struct Parameters; //Character.hに記載
struct Parts;
class Weapon;

//画面の最前面に表示を行うクラス（シングルトン）
//主にHPバーなどの情報を表示
class FrontDisplay {
public:
	//カーソルを表示させるときの場所を指定する
	enum Cursor {
		CURSOR_TEXT		//テキスト表示上でカーソルを表示するとき
	};
	enum Minimap {
		MAP_MASS_NORMAL,
		MAP_MASS_HOLE,
		MAP_MASS_GOAL,
		MAP_TEAM_PLAYER,
		MAP_TEAM_ENEMY,
	};
	static FrontDisplay* instance();
	static void create();//使う前に1回呼び出す
	static void destroy();
	void draw();	//毎フレームのどこかで1回これを呼ぶ。今回はmain.cppで呼んでいる
	void drawRect(GameLib::Math::Vector2* p, unsigned c1, unsigned c2);
	void renderString(std::string str, int posX, int posY, unsigned color);
	void renderString(std::string str, int posX, int posY);
	void renderString(int value, int posX, int posY);
	//メッセージを出す。numberの範囲は１～最大メッセージ数の間
	void showMessage(int number);
	//文章を直接渡して表示してもらう
	void showMessage(std::string message, int placeX, int placeY);
	//カーソル表示を出し、決定キーが押されたら現在のカーソルの場所を返す。決定キーが押されていないなら-1を返す
	int showCursor(int cursorNum, Cursor place);
	//パラメータを表示する
	//プレイヤー用
	void showParameter(const std::string name, const int maxHP, const int nowHP, const Weapon& weapon);
	//モンスター用
	void showParameterOfMonsters(int num, const std::string* names, const Parameters* param, const int* nowHPs);
	//ステージ生成時に階層を表示
	void showStageNumber(int stageNum);
	//ミニマップの表示
	void showMiniMap(int height, int width, Minimap* map);
	//プレイヤーのアクション選択時に、マス干渉に必要なパーツを表示する
	void showNeedParts(Parts need);
	//獲得したパーツを表示する
	void showGotParts(Parts got);
	//残りターン数を表示
	void showTurnLimit(int turn);
	//持っているパーツを表示
	void showParts(Parts* parts);

	//デバッグ用
	void showMasses(int** params, int height, int width);
private:
	//隠す
	FrontDisplay();
	~FrontDisplay();
	//このクラス内でしか使わないもの（汎用的な処理をまとめたり）
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
	std::string* mMessages;		//表示するメッセージ群を格納
	int mCursor;				//現在のカーソルの位置
	int mMessageLineInterval;	//メッセージの1行の幅
	//定数群　ファイルから読み出す
	//初期設定
	int PRIM_VERTEX_CAPACITY;
	int PRIM_COMMAND_CAPACITY;
	int STR_CHAR_CAPACITY;
	int STR_REQUEST_CAPACITY;
	int FONT_1_SIZE;
	int FONT_2_SIZE;
	int FONT_3_SIZE;
	int WINDOW_WIDTH;
	int WINDOW_HEIGTH;
	//メッセージ表示の細かい設定値
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
	//カーソル表示
	float CURSOR_PLACE_X;
	float CURSOR_PLACE_Y;
	float CURSOR_INTERVAL;
	float CURSOR_HALF_SCALE_X;
	float CURSOR_HALF_SCALE_Y;
	//パラメータ表示
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
	//ミニマップ表示
	float MINIMAP_PLACE_RIGHT;
	float MINIMAP_PLACE_DOWN;
	float MINIMAP_LENGTH;
};