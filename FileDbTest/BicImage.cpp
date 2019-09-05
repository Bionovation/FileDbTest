

#include <iostream>
#include <time.h>
#include <assert.h>
#include <Exception>
#include <string>
#include "BicImage.h"
#include "MyLog.h"

// 自定义图像文件包格式定义1.0


using namespace std;

BicImage::BicImage()
	:header(nullptr), pfile(nullptr), segItemInfo(nullptr)
{
}

BicImage::~BicImage()
{
}

int BicImage::WriteHeader()
{
	if (header == nullptr || pfile == nullptr) {
		assert(false);
		return -1;
	}

	// fseek(pfile, 0, SEEK_SET);
	_fseeki64(pfile, 0, SEEK_SET);
	fwrite(header, sizeof(BicImgHeader), 1, pfile);

	return 0;
}

int BicImage::WriteInfos()
{
	if (segItemInfo == nullptr || pfile == nullptr || segItemInfoSize < 1) {
		assert(false);
		return -1;
	}

	_fseeki64(pfile, sizeof(BicImgHeader), SEEK_SET);
	fwrite(segItemInfo, segItemInfoSize, 1, pfile);

	return 0;
}


int BicImage::Create(string path, int rows, int cols, BicImgSize *layerSizes, int layers)
{
	openMode = ModeWrite;
	
	// check the parament;
	if (rows < 0 || cols < 0 || layerSizes == nullptr || layers != 1 && layers != 3) {
		assert(false);
		throw(new exception("check the parament falied."));
	}

	// create file
	if (pfile)(fclose(pfile));
	CMyLog::debug(path.c_str());
	pfile = fopen(path.c_str(), "wb");

	//fopen_s(&pfile, path.c_str(), "wb");
	assert(pfile != nullptr);


	// header
	if (header != nullptr) {
		delete header;
		header = nullptr;
	}

	header = new BicImgHeader();
	header->rows = rows;
	header->cols = cols;
	header->layerCount = layers;
	for (int i = 0; i < layers; ++i) {
		header->layerSizes[i] = layerSizes[i];
	}

	// info	
	if (segItemInfo) free(segItemInfo);
	segItemInfoSize = rows * cols * sizeof(BicImgItem1);
	if (layers = 3) segItemInfoSize = rows * cols * sizeof(BicImgItem3);

	segItemInfo = malloc(segItemInfoSize);
	memset(segItemInfo, segItemInfoSize, 0x00);

	// 更新计算偏移量
	header->InfosStart = sizeof(BicImgHeader);
	header->DatasStart = header->InfosStart + segItemInfoSize;

	// 写入文件头
	int rtn = WriteHeader();
	rtn = WriteInfos();
	

	return 0;
}

int BicImage::Open(string path)
{
	openMode = ModeRead;

	// check file
	if (pfile)(fclose(pfile));
	pfile = fopen(path.c_str(), "rb");
	//fopen_s(&pfile,path.c_str(), "rb");

	//assert(pfile != nullptr);
	if (pfile == nullptr)
	{
		CMyLog::debug("BicImage::Open() falied. pfile == nullptr");
		return -1;
	}


	// header
	if (header != nullptr) {
		delete header;
		header = nullptr;
	}

	header = new BicImgHeader;
	char *pbuf = (char*)header;

	long long readed = 0;
	do 
	{
		readed = fread(pbuf, sizeof(BicImgHeader), 1, pfile);
	} while (readed < 1);
	
	// offset
	if (segItemInfo) free(segItemInfo);
	segItemInfoSize = header->rows * header->cols * sizeof(BicImgItem1);
	if (header->layerCount = 3) segItemInfoSize = header->rows * header->cols * sizeof(BicImgItem3);

	segItemInfo = malloc(segItemInfoSize);
	memset(segItemInfo, segItemInfoSize, 0x00);

	pbuf = (char*)segItemInfo;
	readed = 0;
	do
	{
		readed = fread(pbuf, segItemInfoSize, 1, pfile);
	} while (readed < 1);
	

	return 0;
}

// 获取info的偏移量
long long BicImage::GetPicOffsetInfo(int row, int col, int layer)
{
	if (segItemInfo == nullptr || header == nullptr) {
		assert(false);
		return -1;
	}

	// 信息的起始位置
	const long long &ItemsInfoStart = header->InfosStart;
	const long long &layers = header->layerCount;

	if (layers != 1 && layers != 3) {
		assert(false);
		return -2;
	}

	/*
	存储方式是先行后列
	r行c列的存储位置是：(r*cols+c) * sizeof(BicImgItem)
	*/
	long long bicImgItemSize = 0;
	if(layers == 1) bicImgItemSize = sizeof(BicImgItem1);
	else bicImgItemSize = sizeof(BicImgItem3);

	long long offsetInInfos = (row * header->cols + col) * bicImgItemSize;

	return offsetInInfos + header->InfosStart;
}

