#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Math/Random.h"
#include "Mass.h"
#include <vector>

/*
Stage�N���X
Mass�N���X��ێ�����B�����̃}�X����X�e�[�W���\������A�����}�X�𐶐��E�Ǘ�����B
�}�b�v��ɑ��݂���Character�N���X�̃I�u�W�F�N�g��ID�ƃ}�b�v���W���Ǘ����Ă����B
����ɂ��A�U���̓����蔻��Ȃǂ����̃N���X���ŏ����ł���悤�ɂ���B
�}�X�̍��W�́A�Ⴆ��4�~4�Ȃ炱��Ȋ����@�i�c,���j
  --- --- --- ---
 |0,0|1,0|2,0|3,0|
  --- --- --- ---
 |0,1|1,1|2,1|3,1|
  --- --- --- ---
 |0,2|1,2|2,2|3,2|
  --- --- --- ---
 |0,3|1,3|2,3|3,3|
  --- --- --- ---
*/

class Stage {
public:
	//�����BNONE��Character�̏����Ɋ֌W�Ȃ��A�N�V�����̂Ƃ��Ɏg��
	enum Team {
		TEAM_PLAYER,	//�v���C���[�`�[��
		TEAM_ENEMY,		//�G�`�[��
		NONE
	};
	//Character��������}�X�B�ړ���U���̍ۂɗL�����ǂ����𔻒肷��悤�B���ۂ̏�����Game�N���X�����
	enum TargetMass {
		TARGET_RIGHT,
		TARGET_LEFT,
		TARGET_UP,
		TARGET_DOWN,
		TARGET_SURROUNDINGS,	//���͂̃}�X�S��
		TARGET_NONE
	};
	struct CombIDandMap {
		int ID;
		int Xmap;
		int Ymap;
		Team team;
		bool exist;
		CombIDandMap() :ID(-1), Xmap(-1), Ymap(-1), team(NONE), exist(false) {}
		void reset() { ID = -1; Xmap = -1; Ymap = -1; team = NONE; exist = false; }
	};
	//���Ɋւ���f�[�^�i�[�p�B�e��悲�ƂɁAWIDTH�̒[2�AHEIGHT�̒[2�̍��W������
	struct mapDataOfDivision {
		int ROOM_NUMBER;		//�����ԍ�
		int DISTANCE_TO_GOAL;	//�S�[���܂ł̋����i�����I�Ɂj
		int MapX;
		int MapY;
		int WIDTH_LEFT;
		int WIDTH_RIGHT;
		int HEIGHT_TOP;
		int HEIGHT_LOW;
		mapDataOfDivision() :
			ROOM_NUMBER(-1), DISTANCE_TO_GOAL(-1), MapX(-1), MapY(-1), 
			WIDTH_LEFT(-1), WIDTH_RIGHT(-1), HEIGHT_TOP(-1), HEIGHT_LOW(-1) {}
		~mapDataOfDivision() {}
		bool checkMapExistFromXY(int x, int y) const;
		void setDistanceToGoal(int distance);
		void setMap(int x, int y);
		void getXY(int* x, int* y);
		bool checkRoomNumber(int i , int j) const;
		int getDistanceToGoal() const;
	};
	//�ʘH�Ɋւ���f�[�^
	struct corridorData {
		int room1;
		int room2;
		int room1_MapX;
		int room1_MapY;
		int room2_MapX;
		int room2_MapY;
		corridorData() :
			room1(-1), room2(-1), room1_MapX(-1), room1_MapY(-1), room2_MapX(-1), room2_MapY(-1) {}
		corridorData(int room1_, int room2_, int room1_MapX_, int room1_MapY_, int room2_MapX_, int room2_MapY_) :
			room1(room1_), room2(room2_), room1_MapX(room1_MapX_), room1_MapY(room1_MapY_), room2_MapX(room2_MapX_), room2_MapY(room2_MapY_) {}
		~corridorData() {}
		//����2�����ԒʘH���ǂ���
		bool getCorridor(int room1_, int room2_) const;
		//���̍��W���ʘH�̏ォ�ǂ���
		bool isOnCorridor(int x, int y) const;
		//�����ԍ�����ʘH�[�̍��W��Ԃ�
		void getXYfromRoomNumber(int roomNumber, int*x, int* y) const;
		//����2�̃C���f�b�N�X���擾
		void getRoomNumbers(int* room1, int* room2);
	};
	Stage( int stageNumber );
	~Stage();
	void update();
	void draw() const;
	//�V�KID��o�^����B�󂯎����Vector3�Ɍ��ʂ𖄂ߍ��ށB�Ƃ肠���������ʒu�̓����_����
	void registerNewID(int id, GameLib::Math::Vector3* position, Team team);
	//������ID���폜����i�L�����N�^�[�����񂾂Ƃ��ɌĂяo���j
	void deleteID(int id);
	//�v���C���[��ID��o�^
	void registerPlayerID(int id);
	//���O�ɓ��܂ꂽ�}�X�̍X�V
	void updatePrevFootMass();
	//�S�Ẵ}�X�̃p�����[�^���X�V����BGame���Ŗ��t���[���X�V���Ă��炤
	void updateMass();
	//�}�X�̃p�����[�^���X�V����B�Ƃ肠�����S�����Z�b�g���āA�v�Z���Ȃ���
	void updateMassParameter();
	//�L�����N�^�[�̏����`�[����ύX����i���ݏ������Ă���̂ƈႤ�����Е��̃`�[���ցj
	void changeTeam(int id);
	//ID(���W)����}�X����肵�A���̃}�X���w��̃p�l���ɕύX
	void changePanel(int id, Mass::Panel toChange);
	void changePanel(int x, int y, Mass::Panel toChange);
	//ID����}�X����肵�A���~��㩂̃J�E���g���f�N�������g����B�O�ɂȂ�����true��Ԃ�
	bool decCountOfTrapStopper(int id);
	//�~�j�}�b�v�\���iFrontDisplay����āj
	void showMiniMap();
	//�^�[�������̃J�E���g�_�E����i�߂�B���݂̃J�E���g��Ԃ�
	int incTurnLimit();
	int getTurnLimit();

