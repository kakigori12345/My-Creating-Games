#include "GameLib/Framework.h"

#include "Stage.h"
#include "ContentReader.h"
#include "MyRandom.h"
#include "FrontDisplayh.h"
#include <string>

using namespace GameLib;
using namespace GameLib::Math;
using namespace GameLib::Scene;
using namespace std;

const int Stage::MAX_TARGET_QUE = 100;

Stage::Stage(int StageNumber) :
	mMasses(nullptr),
	NumOfRegisterIDs(0),
	playerID(-1),
	backgroundAngle(0.f),
	ROOM_NUMBER_OF_GOAL(-1),
	GoalMapX(-1),
	GoalMapY(-1),
	targetQueIndex(0),
	targetQue(nullptr),
	mapData(nullptr)
{
	//ファイルから定数を読み込む（ContentReaderクラスを使う）
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//読み込みスタート
	//どのステージでも共通な定数の読み込み
	Stage::MASS_SCALE		= cr->getData("Stage", "MASS_SCALE");
	Stage::MASS_LENGTH		= cr->getData("Stage", "MASS_LENGTH");
	Stage::MASS_HALF_LENGTH = MASS_LENGTH * 0.5f;
	Stage::MAX_NUM_OF_ID	= (int)cr->getData("Stage", "MAX_NUM_OF_ID");
	//ステージの番号ごとに異なる、定数の読み込み
	string stName						= cr->getData_str("StageNumber", to_string(StageNumber).c_str());
	const char* chStName				= stName.c_str();
	Stage::MASS_HEIGHT					= (int)cr->getData(chStName, "MASS_HEIGHT");
	Stage::MASS_WIDTH					= (int)cr->getData(chStName, "MASS_WIDTH");
	Stage::MASS_NUM						= MASS_HEIGHT * MASS_WIDTH;
	Stage::MASS_EFFECT_NUM_OF_HOLE		= (int)cr->getData(chStName, "MASS_EFFECT_NUM_OF_HOLE");
	Stage::MASS_EFFECT_NUM_OF_WARP		= (int)cr->getData(chStName, "MASS_EFFECT_NUM_OF_WARP");
	Stage::MASS_EFFECT_NUM_OF_BROKEN	= (int)cr->getData(chStName, "MASS_EFFECT_NUM_OF_BROKEN");
	Stage::MASS_EFFECT_NUM_OF_RECOVERY	= (int)cr->getData(chStName, "MASS_EFFECT_NUM_OF_RECOVERY");
	Stage::MASS_EFFECT_NUM_OF_PARTS		= (int)cr->getData(chStName, "MASS_EFFECT_NUM_OF_PARTS");
	Stage::ACTIVE_AREA_FROM_PC			= (int)cr->getData(chStName, "ACTIVE_AREA_FROM_PC");
	Stage::TURN_LIMIT					= (int)cr->getData(chStName, "TURN_LIMIT");
	Stage::ROOM_NUM_WIDTH				= (int)cr->getData(chStName, "ROOM_NUM_WIDTH");
	Stage::ROOM_NUM_HEIGHT				= (int)cr->getData(chStName, "ROOM_NUM_HEIGHT");
	Stage::ROOM_NUM						= ROOM_NUM_WIDTH * ROOM_NUM_HEIGHT;
	Stage::MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM	= (int)cr->getData(chStName, "MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM");
	Stage::MASSNUM_OF_DIVIDING_TO_ROOM_MAX		= (int)cr->getData(chStName, "MASSNUM_OF_DIVIDING_TO_ROOM_MAX");
	Stage::ROOM_MIN_RANGE				= (int)cr->getData(chStName, "ROOM_MIN_RANGE");
	Stage::ROOM_IS_BOSS					= ((int)cr->getData(chStName, "ROOM_IS_BOSS") == 1) ? true : false;
	SAFE_DELETE(cr);//使い終わったら削除

	//モデル情報が格納されているコンテナを読み込む
	mStageContainer = Container::create("data/model/stage.txt");
	while (!mStageContainer.isReady()) { ; }
	mBackContainer = Container::create("data/model/background.txt");
	while (!mBackContainer.isReady()) { ; }


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////  ここでダンジョンのアルゴリズムを適用して、不思議のダンジョンっぽくする  ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//まずはステージを生成しておく。このときまだただの長方形
	mMasses = new Mass[MASS_NUM];
	for (int i = 0; i < MASS_NUM; i++) {
		//モデルの作成
		mMasses[i].createModel(mStageContainer);
		//マスの横・縦数から位置を決定する
		int temp_map_x = i % MASS_WIDTH;
		int temp_map_y = i / MASS_WIDTH;
		float temp_x = (float)temp_map_x * MASS_LENGTH;
		float temp_y = (float)temp_map_y * MASS_LENGTH;
		//マスに情報を埋め込む
		mMasses[i].setPos(Vector3(temp_x, 0.f, temp_y));
		mMasses[i].setScale(MASS_SCALE);
		mMasses[i].setMap(temp_map_x, temp_map_y);
	}


	//ここからダンジョン生成のアルゴリズム
	//区画の数だけ構造体を確保　※メモリを開放し忘れないこと！
	mapData = new mapDataOfDivision[ROOM_NUM];
	//座標設定
	for (int i = 0; i < ROOM_NUM; i++) {
		int x = i % ROOM_NUM_WIDTH;
		int y = i / ROOM_NUM_WIDTH;
		mapData[i].setMap(x, y);
		mapData[i].ROOM_NUMBER = i;
	}

	//横のラインから区切っていく
	for (int i = 0; i < ROOM_NUM_WIDTH; i++) {
		int tempWidth = i * (MASS_WIDTH - 1) / ROOM_NUM_WIDTH;
		//全ての横ラインに適用していく
		for (int j = 0; j < ROOM_NUM_HEIGHT; j++) {
			//マップを取得
			int mapOfData = getRoomIndex(i, j);
			//X座標の左側を決定
			mapData[mapOfData].WIDTH_LEFT = tempWidth;
			//X座標の右側を決定
			if (i != 0) {
				int tempMapOfData = getRoomIndex(i - 1, j);
				mapData[tempMapOfData].WIDTH_RIGHT = tempWidth - 1;
			}
			//一番右側の区画
			if (i == ROOM_NUM_WIDTH - 1) {
				//エリア全体の右側の端っこを与える
				mapData[mapOfData].WIDTH_RIGHT = MASS_WIDTH - 1;
			}

		}
	}

	//次は縦。処理としてはほとんど上の場合と同様
	for (int j = 0; j < ROOM_NUM_HEIGHT; j++) {
		int tempHeight = j * (MASS_HEIGHT - 1) / ROOM_NUM_HEIGHT;
		//全ての縦ラインに適用していく
		for (int i = 0; i < ROOM_NUM_WIDTH; i++) {
			//マップを取得
			int mapOfData = getRoomIndex(i, j);
			//Y座標の上側を決定
			mapData[mapOfData].HEIGHT_TOP = tempHeight;
			//Y座標の下側を決定
			if (j != 0) {
				int tempMapOfData = getRoomIndex(i, j-1);
				mapData[tempMapOfData].HEIGHT_LOW = tempHeight - 1;
			}
			//一番下側の区画
			if (j == ROOM_NUM_HEIGHT - 1) {
				//エリア全体の下側の端っこを与える
				mapData[mapOfData].HEIGHT_LOW = MASS_HEIGHT - 1;
			}

		}
	}


	//区画から部屋に。各区画の外側から数マスを削ることで、部屋とする
	//削る＝穴あきますにする　ということ
	for (int i = 0; i < ROOM_NUM; i++)
	{
		//区画の左右、上下の幅を測る
		int rangeWidth = mapData[i].WIDTH_RIGHT - mapData[i].WIDTH_LEFT + 1;
		int rangeHeight = mapData[i].HEIGHT_LOW - mapData[i].HEIGHT_TOP + 1;
		int deleteNum(0);
		int deletedNumLeft(0), deletedNumRight(0);
		int deletedNumUp(0), deletedNumLow(0);

		//左右両側を削る
		//左側
		int nowRangeWidth = rangeWidth;
		//削る前に削る余裕があるかチェック
		if ((nowRangeWidth - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeWidth - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeWidth - ROOM_MIN_RANGE);
			else
				//削るだけの余裕がない
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumLeft = deleteNum;

		//右側
		nowRangeWidth = rangeWidth - deletedNumLeft;
		//削る前に削る余裕があるかチェック
		if ((nowRangeWidth - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeWidth - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeWidth - ROOM_MIN_RANGE);
			else
				//削るだけの余裕がない
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumRight = deleteNum;

		//削除実行
		for (int j = 0; j < rangeHeight; j++)
		{
			//左側
			for (int k = 0; k < deletedNumLeft; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + k, mapData[i].HEIGHT_TOP + j);
				mMasses[map].setPanelToHole();
			}
			//右側
			for (int k = 0; k < deletedNumRight; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_RIGHT - k, mapData[i].HEIGHT_TOP + j);
				mMasses[map].setPanelToHole();
			}
		}
		//削り後の各部屋の情報を更新
		mapData[i].WIDTH_LEFT += deletedNumLeft;
		mapData[i].WIDTH_RIGHT -= deletedNumRight;


		//上下両側を削る
		//上側
		int nowRangeHeight = rangeHeight;
		//削る前に削る余裕があるかチェック
		if ((nowRangeHeight - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeHeight - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeHeight - ROOM_MIN_RANGE);
			else
				//削るだけの余裕がない
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumUp = deleteNum;

		//下側
		nowRangeHeight = rangeHeight - deletedNumUp;
		//削る前に削る余裕があるかチェック
		if ((nowRangeHeight - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeHeight - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeHeight - ROOM_MIN_RANGE);
			else
				//削るだけの余裕がない
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumLow = deleteNum;

		//削除実行
		for (int j = 0; j < rangeWidth; j++)
		{
			//上側
			for (int k = 0; k < deletedNumUp; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + j, mapData[i].HEIGHT_TOP + k);
				mMasses[map].setPanelToHole();
			}
			//下側
			for (int k = 0; k < deletedNumLow; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + j, mapData[i].HEIGHT_LOW - k);
				mMasses[map].setPanelToHole();
			}
		}
		//削り後の各部屋の情報を更新
		mapData[i].HEIGHT_TOP += deletedNumUp;
		mapData[i].HEIGHT_LOW -= deletedNumLow;
	}

	//削った後の整合性チェック
	for (int i = 0; i < ROOM_NUM; i++)
	{
		//区画の左右、上下の幅を測る
		int rangeWidth = mapData[i].WIDTH_RIGHT - mapData[i].WIDTH_LEFT + 1;
		int rangeHeight = mapData[i].HEIGHT_LOW - mapData[i].HEIGHT_TOP + 1;
		//警告文
		if (rangeWidth < ROOM_MIN_RANGE || rangeHeight < ROOM_MIN_RANGE)
			HALT("File: Stage.cpp Constructor Deleted Room Is Error");
	}

	//通路をつなぐ
	for (int y = 0; y < ROOM_NUM_HEIGHT; y++)
	{
		for (int x = 0; x < ROOM_NUM_WIDTH; x++)
		{
			int map = getRoomIndex(x, y);

			//まずは左右のつながりを形成する。一番右側は何もしない
			if (x != ROOM_NUM_WIDTH - 1)
			{
				int mapNext = getRoomIndex(x + 1, y);
				int topMap(0), lowMap(0), leftMap(0), rightMap(0);
				//左右は確定
				leftMap = mapData[map].WIDTH_RIGHT;
				rightMap = mapData[mapNext].WIDTH_LEFT;
				//上下は2つの区画の上下に収まるように範囲設定
				topMap = (mapData[map].HEIGHT_TOP >= mapData[mapNext].HEIGHT_TOP) ? mapData[map].HEIGHT_TOP : mapData[mapNext].HEIGHT_TOP;
				lowMap = (mapData[map].HEIGHT_LOW <= mapData[mapNext].HEIGHT_LOW) ? mapData[map].HEIGHT_LOW : mapData[mapNext].HEIGHT_LOW;
				//上で求めた上下の範囲の中で、適当なY座標を選択
				if (topMap >= lowMap)
					HALT("File: Stage.cpp Constructor [Dungion's algorithm ]Error: topMap >= lowMap");
				int mapY = MyRandom::instance()->getInt(topMap, lowMap);
				//通路を敷いていく
				for (int mapX = leftMap; mapX <= rightMap; mapX++)
				{
					changePanel(mapX, mapY, Mass::Panel::CORRIDOR);
				}
				//通路情報をvector配列に追加
				corridors.push_back(new corridorData(map, mapNext, leftMap, mapY, rightMap, mapY));
			}

			//上の処理と同様に、つぎに上下のつながりを形成する。一番下側は何もしない
			if (y != ROOM_NUM_HEIGHT - 1)
			{
				int mapNext = getRoomIndex(x, y+1);
				int topMap, lowMap, leftMap, rightMap;
				//上下は確定
				topMap = mapData[map].HEIGHT_LOW;
				lowMap = mapData[mapNext].HEIGHT_TOP;
				//左右は2つの区画に収まるように範囲設定
				leftMap = (mapData[map].WIDTH_LEFT >= mapData[mapNext].WIDTH_LEFT) ? mapData[map].WIDTH_LEFT : mapData[mapNext].WIDTH_LEFT;
				rightMap = (mapData[map].WIDTH_RIGHT <= mapData[mapNext].WIDTH_RIGHT) ? mapData[map].WIDTH_RIGHT : mapData[mapNext].WIDTH_RIGHT;
				//上で求めた左右の範囲の中で、適当なX座標を選択
				if(leftMap >= rightMap)
					HALT("File: Stage.cpp Constructor [Dungion's algorithm ]Error: leftMap>= rightMap");
				int mapX = MyRandom::instance()->getInt(leftMap, rightMap);
				//通路を敷いていく
				for (int mapY = topMap; mapY <= lowMap; mapY++)
				{
					changePanel(mapX, mapY, Mass::Panel::CORRIDOR);
				}
				//通路情報をvector配列に追加
				corridors.push_back(new corridorData(map, mapNext, mapX, topMap, mapX, lowMap));
			}
		}

	}

	///////////////////////////////////////////////////////////////////////////////////
	////////////////////  ダンジョンの生成終了  ///////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////

	//どこかのマスをランダムでゴール（階段）にする。ボス部屋には作らない
	if (!ROOM_IS_BOSS) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::GOAL);
				//ゴール部屋特定
				mMasses[temp_rand].getMap(&GoalMapX, &GoalMapY);
				ROOM_NUMBER_OF_GOAL = getRoomNumberFromXY(GoalMapX, GoalMapY);
				break;
			}
		}
	}

	//マスに効果を付与。ランダムに
	//HOLE
	for (int i = 0; i < MASS_EFFECT_NUM_OF_HOLE; ++i) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanelToHole();
				break;
			}
		}
	}
	//WARP
	warpMassList = new int[MASS_EFFECT_NUM_OF_WARP];
	for (int i = 0; i < MASS_EFFECT_NUM_OF_WARP; ++i) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::WARP);
				//リストにマス目を格納
				warpMassList[i] = temp_rand;
				break;
			}
		}
	}
	//TRAP_HOLE
	for (int i = 0; i < MASS_EFFECT_NUM_OF_BROKEN; ++i) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::TRAP_HOLE);
				break;
			}
		}
	}
	//RECOVERY
	for (int i = 0; i < MASS_EFFECT_NUM_OF_RECOVERY; ++i) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::RECOVERY);
				break;
			}
		}
	}
	//PARTS
	for (int i = 0; i < MASS_EFFECT_NUM_OF_PARTS; ++i) {
		while (true) {
			//どのマスを張り替えるかランダムに決める
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//通常マスなら張り替える
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::PARTS);
				break;
			}
		}
	}

	//それぞれの部屋とゴール部屋との距離を測る
	for (int i = 0; i < ROOM_NUM; i++)
	{
		int roomX(0), roomY(0);
		mapData[i].getXY(&roomX, &roomY);
		int goalRoom = getRoomNumberFromXY(GoalMapX, GoalMapY);
		int goalRoomX = mapData[goalRoom].MapX;
		int goalRoomY = mapData[goalRoom].MapY;
		int distance = getDistance(goalRoomX, goalRoomY, roomX, roomY);
		mapData[i].setDistanceToGoal(distance);
	}


	//その他初期設定
	mIDandMap = new CombIDandMap[MAX_NUM_OF_ID];
	mTurnLimit = TURN_LIMIT;
	//背景の設定
	mModelBackUp = mBackContainer.createModel("back");
	mModelBackDown = mBackContainer.createModel("back");
	mModelBackDown.setAngle(Vector3(180.f, 0.f, 0.f));
	Vector3 backgroundPos;
	backgroundPos.x = (float)(MASS_WIDTH) / 2.f * MASS_LENGTH;
	backgroundPos.y = -1.f;
	backgroundPos.z = (float)(MASS_HEIGHT) / 2.f * MASS_LENGTH;
	mModelBackUp.setPosition(backgroundPos);
	backgroundPos.y = 1.f;
	mModelBackDown.setPosition(backgroundPos);
	Vector3 backgroundScale;
	backgroundScale.x = (float)(MASS_WIDTH)* MASS_LENGTH * 3.f;
	backgroundScale.z = (float)(MASS_HEIGHT)* MASS_LENGTH * 3.f;
	backgroundScale.y = (backgroundScale.x > backgroundScale.z) ? backgroundScale.x : backgroundScale.z;
	mModelBackUp.setScale(backgroundScale);
	mModelBackDown.setScale(backgroundScale);

}

