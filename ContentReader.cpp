#include "ContentReader.h"
#include "GameLib/Framework.h"
#include <fstream>
#include <string>
using namespace std;
using namespace GameLib;

const string ContentReader::NullDataStr = "NULL";
const float ContentReader::NullDataFloat = -1.f;

/*
191218:	�R�����g�Ƃ��Č��X�u/�v���g�p���Ă������A���ꂾ�ƃt�@�C���p�X���w�肷��Ƃ��ɃR�����g�Ƃ��Ĉ����Ă��܂��B
		���̂��߁u/�v�̑���Ƃ��āu<�v�Ɓu>�v�̑g�ݍ��킹���g�����Ƃɂ���
201005: �X�y�[�X�͒l�Ƃ��Ĕ`���Ă������A�܂߂邱�Ƃɂ����i���b�Z�[�W�\���ŃX�y�[�X���~�����������߁j
		���̑��L�����ǉ��@[!, ?]
*/

//�L���ȕ�����̔���
bool static isNormalChar(char c) {
	if (c >= '0' && c <= '9') {
		return true;
	}
	if (c >= 'a' && c <= 'z') {
		return true;
	}
	if (c >= 'A' && c <= 'Z') {
		return true;
	}
	if (c == '_' || c == '.' || c == '-' || c == '/' || c == '!' || c == '?') {
		return true;
	}
	return false;
}

//�u �p�v�̐��i�f�[�^�t���[�����j�𐔂��邾���̊֐�
int static countDataFrameNum(const char** p, const char* e)
{
	int count = 0;
	int replace = 0;
	while (*p < e) {
		char c = **p;
		++(*p);
		replace++;
		if (c == '}')
			count++;
	}
	(*p) -= replace;
	return count;
}

//�u ; �v�̐��i�f�[�^�t���[�����̗v�f���j�𐔂��邾���̊֐�
int static countElementOfOneDataFrame(const char** p, const char* e)
{
	int count = 0;
	int replace = 0;//�i�܂����|�C���^��߂�
	while (*p < e) {
		char c = **p;
		++(*p);
		replace++;
		if (c == ';')
			count++;
		if (c == '}')
			break;
	}
	(*p) -= replace;
	return count;
}

//�ǂ̗̈�̃f�[�^�����o���������肷��
string static extractTarget(const char** p, const char* e)
{
	string result;
	bool coment = false;//�R�����g�����ǂ����̔���

	while (*p < e) {
		char c = **p;//p�̎w��������ǂ�
		++(*p);//�|�C���^��i�߂�
		if (c == '<')
			coment = true;
		if (c == '>')
			coment = false;
		if (coment)//�R�����g�͖ⓚ���p�Ŕ�΂�
			continue;
		if (c == '{' || c == '}')
			break;
		if (isNormalChar(c))
			result += c;
	}
	return result;
}

//�����iname�j�Ƃ��̒l�ivalue�j���P�g���o
//�f�[�^�𒊏o������true��Ԃ��B�I���Ȃ�false
bool static extractNameAndValue_oneMore(
	const char** p, const char* e,
	string *name, string *value)
{
	bool isName = true;//name, value�ǂ������������
	bool coment = false;

	while (*p < e) {
		char c = **p;
		++(*p);
		if (c == '<')
			coment = true;
		if (c == '>')
			coment = false;
		if (coment)//�R�����g�͖ⓚ���p�Ŕ�΂�
			continue;
		if (c == '}')
			return false;
		if (c == '"') {
			if (isName)	//1��ڂ́u�h�v
				isName = false;
			else		//2��ڂ́u�h�v
				return true;
		}
		/*if (isNormalChar(c)) {
			if (isName)
				*name += c;
			else
				*value += c;
		}
		*/
		//���O
		if (isName) {
			if (isNormalChar(c))
				*name += c;
		}
		//�l�B�������͗�O�I�ɃX�y�[�X������Ƃ���
		else {
			if (isNormalChar(c) || c == ' ')
				*value += c;
		}
	}
	return false;
}


