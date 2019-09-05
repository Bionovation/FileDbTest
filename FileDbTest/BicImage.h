#ifndef _BIC_IMAGE_H_97666_
#define _BIC_IMAGE_H_97666_

/*
�Զ���ͼ���ļ�����ʽ����1.0
������Ƭ������ȫ���洢������
�ṹ��

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
	long long fileOffset;	// ���ļ��е�ƫ����
	long long binarySize;	// �ļ��Ĵ�С
};

// һ�������
struct BicImgItem1
{
	BicImgItemPos picPos;
	BicImgIndex   idxLayer0;
};

// ���������
struct BicImgItem2
{
	BicImgItemPos picPos;
	BicImgIndex   idxLayer0;
	BicImgIndex   idxLayer1;
};

// ���������
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
	long long InfosStart;		// ƫ��������Ϣ�ε���ʼλ��
	long long DatasStart;		// �������ļ�����ʼλ��
	char reserved[128]; // ����

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
	int offsetNow;		// д�ļ���ʱ�򣬵�ǰ��ƫ����
	FILE *pfile;

	std::mutex m_lock;	// ���̶߳�ȡʱ�����

	OpenMode openMode;


private:

};


#endif // !_BIC_IMAGE_H_97666_