Stage::~Stage()
{
	SAFE_DELETE_ARRAY(mMasses);
	SAFE_DELETE_ARRAY(mIDandMap);
	SAFE_DELETE_ARRAY(mapData);
	for (size_t i = 0; i < corridors.size(); i++)
	{
		SAFE_DELETE(corridors[i]);
	}
}

void Stage::update()
{
	backgroundAngle += 0.1f;
	updateMass();
	//背景をちょっとずつ回転させる
	mModelBackUp.setAngle(Vector3(0.f, backgroundAngle, 0.f));
	mModelBackDown.setAngle(Vector3(180.f, backgroundAngle, 0.f));
}

void Stage::draw() const
{
	//背景描画
	mModelBackUp.draw();
	mModelBackDown.draw();
	//マス
	for (int i = 0; i < MASS_NUM; ++i)
		mMasses[i].draw();
}

void Stage::registerNewID(int id, Vector3* result, Team team)
{
	CombIDandMap registering;
	registering.exist = true;
	registering.ID = id;
	registering.team = team;
	//他のIDの座標と被らないようにする
	if (id == playerID || team == Team::TEAM_ENEMY) {
		while (true) {
			//ここでマップの座標をランダムに決定している。todo:マップ決定を凝るなら改良必要
			registering.Xmap = MyRandom::instance()->getInt(0, MASS_WIDTH - 1);
			registering.Ymap = MyRandom::instance()->getInt(0, MASS_HEIGHT - 1);
			//対象マスが穴あきならやり直し
			int map = getMapFromXY(registering.Xmap, registering.Ymap);
			if (mMasses[map].getPanel() == Mass::Panel::HOLE || mMasses[map].getPanel() == Mass::Panel::CORRIDOR)
				continue;
			//他と一致していないか判定。一致していないときbreakでループを抜ける
			if (!idenfifyID(registering))
				break;
		}
	}
	//仲間モンスターだけ特別な処理
	else {
		int distance = 2;
		int pX(0), pY(0);
		bool finished = false;
		getXYfromID(playerID, &pX, &pY);
		while (true) {
			for (int x = 0; x <= distance; ++x)	//searchDistanceも含めている点に注意
			{
				for (int y = 0; y <= distance - x; ++y)
				{
					int Xmap, Ymap, map;
					// +x +y
					Xmap = pX + x;
					Ymap = pY + y;
					map = getMapFromXY(Xmap, Ymap);
					//条件チェック
					if (map != -1 && !identifyID(Xmap, Ymap) && getPanelFromXY(Xmap, Ymap) != Mass::Panel::HOLE) {
						finished = true;
						registering.Xmap = Xmap;
						registering.Ymap = Ymap;
						break;
					}
					// +x -y
					Xmap = pX + x;
					Ymap = pY - y;
					map = getMapFromXY(Xmap, Ymap);
					//条件チェック
					if (map != -1 && !identifyID(Xmap, Ymap) && getPanelFromXY(Xmap, Ymap) != Mass::Panel::HOLE) {
						finished = true;
						registering.Xmap = Xmap;
						registering.Ymap = Ymap;
						break;
					}
					// -x +y
					Xmap = pX - x;
					Ymap = pY + y;
					map = getMapFromXY(Xmap, Ymap);
					//条件チェック
					if (map != -1 && !identifyID(Xmap, Ymap) && getPanelFromXY(Xmap, Ymap) != Mass::Panel::HOLE) {
						finished = true;
						registering.Xmap = Xmap;
						registering.Ymap = Ymap;
						break;
					}
					// -x -y
					Xmap = pX - x;
					Ymap = pY - y;
					map = getMapFromXY(Xmap, Ymap);
					//条件チェック
					if (map != -1 && !identifyID(Xmap, Ymap) && getPanelFromXY(Xmap, Ymap) != Mass::Panel::HOLE) {
						finished = true;
						registering.Xmap = Xmap;
						registering.Ymap = Ymap;
						break;
					}
				}
				if (finished)
					break;
			}
			if (finished)
				break;
			else
				distance++;
		}
	}

	//リストに登録する
	mIDandMap[NumOfRegisterIDs] = registering;
	//登録者数を一つ分増やす
	NumOfRegisterIDs++;

	//次に、登録した座標の位置を、受け取ったポインタに埋め込んで返す
	*result = *returnPlaceFromID(id);
}

