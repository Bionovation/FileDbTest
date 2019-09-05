#ifndef _BIC_CELLPICS_H_542253_
#define _BIC_CELLPICS_H_542253_

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string>
#include <mutex>

using std::string;

/*
文件信息：
 |- 后缀：".cells"
 |- 文件结构：
     |- 文件头 BicCellsHeader
	 |- 索引 [BicCellsIndex0, index1, ...], 取数据时候使用索引
	 |- 二进制文件 [binary0, binary1, ...]

*/


struct BicCellsIndex {
	long long fileOffset;	// 在文件中的偏移量
	long long binarySize;	// 文件的大小
};

struct BicCellsHeader
{
	char type[8];			// Bic 1.0
	char fileType[8];		// .jpeg
	char date[20];			// 2018-01-01 10:09:59
	char  mag;				// 20 40 100
	char  isOil;			// 0 1
	int32_t   MaxCount;		// 默认最大支持100W个小图文件
	long long IndexStart;	// 偏移量等信息段的起始位置
	long long DatasStart;	// 二进制文件的起始位置
	char reserved[256];		// 保留

	BicCellsHeader()
		:type("Bic 1.0"), fileType(".jpg"), mag(100), isOil(1)
		, MaxCount(1000000)
	{
		IndexStart = sizeof(BicCellsHeader);
		DatasStart = IndexStart + sizeof(BicCellsIndex) * MaxCount;
		time_t t = time(0);
		struct tm p;
		gmtime_s(&p, &t);
		strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &p);
		memset(reserved, 0x00, sizeof(reserved));
	}
};

class BicCellpics
{
protected:
	enum OpenMode
	{
		ModeRead,
		ModeWrite
	};
public:
	BicCellpics();
	~BicCellpics();

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
	BicCellsHeader *header;
	void *segItemInfo;
	int segItemInfoSize;
	int offsetNow;		// 写文件的时候，当前的偏移量
	FILE *pfile;

	std::mutex m_lock;	// 多线程读取时候枷锁

	OpenMode openMode;


private:


private:

};



#endif // !_BIC_CELLPICS_H_542253_
