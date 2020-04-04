#include "GameLib/GameLib.h"
#include "GameLib/Framework.h"
#include "GameLib/Math/Vector2.h"
#include "GameLib/Math/Vector4.h"
#include <sstream>
#include "Character.h"
#include "Weapon.h"
#include "FrontDisplayh.h"
#include "ContentReader.h"
#include "Button.h"

using namespace GameLib;
using namespace GameLib::Scene;
using namespace GameLib::Math;
using namespace std;

FrontDisplay* FrontDisplay::mInstance = 0;

FrontDisplay::FrontDisplay() :
	mCursor(1),
	mMessageLineInterval(10)
{
	//定数の読み込み
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	PRIM_VERTEX_CAPACITY	= (int)cr->getData("FrontDisplay", "PRIM_VERTEX_CAPACITY");
	PRIM_VERTEX_CAPACITY	+= 10 * (int)cr->getData("Stage", "MAX_MASS_WIDTH") * (int)cr->getData("Stage", "MAX_MASS_HEIGHT");
	PRIM_COMMAND_CAPACITY	= (int)cr->getData("FrontDisplay", "PRIM_COMMAND_CAPACITY");
	PRIM_COMMAND_CAPACITY   += (int)cr->getData("Stage", "MAX_MASS_WIDTH") * (int)cr->getData("Stage", "MAX_MASS_HEIGHT");
	STR_CHAR_CAPACITY		= (int)cr->getData("FrontDisplay", "STR_CHAR_CAPACITY");
	STR_REQUEST_CAPACITY	= (int)cr->getData("FrontDisplay", "STR_REQUEST_CAPACITY");
	FONT_1_SIZE				= (int)cr->getData("FrontDisplay", "FONT_1_SIZE");
	FONT_2_SIZE				= (int)cr->getData("FrontDisplay", "FONT_2_SIZE");
	FONT_3_SIZE				= (int)cr->getData("FrontDisplay", "FONT_3_SIZE");
	WINDOW_WIDTH			= (int)cr->getData("Main", "WIDTH");
	WINDOW_HEIGTH			= (int)cr->getData("Main", "HEIGHT");
	MESSAGE_BACK_UP			=  cr->getData("FrontDisplay", "MESSAGE_BACK_UP");
	MESSAGE_BACK_DOWN		=  cr->getData("FrontDisplay", "MESSAGE_BACK_DOWN");
	MESSAGE_BACK_LEFT		=  cr->getData("FrontDisplay", "MESSAGE_BACK_LEFT");
	MESSAGE_BACK_RIGHT		=  cr->getData("FrontDisplay", "MESSAGE_BACK_RIGHT");
	MESSAGE_BACK_FRAME_THICK=  cr->getData("FrontDisplay", "MESSAGE_BACK_FRAME_THICK");
	MESSAGE_PLACE_X			=  cr->getData("FrontDisplay", "MESSAGE_PLACE_X");
	MESSAGE_PLACE_X_INT		= (int)(MESSAGE_PLACE_X * WINDOW_WIDTH);
	MESSAGE_PLACE_Y			=  cr->getData("FrontDisplay", "MESSAGE_PLACE_Y");
	MESSAGE_PLACE_Y_INT		= (int)(MESSAGE_PLACE_Y * WINDOW_HEIGTH);
	MESSAGE_WORDS_PER_LINE	= (int)cr->getData("FrontDisplay", "MESSAGE_WORDS_PER_LINE");
	MESSAGE_LINE_INTERVAL	= (int)cr->getData("FrontDisplay", "MESSAGE_LINE_INTERVAL");
	CURSOR_PLACE_X			=  cr->getData("FrontDisplay", "CURSOR_PLACE_X");
	CURSOR_PLACE_Y			=  cr->getData("FrontDisplay", "CURSOR_PLACE_Y");
	CURSOR_INTERVAL			=  cr->getData("FrontDisplay", "CURSOR_INTERVAL");
	CURSOR_HALF_SCALE_X		=  cr->getData("FrontDisplay", "CURSOR_HALF_SCALE_X");
	CURSOR_HALF_SCALE_Y		=  cr->getData("FrontDisplay", "CURSOR_HALF_SCALE_Y");
	PARAM_BACK_UP			= cr->getData("FrontDisplay", "PARAM_BACK_UP");
	PARAM_BACK_DOWN			= cr->getData("FrontDisplay", "PARAM_BACK_DOWN");
	PARAM_BACK_LEFT			= cr->getData("FrontDisplay", "PARAM_BACK_LEFT");
	PARAM_BACK_RIGHT		= cr->getData("FrontDisplay", "PARAM_BACK_RIGHT");
	PARAM_BACK_FRAME_THICK	= cr->getData("FrontDisplay", "PARAM_BACK_FRAME_THICK");
	PARAM_PLACE_X			= cr->getData("FrontDisplay", "PARAM_PLACE_X");
	PARAM_PLACE_X_INT		= (int)(PARAM_PLACE_X * cr->getData("Main", "WIDTH"));
	PARAM_PLACE_Y			= cr->getData("FrontDisplay", "PARAM_PLACE_Y");
	PARAM_PLACE_Y_INT		= (int)(PARAM_PLACE_Y * cr->getData("Main", "HEIGHT"));
	PARAM_LINE_INTERVAL		= (int)cr->getData("FrontDisplay", "PARAM_LINE_INTERVAL");
	PARAM_PLACE_X_FOR_MONSTERS			= cr->getData("FrontDisplay", "PARAM_PLACE_X_FOR_MONSTERS");
	PARAM_PLACE_X_INT_FOR_MONSTERS		= (int)(PARAM_PLACE_X_FOR_MONSTERS * cr->getData("Main", "WIDTH"));
	PARAM_PLACE_Y_FOR_MONSTERS			= cr->getData("FrontDisplay", "PARAM_PLACE_Y_FOR_MONSTERS");
	PARAM_PLACE_Y_INT_FOR_MONSTERS		= (int)(PARAM_PLACE_Y_FOR_MONSTERS * cr->getData("Main", "HEIGHT"));
	PARAM_LINE_INTERVAL_FOR_MONSTERS	= (int)cr->getData("FrontDisplay", "PARAM_LINE_INTERVAL_FOR_MONSTERS");
	MINIMAP_PLACE_RIGHT					= cr->getData("FrontDisplay", "MINIMAP_PLACE_RIGHT");
	MINIMAP_PLACE_DOWN					= cr->getData("FrontDisplay", "MINIMAP_PLACE_DOWN");
	MINIMAP_LENGTH						= cr->getData("FrontDisplay", "MINIMAP_LENGTH");
	//メッセージ数は可変。読み込んでいく
	NUM_OF_MESSAGE			= (int)cr->getData("FrontDisplay", "NUM_OF_MESSAGE");
	mMessages = new string[NUM_OF_MESSAGE];
	for (int i = 0; i < NUM_OF_MESSAGE; ++i)
	{
		string dataName = "MESSAGE_" + to_string(i + 1);
		mMessages[i] = cr->getData_str("FrontDisplay", dataName.c_str());
	}
	SAFE_DELETE(cr);//使い終わったら削除
	//その他初期設定
	mPrim = PrimitiveRenderer::create(PRIM_VERTEX_CAPACITY, PRIM_COMMAND_CAPACITY);
	mStr = StringRenderer::create(STR_CHAR_CAPACITY, STR_REQUEST_CAPACITY);
	mFont_1 = Font::create("ＭＳ ゴシック", FONT_1_SIZE, false, false, false, true);
	mFont_2 = Font::create("ＭＳ ゴシック", FONT_2_SIZE, false, false, false, true);
	mFont_3 = Font::create("ＭＳ ゴシック", FONT_3_SIZE, false, false, false, true);
	mStr.setFont(mFont_1);
}