	//����Ԃ��֐�
	//���W��Ԃ�
	const GameLib::Math::Vector3* returnPlaceFromID(int id) const;
	//���W�ƃ^�[�Q�b�g������W��Ԃ�
	const GameLib::Math::Vector3* returnPlaceFromIDandTarget(int id, TargetMass target) const;
	//�L�����N�^�[���m�����͈͓��ɂ��邩�𔻒�B��{�I�Ȏg�����Ƃ��ẮA
	//�G���v���C���[�Ƌ߂����ǂ������肵�A�A�N�e�B�u�ɂ��邩�����肷��i�����Ƃ���ɂ���G�͓����Ȃ��j
	bool judgeActive(int judgeId, int baseId) const;
	//�ړ��\���̔���B�\�Ȃ炻�̍��W��Ԃ��AID�ƃ}�b�v�̍X�V�������ōs��
	bool judgeMove(int id, TargetMass mass, GameLib::Math::Vector3* rePos, int* passiveID);
	//�ړ��\�������Ԃ�
	bool judgeMove(int x, int y, TargetMass target, int id) const;
	//���[�v�\���̔���B��邱�Ƃ͏��judgeMove�Ɠ��l
	bool judgeWarp(int id, GameLib::Math::Vector3* rePos);
	//�}�X�ւ̊��̗L�����𔻒�
	bool judgeMassChange(int id, TargetMass mass, Mass::Panel toChange);
	//����}�X�ɂ���L�����N�^�[��ID��n���B�����L������z�肵��ID�͔z��Ŏ󂯎���ĕԂ�
	//���܂̂Ƃ��땡���͂Ƃ肠�����Ȃ��� todo:
	void getIDs(int id, TargetMass mass, int* IDs, int* IDnum);
	//�L�����N�^�[���ǂ̎�ނ̃}�X�ɂ��邩��Ԃ��B
	//�p�r�Ƃ��ẮA�v���C���[���K�i�̏�ɗ����Ă���ꍇ�𔻒肵�āA�K�w��i�߂�Ȃ�
	Mass::Panel getPanelFromID(int id) const;
	Mass::Panel getPanelFromIDandTarget(int id, TargetMass target) const;
	//�L�����N�^�[�Ɏ��g�̎���̏󋵂�c�������邽�߂̊֐�
	//�G���ׂɂ��邩�A�������͋߂��ɂ��邩
	//�ׂłȂ��߂��̓G���^�[�Q�b�g�ɂ���Ȃ�A���̍��G�͈͂��}�X�̒����Ŏw�肷��B0��1���w�肵���牓�����͍��G���Ȃ����Ƃɂ���
	//���v���C���[�̏ꏊ�����G�ł��邪�A�G������ɑS�����Ȃ��ꍇ�����T���͍s���Ă��Ȃ�
	void information(int id, bool& playerIsNear, bool& enemyIsNext, bool& enemyIsNear, bool& canRunAway,
		TargetMass& targetNext, TargetMass& targetNear, TargetMass& targetPlayer, TargetMass& targetRunAway, int searchDistance, int& nearTargetID) const;
	//ID������W���擾
	const GameLib::Math::Vector3* getPositionFromID(int id) const;
	//id��X����Y�����̍������󂯎���Ă����ɂ���L�����N�^�[��ID��Ԃ��B���Ȃ��Ȃ�-1��Ԃ�
	int getIDFromIDandMassDiff(int id, int xDiff, int yDiff) const;
	//�X�e�[�W���{�X�������ǂ���
	bool thisStageIsBossRoom() const;
	//�G���S�ł��Ă��邩�ǂ����B�{�X�����̏ꍇ�ɏ�������p�Ƃ��Ďg��
	bool AllEnemyIsDied() const;