void Stage::deleteID(int id)
{
	//IDリストから該当のIDを検索
	int target = getIndexFromID(id);
	mIDandMap[target].reset();
}

void Stage::registerPlayerID(int id)
{
	playerID = id;
}

void Stage::updatePrevFootMass()
{
	//空でないなら更新
	if (prevFootMass)
		prevFootMass->updateFoot();
	//空にする
	prevFootMass = nullptr;
}

void Stage::updateMass()
{
	//とりあえず全部のマスをまずリセット
	for (int i = 0; i < MASS_NUM; ++i)
	{
		mMasses[i].setFoot(Mass::Foot::NONE);
	}
	//IDが登録してあるマスのみ更新
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		int map = getMapFromID(mIDandMap[i].ID);
		if (mIDandMap[i].team == Team::TEAM_PLAYER)
			mMasses[map].setFoot(Mass::Foot::FOOT_PLAYER);
		else if (mIDandMap[i].team == Team::TEAM_ENEMY)
			mMasses[map].setFoot(Mass::Foot::FOOT_ENEMY);
	}
}

void Stage::updateMassParameter()
{
	/*処理の説明
	例として、プレイヤーが立っているマスを起点として、
	MassクラスのMassParam::TeamPlayerパラメータの計算を考える。
	まず、プレイヤーが立っているマスは最大値となる。
	そのマスから1マス分離れるごとに-1していき、周りのマスも
	同様に計算していく。
	*/
	//すべてのマスのパラメータをリセット
	for (int i = 0; i < MASS_NUM; ++i) {
		mMasses[i].resetMassParam();
	}
	//一つ一つのIDから計算する
	for (int id = 0; id < NumOfRegisterIDs; ++id) {
		if (mIDandMap[id].team == Team::NONE)
			continue;
		Mass::MassParamName toChangeParam;
		if (mIDandMap[id].team == Team::TEAM_PLAYER)
			toChangeParam = Mass::MassParamName::MP_TEAM_PLAYER;
		else {
			ASSERT(mIDandMap[id].team == Team::TEAM_ENEMY);
			toChangeParam = Mass::MassParamName::MP_TEAM_ENEMY;
		}
		//実際の計算
		for (int x = 0; x < ACTIVE_AREA_FROM_PC; ++x)
		{
			for (int y = x - ACTIVE_AREA_FROM_PC; y <= ACTIVE_AREA_FROM_PC - x; ++y)
			{
				int targetX(0), targetY(0);
				targetX = mIDandMap[id].Xmap + x;
				targetY = mIDandMap[id].Ymap + y;
				//エリア外チェック
				if (!(targetX < 0 || targetX >= MASS_WIDTH ||
					targetY < 0 || targetY >= MASS_HEIGHT))
				{	
					int map = getMapFromXY(targetX, targetY);
					int value;
					if (y > 0)
						value = ACTIVE_AREA_FROM_PC - x - y;
					else
						value = ACTIVE_AREA_FROM_PC - x + y;
					mMasses[map].changeMassParam(toChangeParam, value);
				}
				//x=0以外は、-x方向にも同じ処理を施す
				if (x != 0)
				{
					targetX = mIDandMap[id].Xmap - x;
					targetY = mIDandMap[id].Ymap + y;
					//エリア外チェック
					if (!(targetX < 0 || targetX >= MASS_WIDTH ||
						targetY < 0 || targetY >= MASS_HEIGHT))
					{
						int map = getMapFromXY(targetX, targetY);
						int value;
						if (y > 0)
							value = ACTIVE_AREA_FROM_PC - x - y;
						else
							value = ACTIVE_AREA_FROM_PC - x + y;
						mMasses[map].changeMassParam(toChangeParam, value);
					}
				}
			}
		}
	}
}

