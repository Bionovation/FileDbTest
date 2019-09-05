#ifndef _BIC_CELLPICS_H_542253_
#define _BIC_CELLPICS_H_542253_

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string>
#include <mutex>

using std::string;

/*
�ļ���Ϣ��
 |- ��׺��".cells"
 |- �ļ��ṹ��
     |- �ļ�ͷ BicCellsHeader
	 |- ���� [BicCellsIndex0, index1, ...], ȡ����ʱ��ʹ������
	 |- �������ļ� [binary0, binary1, ...]

*/


struct BicCellsIndex {
	long long fileOffset;	// ���ļ��е�ƫ����
	long long binarySize;	// �ļ��Ĵ�С
};

struct BicCellsHeader
{
	char type[8];			// Bic 1.0
	char fileType[8];		// .jpeg
	char date[20];			// 2018-01-01 10:09:59
	char  mag;				// 20 40 100
	char  isOil;			// 0 1
	int32_t   MaxCount;		// Ĭ�����֧��100W��Сͼ�ļ�
	long long IndexStart;	// ƫ��������Ϣ�ε���ʼλ��
	long long DatasStart;	// �������ļ�����ʼλ��
	char reserved[256];		// ����

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
	int offsetNow;		// д�ļ���ʱ�򣬵�ǰ��ƫ����
	FILE *pfile;

	std::mutex m_lock;	// ���̶߳�ȡʱ�����

	OpenMode openMode;


private:


private:

};



#endif // !_BIC_CELLPICS_H_542253_
