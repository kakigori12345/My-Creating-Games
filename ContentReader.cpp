#include "ContentReader.h"
#include "GameLib/Framework.h"
#include <fstream>
#include <string>
using namespace std;
using namespace GameLib;

const string ContentReader::NullDataStr = "NULL";
const float ContentReader::NullDataFloat = -1.f;

/*
191218:	コメントとして元々「/」を使用していたが、これだとファイルパスを指定するときにコメントとして扱われてしまう。
		そのため「/」の代わりとして「<」と「>」の組み合わせを使うことにした
201005: スペースは値として覗いていたが、含めることにした（メッセージ表示でスペースが欲しかったため）
		その他記号も追加　[!, ?]
*/

//有効な文字列の判定
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

//「 ｝」の数（データフレーム数）を数えるだけの関数
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

//「 ; 」の数（データフレーム内の要素数）を数えるだけの関数
int static countElementOfOneDataFrame(const char** p, const char* e)
{
	int count = 0;
	int replace = 0;//進ませたポインタを戻す
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

//どの領域のデータを取り出すかを決定する
string static extractTarget(const char** p, const char* e)
{
	string result;
	bool coment = false;//コメント中かどうかの判別

	while (*p < e) {
		char c = **p;//pの指す文字を読む
		++(*p);//ポインタを進める
		if (c == '<')
			coment = true;
		if (c == '>')
			coment = false;
		if (coment)//コメントは問答無用で飛ばす
			continue;
		if (c == '{' || c == '}')
			break;
		if (isNormalChar(c))
			result += c;
	}
	return result;
}

//属性（name）とその値（value）を１組抽出
//データを抽出したらtrueを返す。終了ならfalse
bool static extractNameAndValue_oneMore(
	const char** p, const char* e,
	string *name, string *value)
{
	bool isName = true;//name, valueどちらを処理中か
	bool coment = false;

	while (*p < e) {
		char c = **p;
		++(*p);
		if (c == '<')
			coment = true;
		if (c == '>')
			coment = false;
		if (coment)//コメントは問答無用で飛ばす
			continue;
		if (c == '}')
			return false;
		if (c == '"') {
			if (isName)	//1回目の「”」
				isName = false;
			else		//2回目の「”」
				return true;
		}
		/*if (isNormalChar(c)) {
			if (isName)
				*name += c;
			else
				*value += c;
		}
		*/
		//名前
		if (isName) {
			if (isNormalChar(c))
				*name += c;
		}
		//値。こっちは例外的にスペースもありとする
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
		//読み込み位置をファイルの末尾に移動
		//この状態でtellg()を呼びことで、ファイルのサイズがわかる
		input_file.seekg(0, ifstream::end);
		mFileSize = static_cast<int>(input_file.tellg());
		mFileData = new char[mFileSize];
		//ファイルの先頭に移動して、ファイルの中身を取り出す
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
	const char** begin = &data;//テキストを読む場所を保存するためにダブルポインタ
	const char* end = data + mFileSize;

	//まずは読み込むデータ単位(DataFrame)を計数
	//その数に応じて、データフレームの配列を生成
	mDataFrameNum = countDataFrameNum(begin, end);
	mNameListOfDataFrame = new string[mDataFrameNum];
	mDataFrameList = new DataFrame[mDataFrameNum];
	//読み込む位置をリセット
	begin = &data;
	int index = 0;
	while (*begin < end) {
		//扱うデータ対象の名前を取り出す
		string name = extractTarget(begin, end);
		if (*begin < end)//このチェックをしないと、最後の空白を存在しない領域に書き込むかもしれない
			mNameListOfDataFrame[index] = name;
		int elementNum = countElementOfOneDataFrame(begin, end);
		mDataFrameList[index].elementnum = elementNum;
		mDataFrameList[index].names = new string[elementNum];
		mDataFrameList[index].values = new string[elementNum];
		//実際にデータを入れ込んでいく
		bool loop = true;
		int dataframeIndex = 0;
		while (loop) {
			string name, value;
			//データがあればtrueを返す。なければ次のデータ領域へ移行
			loop = extractNameAndValue_oneMore(begin, end, &name, &value);
			if (loop) {
				mDataFrameList[index].names[dataframeIndex] = name;
				mDataFrameList[index].values[dataframeIndex] = value;
			}
			dataframeIndex++;
		}
		index++;//次の配列へ
	}
	wasRead = true;
}

string ContentReader::getData_str(const char* target, const char* key) const
{
	ASSERT(wasRead);
	//どのデータフレームか探す
	int targetIndex;
	for (targetIndex = 0; targetIndex < mDataFrameNum; targetIndex++) {
		if (mNameListOfDataFrame[targetIndex] == (string)target)
			break;
	}
	//見つからなかったらエラー
	if (targetIndex == mDataFrameNum)
		HALT("File:ContentReader.cpp [getData_str] (That's DataFrame is not exist) Error");

	//データフレーム内から対象要素を検索
	int elementIndex;
	for (elementIndex = 0; elementIndex < mDataFrameList[targetIndex].elementnum; elementIndex++) {
		string test = mDataFrameList[targetIndex].values[elementIndex];
		if (mDataFrameList[targetIndex].names[elementIndex] == (string)key)
			break;
	}
	//見つからなかったら
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