void Stage::changeTeam(int id)
{
	//IDリストから該当のIDを検索
	int target = getIndexFromID(id);
	//チームを変更する
	if (mIDandMap[target].team == Team::TEAM_PLAYER)
		mIDandMap[target].team = Team::TEAM_ENEMY;
	else
		mIDandMap[target].team = Team::TEAM_PLAYER;
}

void Stage::changePanel(int id, Mass::Panel panel)
{
	int map = getMapFromID(id);
	mMasses[map].setPanel(mStageContainer, panel);
}

void Stage::changePanel(int x, int y, Mass::Panel panel)
{
	int map = getMapFromXY(x, y);
	mMasses[map].setPanel(mStageContainer, panel);
}

bool Stage::decCountOfTrapStopper(int id)
{
	int map = getMapFromID(id);
	if (mMasses[map].decStopCount())
		return true;

	return false;
}

void Stage::showMiniMap()
{
	FrontDisplay::Minimap* minimap = new FrontDisplay::Minimap[MASS_NUM];

	for (int i = 0; i < MASS_NUM; i++)
	{
		//上から表示の優先度が高い
		if (mMasses[i].getPanel() == Mass::Panel::GOAL)
			minimap[i] = FrontDisplay::Minimap::MAP_MASS_GOAL;
		else if (mMasses[i].getPanel() == Mass::Panel::HOLE)
			minimap[i] = FrontDisplay::Minimap::MAP_MASS_HOLE;
		else if (mMasses[i].getFoot() == Mass::Foot::FOOT_PLAYER)
			minimap[i] = FrontDisplay::Minimap::MAP_TEAM_PLAYER;
		else if (mMasses[i].getFoot() == Mass::Foot::FOOT_ENEMY)
			minimap[i] = FrontDisplay::Minimap::MAP_TEAM_ENEMY;
		else
			minimap[i] = FrontDisplay::Minimap::MAP_MASS_NORMAL;
	}

	FrontDisplay::instance()->showMiniMap(MASS_HEIGHT, MASS_WIDTH, minimap);

	SAFE_DELETE_ARRAY(minimap);
}

int Stage::incTurnLimit()
{
	mTurnLimit--;
	int result = mTurnLimit;

	if (mTurnLimit <= 0)
		mTurnLimit = TURN_LIMIT;
	

	return result;
}

int Stage::getTurnLimit()
{
	return mTurnLimit;
}


///////////////////////////////////////////////////////
//////////////情報を返す///////////////////////////////
///////////////////////////////////////////////////////

const Vector3* Stage::returnPlaceFromID(int id) const
{
	int map = getMapFromID(id);
	
	return mMasses[map].getPosition();
}

const Vector3* Stage::returnPlaceFromIDandTarget(int id, TargetMass target) const
{
	int map = getMapFromIDandTarget(id, target);

	return mMasses[map].getPosition();
}

bool Stage::judgeActive(int judgeID, int baseID) const
{
	int judgeX(0), judgeY(0);
	int baseX(0),  baseY(0);
	getXYfromID(judgeID, &judgeX, &judgeY);
	getXYfromID(baseID, &baseX, &baseY);
	//差をとる。正負を考慮
	int diffX = judgeX - baseX;
	int diffY = judgeY - baseY;
	if (diffX < 0)
		diffX *= -1;
	if (diffY < 0)
		diffY *= -1;
	//範囲チェック
	if (diffX <= ACTIVE_AREA_FROM_PC && diffY <= ACTIVE_AREA_FROM_PC)
		return true;

	return false;
}

bool Stage::judgeMove(int id, TargetMass mass, Vector3* rePos, int* passiveID)
{
	//IDリストから該当のIDを検索
	int target = getIndexFromID(id);
	//対象のマスを特定する
	int x = mIDandMap[target].Xmap;
	int y = mIDandMap[target].Ymap;
	int preX = x;
	int preY = y;
	//移動前のmapを用意しておく
	int prevMap = getMapFromXY(x, y);
	switch (mass) {
	case TargetMass::TARGET_RIGHT:
		x--;
		break;
	case TargetMass::TARGET_LEFT:
		x++;
		break;
	case TargetMass::TARGET_UP:
		y++;
		break;
	case TargetMass::TARGET_DOWN:
		y--;
		break;
	case TargetMass::TARGET_SURROUNDINGS:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	default:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	}
	//エリア外チェック
	if (x < 0 || x == MASS_WIDTH ||
		y < 0 || y == MASS_HEIGHT)
	{
		return false;
	}
	//該当マスが穴あきかみる。そうなら無効
	int map = getMapFromXY(x, y);
	if (mMasses[map].getPanel() == Mass::Panel::HOLE)
		return false;
	//該当マスに他のIDのものがいるか判定する
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//ここまできたら、対象マスに誰かが存在しているということ。同じチーム同士か判定
		int map_ = getMapFromXY(mIDandMap[i].Xmap, mIDandMap[i].Ymap);
		int id_ = getIDfromMap(map_);
		Stage::Team team_ = getTeamFromID(id_);
		if (id == playerID && getTeamFromID(id) == team_) {
			*passiveID = id_;
			break;
		}
		else
			return false;
	}

	//該当マスの座標を埋め込んで返す
	*rePos = *mMasses[map].getPosition();
	//このタイミングでIDとマップの対応を更新する。移動する瞬間に更新していることに注意。
	mIDandMap[target].Xmap = x;
	mIDandMap[target].Ymap = y;
	if (*passiveID != -1) {
		target = getIndexFromID(*passiveID);
		mIDandMap[target].Xmap = preX;
		mIDandMap[target].Ymap = preY;
	}
	//特定のマスに対象マスを埋め込む
	else
		prevFootMass = &mMasses[prevMap];

	return true;
}

bool Stage::judgeMove(int x, int y, TargetMass target, int id) const
{
	switch (target) {
	case TargetMass::TARGET_RIGHT:
		x--;
		break;
	case TargetMass::TARGET_LEFT:
		x++;
		break;
	case TargetMass::TARGET_UP:
		y++;
		break;
	case TargetMass::TARGET_DOWN:
		y--;
		break;
	case TargetMass::TARGET_SURROUNDINGS:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	default:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	}
	//エリア外チェック
	if (x < 0 || x == MASS_WIDTH ||
		y < 0 || y == MASS_HEIGHT)
	{
		return false;
	}
	//該当マスが穴あきかみる。そうなら無効
	int map = getMapFromXY(x, y);
	if (mMasses[map].getPanel() == Mass::Panel::HOLE)
		return false;
	//該当マスに他のIDのものがいるか判定する
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//ここまできたら、対象マスに誰かが存在しているということ。同じチーム同士か判定
		int map_ = getMapFromXY(mIDandMap[i].Xmap, mIDandMap[i].Ymap);
		int id_ = getIDfromMap(map_);
		Stage::Team team_ = getTeamFromID(id_);
		if (id == playerID && getTeamFromID(id) == team_) {
			return true;
		}
		else
			return false;
	}

	return true;
}

