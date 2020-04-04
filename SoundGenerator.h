#pragma once

#include "GameLib/Sound/Wave.h"
#include "GameLib/Sound/Player.h"

//BGM、SEを鳴らすもの全般を担当。シングルトン
class SoundGenerator {
public:
	static SoundGenerator* instance();
	static void create();//音を鳴らす前に1回必ず呼ぶ
	static void destroy();
	bool hasLoaded();//ロードが完了しているか確認

	enum Bgm {
		GAME,			//ゲーム中
		GAME_CLEAR,		//ゲームクリア
		GAME_OVER,		//ゲームオーバ

		BGM_INVALID
	};
	enum Se {
		//シーケンスごとの音楽
		TITLE,			//タイトル画面
		//ゲーム中の効果音
		//基本的な効果音。例えば選択音など
		SELECT_CURSOR_MOVE,		//選択中にカーソルを動かした
		SELECT_FINISH,			//選択で決定した
		//イベントごとの特殊SE
		MAKE_FRIEND_OCCUR,		//仲間イベント発声
		MAKE_FRIEND_BAD,		//仲間にするかどうか→しない
		MAKE_FRIEND_GOOD,		//仲間にするかどうか→する
		//キャラクターの動作に関するSE
		CHARACTER_MOVE,			//移動
		CHARACTER_WARP,			//ワープ
		CHARACTER_ATTACK_NORMAL,//通常攻撃
		CHARACTER_ATTACK_MAGIC,	//遠距離攻撃
		CHARACTER_FALL,			//落とし穴
		CHARACTER_CATCH,		//捕獲罠
		//ステージのマスの効果に関するSE
		STAGE_MASS_STOPPER,		//足止め罠発動
		STAGE_MASS_BOMB,		//地雷爆発
		STAGE_MASS_RECOVERY,	//回復
		STAGE_MASS_PARTS,		//パーツゲット

		SE_INVALID
	};
	//BGMを鳴らす。ループ再生
	void playBgm(Bgm);
	void stopBgm();
	//SEを鳴らす。
	void playSe(Se);
	//音楽のスイッチの切り替え
	void switchBGMandSE();
	void switchBGM();	//上のstopBgmとの違いとして、上は現在なっているBGMを止めるのに対し、こっちはずっとBGMがならない
	void switchSe();	//こちらも同様。ずっとSEがならなくなる
private:
	SoundGenerator();
	~SoundGenerator();

	//Waveははじめにロード
	GameLib::Sound::Wave mBgmWaves[BGM_INVALID];
	GameLib::Sound::Wave mSeWaves[SE_INVALID];
	//Playerは再生時に生成
	GameLib::Sound::Player mBgmPlayer;
	static const int SE_PLAYER_MAX = 4;
	GameLib::Sound::Player mSePlayers[SE_PLAYER_MAX];
	int mSePlayerPos;//次にSEを入れる場所
	bool mSEswitchOn;	//SEのON,OFF
	bool mBGMswitchOn;	//BGMのON,OFF
	Bgm mSetBGM;		//現在セットされているBGM

	static SoundGenerator* mInstance;
};