int BicImage::Read(int row, int col, int layer, void* buf, int bufMaxSize, int &bufRead) {

	if (pfile == nullptr || segItemInfo == nullptr || header == nullptr) {
		//assert(false);
		return -1;
	}

	if (row < 0 || row >= header->rows || col < 0 || col >= header->cols || layer < 0 || layer >= header->layerCount) {
		//assert(false);
		return -2;
	}

	// 加锁
	m_lock.lock();

	long long fileOffset = GetPicOffsetInfo(row, col, layer);
	if (fileOffset < 0) {
		m_lock.unlock();
		//assert(false);
		return -3;
	}

	_fseeki64(pfile, fileOffset, SEEK_SET);

	BicImgIndex imgIdx;

	long long posStart, posEnd;
	if (header->layerCount == 1) {
		BicImgItem1 bicImg;
		fread(&bicImg, sizeof(bicImg), 1, pfile);
		imgIdx = bicImg.idxLayer0;
	}
	else
	{
		BicImgItem3 bicImg;
		fread(&bicImg, sizeof(bicImg), 1, pfile);
		
		if(layer == 0) imgIdx = bicImg.idxLayer0;
		else if (layer == 1) imgIdx = bicImg.idxLayer1;
		else imgIdx = bicImg.idxLayer2;
	}

	if (imgIdx.binarySize > bufMaxSize) {
		std::cout << "bufsize is not enough." << endl;
		//assert(false);
		m_lock.unlock();
		return -4;
	}

	// 如果没有这一张小图
	if (imgIdx.fileOffset < sizeof(BicImgHeader) + segItemInfoSize || imgIdx.binarySize < 1) {
		// std::cout << "not have this jpeg file." << endl;
		//assert(false);
		m_lock.unlock();
		return -5;	// 没有这张图
	}

	_fseeki64(pfile, imgIdx.fileOffset, SEEK_SET);

	long long readed = 0;

	readed = fread(buf, imgIdx.binarySize, 1, pfile);
	if (readed != 1) {
		m_lock.unlock();
		assert(false);
		return -6;
	}

	bufRead = imgIdx.binarySize;

	m_lock.unlock();

	return 0;
}

int BicImage::Write(int row, int col, int layer, void *buf, int bufSize)
{
	if (pfile == nullptr || segItemInfo == nullptr || header == nullptr) {
		//assert(false);
		return -1;
	}

	if (row < 0 || row >= header->rows || col < 0 || col >= header->cols || layer < 0 || layer >= header->layerCount) {
		//assert(false);
		//return -2;
		return 0;
	}

	long long fileOffset = GetPicOffsetInfo(row, col, layer);
	if (fileOffset < 0) {
		//assert(false);
		return -3;
	}

	_fseeki64(pfile, 0, SEEK_END);

	BicImgIndex imgIdx;
	//imgIdx.fileOffset = ftell(pfile);
	imgIdx.fileOffset = _ftelli64(pfile);
	imgIdx.binarySize = bufSize;

	long long offsetInInfos = fileOffset - sizeof(BicImgHeader);
	if (header->layerCount == 1) {
		BicImgItem1* bicImg = (BicImgItem1*)((char*)segItemInfo + offsetInInfos);
		bicImg->idxLayer0 = imgIdx;
	}
	else
	{
		BicImgItem3* bicImg = (BicImgItem3*)((char*)segItemInfo + offsetInInfos);
		
		if (layer == 0)  bicImg->idxLayer0 = imgIdx;
		else if (layer == 1) bicImg->idxLayer1 = imgIdx;
		else imgIdx = bicImg->idxLayer2 = imgIdx;
	}
	

	fwrite(buf, bufSize, 1, pfile);

	return 0;
}


int BicImage::Check(int row, int col, int layer, int &imgOffset, int &bufSize)
{
	if (pfile == nullptr || segItemInfo == nullptr || header == nullptr) {
		return -1;
	}

	if (row < 0 || row >= header->rows || col < 0 || col >= header->cols || layer < 0 || layer >= header->layerCount) {
		return -2;
	}

	long long fileOffset = GetPicOffsetInfo(row, col, layer);
	if (fileOffset < 0) {
		return -3;
	}

	_fseeki64(pfile, fileOffset, SEEK_SET);

	BicImgIndex imgIdx;

	long long posStart, posEnd;
	if (header->layerCount == 1) {
		BicImgItem1 bicImg;
		fread(&bicImg, sizeof(bicImg), 1, pfile);
		imgIdx = bicImg.idxLayer0;
	}
	else
	{
		BicImgItem3 bicImg;
		fread(&bicImg, sizeof(bicImg), 1, pfile);

		if (layer == 0) imgIdx = bicImg.idxLayer0;
		else if (layer == 1) imgIdx = bicImg.idxLayer1;
		else imgIdx = bicImg.idxLayer2;
	}

	// 如果没有这一张小图
	if (imgIdx.fileOffset < sizeof(BicImgHeader) + segItemInfoSize || imgIdx.binarySize < 1) {
		return -5;	// 没有这张图
	}

	// 
	imgOffset = imgIdx.fileOffset;
	bufSize = imgIdx.binarySize;

	return 0;
}

int BicImage::Close()
{
	int rtn = 0;
	switch (openMode)
	{
	case BicImage::ModeRead:
		fclose(pfile);
		pfile = nullptr;
		break;
	case BicImage::ModeWrite:
		WriteHeader();
		WriteInfos();
		fclose(pfile);
		pfile = nullptr;
		break;
	default:
		rtn = -1;
		break;
	}

	// 清理内存
	if (header != nullptr) {
		delete header;
		header = nullptr;
	}

	if (segItemInfo) {
		free(segItemInfo);
		segItemInfo = nullptr;
	}

	return rtn;
}