bool Stage::judgeWarp(int id, Vector3* rePos)
{
	//bool配列。ワープ先として使用できるマスの判定用
	bool* canWarp = new bool[MASS_EFFECT_NUM_OF_WARP];
	//まずワープマスが埋まっていないか確認
	for (int i = 0; i < MASS_EFFECT_NUM_OF_WARP; ++i)
	{
		//初期化
		canWarp[i] = true;
		int map = warpMassList[i];
		int x(0), y(0);
		mMasses[map].getMap(&x, &y);
		//判定開始。IDリストを探索していく
		for (int j = 0; j < NumOfRegisterIDs; ++j)
		{
			if (x != mIDandMap[j].Xmap)
				continue;
			if (y != mIDandMap[j].Ymap)
				continue;
			//ここまで突破してきたらそのワープ先は使えない
			canWarp[i] = false;
		}
	}
	//埋まっているならfalseを返す（ワープできない）
	bool result = false;
	for (int i = 0; i < MASS_EFFECT_NUM_OF_WARP; ++i)
		result |= canWarp[i];
	//ワープ可能な中からワープ先を選ぶ
	if (result) {
		//ワープ先が決まるまでループ
		while (true) {
			int temp_rand = MyRandom::instance()->getInt(0, MASS_EFFECT_NUM_OF_WARP);
			if (canWarp[temp_rand]) {
				int toWarp = warpMassList[temp_rand];
				*rePos = *mMasses[toWarp].getPosition();
				//マップ更新
				int toX(0), toY(0);
				mMasses[toWarp].getMap(&toX, &toY);
				//IDリストから該当のIDを検索
				int target = 0;
				for (int i = 0; i < MAX_NUM_OF_ID; i++)
				{
					if (mIDandMap[i].ID == id) {
						//indexをtargetに格納
						target = i;
						break;
					}

				}
				mIDandMap[target].Xmap = toX;
				mIDandMap[target].Ymap = toY;
				break;
			}
		}
	}
	//最後に作成したbool配列をしっかりと解放しておく
	SAFE_DELETE_ARRAY(canWarp);

	return result;
}

bool Stage::judgeMassChange(int id, TargetMass mass, Mass::Panel toChange)
{
	//IDとターゲットから、該当マスのマップとパネルの種類を抽出
	Mass::Panel panel = getPanelFromIDandTarget(id, mass);
	int map = getMapFromIDandTarget(id, mass);
	//指定マスに誰かいないか確認。いるなら変更不可
	if (characterOnTheMap(map))
		return false;
	//通路はどんなことがあっても変更不可
	if (panel == Mass::Panel::CORRIDOR)
		return false;
	//指定のマスに変更する処理。有効ならここで変更までやってしまう
	switch (toChange) {
	case Mass::Panel::NORMAL:
		if (panel == Mass::Panel::HOLE) {
			mMasses[map].setPanel(mStageContainer, Mass::Panel::NORMAL);
			return true;
		}
		else
			return false;
	case Mass::Panel::TRAP_HOLE:
		if (panel == Mass::Panel::NORMAL) {
			mMasses[map].setPanel(mStageContainer, Mass::Panel::TRAP_HOLE);
			return true;
		}
		else
			return false;
	case Mass::Panel::TRAP_CATCH:
		if (panel == Mass::Panel::NORMAL) {
			mMasses[map].setPanel(mStageContainer, Mass::Panel::TRAP_CATCH);
			return true;
		}
		else
			return false;
	case Mass::Panel::TRAP_STOPPER:
		if (panel == Mass::Panel::NORMAL) {
			mMasses[map].setPanel(mStageContainer, Mass::Panel::TRAP_STOPPER);
			return true;
		}
		else
			return false;
	case Mass::Panel::TRAP_BOMB:
		if (panel == Mass::Panel::NORMAL) {
			mMasses[map].setPanel(mStageContainer, Mass::Panel::TRAP_BOMB);
			return true;
		}
		else
			return false;
	default:
		HALT("File:Stage.cpp [judgeMassChange()] That's panel is invalid. Error");
		break;
	}

	return false;
}

void Stage::getIDs(int id, TargetMass mass, int* IDs, int* IDnum)
{
	//とりあえず今は複数のIDには対応しない
	*IDnum = 1;
	int map = getMapFromIDandTarget(id, mass);
	int searchID = getIDfromMap(map);
	//そのマスにIDがいないなら、結果に-1を埋め込む
	*IDs = searchID;
}

Mass::Panel Stage::getPanelFromID(int id) const
{
	int map = getMapFromID(id);

	return mMasses[map].getPanel();
}

Mass::Panel Stage::getPanelFromIDandTarget(int id, TargetMass target) const
{
	int map = getMapFromIDandTarget(id, target);

	if (map == -1)
		return Mass::Panel::HOLE;

	return mMasses[map].getPanel();
}

