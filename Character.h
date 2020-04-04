#pragma once

#include "GameLib/Scene/Container.h"
#include "GameLib/Scene/Model.h"
#include "GameLib/Math/Vector3.h"
#include "MyRandom.h"
#include "Weapon.h"
#include <string>

class Action;
struct Parts;	//�p�[�c�Ɋւ��ẮA���̕��ɋL�q���Ă���

//�L�����N�^�[�����p�����[�^�Q�̌^�B�t�@�C������ǂݏo���Ă���
//todo:insert �i�p�����[�^��V���ɒǉ�����Ȃ�ҏW�K�v�j
//FrontDisplay�ɂ����̃p�����[�^�ɏ�����������������̂ŁA�ύX����Ȃ炻�����	
struct Parameters {
	std::string parent;
	int health;
	int power;
	int speed;
	int defence;
	int capture;
	std::string weapon;
	int parts;
	float scale;
	std::string image;
	//�R���X�g���N�^�ɂāA�f�t�H���g�l�Ƃ��ēK���ɒl���Z�b�g���Ă���
	Parameters() :
		parent(""), health(100), power(100), speed(100), defence(100), capture(100), weapon("NULL"), parts(0), scale(1.f), image("") {}
	~Parameters() {}
};

//AI���s�������߂�ۂɎQ�l�ɂ���bool�l�̏W���B�L�����N�^�[���ƂɎ��g�̏󋵂�c������
struct Information {
	bool youAreEnemyTeam;	//�������G���ł��邩�m��
	bool playerIsNearMass;	//�v���C���[���߂��ɂ���
	bool EnemyIsNextMass;	//�G���ׂɂ���
	bool EnemyIsNearMass;	//�G���߂��ɂ���
	bool CanRunAway;		//������}�X������
	int NearTargetID;		//�������U���p�@�^�[�Q�b�gID
	int direction;			//�^�[�Q�b�g�̕����i1:��@2:�E�@3:���@4:���j
	int directionForNear;	//�߂��̓G��ǂ�������
	int directionForPlayer;	//�v���C���[��ǂ�������
	int directionForRun;	//������悤����
	//�K���ȃR���X�g���N�^
	Information() :
		youAreEnemyTeam(false), playerIsNearMass(false), EnemyIsNextMass(false), EnemyIsNearMass(false), CanRunAway(false), NearTargetID(-1), 
		direction(1), directionForNear(1), directionForPlayer(1), directionForRun(1){}
	~Information() {}
};

//AI���s����]������Ƃ��̒l
struct ParameterAI {
	int attack;				//�U������x����
	int attackMagic;		//���@�U������x����
	int moveToEnemy;		//�G�ɋ߂Â��x����
	int playerLove;			//�v���C���[�ɋ߂Â��x�����i���ԃ����X�^�[���j
	int runawayFromEnemy;	//�G���瓦����x����
	int runawayHP;			//����HP�����ɂȂ����瓦���锻�肪�L���Ȃ�

	ParameterAI() :
		attack(0), attackMagic(0), moveToEnemy(0),  playerLove(0), runawayFromEnemy(0), runawayHP(30) {}
	~ParameterAI() {}
	void set(int attack_, int attackMagic_, int moveToEnemy_, int playerLove_, int runawayFromEnemy_, int runawayHP_) {
		attack = attack_;
		attackMagic = attackMagic_;
		moveToEnemy = moveToEnemy_;
		playerLove = playerLove_;
		runawayFromEnemy = runawayFromEnemy_;
		runawayHP = runawayHP_;
	}
};

//�v���C��AI�p�́A���̂ЂƉ�B�����������͗����邪�A�F�X�ȏ����܂Ƃ߂Ă��炤
struct PlayerAIinfo {
	int directionForGoal;			//�S�[���܂ł̕����i1:��@2:�E�@3:���@4:���j
	bool destinationArrived;		//�ړI�n�ɒ������B�������S�[���Ȃ�I��

	PlayerAIinfo() :
		directionForGoal(1), destinationArrived(false) {}
	~PlayerAIinfo() {}
};


