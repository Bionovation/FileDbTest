#ifndef _BIC_IMAGE_H_97666_
#define _BIC_IMAGE_H_97666_

/*
自定义图像文件包格式定义1.0
整个玻片的数据全部存储在这里
结构：

header

{imageoffset
dataindex}[]

data[]

*/


#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string>
#include <mutex>

using std::string;

struct BicImgSize
{
	short width;
	short height;
	BicImgSize(int w = 0, int h = 0)
		:width(w), height(h) {};
};



struct BicImgItemPos {
	int x;
	int y;
};


// 
struct BicImgIndex {
	long long fileOffset;	// 在文件中的偏移量
	long long binarySize;	// 文件的大小
};

// 一层金字塔
struct BicImgItem1
{
	BicImgItemPos picPos;
	BicImgIndex   idxLayer0;
};

// 二层金字塔
struct BicImgItem2
{
	BicImgItemPos picPos;
	BicImgIndex   idxLayer0;
	BicImgIndex   idxLayer1;
};

// 三层金字塔
struct BicImgItem3
{
	BicImgItemPos picPos;
	BicImgIndex   idxLayer0;
	BicImgIndex   idxLayer1;
	BicImgIndex   idxLayer2;
};

struct BicImgHeader
{
	char type[8];// Bic 1.0
	char fileType[8]; // .jpeg
	char date[20]; // 2018-01-01 10:09:59
	char  mag; // 20 40 100
	char  isOil; // 0 1
	short rows;
	short cols;
	short layerCount;
	BicImgSize bigSize; // not uused now
	BicImgSize layerSizes[3];
	long long InfosStart;		// 偏移量等信息段的起始位置
	long long DatasStart;		// 二进制文件的起始位置
	char reserved[128]; // 保留

	BicImgHeader()
		: type("Bic 1.0"),fileType(".jpg"), mag(100), isOil(1)
		,rows(0),cols(0), layerCount(0)
		, InfosStart(0),DatasStart(0)
	{
		time_t t = time(0);
		struct tm p;
		//p = gmtime(&t);

		gmtime_s(&p, &t);
		
		strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &p);
		memset(reserved, 0x00, sizeof(reserved));
	}
};





class BicImage
{
protected:
	enum OpenMode
	{
		ModeRead,
		ModeWrite
	};
public:
	BicImage();
	~BicImage();

public:
	int Create(string path, int rows, int cols, BicImgSize *layerSizes, int layers);
	int Open(string path);
	int Close();

	int Read(int row, int col, int layer, void *buf, int bufSize, int &bufRead);
	int Write(int row, int col, int layer, void *buf, int bufSize);

	int Check(int row, int col, int layer, int &fileOffset, int &bufSize);

public:
	int GetRows() { if (header != nullptr) return header->rows; }
	int GetCols() { if (header != nullptr) return header->cols; }
	int GetLayers() { if (header != nullptr) return header->layerCount; }


protected:
	long long GetPicOffsetInfo(int row, int col, int layer);
	int WriteHeader();
	int WriteInfos();

protected:
	BicImgHeader *header;
	void *segItemInfo;
	int segItemInfoSize; 
	int offsetNow;		// 写文件的时候，当前的偏移量
	FILE *pfile;

	std::mutex m_lock;	// 多线程读取时候枷锁

	OpenMode openMode;


private:

};


#endif // !_BIC_IMAGE_H_97666_