FrontDisplay::~FrontDisplay() {
	mPrim.release();
	mStr.release();
	mFont_1.release();
	mFont_2.release();
	mFont_3.release();
	SAFE_DELETE_ARRAY(mMessages);
}

void FrontDisplay::create() {
	ASSERT(!mInstance);
	mInstance = new FrontDisplay();
}

void FrontDisplay::destroy() {
	ASSERT(mInstance);
	SAFE_DELETE(mInstance);
}

void FrontDisplay::draw()
{
	mPrim.draw();
	mStr.draw();
}

FrontDisplay* FrontDisplay::instance() {
	return mInstance;
}

void FrontDisplay::drawRect(Vector2* p, unsigned c1, unsigned c2)
{
	Vector4 p4[4];
	for (int i = 0; i < 4; ++i) {
		p4[i].x = p[i].x;
		p4[i].y = p[i].y;
		p4[i].z = 0.0; //zは0固定
		p4[i].w = 1.0; //wは1固定
	}
	mPrim.addTransformedTriangle(p4[0], p4[1], p4[2], c1, c1, c2);
	mPrim.addTransformedTriangle(p4[3], p4[1], p4[2], c2, c1, c2);
}

void FrontDisplay::renderString(std::string str, int posX, int posY, unsigned color)
{
	mStr.add(posX, posY, str.c_str(), color, true);
}

