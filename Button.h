#pragma once

//キーボードからの入力を受け付けるクラス
//ゲーム中一つしか存在しないのでシングルトン
class Button {
public:
	enum Key {
		KEY_W,		//[w]
		KEY_A,		//[a]
		KEY_S,		//[s]
		KEY_D,		//[d]
		KEY_O,		//[o]
		KEY_P,		//[p]
		KEY_R,		//[r]
		KEY_LEFT,	//左方向キー
		KEY_RIGHT,	//右方向キー
		KEY_SPACE,	//SPACEキー
		KEY_SHIFT	//SHIFTキー
	};
	static Button* instance();//使うときに中継として介す
	static void create();//使う前にどこかで1回呼び出す
	static void destroy();
	bool isOn(Key) const;//押している間trueを返し続ける
	bool isTriggered(Key) const;//1回の入力だけtrueを返す。押し続けてもtrueを返すのは押した1フレームのみ
	void thisGameIsDemo();	//デモプレイであることを伝える
	void resetDemo();		//デモプレイを解除
	bool demo() const;		//デモプレイか？
private:
	Button();
	~Button();
	static Button* mInstance;
	mutable bool demoPlay;
};