	//�v���C���[��AI�p�֐��Q
	//�o�H�T���B������x�ėp�I�ɍ��
	void doPathPlanning(int id, Mass::Panel destination);
	//�o�H��Ԃ�
	void getPath(TargetMass& target);
	//XY���W���畔���ԍ���Ԃ�
	int getRoomNumberFromXY(int x, int y) const;
	//���ݕ����ƗׂɈʒu���镔���̂�������A�S�[���ɋ߂������Ԃ��擾
	int nextRoomEvaluation(int nowRoomNumber);
	//������W���ʘH�̏ォ�ǂ����B�܂����̒ʘH�̃C���f�b�N�X��ԋp
	bool isOnCorridor(int x, int y, int* corridorResult);
	//�����Q����ʘH�̃C���f�b�N�X���擾
	int getCorridorIndex(int room1, int room2) const;
	//���镔���́A�����_���ȏꏊ��Ԃ�
	void getRandomMassOfRoom(int roomNumber, int* x, int* y) const;

	//�f�o�b�O�p
	void showMassParameter();
private:
	Mass *mMasses;	//�X�e�[�W���\��������̃}�X
	//���f���⃂�f���̏�񂪋L����Ă���R���e�i
	GameLib::Scene::Container mStageContainer;//�X�e�[�W�̃��f�����
	GameLib::Scene::Container mBackContainer;	//�w�i�̃��f�����
	GameLib::Scene::Model mModelBackUp;	//�w�i���f���i�㔼���j
	GameLib::Scene::Model mModelBackDown;	//�w�i���f���i�������j
	float backgroundAngle;
	//���̑��֗��Ɏg��
	int NumOfRegisterIDs; //���ݓo�^���Ă���ID�̐�
	CombIDandMap* mIDandMap; //�L�����N�^�[��ID�ƃ}�b�v���𕹂�����
	int playerID;			//�v���C���[��ID�����ʂɕۑ�
	int* warpMassList;		//���[�v�}�X�ɂȂ��Ă���}�b�v���i�[����
	Mass* prevFootMass;		//���O�ɓ��܂ꂽ�}�X���i�[�BBroken�}�X�Ȃǂ̂��߂Ɏg�p
	int mTurnLimit;			//�^�[������

	//���̃N���X���ł����g��Ȃ��֐�
	//ID���m�ł̃}�X�̔��Ȃǂ��`�F�b�N
	bool idenfifyID(const CombIDandMap id) const;
	bool identifyID(int x, int y) const;
	//mIDandMap���X�g����A�w�肳�ꂽID�̃C���f�b�N�X��Ԃ�
	int getIndexFromID(int id) const;
	//x���W��y���W����}�b�v�̘A�Ԃ�Ԃ��B�}�b�v�̐����@�ɂ���ď�����ς���K�v������
	//������x,y���G���A�O���ǂ���������s���A�G���A�O�Ȃ�-1��Ԃ�
	int getMapFromXY(int x, int y) const;
	//�}�b�v����x���W��y���W��Ԃ�
	void getXYfromMap(int map, int* x, int* y) const;
	//ID����}�b�v�̘A�Ԃ�Ԃ�
	int getMapFromID(int id) const;
	int getMapFromIDandTarget(int id, TargetMass target) const;
	//ID����x���W��y���W��Ԃ�
	void getXYfromID(int id, int* x, int* y) const;
	void getXYfromIDandTarget(int id, TargetMass target, int* x, int* y) const;
	void getXYfromXYandTarget(int x, int y, TargetMass, int* reX, int* reY) const;
	Team getTeamFromID(int id) const;
	//ID��T��
	int getIDfromMap(int map) const;
	//�Y���}�b�v�̃}�X��ɁA�L�����N�^�[�����݂��邩�ǂ������ׂ�
	bool characterOnTheMap(int map) const;
	//�Y���}�b�v�̃}�X��ɁA�G�����݂��邩�ǂ������ׂ�
	bool enemyOnTheMap(int map, Team myTeam) const;
	//�����ԍ���Ԃ�
	int getRoomIndex(int i, int j);
	//�p�l����Ԃ�
	Mass::Panel getPanelFromXY(int x, int y) const;
	//2�n�_�̍��W�̋����𑪂�
	int getDistance(int x1, int y1, int x2, int y2) const;

