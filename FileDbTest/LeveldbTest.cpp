#include <iostream>
#include <string>

#include "leveldb\db.h"

using namespace std;

void testLeveldb()
{
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, "lev", &db);
	//std::cout << "connect status : " << status.ok() << std::endl;

	db->Put(leveldb::WriteOptions(), "say", "hello leveldb");
	
	int imgBufSize = 60 * 1024 * 1024;
	void *imgBuf = malloc(imgBufSize);


	std::string val = "";
	db->Get(leveldb::ReadOptions(), "say", &val);

	std::cout << val << std::endl;

}