void FrontDisplay::renderString(std::string str, int posX, int posY)
{
	renderString(str, posX, posY, 0xffffffff);
}

void FrontDisplay::renderString(int value, int posX, int posY)
{
	std::ostringstream oss;
	oss << value;
	renderString(oss.str(), posX, posY, 0xffffffff);
}

void FrontDisplay::showMessage(int number)
{
	//エラーチェック。１～最大メッセージ数
	if (number < 1 || number > NUM_OF_MESSAGE)
		HALT("File:FrontDisplay.cpp [showMessage()] Argument is invalid. Error");
	//フォンとのサイズをセット
	mStr.setFont(mFont_2);
	//メッセージは配列に格納しているので、引数を-1して配列のインデックスに合わせる
	number -= 1;
	//メッセージの幅を調整
	mMessageLineInterval = MESSAGE_LINE_INTERVAL;
	//メッセージのバックに枠を表示
	renderMessageFrame(MESSAGE_BACK_LEFT, MESSAGE_BACK_RIGHT, MESSAGE_BACK_UP, MESSAGE_BACK_DOWN, 0xcc802010, 0xcc802010);
	//メッセージを表示
	string targetMessage = mMessages[number];
	showMessage(targetMessage, MESSAGE_PLACE_X_INT, MESSAGE_PLACE_Y_INT);
}

void FrontDisplay::showMessage(std::string targetMessage, int placeX, int placeY)
{
	int count = 0;
	int line = 0;
	while (true) {
		//メッセージの一部を抽出。1行の最大文字数に到達するか改行のときに、行を新しくする
		string message = "";
		for (int i = 0; i < MESSAGE_WORDS_PER_LINE; ++i) {
			//「/」を改行指定文字として扱う
			if (targetMessage[count] == '/') {
				count++;
				break;
			}
			//1文字ずつ読み込んでいく
			message += targetMessage[count];
			count++;
			//文字が最後に到達したら終了
			if (count == (int)targetMessage.length())
				break;
		}
		//表示
		renderString(message, placeX, placeY + mMessageLineInterval * line);
		line++;
		//文字が最後に到達したら終了
		if (count == (int)targetMessage.length())
			break;
	}
}

int FrontDisplay::showCursor(int cursorNum, Cursor place)
{
	//カーソルの位置の処理
	//入力受付
	Button* button = Button::instance();
	//デモプレイなら1を返す
	if (button->demo())
		return 1;
	// W キー
	if (button->isTriggered(Button::KEY_W))
		mCursor--;
	// S キー
	else if (button->isTriggered(Button::KEY_S))
		mCursor++;
	//範囲チェック
	if (mCursor < 1)
		mCursor = cursorNum;
	if (mCursor > cursorNum)
		mCursor = 1;

	//カーソルを表示する処理
	Vector2 p[4];
	float x, y;

	switch(place){
	case Cursor::CURSOR_TEXT:
		x = CURSOR_PLACE_X;
		y = CURSOR_PLACE_Y;
		break;
	default:
		HALT("FIle:FrontDisplay.cpp [showCursor()] That's Cursor place is not exist. Error");
		break;
	}
	//白い四角を描いておく
	mPrim.setBlendMode(GameLib::Graphics::BLEND_MAX);
	p[0].set(x - CURSOR_HALF_SCALE_X, y + CURSOR_HALF_SCALE_Y - CURSOR_INTERVAL * (float)(mCursor - 1));
	p[1].set(x - CURSOR_HALF_SCALE_X, y - CURSOR_HALF_SCALE_Y - CURSOR_INTERVAL * (float)(mCursor - 1));
	p[2].set(x + CURSOR_HALF_SCALE_X, y + CURSOR_HALF_SCALE_Y - CURSOR_INTERVAL * (float)(mCursor - 1));
	p[3].set(x + CURSOR_HALF_SCALE_X, y - CURSOR_HALF_SCALE_Y - CURSOR_INTERVAL * (float)(mCursor - 1));
	drawRect(p, 0xffffffff, 0xffffffff);

	return mCursor;
}

void FrontDisplay::showParameter(const string name, const int maxHP, const int nowHP, const Weapon& weapon)
{
	//フォンとのサイズをセット
	mStr.setFont(mFont_2);
	//メッセージの1行の幅を調整
	mMessageLineInterval = 30;
	//メッセージのバックに枠を表示
	renderMessageFrame(PARAM_BACK_LEFT, PARAM_BACK_RIGHT, PARAM_BACK_UP, PARAM_BACK_DOWN, 0xcc101010, 0xcc101010);
	std::ostringstream oss;
	//名前
	oss << name << "/";
	//HP残量
	oss << nowHP << "／" << maxHP << "/";
	//武器情報
	oss << "Weapon: " << weapon.getWeaponName() << "/";
	oss << "        Level " << weapon.getWeaponLevel();
	showMessage(oss.str(), PARAM_PLACE_X_INT, PARAM_PLACE_Y_INT);
}