	//�v���C���[��AI�p�ɂ��炽�ȃp�����[�^��ǉ�
	mapDataOfDivision* mapData;
	std::vector<corridorData*> corridors;
	int ROOM_NUMBER_OF_GOAL;			//�S�[�������镔���ԍ�
	int GoalMapX;						//�S�[���̍��W
	int GoalMapY;						//�S�[���̍��W
	TargetMass* targetQue;				//�o�H���i�[
	int targetQueIndex;					//���̌o�H������
	static const int MAX_TARGET_QUE;	//�o�H�L���[�̍ő吔

	//�萔�Q�@�R���X�g���N�^�̏������Ƀt�@�C������ǂݏo��
	//�}�X���̂̃p�����[�^
	int		MASS_HEIGHT;		//�c�}�X��		���Ȃ݂Ƀ}�X�̍��W��0����X�^�[�g�Ȃ̂ŁA
	int		MASS_WIDTH;			//���}�X���@	�c�������ő�}�X�ڂ̍��W�͂��̐��l��-1�����l�ɂȂ�
	int		MASS_NUM;			//�S�Ẵ}�X���i��MASS_HEIGHT�~MASS_WIDTH�j
	float	MASS_SCALE;			//�}�X�̑傫��
	float	MASS_LENGTH;		//1�}�X�̒����i�����`�j
	float	MASS_HALF_LENGTH;	//1�}�X�̔����̒��� (MASS_LENGTH / 2.f)
	//�}�X�̓��ʂȌ��ʂɊւ���
	int		MASS_EFFECT_NUM_OF_HOLE;	//�}�X�uHOLE�i�������j�v�̐�
	int		MASS_EFFECT_NUM_OF_WARP;	//�}�X�uWARP�i���[�v�j�v�̐�
	int		MASS_EFFECT_NUM_OF_BROKEN;	//�}�X�uBROKEN�i��ꂩ���j�v�̐�
	int		MASS_EFFECT_NUM_OF_RECOVERY;//�}�X�uRECOVERY�i�񕜁j�v�̐�
	int		MASS_EFFECT_NUM_OF_PARTS;	//�}�X�uPARTS�i�p�[�c����j�v�̐�
	//�_���W�����������ɕK�v�ƂȂ�p�����[�^
	int		ROOM_NUM_WIDTH;						//�����̉��̐�
	int		ROOM_NUM_HEIGHT;					//�����̏c�̐�
	int		ROOM_NUM;							//�����̑����@���2�̐�
	int		MASSNUM_OF_DIVIDING_TO_ROOM_MINIMUM;	//���}�X���̍ŏ��l
	int		MASSNUM_OF_DIVIDING_TO_ROOM_MAX;		//���}�X���̍ő�l
	int		ROOM_MIN_RANGE;							//�����͍Œ�ł����̕���ۏ؂���
	bool	ROOM_IS_BOSS;							//�{�X�������ǂ���

	//���̑�
	int		MAX_NUM_OF_ID;			//ID���X�g�̍ő吔
	int		ACTIVE_AREA_FROM_PC;	//�����X�^�[����������G����͈́B�v���C���\�`�[�����炱�͈͓̔��ɂ���G���A�N�e�B�u�ɂȂ肤��B����ȏ㗣��Ă���Ȃ��{�������Ȃ�
	int		TURN_LIMIT;				//��̃X�e�[�W�ŁA���̃^�[�������o�߂���ƃy�i���e�B
};