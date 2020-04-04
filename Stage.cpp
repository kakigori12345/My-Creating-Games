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
	//�t�@�C������萔��ǂݍ��ށiContentReader�N���X���g���j
	ContentReader* cr = new ContentReader("data/contents.txt");
	cr->readStart();//�ǂݍ��݃X�^�[�g
	//�ǂ̃X�e�[�W�ł����ʂȒ萔�̓ǂݍ���
	Stage::MASS_SCALE		= cr->getData("Stage", "MASS_SCALE");
	Stage::MASS_LENGTH		= cr->getData("Stage", "MASS_LENGTH");
	Stage::MASS_HALF_LENGTH = MASS_LENGTH * 0.5f;
	Stage::MAX_NUM_OF_ID	= (int)cr->getData("Stage", "MAX_NUM_OF_ID");
	//�X�e�[�W�̔ԍ����ƂɈقȂ�A�萔�̓ǂݍ���
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
	SAFE_DELETE(cr);//�g���I�������폜

	//���f����񂪊i�[����Ă���R���e�i��ǂݍ���
	mStageContainer = Container::create("data/model/stage.txt");
	while (!mStageContainer.isReady()) { ; }
	mBackContainer = Container::create("data/model/background.txt");
	while (!mBackContainer.isReady()) { ; }


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////  �����Ń_���W�����̃A���S���Y����K�p���āA�s�v�c�̃_���W�������ۂ�����  ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�܂��̓X�e�[�W�𐶐����Ă����B���̂Ƃ��܂������̒����`
	mMasses = new Mass[MASS_NUM];
	for (int i = 0; i < MASS_NUM; i++) {
		//���f���̍쐬
		mMasses[i].createModel(mStageContainer);
		//�}�X�̉��E�c������ʒu�����肷��
		int temp_map_x = i % MASS_WIDTH;
		int temp_map_y = i / MASS_WIDTH;
		float temp_x = (float)temp_map_x * MASS_LENGTH;
		float temp_y = (float)temp_map_y * MASS_LENGTH;
		//�}�X�ɏ��𖄂ߍ���
		mMasses[i].setPos(Vector3(temp_x, 0.f, temp_y));
		mMasses[i].setScale(MASS_SCALE);
		mMasses[i].setMap(temp_map_x, temp_map_y);
	}


	//��������_���W���������̃A���S���Y��
	//���̐������\���̂��m�ہ@�����������J�����Y��Ȃ����ƁI
	mapData = new mapDataOfDivision[ROOM_NUM];
	//���W�ݒ�
	for (int i = 0; i < ROOM_NUM; i++) {
		int x = i % ROOM_NUM_WIDTH;
		int y = i / ROOM_NUM_WIDTH;
		mapData[i].setMap(x, y);
		mapData[i].ROOM_NUMBER = i;
	}

	//���̃��C�������؂��Ă���
	for (int i = 0; i < ROOM_NUM_WIDTH; i++) {
		int tempWidth = i * (MASS_WIDTH - 1) / ROOM_NUM_WIDTH;
		//�S�Ẳ����C���ɓK�p���Ă���
		for (int j = 0; j < ROOM_NUM_HEIGHT; j++) {
			//�}�b�v���擾
			int mapOfData = getRoomIndex(i, j);
			//X���W�̍���������
			mapData[mapOfData].WIDTH_LEFT = tempWidth;
			//X���W�̉E��������
			if (i != 0) {
				int tempMapOfData = getRoomIndex(i - 1, j);
				mapData[tempMapOfData].WIDTH_RIGHT = tempWidth - 1;
			}
			//��ԉE���̋��
			if (i == ROOM_NUM_WIDTH - 1) {
				//�G���A�S�̂̉E���̒[������^����
				mapData[mapOfData].WIDTH_RIGHT = MASS_WIDTH - 1;
			}

		}
	}

	//���͏c�B�����Ƃ��Ă͂قƂ�Ǐ�̏ꍇ�Ɠ��l
	for (int j = 0; j < ROOM_NUM_HEIGHT; j++) {
		int tempHeight = j * (MASS_HEIGHT - 1) / ROOM_NUM_HEIGHT;
		//�S�Ă̏c���C���ɓK�p���Ă���
		for (int i = 0; i < ROOM_NUM_WIDTH; i++) {
			//�}�b�v���擾
			int mapOfData = getRoomIndex(i, j);
			//Y���W�̏㑤������
			mapData[mapOfData].HEIGHT_TOP = tempHeight;
			//Y���W�̉���������
			if (j != 0) {
				int tempMapOfData = getRoomIndex(i, j-1);
				mapData[tempMapOfData].HEIGHT_LOW = tempHeight - 1;
			}
			//��ԉ����̋��
			if (j == ROOM_NUM_HEIGHT - 1) {
				//�G���A�S�̂̉����̒[������^����
				mapData[mapOfData].HEIGHT_LOW = MASS_HEIGHT - 1;
			}

		}
	}


	//��悩�畔���ɁB�e���̊O�����琔�}�X����邱�ƂŁA�����Ƃ���
	//��遁�������܂��ɂ���@�Ƃ�������
	for (int i = 0; i < ROOM_NUM; i++)
	{
		//���̍��E�A�㉺�̕��𑪂�
		int rangeWidth = mapData[i].WIDTH_RIGHT - mapData[i].WIDTH_LEFT + 1;
		int rangeHeight = mapData[i].HEIGHT_LOW - mapData[i].HEIGHT_TOP + 1;
		int deleteNum(0);
		int deletedNumLeft(0), deletedNumRight(0);
		int deletedNumUp(0), deletedNumLow(0);

		//���E���������
		//����
		int nowRangeWidth = rangeWidth;
		//���O�ɍ��]�T�����邩�`�F�b�N
		if ((nowRangeWidth - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeWidth - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeWidth - ROOM_MIN_RANGE);
			else
				//��邾���̗]�T���Ȃ�
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumLeft = deleteNum;

		//�E��
		nowRangeWidth = rangeWidth - deletedNumLeft;
		//���O�ɍ��]�T�����邩�`�F�b�N
		if ((nowRangeWidth - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeWidth - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeWidth - ROOM_MIN_RANGE);
			else
				//��邾���̗]�T���Ȃ�
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumRight = deleteNum;

		//�폜���s
		for (int j = 0; j < rangeHeight; j++)
		{
			//����
			for (int k = 0; k < deletedNumLeft; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + k, mapData[i].HEIGHT_TOP + j);
				mMasses[map].setPanelToHole();
			}
			//�E��
			for (int k = 0; k < deletedNumRight; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_RIGHT - k, mapData[i].HEIGHT_TOP + j);
				mMasses[map].setPanelToHole();
			}
		}
		//����̊e�����̏����X�V
		mapData[i].WIDTH_LEFT += deletedNumLeft;
		mapData[i].WIDTH_RIGHT -= deletedNumRight;


		//�㉺���������
		//�㑤
		int nowRangeHeight = rangeHeight;
		//���O�ɍ��]�T�����邩�`�F�b�N
		if ((nowRangeHeight - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeHeight - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeHeight - ROOM_MIN_RANGE);
			else
				//��邾���̗]�T���Ȃ�
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumUp = deleteNum;

		//����
		nowRangeHeight = rangeHeight - deletedNumUp;
		//���O�ɍ��]�T�����邩�`�F�b�N
		if ((nowRangeHeight - MASSNUM_OF_DIVIDING_TO_ROOM_MAX) <= ROOM_MIN_RANGE) {
			if ((nowRangeHeight - ROOM_MIN_RANGE) > 1)
				deleteNum = MyRandom::instance()->getInt(1, nowRangeHeight - ROOM_MIN_RANGE);
			else
				//��邾���̗]�T���Ȃ�
				deleteNum = 0;
		}
		else deleteNum = MyRandom::instance()->getInt(MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM, MASSNUM_OF_DIVIDING_TO_ROOM_MAX);
		deletedNumLow = deleteNum;

		//�폜���s
		for (int j = 0; j < rangeWidth; j++)
		{
			//�㑤
			for (int k = 0; k < deletedNumUp; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + j, mapData[i].HEIGHT_TOP + k);
				mMasses[map].setPanelToHole();
			}
			//����
			for (int k = 0; k < deletedNumLow; k++)
			{
				int map = getMapFromXY(mapData[i].WIDTH_LEFT + j, mapData[i].HEIGHT_LOW - k);
				mMasses[map].setPanelToHole();
			}
		}
		//����̊e�����̏����X�V
		mapData[i].HEIGHT_TOP += deletedNumUp;
		mapData[i].HEIGHT_LOW -= deletedNumLow;
	}

	//�������̐������`�F�b�N
	for (int i = 0; i < ROOM_NUM; i++)
	{
		//���̍��E�A�㉺�̕��𑪂�
		int rangeWidth = mapData[i].WIDTH_RIGHT - mapData[i].WIDTH_LEFT + 1;
		int rangeHeight = mapData[i].HEIGHT_LOW - mapData[i].HEIGHT_TOP + 1;
		//�x����
		if (rangeWidth < ROOM_MIN_RANGE || rangeHeight < ROOM_MIN_RANGE)
			HALT("File: Stage.cpp Constructor Deleted Room Is Error");
	}

	//�ʘH���Ȃ�
	for (int y = 0; y < ROOM_NUM_HEIGHT; y++)
	{
		for (int x = 0; x < ROOM_NUM_WIDTH; x++)
		{
			int map = getRoomIndex(x, y);

			//�܂��͍��E�̂Ȃ�����`������B��ԉE���͉������Ȃ�
			if (x != ROOM_NUM_WIDTH - 1)
			{
				int mapNext = getRoomIndex(x + 1, y);
				int topMap(0), lowMap(0), leftMap(0), rightMap(0);
				//���E�͊m��
				leftMap = mapData[map].WIDTH_RIGHT;
				rightMap = mapData[mapNext].WIDTH_LEFT;
				//�㉺��2�̋��̏㉺�Ɏ��܂�悤�ɔ͈͐ݒ�
				topMap = (mapData[map].HEIGHT_TOP >= mapData[mapNext].HEIGHT_TOP) ? mapData[map].HEIGHT_TOP : mapData[mapNext].HEIGHT_TOP;
				lowMap = (mapData[map].HEIGHT_LOW <= mapData[mapNext].HEIGHT_LOW) ? mapData[map].HEIGHT_LOW : mapData[mapNext].HEIGHT_LOW;
				//��ŋ��߂��㉺�͈̔͂̒��ŁA�K����Y���W��I��
				if (topMap >= lowMap)
					HALT("File: Stage.cpp Constructor [Dungion's algorithm ]Error: topMap >= lowMap");
				int mapY = MyRandom::instance()->getInt(topMap, lowMap);
				//�ʘH��~���Ă���
				for (int mapX = leftMap; mapX <= rightMap; mapX++)
				{
					changePanel(mapX, mapY, Mass::Panel::CORRIDOR);
				}
				//�ʘH����vector�z��ɒǉ�
				corridors.push_back(new corridorData(map, mapNext, leftMap, mapY, rightMap, mapY));
			}

			//��̏����Ɠ��l�ɁA���ɏ㉺�̂Ȃ�����`������B��ԉ����͉������Ȃ�
			if (y != ROOM_NUM_HEIGHT - 1)
			{
				int mapNext = getRoomIndex(x, y+1);
				int topMap, lowMap, leftMap, rightMap;
				//�㉺�͊m��
				topMap = mapData[map].HEIGHT_LOW;
				lowMap = mapData[mapNext].HEIGHT_TOP;
				//���E��2�̋��Ɏ��܂�悤�ɔ͈͐ݒ�
				leftMap = (mapData[map].WIDTH_LEFT >= mapData[mapNext].WIDTH_LEFT) ? mapData[map].WIDTH_LEFT : mapData[mapNext].WIDTH_LEFT;
				rightMap = (mapData[map].WIDTH_RIGHT <= mapData[mapNext].WIDTH_RIGHT) ? mapData[map].WIDTH_RIGHT : mapData[mapNext].WIDTH_RIGHT;
				//��ŋ��߂����E�͈̔͂̒��ŁA�K����X���W��I��
				if(leftMap >= rightMap)
					HALT("File: Stage.cpp Constructor [Dungion's algorithm ]Error: leftMap>= rightMap");
				int mapX = MyRandom::instance()->getInt(leftMap, rightMap);
				//�ʘH��~���Ă���
				for (int mapY = topMap; mapY <= lowMap; mapY++)
				{
					changePanel(mapX, mapY, Mass::Panel::CORRIDOR);
				}
				//�ʘH����vector�z��ɒǉ�
				corridors.push_back(new corridorData(map, mapNext, mapX, topMap, mapX, lowMap));
			}
		}

	}

	///////////////////////////////////////////////////////////////////////////////////
	////////////////////  �_���W�����̐����I��  ///////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////

	//�ǂ����̃}�X�������_���ŃS�[���i�K�i�j�ɂ���B�{�X�����ɂ͍��Ȃ�
	if (!ROOM_IS_BOSS) {
		while (true) {
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::GOAL);
				//�S�[����������
				mMasses[temp_rand].getMap(&GoalMapX, &GoalMapY);
				ROOM_NUMBER_OF_GOAL = getRoomNumberFromXY(GoalMapX, GoalMapY);
				break;
			}
		}
	}

	//�}�X�Ɍ��ʂ�t�^�B�����_����
	//HOLE
	for (int i = 0; i < MASS_EFFECT_NUM_OF_HOLE; ++i) {
		while (true) {
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
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
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::WARP);
				//���X�g�Ƀ}�X�ڂ��i�[
				warpMassList[i] = temp_rand;
				break;
			}
		}
	}
	//TRAP_HOLE
	for (int i = 0; i < MASS_EFFECT_NUM_OF_BROKEN; ++i) {
		while (true) {
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::TRAP_HOLE);
				break;
			}
		}
	}
	//RECOVERY
	for (int i = 0; i < MASS_EFFECT_NUM_OF_RECOVERY; ++i) {
		while (true) {
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::RECOVERY);
				break;
			}
		}
	}
	//PARTS
	for (int i = 0; i < MASS_EFFECT_NUM_OF_PARTS; ++i) {
		while (true) {
			//�ǂ̃}�X�𒣂�ւ��邩�����_���Ɍ��߂�
			int temp_rand = MyRandom::instance()->getInt(0, MASS_NUM);
			ASSERT(temp_rand < MASS_NUM);
			//�ʏ�}�X�Ȃ璣��ւ���
			if (mMasses[temp_rand].getPanel() == Mass::NORMAL) {
				mMasses[temp_rand].setPanel(mStageContainer, Mass::Panel::PARTS);
				break;
			}
		}
	}

	//���ꂼ��̕����ƃS�[�������Ƃ̋����𑪂�
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


	//���̑������ݒ�
	mIDandMap = new CombIDandMap[MAX_NUM_OF_ID];
	mTurnLimit = TURN_LIMIT;
	//�w�i�̐ݒ�
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
	//�w�i��������Ƃ���]������
	mModelBackUp.setAngle(Vector3(0.f, backgroundAngle, 0.f));
	mModelBackDown.setAngle(Vector3(180.f, backgroundAngle, 0.f));
}