void Stage::information(int id, bool& playerIsNear, bool& enemyIsNext, bool& enemyIsNear, bool& canRunAway, 
	TargetMass& targetNext, TargetMass& targetNear, TargetMass& targetPlayer, TargetMass& targetRunAway, int searchDistance, int& nearTargetID) const
{
	//該当IDの所属チームについて
	Team targetTeam = getTeamFromID(id);
	//隣り合うマス4つ分について、敵がいるか、また敵パラメータ（敵との距離的な近さ）がいくつかを検索する
	TargetMass targetMass[4] = {};
	int Xmap[4] = {};
	int Ymap[4] = {};
	int enemyParam[4] = {};
	bool isEnemy[4] = {false, false, false, false};
	bool isMass[4] = { true, true, true, true };
	//検索前の設定
	targetMass[0] = TargetMass::TARGET_UP;
	targetMass[1] = TargetMass::TARGET_DOWN;
	targetMass[2] = TargetMass::TARGET_RIGHT;
	targetMass[3] = TargetMass::TARGET_LEFT;
	for (int i = 0; i < 4; ++i)
		getXYfromIDandTarget(id, targetMass[i], &Xmap[i], &Ymap[i]);
	//該当マスが存在するかの判定
	for (int i = 0; i < 4; ++i)
	{
		//マスが穴あきになっていないかチェック
		int map = getMapFromXY(Xmap[i], Ymap[i]);
		//エリア外チェック
		if (map == -1) {
			isMass[i] = false;
			continue;
		}
		if (mMasses[map].getPanel() == Mass::Panel::HOLE)
			isMass[i] = false;
	}
	//計算していく
	//隣り合うマスの、敵テーム存在度を調べる
	for (int i = 0; i < 4; ++i)
	{
		if (!isMass[i])
			continue;

		int map = getMapFromXY(Xmap[i], Ymap[i]);
		if (targetTeam == Team::TEAM_PLAYER)
			enemyParam[i] = mMasses[map].getMassParam()->TeamEnemy;
		else
			enemyParam[i] = mMasses[map].getMassParam()->TeamPlayer;
	}
	//隣り合うマスに敵チームがいるかどうか
	for (int i = 0; i < 4; ++i)
	{
		if (!isMass[i])
			continue;
		//該当マスに他のIDのものがいるか判定する
		for (int j = 0; j < NumOfRegisterIDs; ++j)
		{
			if (Xmap[i] != mIDandMap[j].Xmap)
				continue;
			if (Ymap[i] != mIDandMap[j].Ymap)
				continue;
			//ここまできたら、対象マスに誰かが存在しているということ
			//それが敵側のものであるかどうか判定
			if (targetTeam != mIDandMap[j].team) {
				isEnemy[i] = true;
			}
			//味方なら、そこには動かないようにする（とりあえず。今後変更の可能性あり）
			else {
				isMass[i] = false;
			}
			break;
		}
	}


	//最後に、結果を整理
	for (int i = 0; i < 4; ++i) {
		if (!isMass[i])
			continue;
		enemyIsNext |= isEnemy[i];
		enemyIsNear |= (enemyParam[i] > 0);
	}
	//敵が隣にいるなら
	if (enemyIsNext) {
		for (int i = 0; i < 4; ++i) {
			if (isEnemy[i]) {
				targetNext = targetMass[i];
				break;
			}
		}
	}
	//近くにいるか
	if (enemyIsNear) {
		//敵チーム存在度が最も高いものを抽出
		int max = 0;
		for (int i = 0; i < 4; ++i) {
			if (!isMass[i])
				continue;
			//最大値交換
			if (max <= enemyParam[i]) {
				//値が同じときは、ランダムで
				if (max == enemyParam[i]) {
					if (MyRandom::instance()->getInt(0, 100) < 50)
						continue;
				}
				max = enemyParam[i];
				//そこをターゲットにする
				targetNear = targetMass[i];
			}
		}
		//敵チーム存在度が最も低いものを抽出
		int min = 1000000;
		for (int i = 0; i < 4; ++i) {
			if (!isMass[i])
				continue;
			if (isEnemy[i])
				continue;
			//最小値交換
			if (min > enemyParam[i]) {
				min = enemyParam[i];
				//そこをターゲットにする
				targetRunAway = targetMass[i];
				//フラグをセットしておく
				canRunAway = true;
			}
		}
		//最後に、自身が今いるマスのパラメータとの比較を行う
		bool temp = false;
		int map = getMapFromID(id);
		if (targetTeam == Team::TEAM_PLAYER) {
			if (max < mMasses[map].getMassParam()->TeamEnemy)
				temp = true;
		}
		else {
			if (max < mMasses[map].getMassParam()->TeamPlayer)
				temp = true;
		}
		//自身が今いるマスの方が敵存在度が高いなら動く必要なしとする
		if (temp)
			enemyIsNear = false;
	}
	//味方モンスターがプレイヤ―を索敵する範囲
	int distanceOfSearchPlayer = 5;
	//プレイヤーとのマス的距離
	int pX, pY, mX, mY;
	getXYfromID(playerID, &pX, &pY);	//プレイヤー
	getXYfromID(id, &mX, &mY);			//モンスター
	int diffX = ((pX - mX) >= 0) ? pX - mX : mX - pX;
	int diffY = ((pY - mY) >= 0) ? pY - mY : mY - pY;
	//距離が索敵範囲内かつ隣同士でない
	if ( (diffX + diffY) <= distanceOfSearchPlayer && (diffX + diffY) != 1) {
		playerIsNear = true;
		bool tempFlag = false;
		//Xから
		if(diffX != 0){
			if ((pX - mX) > 0)
				targetPlayer = TargetMass::TARGET_LEFT;
			else
				targetPlayer = TargetMass::TARGET_RIGHT;
			if (getPanelFromIDandTarget(id, targetPlayer) != Mass::Panel::HOLE)
				tempFlag = true;
		}
		if (!tempFlag && diffY != 0) {
			if ((pY - mY) > 0)
				targetPlayer = TargetMass::TARGET_UP;
			else
				targetPlayer = TargetMass::TARGET_DOWN;
			if (getPanelFromIDandTarget(id, targetPlayer) != Mass::Panel::HOLE)
				tempFlag = true;
		}
		//最終的に、マスが穴あきでプレイヤーに近づけないなら移動判定なし
		if (!tempFlag)
			playerIsNear = false;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//ここからの処理は遠距離攻撃用。指定マス以内にいる敵を索敵してそのIDを保存する///
	/////////////////////////////////////////////////////////////////////////////////
	//敵が隣にいるならわざわざ遠くの敵を索敵しない。索敵距離を0（もしくは１）と指定した場合も索敵しない
	if (enemyIsNext || searchDistance == 0 || searchDistance == 1) {
		nearTargetID = -1;
		return;
	}
	//引数のsearchDistanceに従って索敵を行う
	int centerX, centerY;					//動作の中心人物の座標
	int minimumDistance = searchDistance+1;	//索敵内にいる敵への最小距離。初期値をこうすることで確実に索敵内にいる敵を検知できる
	getXYfromID(id, &centerX, &centerY);
	for (int x = 0; x <= searchDistance; ++x)	//searchDistanceも含めている点に注意
	{
		for (int y = 0; y <= searchDistance - x; ++y)
		{
			int Xmap, Ymap, map;

			// +x +y
			Xmap = centerX + x;
			Ymap = centerY + y;
			map = getMapFromXY(Xmap, Ymap);
			//エリア外チェックを通るか
			if (map != -1) {
				//そのマスに敵がいるかどうか
				if (enemyOnTheMap(map, targetTeam)) {
					//その敵への距離を測る。絶対距離
					int distance = ((x >= 0) ? x : -x) + ((y >= 0) ? y : -y);
					//現時点で索敵している敵のものより距離が短いなら更新
					if (distance < minimumDistance) {
						minimumDistance = distance;
						nearTargetID = getIDfromMap(map);
					}
				}
			}
			// +x -y
			Xmap = centerX + x;
			Ymap = centerY - y;
			map = getMapFromXY(Xmap, Ymap);
			if (map != -1) {
				if (enemyOnTheMap(map, targetTeam)) {
					int distance = ((x >= 0) ? x : -x) + ((y >= 0) ? y : -y);
					if (distance < minimumDistance) {
						minimumDistance = distance;
						nearTargetID = getIDfromMap(map);
					}
				}
			}
			// -x +y
			Xmap = centerX - x;
			Ymap = centerY + y;
			map = getMapFromXY(Xmap, Ymap);
			if (map != -1) {
				if (enemyOnTheMap(map, targetTeam)) {
					int distance = ((x >= 0) ? x : -x) + ((y >= 0) ? y : -y);
					if (distance < minimumDistance) {
						minimumDistance = distance;
						nearTargetID = getIDfromMap(map);
					}
				}
			}
			// -x -y
			Xmap = centerX - x;
			Ymap = centerY - y;
			map = getMapFromXY(Xmap, Ymap);
			if (map != -1) {
				if (enemyOnTheMap(map, targetTeam)) {
					int distance = ((x >= 0) ? x : -x) + ((y >= 0) ? y : -y);
					if (distance < minimumDistance) {
						minimumDistance = distance;
						nearTargetID = getIDfromMap(map);
					}
				}
			}
		}
	}
}

const Vector3* Stage::getPositionFromID(int id) const
{
	int map = getMapFromID(id);
	return mMasses[map].getPosition();
}

int Stage::getIDFromIDandMassDiff(int id, int xDiff, int yDiff) const
{
	int x, y;
	getXYfromID(id, &x, &y);
	x += xDiff;
	y += yDiff;
	int map = getMapFromXY(x, y);
	return getIDfromMap(map);
}

bool Stage::thisStageIsBossRoom() const
{
	return ROOM_IS_BOSS;
}

bool Stage::AllEnemyIsDied() const
{
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (mIDandMap[i].team == Team::TEAM_ENEMY)
			return false;
	}
	//敵が全滅している
	return true;
}