ContentReader::ContentReader(const char* filename) :
	mFileSize(0),
	mFileData(nullptr),
	wasRead(false),
	mDataFrameNum(0),
	mNameListOfDataFrame(nullptr),
	mDataFrameList(nullptr)
{
	ifstream input_file(filename, ifstream::binary);
	if (input_file) {
		//�ǂݍ��݈ʒu���t�@�C���̖����Ɉړ�
		//���̏�Ԃ�tellg()���Ăт��ƂŁA�t�@�C���̃T�C�Y���킩��
		input_file.seekg(0, ifstream::end);
		mFileSize = static_cast<int>(input_file.tellg());
		mFileData = new char[mFileSize];
		//�t�@�C���̐擪�Ɉړ����āA�t�@�C���̒��g�����o��
		input_file.seekg(0, ifstream::beg);
		input_file.read(mFileData, mFileSize);
	}
	else {
		HALT("File:ContentReader [ContentReader's Constructor] Error");
	}
}

ContentReader::~ContentReader()
{
	SAFE_DELETE(mFileData);
	SAFE_DELETE_ARRAY(mNameListOfDataFrame);
	SAFE_DELETE_ARRAY(mDataFrameList);
}

void ContentReader::readStart()
{
	const char* data = mFileData;
	const char** begin = &data;//�e�L�X�g��ǂޏꏊ��ۑ����邽�߂Ƀ_�u���|�C���^
	const char* end = data + mFileSize;

	//�܂��͓ǂݍ��ރf�[�^�P��(DataFrame)���v��
	//���̐��ɉ����āA�f�[�^�t���[���̔z��𐶐�
	mDataFrameNum = countDataFrameNum(begin, end);
	mNameListOfDataFrame = new string[mDataFrameNum];
	mDataFrameList = new DataFrame[mDataFrameNum];
	//�ǂݍ��ވʒu�����Z�b�g
	begin = &data;
	int index = 0;
	while (*begin < end) {
		//�����f�[�^�Ώۂ̖��O�����o��
		string name = extractTarget(begin, end);
		if (*begin < end)//���̃`�F�b�N�����Ȃ��ƁA�Ō�̋󔒂𑶍݂��Ȃ��̈�ɏ������ނ�������Ȃ�
			mNameListOfDataFrame[index] = name;
		int elementNum = countElementOfOneDataFrame(begin, end);
		mDataFrameList[index].elementnum = elementNum;
		mDataFrameList[index].names = new string[elementNum];
		mDataFrameList[index].values = new string[elementNum];
		//���ۂɃf�[�^����ꍞ��ł���
		bool loop = true;
		int dataframeIndex = 0;
		while (loop) {
			string name, value;
			//�f�[�^�������true��Ԃ��B�Ȃ���Ύ��̃f�[�^�̈�ֈڍs
			loop = extractNameAndValue_oneMore(begin, end, &name, &value);
			if (loop) {
				mDataFrameList[index].names[dataframeIndex] = name;
				mDataFrameList[index].values[dataframeIndex] = value;
			}
			dataframeIndex++;
		}
		index++;//���̔z���
	}
	wasRead = true;
}

string ContentReader::getData_str(const char* target, const char* key) const
{
	ASSERT(wasRead);
	//�ǂ̃f�[�^�t���[�����T��
	int targetIndex;
	for (targetIndex = 0; targetIndex < mDataFrameNum; targetIndex++) {
		if (mNameListOfDataFrame[targetIndex] == (string)target)
			break;
	}
	//������Ȃ�������G���[
	if (targetIndex == mDataFrameNum)
		HALT("File:ContentReader.cpp [getData_str] (That's DataFrame is not exist) Error");

	//�f�[�^�t���[��������Ώۗv�f������
	int elementIndex;
	for (elementIndex = 0; elementIndex < mDataFrameList[targetIndex].elementnum; elementIndex++) {
		string test = mDataFrameList[targetIndex].values[elementIndex];
		if (mDataFrameList[targetIndex].names[elementIndex] == (string)key)
			break;
	}
	//������Ȃ�������
	if (elementIndex == mDataFrameList[targetIndex].elementnum)
		return NullDataStr;

	return mDataFrameList[targetIndex].values[elementIndex];
}

float ContentReader::getData(const char* target, const char* key) const
{
	string reply = getData_str(target, key);
	if (reply == "NULL")
		return NullDataFloat;
	return stof(reply);
}

int ContentReader::getNumOfDataFrames() const
{
	return mDataFrameNum;
}

string ContentReader::getNameOfDataFrame(int index) const
{
	return mNameListOfDataFrame[index];
}