////////////////////////////////////////////////////////////////
//�v���C���[�A�G�̃L�����N�^�[�̊��N���X
//Player�N���X��Enemy�N���X�͂��̃N���X���p������
//���ꂼ��ɋ��ʂ���ϐ��E�֐��͂����ł��ׂĒ�`����
/////////////////////////////////////////////////////////////////
class Character {
public:
	//�ӎv����̕��@�i�v���C���[�p�̓��͌^�ƁA����ȊO�ɑ�ʂ����j
	enum Mind {
		MIND_INPUT,		//���͂��󂯕t����B��{�I�Ƀv���C�L�����N�^�[�p
		MIND_PLAYER_AI,	//�v���C���[��AI�B�f������ȂǂŎg�p
		MIND_AI_1,		//AI�ňӎv����B�p�^�[���@
	};
	//���ۂɍs������ۂ̎�ނ��`
	enum Act {
		ACT_WAIT,
		ACT_MOVE,			//�ړ��B�Z�b�g����Ƃ���Destination���ꏏ�ɃZ�b�g����i���ꂼ��Z�b�g���邽�߂ɁA�ʁX�̊֐����p�ӂ��Ă���j
		ACT_WARP,			//���[�v�B������
		ACT_ATTACK_NORMAL,	//�ʏ�U��
		ACT_ATTACK_MAGIC,	//���������@�U��
		ACT_FALL,			//�����B�����X�^�[��p
		ACT_CATCH,			//�ߊl�����B�����X�^�[��p
		ACT_NONE
	};
	//�s���I�����ɃQ�[���}�X�^�[�Ɉӎv��`���邽�߂ɗ��p
	enum Reply {
		REPLY_WANT_INFORMATION,	//��񂪗~�����B���AI�p
		REPLY_MOVE,				//�ړ��B������Direction���Q��
		REPLY_ATTACK_NORMAL,	//�U���i�ʏ�j
		REPLY_ATTACK_MAGIC,		//����g�����������U��
		REPLY_MASS_TRAP_HOLE,	//���Ƃ��������
		REPLY_MASS_TRAP_CATCH,	//�ߊl㩂����
		REPLY_MASS_TRAP_STOPPER,//���~�߂����
		REPLY_MASS_TRAP_BOMB,	//�n�������
		REPLY_MASS_MAKE,		//�������

		//�v���C���[AI�p
		//AI_GOAL
		REPLY_GOAL_PATH_PLANNNING,	//�V�����o�H�T�����s���Ă��炤
		REPLY_GOAL_ROOT,			//�o�H(����)�������Ă��炤

		REPLY_NONE				//�������Ȃ�
	};
	//�L�����N�^�[�̌����Ă���������`�B�Ƃ肠�����΂߂Ȃ���4����
	enum Direction {
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		INVALID
	};
	//���g�̏�Ԃ��Ǘ��B���selectAction�֐��p�̃p�����[�^
	enum State {
		STATE_WAIT,				//�ʏ펞
		STATE_THINKING,			//�s���I�����B���AI�p
		STATE_ACTION_SELECT,	//�ړ��ȊO�̓����I������
		STATE_ATTACK_SELECT,	//�ǂ̍U�����g������I��
		STATE_MAGIC_TARGET_SELECT,	//���@�U���̑Ώۂ�I������
		STATE_MASS_ACTION_SELECT,	//�}�X�ɑ΂���ǂ̂悤�ȏ��������������I��
		STATE_MASS_SELECT,		//�l���ǂ̃}�X��I�����邩�i�U�����A�}�X�ւ̊������l�ɂ��̏�ԂɈڍs�j
		STATE_WEAPON_POWERUP,	//���틭���B�������Ă��镐�����������
		STATE_ANNOUNCE,			//���ӏ�����\��
		STATE_NONE
	};
	//�v���C���[AI�p�̑傫�ȕ��j
	enum PlayerAI_Strategy {
		AI_GOAL,		//�Ƃɂ����S�[����
		AI_BATTLE,		//�߂��̓G�Ɛ퓬
		AI_RUNAWAY,		//�Ƃɂ���������

		AI_NONE
	};
	//�v���C���[AI�p�̕��j�ɑ΂����̓I�ȍs���w�j
	enum PlayerAI_Behavior {
		//AI_GOAL
		GOAL_PATH_PLANNING,		//�o�H�T��
		GOAL_RECEIVE_ROOT,		//�o�H�������Ă��炤
		GOAL_GO_ALONG_PATH,		//�o�H�ɉ����Ĉړ�
		//AI_BATTLE
		BATTLE_ATTACK_NEXT,		//�ׂ̓G���U��
		BATTLE_ATTACK_NEAR,		//�߂��̓G���U��
		BATTLE_THINK_TRAP,		//㩂̐ݒu���l����
		BATTLE_PUT_TRAP,		//㩂𒣂�
		BATTLE_DECOY,			//�G��㩂ɗU��
		//AI_RUNAWAY
		RUNAWAY_GO,				//��������������

		BEHAVE_NONE
	};

	friend class Action;
	Character();
	virtual ~Character();
	//���t���[���̍X�V
	virtual void update();
	//�����̃^�[��������Ă����Ƃ��ɂǂ��s�����邩���肷��B���肵����true��Ԃ��悤�ɂ���
	virtual bool selectAction();
	virtual void draw();
	//���ꂼ��̍s�����ƂɁA�s������Ƃ��ɍ��킹�čs������
	virtual void actEffect(Reply rep);