//////////////////プレイヤーのAI用/////////////////
//経路探索
void Stage::doPathPlanning(int id, Mass::Panel destination)
{
	//経路キューの初期化
	SAFE_DELETE_ARRAY(targetQue);
	targetQueIndex = 0;
	targetQue = new TargetMass[MAX_TARGET_QUE];
	for (int i = 0; i < MAX_TARGET_QUE; i++)
		targetQue[i] = TargetMass::TARGET_NONE;

	//動作主の座標
	int actorX(0), actorY(0);
	getXYfromID(id, &actorX, &actorY);
	//同じ部屋に目的が存在するか
	int destX(0), destY(0);
	//とりあえず今はゴール以外考えない
	destX = GoalMapX;
	destY = GoalMapY;
	//目的地との差異
	int actorRoomNumber = getRoomNumberFromXY(actorX, actorY);
	int destRoomNumber = getRoomNumberFromXY(destX, destY);
	//通路
	int corridorIndex(-1);

	//階段が同じ部屋にないなら
	if (actorRoomNumber != destRoomNumber) {
		//一番はじめに通路の上にいるかどうか
		bool tempFlag = false;
		if (isOnCorridor(actorX, actorY, &corridorIndex)) {
			tempFlag = true;
			int room1(0), room2(0);
			int dis1(0), dis2(0);
			corridors[corridorIndex]->getRoomNumbers(&room1, &room2);
			//距離測る
			dis1 = mapData[room1].getDistanceToGoal();
			dis2 = mapData[room2].getDistanceToGoal();
			//近いほうを選択
			if (dis1 <= dis2)
				corridors[corridorIndex]->getXYfromRoomNumber(room1, &destX, &destY);
			else
				corridors[corridorIndex]->getXYfromRoomNumber(room2, &destX, &destY);
		}
		//同じ部屋にゴールがないなら、中間地点を目的地と置く
		if(!tempFlag || actorX == destX && actorY == destY){
			//通路端を中間値にする
			//目指すべき部屋を確定させる
			destRoomNumber = nextRoomEvaluation(actorRoomNumber);
			//自分が今いる部屋とその部屋を結ぶ通路を取得
			corridorIndex = getCorridorIndex(actorRoomNumber, destRoomNumber);
			//通路端までの座標を取得
			corridors[corridorIndex]->getXYfromRoomNumber(actorRoomNumber, &destX, &destY);
		}
	}

	//整合性が取れるまで繰り返し
	int roopCount = 0;
	while (true) {
		//ループカウント
		roopCount++;
		if (roopCount > 10) {
			roopCount = 0;
			getRandomMassOfRoom(actorRoomNumber, &destX, &destY);
		}
		bool finishFlag = false;//ループから抜けるよう
		//差を求める
		int diffX = destX - actorX;
		int diffY = destY - actorY;
		int nowX = actorX;
		int nowY = actorY;
		//目的地までの経路を格納していく
		TargetMass moveX = (diffX > 0) ? TARGET_LEFT : TARGET_RIGHT;
		TargetMass moveY = (diffY > 0) ? TARGET_UP : TARGET_DOWN;
		//正に直す
		if (diffX < 0)
			diffX = -diffX;
		if (diffY < 0)
			diffY = -diffY;
		int diffSum = diffX + diffY;

		for (int i = 0; i < diffSum; i++)
		{
			int rand = MyRandom::instance()->getInt(0, diffX + diffY);
			if (diffX > rand) {
				targetQue[i] = moveX;
				diffX-=1;
			}
			else {
				targetQue[i] = moveY;
				diffY-=1;
			}
			//移動可能か確認
			if (!judgeMove(nowX, nowY, targetQue[i], id))
				break;
			//許可が通ったので、現在地を更新
			getXYfromXYandTarget(nowX, nowY, targetQue[i], &nowX, &nowY);
			if (i == diffSum - 1)
				finishFlag = true;
		}
		for (int i = 0; i < diffSum; i++) {
			TargetMass temp = targetQue[i];
			TargetMass dsf = temp;
		}
		//終了判定
		if (finishFlag)
			break;
	}
}

//経路（方向）を取得
void Stage::getPath(TargetMass& target)
{
	target = targetQue[targetQueIndex];

	targetQueIndex += 1;
}

//部屋番号を返す部屋番号を返す
int Stage::getRoomNumberFromXY(int x, int y) const
{
	for (int i = 0; i < ROOM_NUM; i++)
	{
		if (mapData[i].checkMapExistFromXY(x, y))
			return i;
	}

	return -1;
}

//現在部屋と隣に位置する部屋の部屋番を取得
int Stage::nextRoomEvaluation(int nowRoomNumber)
{
	//隣の部屋の内、ゴールに近いほうを返す
	//複数ある場合はランダムで選択

	//現在地情報
	int nowMapX = mapData[nowRoomNumber].MapX;
	int nowMapY = mapData[nowRoomNumber].MapY;
	
	//上：０　右：１　下：２　左：３
	bool roomExists[4] = { true, true, true, true };
	int roomNumber[4];
	int goalValue[4];
	int nextMapX[4] = {
		nowMapX,
		nowMapX + 1,
		nowMapX,
		nowMapX - 1
	};
	int nextMapY[4] = {
		nowMapY - 1,
		nowMapY,
		nowMapY + 1,
		nowMapY
	};
	
	for (int i = 0; i < 4; i++) {
		roomNumber[i] = getRoomNumberFromXY(nextMapX[i], nextMapY[i]);
		goalValue[i] = mapData[roomNumber[i]].getDistanceToGoal();
	}

	for (int i = 0; i < 4; i++)
	{
		if (nextMapX[i] < 0 || nextMapX[i] > ROOM_NUM_WIDTH - 1)
			roomExists[i] = false;
		if (nextMapY[i] < 0 || nextMapY[i] > ROOM_NUM_HEIGHT - 1)
			roomExists[i] = false;
	}

	for (int i = 0; i < 4; i++)
	{
		if (!roomExists[i])
			continue;
		for (int j = 0; j < ROOM_NUM; j++)
		{
			if (mapData[j].checkRoomNumber(nextMapX[i], nextMapY[i]))
			{
				roomNumber[i] = mapData[j].ROOM_NUMBER;
				goalValue[i] = mapData[j].DISTANCE_TO_GOAL;
				break;
			}
		}
	}

	//評価値の比較
	int minDis = 100000;
	int resultIndex = -1;
	for (int i = 0; i < 4; i++)
	{
		if (!roomExists[i])
			continue;
		if (minDis >= goalValue[i]) {
			if (minDis == goalValue[i]) {
				if (MyRandom::instance()->getInt(0, 100) < 50) {
					minDis = goalValue[i];
					resultIndex = i;
				}
			}
			else {
				minDis = goalValue[i];
				resultIndex = i;
			}
		}
	}

	ASSERT(resultIndex != -1);

	return roomNumber[resultIndex];
}

bool Stage::isOnCorridor(int x, int y, int* result)
{
	for (int i = 0; i < (int)corridors.size(); i++)
	{
		if (corridors[i]->isOnCorridor(x, y)) {
			*result = i;
			return true;
		}
	}

	return false;
}

int Stage::getCorridorIndex(int room1, int room2) const
{
	int x = (int)corridors.size();

	for (int i = 0; i < (int)corridors.size(); i++)
	{
		if (corridors[i]->getCorridor(room1, room2))
			return i;
	}

	HALT("File:Stage.cpp getCorridorIndex Error");
	return -1;
}

void Stage::getRandomMassOfRoom(int roomNumber, int* x, int* y) const
{
	int resultX(-1), resultY(-1);

	int minX = mapData[roomNumber].WIDTH_LEFT;
	int maxX = mapData[roomNumber].WIDTH_RIGHT;
	int minY = mapData[roomNumber].HEIGHT_TOP;
	int maxY = mapData[roomNumber].HEIGHT_LOW;

	int roopCount = 0;
	while (true) {
		roopCount++;
		if (roopCount > 10)
			HALT("File Stage.cpp getRandomMassOfRandom LOOP_COUNT ERROR");
		resultX = MyRandom::instance()->getInt(minX, maxX + 1);
		resultY = MyRandom::instance()->getInt(minY, maxY + 1);
		if (getPanelFromXY(resultX, resultY) == Mass::Panel::HOLE)
			continue;
		if (*x == resultX && *y == resultY)
			continue;
		break;
	}

	*x = resultX;
	*y = resultY;
}

////////////////////デバッグ用//////////////////////////
void Stage::showMassParameter()
{
	//メモリ確保
	int** arr = new int*[MASS_HEIGHT];
	for (int i = 0; i < MASS_HEIGHT; ++i)
		arr[i] = new int[MASS_WIDTH];

	//値を入れ込んでいく。とりあえずプレイヤーチームパラメータを表示
	for (int i = 0; i < MASS_HEIGHT; ++i) {
		for (int j = 0; j < MASS_WIDTH; ++j) {
			int map = getMapFromXY(j, i);
			arr[i][j] = mMasses[map].getMassParam()->TeamPlayer;
		}
	}

	//表示
	FrontDisplay::instance()->showMasses(arr, MASS_HEIGHT, MASS_WIDTH);

	//メモリ解放
	for (int i = 0; i < MASS_HEIGHT; ++i)
		SAFE_DELETE_ARRAY(arr[i]);
}

