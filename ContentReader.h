#pragma once

#include <string>

//�e�L�X�g�t�@�C������萔����肾��
//�Q�[�����ׂ̍������l��ύX���邽�тɃR���p�C���������̂�h�����߁A
//���̂悤�Ȓ萔�͕ʂ̃e�L�X�g�t�@�C���ŊǗ�����B
//���̃N���X�͂��̃t�@�C������萔�������o���A
//�e�N���X�Ƀf�[�^��z�z�������������
class ContentReader {
public:
	//getData�őΏۂ̃f�[�^��������Ȃ������Ƃ��ɕԂ��l
	static const std::string NullDataStr;
	static const float NullDataFloat;
	//�f�[�^�t���[���Ƃ��ăe�L�X�g�t�@�C�����̃f�[�^���܂Ƃ߂�
	struct DataFrame {
		DataFrame() : elementnum(0), names(nullptr), values(nullptr) {}
		~DataFrame() {
			delete[] names;
			names = nullptr;
			delete[] values;
			values = nullptr;
		}
		int elementnum;//�v�f��
		std::string *names;//�v�f��
		std::string *values;//�v�f�̒l
	};

	ContentReader(const char* filename);
	~ContentReader();
	//�e�L�X�g�f�[�^����l��ǂݍ��݂͂��߂�B1�񂾂��Ăяo��
	void readStart();
	//����̃f�[�^�̈悩��Y���f�[�^�����o��
	//�Y������f�[�^���Ȃ��Ȃ� NULL or -1.f ��Ԃ�
	std::string getData_str(const char* dataframeName, const char* keyName) const;
	float getData(const char* dataframeName, const char* keyName) const;
	//����Ԃ��֐�
	int getNumOfDataFrames() const;
	std::string getNameOfDataFrame( int index ) const;
private:
	int mFileSize;
	char* mFileData;
	bool wasRead;
	//�e�L�X�g�t�@�C������̃f�[�^���i�[����f�[�^�t���[��
	int mDataFrameNum;
	std::string* mNameListOfDataFrame;
	DataFrame* mDataFrameList;
};