void FrontDisplay::showParameterOfMonsters(int num, const std::string* names, const Parameters* param, const int* nowHPs)
{
	//フォンとのサイズをセット
	mStr.setFont(mFont_2);
	//パラメータ表示していく
	//todo:insert
	int intervalLine = 0;
	//引数のnum回繰り返す
	for (int i = 0; i < num; ++i)
	{
		std::ostringstream oss;
		//名前
		renderString(names[i], PARAM_PLACE_X_INT_FOR_MONSTERS, PARAM_PLACE_Y_INT_FOR_MONSTERS + intervalLine, 0xff1010cc);
		intervalLine += PARAM_LINE_INTERVAL_FOR_MONSTERS;
		//HP
		oss << "HP:" << nowHPs[i] << " / " << param[i].health;
		renderString(oss.str(), PARAM_PLACE_X_INT_FOR_MONSTERS, PARAM_PLACE_Y_INT_FOR_MONSTERS + intervalLine, 0xffffffff);
		intervalLine += PARAM_LINE_INTERVAL_FOR_MONSTERS;
	}
}

void FrontDisplay::showStageNumber(int stageNum)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_3);
	//画面を真っ黒にする
	Vector2 p[4];
	mPrim.setBlendMode(GameLib::Graphics::BLEND_LINEAR);
	p[0].set(-1.f, -1.f);
	p[1].set(-1.f,  1.f);
	p[2].set( 1.f, -1.f);
	p[3].set( 1.f,  1.f);
	drawRect(p, 0xff000000, 0xff000000);
	//テキスト表示
	std::ostringstream oss;
	oss << "B " << stageNum << " F";
	renderString(oss.str(), WINDOW_WIDTH / 2 - 150, WINDOW_HEIGTH / 2 -100, 0xffffffff);
}

void FrontDisplay::showMiniMap(int height, int width, Minimap* map)
{
	//背景
	Vector2 p[4];
	mPrim.setBlendMode(GameLib::Graphics::BLEND_LINEAR);
	p[0].set(MINIMAP_PLACE_RIGHT,					MINIMAP_PLACE_DOWN + MINIMAP_LENGTH);
	p[1].set(MINIMAP_PLACE_RIGHT,					MINIMAP_PLACE_DOWN);
	p[2].set(MINIMAP_PLACE_RIGHT - MINIMAP_LENGTH,	MINIMAP_PLACE_DOWN + MINIMAP_LENGTH);
	p[3].set(MINIMAP_PLACE_RIGHT - MINIMAP_LENGTH,	MINIMAP_PLACE_DOWN);
	//drawRect(p, 0x10404040, 0xff404040);
	//中にマップを表示していく
	//1つのマスの長さ
	float LengthOfOneMass;
	if (height > width)
		LengthOfOneMass = MINIMAP_LENGTH / (float)height;
	else
		LengthOfOneMass = MINIMAP_LENGTH / (float)width;
	//描画
	float y = MINIMAP_PLACE_DOWN;
	for (int i = 0; i < height; i++) 
	{
		float x = MINIMAP_PLACE_RIGHT;
		for (int j = 0; j < width; j++) 
		{
			p[0].set(x,					y+LengthOfOneMass);
			p[1].set(x,					y);
			p[2].set(x-LengthOfOneMass,	y+LengthOfOneMass);
			p[3].set(x-LengthOfOneMass,	y);
			int index = i * width + j;
			unsigned color;
			switch (map[index]) {
			case MAP_MASS_NORMAL:
				color = 0x80100000;
				break;
			case MAP_MASS_HOLE:
				color = 0xcc000000;
				break;
			case MAP_MASS_GOAL:
				color = 0xcc108010;
				break;
			case MAP_TEAM_PLAYER:
				color = 0xcc101080;
				break;
			case MAP_TEAM_ENEMY:
				color = 0xcc801010;
				break;
			default:
				HALT("File: FrontDisplay.cpp [showMiniMap()] Switch Error");
				break;
			}
			drawRect(p, color, color);
			//列を更新
			x -= LengthOfOneMass;
		}
		//行を更新
		y += LengthOfOneMass;
	}
}