//////////////このクラス内でしか使わない関数////////////////////

//あるIDの座標がその他の全てのIDの座標と一致しているかどうかを判定。一致しているならtrueを返す。
//もし登録されているIDが他にないならfalseを返す
bool Stage::idenfifyID(const Stage::CombIDandMap id) const
{
	return identifyID(id.Xmap, id.Ymap);
}

bool Stage::identifyID(int x, int y) const
{
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//ここまで突破してきたらそれは2つのIDが一致しているということ
		return true;
	}

	return false;
}

int Stage::getIndexFromID(int id) const {
	//IDリストから該当のIDを検索
	for (int i = 0; i < MAX_NUM_OF_ID; i++)
	{
		if (mIDandMap[i].ID == id)
			return i;
	}
	
	HALT("File: Stage.cpp [getIndexFromID] That's ID is not exist. Error"); 
	return -1;
}

//エリア外なら-1を返す
int Stage::getMapFromXY(int x, int y) const
{
	//エリア外チェック
	if (x < 0 || x >= MASS_WIDTH ||
		y < 0 || y >= MASS_HEIGHT)
		return -1;

	return x + y * MASS_WIDTH;
}

void Stage::getXYfromMap(int map, int* x, int* y) const
{
	*x = map % MASS_WIDTH;
	*y = map / MASS_WIDTH;
}

int Stage::getMapFromID(int id)const
{
	//IDリストから該当のIDを検索
	int target = -1;
	for (int i = 0; i < MAX_NUM_OF_ID; i++)
	{
		if (mIDandMap[i].ID == id) {
			//indexをtargetに格納
			target = i;
			break;
		}
	}
	if (target == -1) {
		HALT("File:Stage.cpp [getMapFromID()] Error");
		int a = 0;
	}
	//対象のマスを特定する
	int x = mIDandMap[target].Xmap;
	int y = mIDandMap[target].Ymap;

	return getMapFromXY(x, y);
}

int Stage::getMapFromIDandTarget(int id, TargetMass targetMass) const
{
	int x(0), y(0);
	getXYfromIDandTarget(id, targetMass, &x, &y);
	//エリア外チェック
	if (x < 0 || x == MASS_WIDTH ||
		y < 0 || y == MASS_HEIGHT)
	{
		return -1;
	}

	return getMapFromXY(x, y);
}

void Stage::getXYfromID(int id, int* x, int* y) const
{
	int target = getIndexFromID(id);
	//対象のマスを特定する
	*x = mIDandMap[target].Xmap;
	*y = mIDandMap[target].Ymap;
}

void Stage::getXYfromIDandTarget(int id, TargetMass target, int* rx, int* ry) const
{
	int x(0), y(0);
	getXYfromID(id, &x, &y);
	getXYfromXYandTarget(x, y, target, rx, ry);
}

void Stage::getXYfromXYandTarget(int x, int y, TargetMass target, int* reX, int* reY) const
{
	switch (target) {
	case TargetMass::TARGET_RIGHT:
		x--;
		break;
	case TargetMass::TARGET_LEFT:
		x++;
		break;
	case TargetMass::TARGET_UP:
		y++;
		break;
	case TargetMass::TARGET_DOWN:
		y--;
		break;
		//隣り合うマス以外のターゲットは今のところ実装できていない
	case TargetMass::TARGET_SURROUNDINGS:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	default:
		HALT("File:Stage.cpp [judgeMove] Error");
		break;
	}
	
	*reX = x;
	*reY = y;

}

Stage::Team Stage::getTeamFromID(int id) const
{
	int target = getIndexFromID(id);
	
	return mIDandMap[target].team;
}

//そのマスに誰もいないなら―1を返す
int Stage::getIDfromMap(int map) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);
	//該当マスのIDがいるか判定する
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//ここまできたら、対象マスに誰かが存在しているということ
		return mIDandMap[i].ID;
	}

	//HALT("File:Stage.cpp [getIDfromMap()] Error");

	return -1;
}

bool Stage::characterOnTheMap(int map) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);
	
	//該当マスに他のIDのものがいるか判定する
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//ここまできたら、対象マスに誰かが存在しているということ
		return true;
	}

	return false;
}

bool Stage::enemyOnTheMap(int map, Team myTeam) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);

	//該当マスに他のIDのものがいるか判定する
	for (int j = 0; j < NumOfRegisterIDs; ++j)
	{
		if (x != mIDandMap[j].Xmap)
			continue;
		if (y != mIDandMap[j].Ymap)
			continue;
		//ここまできたら、対象マスに誰かが存在しているということ
		//それが敵側のものであるかどうか判定
		if (myTeam != mIDandMap[j].team) {
			return true;
		}
		return false;
	}

	return false;
}

int Stage::getRoomIndex(int i, int j)
{
	return i + j * ROOM_NUM_WIDTH;
}


Mass::Panel Stage::getPanelFromXY(int x, int y) const
{
	int map = getMapFromXY(x, y);

	return mMasses[map].getPanel();
}

int Stage::getDistance(int x1, int y1, int x2, int y2) const
{
	int distanceX = (x1 > x2) ? x1 - x2 : x2 - x1;
	int distanceY = (y1 > y2) ? y1 - y2 : y2 - y1;

	return distanceX + distanceY;
}

//////////////////////////////////////////////////////////////////


bool Stage::mapDataOfDivision::checkMapExistFromXY(int x, int y) const
{
	if (x < WIDTH_LEFT || x > WIDTH_RIGHT)
		return false;
	if (y < HEIGHT_TOP || y > HEIGHT_LOW)
		return false;

	return true;
}

void Stage::mapDataOfDivision::setDistanceToGoal(int distance)
{
	DISTANCE_TO_GOAL = distance;
}

void Stage::mapDataOfDivision::setMap(int x, int y)
{
	MapX = x;
	MapY = y;
}

void Stage::mapDataOfDivision::getXY(int* x, int* y)
{
	*x = MapX;
	*y = MapY;
}

bool Stage::mapDataOfDivision::checkRoomNumber(int i, int j) const
{
	if (i != MapX || j != MapY)
		return false;

	return true;
}

int Stage::mapDataOfDivision::getDistanceToGoal() const
{
	return DISTANCE_TO_GOAL;
}


bool Stage::corridorData::getCorridor(int room1_, int room2_) const
{
	bool room1finish(false), room2finish(false);
	if (room1_ == room1 || room1_ == room2) {
		if (room1_ == room1)
			room1finish = true;
		else {
			ASSERT(room1_ == room2);
			room2finish = true;
		}
	}
	else
		return false;

	if (room1finish) {
		if (room2_ == room2)
			return true;
		else
			return false;
	}
	else {
		ASSERT(room2finish);
		if (room2_ == room1)
			return true;
		else
			return false;
	}

}

bool Stage::corridorData::isOnCorridor(int x, int y) const
{
	//縦方向の端
	if (room1_MapX == room2_MapX) {
		//x座標
		if (x != room1_MapX)
			return false;
		//y座標
		int big(0), small(0);
		if (room1_MapY > room2_MapY) {
			big = room1_MapY;
			small = room2_MapY;
		}
		else {
			big = room2_MapY;
			small = room1_MapY;
		}
		if (y > big || y < small)
			return false;
	}
	//横方向の端
	else {
		ASSERT(room1_MapY == room2_MapY);
		//y座標
		if (y != room1_MapY)
			return false;
		//x座標
		int big(0), small(0);
		if (room1_MapX > room2_MapX) {
			big = room1_MapX;
			small = room2_MapX;
		}
		else {
			big = room2_MapX;
			small = room1_MapX;
		}
		if (x > big || x < small)
			return false;
	}

	return true;
}

void Stage::corridorData::getXYfromRoomNumber(int roomNum, int* x, int* y) const
{
	if (roomNum == room1) {
		*x = room1_MapX;
		*y = room1_MapY;
	}
	else {
		ASSERT(roomNum == room2);
		*x = room2_MapX;
		*y = room2_MapY;
	}
}

void Stage::corridorData::getRoomNumbers(int* room1_, int* room2_)
{
	*room1_ = room1;
	*room2_ = room2;
}