	//�����o�ϐ��ɏ����Z�b�g����֐����낢��
	virtual void setModel(const char* batchname);
	virtual void setPosition(const GameLib::Math::Vector3& pos);
	virtual void setAngleY(const float& angle);
	virtual void setScale(float scale);
	//�s������Ƃ��̃^�[�Q�b�g�Ƃ�����̂̍��W�i�U���ʒu�E�ړ���Ȃǁj
	virtual void setDestination(const GameLib::Math::Vector3& pos);
	//�s�����N�������邽�߂ɃA�N�V�����I�����ʂ�������B�����ŏ��X�̃p�����[�^�����������Ă���
	virtual void setAction(Act act);
	virtual void setID(int id);
	virtual void setInformation(Information& st);
	virtual void setPlayerAIinfo(PlayerAIinfo& info);
	virtual void setState(State state);
	virtual void resetActTime();
	virtual void setBeingStopped();		//���~��㩂ɂ��������Ƃ��B�����X�^�[��p
	virtual void resetBeingStopped();	//���~�ߏ�Ԃ�����
	virtual void setMagicTargetID(int target);
	virtual void pathPlanningWasFinish();

	//�����o�ϐ��̏���n���֐����낢��
	virtual const GameLib::Math::Vector3* position() const;
	virtual const float angle() const;
	virtual const float getActTime() const;
	virtual const Act getAct() const;
	virtual bool actFinished() const;
	virtual const Reply getSelect() const;
	virtual const int getCharacterIDs() const;	//���݂̃C���X�^���X�̐��Ɠ���
	virtual const int getID() const;			//�������͂���C���X�^���X��ID
	virtual const Direction getDirection() const;
	virtual const State getState() const;
	virtual const bool getAttackIsDone() const;
	virtual const bool getBeStopped() const;
	virtual const int getMagicTargetID() const;	//���������@�U�����̑Ώ�ID
	virtual const void getMagicTargetPlace(int* x, int* y) const;	//���@�U�����̑Ώۃ}�X�F�v���C���[��p

	//�p�����[�^�֘A
	//�l���Z�b�g
	virtual void setParameter(Parameters& param);
	virtual void setName(std::string name);
	virtual void setParameterAI(ParameterAI& paramAI);
	//�l��Ԃ�
	virtual const std::string getName() const;
	virtual const Parameters* getParameter() const;
	virtual const ParameterAI* getParameterAI() const;
	//HP�A�_���[�W�v�Z�Ȃ�
	virtual void damage(int receivePower);
	virtual void recovery(int plus);
	virtual void subHP(int num);
	virtual int getHP() const;
	virtual bool isLiving() const;
	//�p�[�c��n��
	virtual Parts getParts() const;

	//����֘A
	//��������炤
	virtual void getWeapon(Weapon::Type type);
	//���g�̍ő�˒�����
	virtual int canAttackToNear() const;
	//������
	virtual const Weapon& weapon() const;

	//���̑�
	void resetAllParameters();
	void turnLimitAction();		//�v���C���[��p�B�^�[�������̃y�i���e�B
	void moveFault();			//�ړ������ۂ��ꂽ�Ƃ���1��Ăяo��
	void resetAIparameter();	//AI�̃p�����[�^�����Z�b�g
protected:
	//���f���̕\���Ɋւ���l
	GameLib::Scene::Container mCharacterContainer;	//�v���C���[�A�G�����̃��f����񂪊i�[����Ă���R���e�i
	GameLib::Scene::Model mModelCharacter;			//�L�����N�^�[�̃��f��
	GameLib::Scene::Model mModelHPbar;				//HP�o�[�̃��f��
	GameLib::Math::Vector3 mPosition;				//���݂̈ʒu
	GameLib::Math::Vector3 mPrevPosition;			//�ʒu���ς��O�̈ʒu
	GameLib::Math::Vector3 mDestination;			//�ړI�n�B���ꂩ��ړ�����ׂ��ꏊ
	float mAngleY;//�p�x

	//enum�Œ�`�����p�����[�^���g�p������̂���
	Mind mMind;
	Act mAct;
	float mActTime;		//0.f�`1.f�̊ԂŃ^�C�~���O���Ǘ�����
	Reply mReply;
	Direction mDirection;
	State mState;
	Information mInformation;
	//�p�����[�^�֘A
	Parameters mParam;
	mutable Parameters mReturnParam;
	std::string mName;
	ParameterAI mParamAI;
	int HP;//���݂�HP�B�p�����[�^��health�͍ő�HP
	Parts* mParts;
	Weapon mWeapon;
	//�v���C���[��AI�p
	PlayerAIinfo	  mAIinfo;
	PlayerAI_Strategy mAIstrategy;
	PlayerAI_Behavior mAIbehavior;