void Stage::draw() const
{
	//�w�i�`��
	mModelBackUp.draw();
	mModelBackDown.draw();
	//�}�X
	for (int i = 0; i < MASS_NUM; ++i)
		mMasses[i].draw();
}

void Stage::registerNewID(int id, Vector3* result, Team team)
{
	CombIDandMap registering;
	registering.exist = true;
	registering.ID = id;
	registering.team = team;
	//����ID�̍��W�Ɣ��Ȃ��悤�ɂ���
	if (id == playerID || team == Team::TEAM_ENEMY) {
		while (true) {
			//�����Ń}�b�v�̍��W�������_���Ɍ��肵�Ă���Btodo:�}�b�v������Â�Ȃ���ǕK�v
			registering.Xmap = MyRandom::instance()->getInt(0, MASS_WIDTH - 1);
			registering.Ymap = MyRandom::instance()->getInt(0, MASS_HEIGHT - 1);
			//�Ώۃ}�X���������Ȃ��蒼��
			int map = getMapFromXY(registering.Xmap, registering.Ymap);
			if (mMasses[map].getPanel() == Mass::Panel::HOLE || mMasses[map].getPanel() == Mass::Panel::CORRIDOR)
				continue;
			//���ƈ�v���Ă��Ȃ�������B��v���Ă��Ȃ��Ƃ�break�Ń��[�v�𔲂���
			if (!idenfifyID(registering))
				break;
		}
	}
	//���ԃ����X�^�[�������ʂȏ���
	else {
		int distance = 2;
		int pX(0), pY(0);
		bool finished = false;
		getXYfromID(playerID, &pX, &pY);
		while (true) {
			for (int x = 0; x <= distance; ++x)	//searchDistance���܂߂Ă���_�ɒ���
			{
				for (int y = 0; y <= distance - x; ++y)
				{
					int Xmap, Ymap, map;
					// +x +y
					Xmap = pX + x;
					Ymap = pY + y;
					map = getMapFromXY(Xmap, Ymap);
					//�����`�F�b�N
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
					//�����`�F�b�N
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
					//�����`�F�b�N
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
					//�����`�F�b�N
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

	//���X�g�ɓo�^����
	mIDandMap[NumOfRegisterIDs] = registering;
	//�o�^�Ґ���������₷
	NumOfRegisterIDs++;

	//���ɁA�o�^�������W�̈ʒu���A�󂯎�����|�C���^�ɖ��ߍ���ŕԂ�
	*result = *returnPlaceFromID(id);
}

void Stage::deleteID(int id)
{
	//ID���X�g����Y����ID������
	int target = getIndexFromID(id);
	mIDandMap[target].reset();
}

void Stage::registerPlayerID(int id)
{
	playerID = id;
}

void Stage::updatePrevFootMass()
{
	//��łȂ��Ȃ�X�V
	if (prevFootMass)
		prevFootMass->updateFoot();
	//��ɂ���
	prevFootMass = nullptr;
}

void Stage::updateMass()
{
	//�Ƃ肠�����S���̃}�X���܂����Z�b�g
	for (int i = 0; i < MASS_NUM; ++i)
	{
		mMasses[i].setFoot(Mass::Foot::NONE);
	}
	//ID���o�^���Ă���}�X�̂ݍX�V
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
	/*�����̐���
	��Ƃ��āA�v���C���[�������Ă���}�X���N�_�Ƃ��āA
	Mass�N���X��MassParam::TeamPlayer�p�����[�^�̌v�Z���l����B
	�܂��A�v���C���[�������Ă���}�X�͍ő�l�ƂȂ�B
	���̃}�X����1�}�X������邲�Ƃ�-1���Ă����A����̃}�X��
	���l�Ɍv�Z���Ă����B
	*/
	//���ׂẴ}�X�̃p�����[�^�����Z�b�g
	for (int i = 0; i < MASS_NUM; ++i) {
		mMasses[i].resetMassParam();
	}
	//����ID����v�Z����
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
		//���ۂ̌v�Z
		for (int x = 0; x < ACTIVE_AREA_FROM_PC; ++x)
		{
			for (int y = x - ACTIVE_AREA_FROM_PC; y <= ACTIVE_AREA_FROM_PC - x; ++y)
			{
				int targetX(0), targetY(0);
				targetX = mIDandMap[id].Xmap + x;
				targetY = mIDandMap[id].Ymap + y;
				//�G���A�O�`�F�b�N
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
				//x=0�ȊO�́A-x�����ɂ������������{��
				if (x != 0)
				{
					targetX = mIDandMap[id].Xmap - x;
					targetY = mIDandMap[id].Ymap + y;
					//�G���A�O�`�F�b�N
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
	//ID���X�g����Y����ID������
	int target = getIndexFromID(id);
	//�`�[����ύX����
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
		//�ォ��\���̗D��x������
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
//////////////����Ԃ�///////////////////////////////
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
	//�����Ƃ�B�������l��
	int diffX = judgeX - baseX;
	int diffY = judgeY - baseY;
	if (diffX < 0)
		diffX *= -1;
	if (diffY < 0)
		diffY *= -1;
	//�͈̓`�F�b�N
	if (diffX <= ACTIVE_AREA_FROM_PC && diffY <= ACTIVE_AREA_FROM_PC)
		return true;

	return false;
}

bool Stage::judgeMove(int id, TargetMass mass, Vector3* rePos, int* passiveID)
{
	//ID���X�g����Y����ID������
	int target = getIndexFromID(id);
	//�Ώۂ̃}�X����肷��
	int x = mIDandMap[target].Xmap;
	int y = mIDandMap[target].Ymap;
	int preX = x;
	int preY = y;
	//�ړ��O��map��p�ӂ��Ă���
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
	//�G���A�O�`�F�b�N
	if (x < 0 || x == MASS_WIDTH ||
		y < 0 || y == MASS_HEIGHT)
	{
		return false;
	}
	//�Y���}�X�����������݂�B�����Ȃ疳��
	int map = getMapFromXY(x, y);
	if (mMasses[map].getPanel() == Mass::Panel::HOLE)
		return false;
	//�Y���}�X�ɑ���ID�̂��̂����邩���肷��
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ������ƁB�����`�[�����m������
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

	//�Y���}�X�̍��W�𖄂ߍ���ŕԂ�
	*rePos = *mMasses[map].getPosition();
	//���̃^�C�~���O��ID�ƃ}�b�v�̑Ή����X�V����B�ړ�����u�ԂɍX�V���Ă��邱�Ƃɒ��ӁB
	mIDandMap[target].Xmap = x;
	mIDandMap[target].Ymap = y;
	if (*passiveID != -1) {
		target = getIndexFromID(*passiveID);
		mIDandMap[target].Xmap = preX;
		mIDandMap[target].Ymap = preY;
	}
	//����̃}�X�ɑΏۃ}�X�𖄂ߍ���
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
	//�G���A�O�`�F�b�N
	if (x < 0 || x == MASS_WIDTH ||
		y < 0 || y == MASS_HEIGHT)
	{
		return false;
	}
	//�Y���}�X�����������݂�B�����Ȃ疳��
	int map = getMapFromXY(x, y);
	if (mMasses[map].getPanel() == Mass::Panel::HOLE)
		return false;
	//�Y���}�X�ɑ���ID�̂��̂����邩���肷��
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ������ƁB�����`�[�����m������
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
	//bool�z��B���[�v��Ƃ��Ďg�p�ł���}�X�̔���p
	bool* canWarp = new bool[MASS_EFFECT_NUM_OF_WARP];
	//�܂����[�v�}�X�����܂��Ă��Ȃ����m�F
	for (int i = 0; i < MASS_EFFECT_NUM_OF_WARP; ++i)
	{
		//������
		canWarp[i] = true;
		int map = warpMassList[i];
		int x(0), y(0);
		mMasses[map].getMap(&x, &y);
		//����J�n�BID���X�g��T�����Ă���
		for (int j = 0; j < NumOfRegisterIDs; ++j)
		{
			if (x != mIDandMap[j].Xmap)
				continue;
			if (y != mIDandMap[j].Ymap)
				continue;
			//�����܂œ˔j���Ă����炻�̃��[�v��͎g���Ȃ�
			canWarp[i] = false;
		}
	}
	//���܂��Ă���Ȃ�false��Ԃ��i���[�v�ł��Ȃ��j
	bool result = false;
	for (int i = 0; i < MASS_EFFECT_NUM_OF_WARP; ++i)
		result |= canWarp[i];
	//���[�v�\�Ȓ����烏�[�v���I��
	if (result) {
		//���[�v�悪���܂�܂Ń��[�v
		while (true) {
			int temp_rand = MyRandom::instance()->getInt(0, MASS_EFFECT_NUM_OF_WARP);
			if (canWarp[temp_rand]) {
				int toWarp = warpMassList[temp_rand];
				*rePos = *mMasses[toWarp].getPosition();
				//�}�b�v�X�V
				int toX(0), toY(0);
				mMasses[toWarp].getMap(&toX, &toY);
				//ID���X�g����Y����ID������
				int target = 0;
				for (int i = 0; i < MAX_NUM_OF_ID; i++)
				{
					if (mIDandMap[i].ID == id) {
						//index��target�Ɋi�[
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
	//�Ō�ɍ쐬����bool�z�����������Ɖ�����Ă���
	SAFE_DELETE_ARRAY(canWarp);

	return result;
}

bool Stage::judgeMassChange(int id, TargetMass mass, Mass::Panel toChange)
{
	//ID�ƃ^�[�Q�b�g����A�Y���}�X�̃}�b�v�ƃp�l���̎�ނ𒊏o
	Mass::Panel panel = getPanelFromIDandTarget(id, mass);
	int map = getMapFromIDandTarget(id, mass);
	//�w��}�X�ɒN�����Ȃ����m�F�B����Ȃ�ύX�s��
	if (characterOnTheMap(map))
		return false;
	//�ʘH�͂ǂ�Ȃ��Ƃ������Ă��ύX�s��
	if (panel == Mass::Panel::CORRIDOR)
		return false;
	//�w��̃}�X�ɕύX���鏈���B�L���Ȃ炱���ŕύX�܂ł���Ă��܂�
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
	//�Ƃ肠�������͕�����ID�ɂ͑Ή����Ȃ�
	*IDnum = 1;
	int map = getMapFromIDandTarget(id, mass);
	int searchID = getIDfromMap(map);
	//���̃}�X��ID�����Ȃ��Ȃ�A���ʂ�-1�𖄂ߍ���
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
	//�Y��ID�̏����`�[���ɂ���
	Team targetTeam = getTeamFromID(id);
	//�ׂ荇���}�X4���ɂ��āA�G�����邩�A�܂��G�p�����[�^�i�G�Ƃ̋����I�ȋ߂��j������������������
	TargetMass targetMass[4] = {};
	int Xmap[4] = {};
	int Ymap[4] = {};
	int enemyParam[4] = {};
	bool isEnemy[4] = {false, false, false, false};
	bool isMass[4] = { true, true, true, true };
	//�����O�̐ݒ�
	targetMass[0] = TargetMass::TARGET_UP;
	targetMass[1] = TargetMass::TARGET_DOWN;
	targetMass[2] = TargetMass::TARGET_RIGHT;
	targetMass[3] = TargetMass::TARGET_LEFT;
	for (int i = 0; i < 4; ++i)
		getXYfromIDandTarget(id, targetMass[i], &Xmap[i], &Ymap[i]);
	//�Y���}�X�����݂��邩�̔���
	for (int i = 0; i < 4; ++i)
	{
		//�}�X���������ɂȂ��Ă��Ȃ����`�F�b�N
		int map = getMapFromXY(Xmap[i], Ymap[i]);
		//�G���A�O�`�F�b�N
		if (map == -1) {
			isMass[i] = false;
			continue;
		}
		if (mMasses[map].getPanel() == Mass::Panel::HOLE)
			isMass[i] = false;
	}
	//�v�Z���Ă���
	//�ׂ荇���}�X�́A�G�e�[�����ݓx�𒲂ׂ�
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
	//�ׂ荇���}�X�ɓG�`�[�������邩�ǂ���
	for (int i = 0; i < 4; ++i)
	{
		if (!isMass[i])
			continue;
		//�Y���}�X�ɑ���ID�̂��̂����邩���肷��
		for (int j = 0; j < NumOfRegisterIDs; ++j)
		{
			if (Xmap[i] != mIDandMap[j].Xmap)
				continue;
			if (Ymap[i] != mIDandMap[j].Ymap)
				continue;
			//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ�������
			//���ꂪ�G���̂��̂ł��邩�ǂ�������
			if (targetTeam != mIDandMap[j].team) {
				isEnemy[i] = true;
			}
			//�����Ȃ�A�����ɂ͓����Ȃ��悤�ɂ���i�Ƃ肠�����B����ύX�̉\������j
			else {
				isMass[i] = false;
			}
			break;
		}
	}


	//�Ō�ɁA���ʂ𐮗�
	for (int i = 0; i < 4; ++i) {
		if (!isMass[i])
			continue;
		enemyIsNext |= isEnemy[i];
		enemyIsNear |= (enemyParam[i] > 0);
	}
	//�G���ׂɂ���Ȃ�
	if (enemyIsNext) {
		for (int i = 0; i < 4; ++i) {
			if (isEnemy[i]) {
				targetNext = targetMass[i];
				break;
			}
		}
	}
	//�߂��ɂ��邩
	if (enemyIsNear) {
		//�G�`�[�����ݓx���ł��������̂𒊏o
		int max = 0;
		for (int i = 0; i < 4; ++i) {
			if (!isMass[i])
				continue;
			//�ő�l����
			if (max <= enemyParam[i]) {
				//�l�������Ƃ��́A�����_����
				if (max == enemyParam[i]) {
					if (MyRandom::instance()->getInt(0, 100) < 50)
						continue;
				}
				max = enemyParam[i];
				//�������^�[�Q�b�g�ɂ���
				targetNear = targetMass[i];
			}
		}
		//�G�`�[�����ݓx���ł��Ⴂ���̂𒊏o
		int min = 1000000;
		for (int i = 0; i < 4; ++i) {
			if (!isMass[i])
				continue;
			if (isEnemy[i])
				continue;
			//�ŏ��l����
			if (min > enemyParam[i]) {
				min = enemyParam[i];
				//�������^�[�Q�b�g�ɂ���
				targetRunAway = targetMass[i];
				//�t���O���Z�b�g���Ă���
				canRunAway = true;
			}
		}
		//�Ō�ɁA���g��������}�X�̃p�����[�^�Ƃ̔�r���s��
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
		//���g��������}�X�̕����G���ݓx�������Ȃ瓮���K�v�Ȃ��Ƃ���
		if (temp)
			enemyIsNear = false;
	}
	//���������X�^�[���v���C���\�����G����͈�
	int distanceOfSearchPlayer = 5;
	//�v���C���[�Ƃ̃}�X�I����
	int pX, pY, mX, mY;
	getXYfromID(playerID, &pX, &pY);	//�v���C���[
	getXYfromID(id, &mX, &mY);			//�����X�^�[
	int diffX = ((pX - mX) >= 0) ? pX - mX : mX - pX;
	int diffY = ((pY - mY) >= 0) ? pY - mY : mY - pY;
	//���������G�͈͓����ד��m�łȂ�
	if ( (diffX + diffY) <= distanceOfSearchPlayer && (diffX + diffY) != 1) {
		playerIsNear = true;
		bool tempFlag = false;
		//X����
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
		//�ŏI�I�ɁA�}�X���������Ńv���C���[�ɋ߂Â��Ȃ��Ȃ�ړ�����Ȃ�
		if (!tempFlag)
			playerIsNear = false;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//��������̏����͉������U���p�B�w��}�X�ȓ��ɂ���G�����G���Ă���ID��ۑ�����///
	/////////////////////////////////////////////////////////////////////////////////
	//�G���ׂɂ���Ȃ�킴�킴�����̓G�����G���Ȃ��B���G������0�i�������͂P�j�Ǝw�肵���ꍇ�����G���Ȃ�
	if (enemyIsNext || searchDistance == 0 || searchDistance == 1) {
		nearTargetID = -1;
		return;
	}
	//������searchDistance�ɏ]���č��G���s��
	int centerX, centerY;					//����̒��S�l���̍��W
	int minimumDistance = searchDistance+1;	//���G���ɂ���G�ւ̍ŏ������B�����l���������邱�ƂŊm���ɍ��G���ɂ���G�����m�ł���
	getXYfromID(id, &centerX, &centerY);
	for (int x = 0; x <= searchDistance; ++x)	//searchDistance���܂߂Ă���_�ɒ���
	{
		for (int y = 0; y <= searchDistance - x; ++y)
		{
			int Xmap, Ymap, map;

			// +x +y
			Xmap = centerX + x;
			Ymap = centerY + y;
			map = getMapFromXY(Xmap, Ymap);
			//�G���A�O�`�F�b�N��ʂ邩
			if (map != -1) {
				//���̃}�X�ɓG�����邩�ǂ���
				if (enemyOnTheMap(map, targetTeam)) {
					//���̓G�ւ̋����𑪂�B��΋���
					int distance = ((x >= 0) ? x : -x) + ((y >= 0) ? y : -y);
					//�����_�ō��G���Ă���G�̂��̂�苗�����Z���Ȃ�X�V
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
	//�G���S�ł��Ă���
	return true;
}


//////////////////�v���C���[��AI�p/////////////////
//�o�H�T��
void Stage::doPathPlanning(int id, Mass::Panel destination)
{
	//�o�H�L���[�̏�����
	SAFE_DELETE_ARRAY(targetQue);
	targetQueIndex = 0;
	targetQue = new TargetMass[MAX_TARGET_QUE];
	for (int i = 0; i < MAX_TARGET_QUE; i++)
		targetQue[i] = TargetMass::TARGET_NONE;

	//�����̍��W
	int actorX(0), actorY(0);
	getXYfromID(id, &actorX, &actorY);
	//���������ɖړI�����݂��邩
	int destX(0), destY(0);
	//�Ƃ肠�������̓S�[���ȊO�l���Ȃ�
	destX = GoalMapX;
	destY = GoalMapY;
	//�ړI�n�Ƃ̍���
	int actorRoomNumber = getRoomNumberFromXY(actorX, actorY);
	int destRoomNumber = getRoomNumberFromXY(destX, destY);
	//�ʘH
	int corridorIndex(-1);

	//�K�i�����������ɂȂ��Ȃ�
	if (actorRoomNumber != destRoomNumber) {
		//��Ԃ͂��߂ɒʘH�̏�ɂ��邩�ǂ���
		bool tempFlag = false;
		if (isOnCorridor(actorX, actorY, &corridorIndex)) {
			tempFlag = true;
			int room1(0), room2(0);
			int dis1(0), dis2(0);
			corridors[corridorIndex]->getRoomNumbers(&room1, &room2);
			//��������
			dis1 = mapData[room1].getDistanceToGoal();
			dis2 = mapData[room2].getDistanceToGoal();
			//�߂��ق���I��
			if (dis1 <= dis2)
				corridors[corridorIndex]->getXYfromRoomNumber(room1, &destX, &destY);
			else
				corridors[corridorIndex]->getXYfromRoomNumber(room2, &destX, &destY);
		}
		//���������ɃS�[�����Ȃ��Ȃ�A���Ԓn�_��ړI�n�ƒu��
		if(!tempFlag || actorX == destX && actorY == destY){
			//�ʘH�[�𒆊Ԓl�ɂ���
			//�ڎw���ׂ��������m�肳����
			destRoomNumber = nextRoomEvaluation(actorRoomNumber);
			//�����������镔���Ƃ��̕��������ԒʘH���擾
			corridorIndex = getCorridorIndex(actorRoomNumber, destRoomNumber);
			//�ʘH�[�܂ł̍��W���擾
			corridors[corridorIndex]->getXYfromRoomNumber(actorRoomNumber, &destX, &destY);
		}
	}

	//������������܂ŌJ��Ԃ�
	int roopCount = 0;
	while (true) {
		//���[�v�J�E���g
		roopCount++;
		if (roopCount > 10) {
			roopCount = 0;
			getRandomMassOfRoom(actorRoomNumber, &destX, &destY);
		}
		bool finishFlag = false;//���[�v���甲����悤
		//�������߂�
		int diffX = destX - actorX;
		int diffY = destY - actorY;
		int nowX = actorX;
		int nowY = actorY;
		//�ړI�n�܂ł̌o�H���i�[���Ă���
		TargetMass moveX = (diffX > 0) ? TARGET_LEFT : TARGET_RIGHT;
		TargetMass moveY = (diffY > 0) ? TARGET_UP : TARGET_DOWN;
		//���ɒ���
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
			//�ړ��\���m�F
			if (!judgeMove(nowX, nowY, targetQue[i], id))
				break;
			//�����ʂ����̂ŁA���ݒn���X�V
			getXYfromXYandTarget(nowX, nowY, targetQue[i], &nowX, &nowY);
			if (i == diffSum - 1)
				finishFlag = true;
		}
		for (int i = 0; i < diffSum; i++) {
			TargetMass temp = targetQue[i];
			TargetMass dsf = temp;
		}
		//�I������
		if (finishFlag)
			break;
	}
}

//�o�H�i�����j���擾
void Stage::getPath(TargetMass& target)
{
	target = targetQue[targetQueIndex];

	targetQueIndex += 1;
}

//�����ԍ���Ԃ������ԍ���Ԃ�
int Stage::getRoomNumberFromXY(int x, int y) const
{
	for (int i = 0; i < ROOM_NUM; i++)
	{
		if (mapData[i].checkMapExistFromXY(x, y))
			return i;
	}

	return -1;
}

//���ݕ����ƗׂɈʒu���镔���̕����Ԃ��擾
int Stage::nextRoomEvaluation(int nowRoomNumber)
{
	//�ׂ̕����̓��A�S�[���ɋ߂��ق���Ԃ�
	//��������ꍇ�̓����_���őI��

	//���ݒn���
	int nowMapX = mapData[nowRoomNumber].MapX;
	int nowMapY = mapData[nowRoomNumber].MapY;
	
	//��F�O�@�E�F�P�@���F�Q�@���F�R
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

	//�]���l�̔�r
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

////////////////////�f�o�b�O�p//////////////////////////
void Stage::showMassParameter()
{
	//�������m��
	int** arr = new int*[MASS_HEIGHT];
	for (int i = 0; i < MASS_HEIGHT; ++i)
		arr[i] = new int[MASS_WIDTH];

	//�l����ꍞ��ł����B�Ƃ肠�����v���C���[�`�[���p�����[�^��\��
	for (int i = 0; i < MASS_HEIGHT; ++i) {
		for (int j = 0; j < MASS_WIDTH; ++j) {
			int map = getMapFromXY(j, i);
			arr[i][j] = mMasses[map].getMassParam()->TeamPlayer;
		}
	}

	//�\��
	FrontDisplay::instance()->showMasses(arr, MASS_HEIGHT, MASS_WIDTH);

	//���������
	for (int i = 0; i < MASS_HEIGHT; ++i)
		SAFE_DELETE_ARRAY(arr[i]);
}

//////////////���̃N���X���ł����g��Ȃ��֐�////////////////////

//����ID�̍��W�����̑��̑S�Ă�ID�̍��W�ƈ�v���Ă��邩�ǂ����𔻒�B��v���Ă���Ȃ�true��Ԃ��B
//�����o�^����Ă���ID�����ɂȂ��Ȃ�false��Ԃ�
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
		//�����܂œ˔j���Ă����炻���2��ID����v���Ă���Ƃ�������
		return true;
	}

	return false;
}

int Stage::getIndexFromID(int id) const {
	//ID���X�g����Y����ID������
	for (int i = 0; i < MAX_NUM_OF_ID; i++)
	{
		if (mIDandMap[i].ID == id)
			return i;
	}
	
	HALT("File: Stage.cpp [getIndexFromID] That's ID is not exist. Error"); 
	return -1;
}

//�G���A�O�Ȃ�-1��Ԃ�
int Stage::getMapFromXY(int x, int y) const
{
	//�G���A�O�`�F�b�N
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
	//ID���X�g����Y����ID������
	int target = -1;
	for (int i = 0; i < MAX_NUM_OF_ID; i++)
	{
		if (mIDandMap[i].ID == id) {
			//index��target�Ɋi�[
			target = i;
			break;
		}
	}
	if (target == -1) {
		HALT("File:Stage.cpp [getMapFromID()] Error");
		int a = 0;
	}
	//�Ώۂ̃}�X����肷��
	int x = mIDandMap[target].Xmap;
	int y = mIDandMap[target].Ymap;

	return getMapFromXY(x, y);
}

int Stage::getMapFromIDandTarget(int id, TargetMass targetMass) const
{
	int x(0), y(0);
	getXYfromIDandTarget(id, targetMass, &x, &y);
	//�G���A�O�`�F�b�N
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
	//�Ώۂ̃}�X����肷��
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
		//�ׂ荇���}�X�ȊO�̃^�[�Q�b�g�͍��̂Ƃ�������ł��Ă��Ȃ�
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

//���̃}�X�ɒN�����Ȃ��Ȃ�\1��Ԃ�
int Stage::getIDfromMap(int map) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);
	//�Y���}�X��ID�����邩���肷��
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ�������
		return mIDandMap[i].ID;
	}

	//HALT("File:Stage.cpp [getIDfromMap()] Error");

	return -1;
}

bool Stage::characterOnTheMap(int map) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);
	
	//�Y���}�X�ɑ���ID�̂��̂����邩���肷��
	for (int i = 0; i < NumOfRegisterIDs; ++i)
	{
		if (x != mIDandMap[i].Xmap)
			continue;
		if (y != mIDandMap[i].Ymap)
			continue;
		//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ�������
		return true;
	}

	return false;
}

bool Stage::enemyOnTheMap(int map, Team myTeam) const
{
	int x(0), y(0);
	getXYfromMap(map, &x, &y);

	//�Y���}�X�ɑ���ID�̂��̂����邩���肷��
	for (int j = 0; j < NumOfRegisterIDs; ++j)
	{
		if (x != mIDandMap[j].Xmap)
			continue;
		if (y != mIDandMap[j].Ymap)
			continue;
		//�����܂ł�����A�Ώۃ}�X�ɒN�������݂��Ă���Ƃ�������
		//���ꂪ�G���̂��̂ł��邩�ǂ�������
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
	//�c�����̒[
	if (room1_MapX == room2_MapX) {
		//x���W
		if (x != room1_MapX)
			return false;
		//y���W
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
	//�������̒[
	else {
		ASSERT(room1_MapY == room2_MapY);
		//y���W
		if (y != room1_MapY)
			return false;
		//x���W
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