//必要パーツ表示
void FrontDisplay::showNeedParts(Parts need)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_2);
	//メッセージの1行の幅を調整
	mMessageLineInterval = 25;

	std::ostringstream oss;
	oss << "   Need Parts/------------------/";

	for (int i = 0; i < Parts::numParts; i++) {
		int num = need.getNumOfParts(i);
		if (num == 0)
			continue;
		oss << need.getNameOfParts(i) << " : " << num << "/";
	}

	oss << "------------------/";

	showMessage(oss.str(), MESSAGE_PLACE_X_INT + 300, MESSAGE_PLACE_Y_INT);
}

//獲得パーツ表示
void FrontDisplay::showGotParts(Parts got)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_2);
	//メッセージの1行の幅を調整
	mMessageLineInterval = 30;
	//メッセージのバックに枠を表示
	renderMessageFrame(MESSAGE_BACK_LEFT, MESSAGE_BACK_RIGHT, MESSAGE_BACK_UP, MESSAGE_BACK_DOWN, 0xcc802010, 0xcc802010);

	std::ostringstream oss;
	oss << " You Got These Parts!/";

	for (int i = 0; i < Parts::numParts; i++) {
		int num = got.getNumOfParts(i);
		if (num == 0)
			continue;
		oss << got.getNameOfParts(i) << " : " << num << "/";
	}

	showMessage(oss.str(), MESSAGE_PLACE_X_INT, MESSAGE_PLACE_Y_INT);
}

//残りターン数表示
void FrontDisplay::showTurnLimit(int turn)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_3);
	int fontSize = FONT_3_SIZE;

	//位置決め
	float left, right, up, down;
	left = -0.15f;
	right = 0.15f;
	up = 0.9f;
	down = 0.6f;

	renderMessageFrame(-0.15f, 0.15f, 0.9f, 0.6f, 0xcc202080, 0xcc1010cc);

	//ターン数表示
	int keta = 0;
	int tempTurn = turn;
	while (true) {
		keta++;
		tempTurn /= 10;
		if (tempTurn <= 0)
			break;
	}
	renderString(turn, WINDOW_WIDTH / 2 - fontSize  / 2, WINDOW_HEIGTH / 12 - 20);
	int djfkls = WINDOW_WIDTH / 2 - fontSize  / 2;
}

void FrontDisplay::showParts(Parts* parts)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_2);
	//メッセージの1行の幅を調整
	mMessageLineInterval = 30;

	std::ostringstream oss;
	oss << "YOU HAVING PARTS:/";
	int num = Parts::numParts;
	for (int i = 0; i < num; i++)
		oss << parts->getNameOfParts(i) << ": " << parts->getNumOfParts(i) << "/";
	int x = WINDOW_WIDTH - 300;
	int y = WINDOW_HEIGTH - 35 * (num+1) - 10;
	//フレーム
	renderMessageFrame(0.5f, 0.95f, -0.6f, -0.95f, 0xcc202020, 0xcc303030);
	showMessage(oss.str(), x, y);
}


//デバッグ用
void FrontDisplay::showMasses(int** params, int height, int width)
{
	//フォントのサイズをセット
	mStr.setFont(mFont_2);

	int y = 300;
	for (int i = 0; i < height; ++i) {
		int x = 1200;
		for (int j = 0; j < width; ++j) {
			renderString(params[i][j], x, y);
			x -= 70;
		}
		y -= 30;
	}
}


//このクラス内のみの関数
void FrontDisplay::renderMessageFrame(
	float left,
	float right,
	float up,
	float down,
	unsigned int color1,
	unsigned int color2
)
{
	//メッセージのバックに枠を表示
	//白のフレーム
	Vector2 p[4];
	mPrim.setBlendMode(GameLib::Graphics::BLEND_LINEAR);
	p[0].set(left - MESSAGE_BACK_FRAME_THICK, down - MESSAGE_BACK_FRAME_THICK);
	p[1].set(left - MESSAGE_BACK_FRAME_THICK, up + MESSAGE_BACK_FRAME_THICK);
	p[2].set(right + MESSAGE_BACK_FRAME_THICK, down - MESSAGE_BACK_FRAME_THICK);
	p[3].set(right + MESSAGE_BACK_FRAME_THICK, up + MESSAGE_BACK_FRAME_THICK);
	drawRect(p, 0xccffffff, 0xccffffff);
	//その中に枠を
	mPrim.setBlendMode(GameLib::Graphics::BLEND_LINEAR);
	p[0].set(left, down);
	p[1].set(left, up);
	p[2].set(right, down);
	p[3].set(right, up);
	drawRect(p, color1, color2);
}