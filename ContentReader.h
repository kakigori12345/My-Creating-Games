#pragma once

#include <string>

//テキストファイルから定数を取りだす
//ゲーム中の細かい数値を変更するたびにコンパイルし直すのを防ぐため、
//そのような定数は別のテキストファイルで管理する。
//このクラスはそのファイルから定数項を取り出し、
//各クラスにデータを配布する役割をもつ
class ContentReader {
public:
	//getDataで対象のデータが見つからなかったときに返す値
	static const std::string NullDataStr;
	static const float NullDataFloat;
	//データフレームとしてテキストファイル内のデータをまとめる
	struct DataFrame {
		DataFrame() : elementnum(0), names(nullptr), values(nullptr) {}
		~DataFrame() {
			delete[] names;
			names = nullptr;
			delete[] values;
			values = nullptr;
		}
		int elementnum;//要素数
		std::string *names;//要素名
		std::string *values;//要素の値
	};

	ContentReader(const char* filename);
	~ContentReader();
	//テキストデータから値を読み込みはじめる。1回だけ呼び出す
	void readStart();
	//特定のデータ領域から該当データを取り出す
	//該当するデータがないなら NULL or -1.f を返す
	std::string getData_str(const char* dataframeName, const char* keyName) const;
	float getData(const char* dataframeName, const char* keyName) const;
	//情報を返す関数
	int getNumOfDataFrames() const;
	std::string getNameOfDataFrame( int index ) const;
private:
	int mFileSize;
	char* mFileData;
	bool wasRead;
	//テキストファイルからのデータを格納するデータフレーム
	int mDataFrameNum;
	std::string* mNameListOfDataFrame;
	DataFrame* mDataFrameList;
};