	//���̑��֗��Ɏg�����߂̏��X�̕ϐ�
	static int CharacterID;	//�L�����N�^�[�����ʂ���ID�BStage�N���X���ŁA����ID�ƃ}�X�̍��W��R�Â��Ă���
	int mCharacterID;		//Character�̃C���X�^���X�������ɐÓI�ϐ�CharacterID�̒l���i�[�B�X�̃����X�^�[�����ʂ��邽�߂̂���
	bool attackIsDone;		//�A�N�V�����̒��ŁA�U���������u�Ԃ�true�ɂ���i�C�ӂ̃^�C�~���O�ōU�������邱�Ƃ��ł���j
	bool beStopped;			//㩁F�����ǂ߂�H����Ă����ԁB�A�N�e�B�u�L�����N�^�[�ύX�̂Ƃ��ɎQ�Ƃ��āAtrue�Ȃ�^�[�����΂�
	bool haveWeapon;		//���펝���Ă邩
	int magicTargetID;		//�������U�����̑Ώ�ID
	int announceMessageNumber;	//���ӂ��s���Ƃ��ɂǂ̃��b�Z�[�W��\�����邩
	State backFromAnnounce;		//���ӂ���߂��
	bool moveWasFault;		//�ړ������ۂ��ꂽ
	bool pathPlanningWasFinished;	//�o�H�T���͍ς�ł���

	//resetAllParameter()�Ń��Z�b�g����ϐ�
	int magicTargetX;	//���@�U�����ɑΏۂ�I������Ƃ���X����
	int magicTargetY;	//			;;					�@Y����

	//���̃N���X�Q�ł����g��Ȃ��֐�
	//�L�����N�^�[�̊p�x��ύX����Ƃ��Ɏ��R�Ȋ����ŕω�����悤�ɒ������邽�߂̂���
	void naturalAngleChange(float targetAngle);
	//�v���C���[�����͂��ăL�����N�^�[�̍s����I������
	bool selectOfInput();
	//�����ōs���I������p�^�[��1�B��{�I�ɂ̓����X�^�[��p
	bool selectOfMind1();
	//�v���C����AI
	bool selectOfPlayerAI();
	bool selectOfPlayerAI_GOAL();
	bool selectOfPLayerAI_BATTLE();

	//�萔�Q�@�t�@�C������ǂݏo��
	float ROTATE_SPEED;				//��]���x
	float WARP_ROTATE_SPEED;		//���[�v���̉�]���x
	float COUNT_INTERVAL_ACT_MOVE;	//COUNT_INTERVAL�́AmActTime�̃J�E���g�̐i�ރX�s�[�h�i/frame�j
	float COUNT_INTERVAL_ACT_WARP;
	float COUNT_INTERVAL_ACT_ATTACK_NORMAL;
	float COUNT_INTERVAL_ACT_ATTACK_MAGIC;
	float COUNT_INTERVAL_ACT_FALL;
	float COUNT_INTERVAL_ACT_CATCH;
	//HP�o�[�ɂ���
	float HP_BAR_SCALE_X;	//HP�o�[�̉��̒���
	float HP_BAR_SCALE_Y;	//�@""    �c�̒���
	float HP_BAR_PLACE_Y;	//�L�������f���̂�����ɕ\�����邩
	float HP_BAR_PINCH;		//�s���`���肪�������炩
	//���̑�
	float MASS_LENGTH;		//1�}�X�̒���
private:
	//���̃N���X���ł����g��Ȃ��֐�
	void toAnnouce(int messageNumber, State backTo);
};



//㩂𐶐�����Ƃ��ɕK�v�ƂȂ�p�[�c
struct Parts {
public:
	static int numParts;		//�p�[�c�̎��
	int Parts_Screw;	//�˂�
	int Parts_Hammer;	//�n���}�[
	int Parts_Net;		//��
	int Parts_Stone;	//��
	//�R���X�g���N�^
	Parts() :Parts_Screw(0), Parts_Hammer(0), Parts_Net(0), Parts_Stone(0) {}
	Parts(int screw, int hammer, int net, int stone) : 
		Parts_Screw(screw), Parts_Hammer(hammer), Parts_Net(net), Parts_Stone(stone) {}
	//�v���ɑ΂��ĕK�v�ȃp�[�c��Ԃ�
	static Parts neededParts(Character::Reply rep);
	//���O��������B�A�Ԃ�
	std::string getNameOfParts(int number);
	//�����Ă���p�[�c�̐���������B�A�Ԃ�
	int getNumOfParts(int number);
	//��������p�[�c�������Ă��邩
	bool hasSomethingParts();
	//����
	void add(Parts parts);
	//����
	void sub(Parts parts);
	//���g�𐔔{����
	void mul(int num);
	//�K�v������Ă��邩����
	bool judge(Parts parts);
	//�����X�^�[�p�B�����_����num�̐��̃p�[�c����������
	void setRandomParts(int num);
};
