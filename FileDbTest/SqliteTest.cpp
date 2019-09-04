#include "sqlite3.h"

#include <iostream>
#include <windows.h>

using namespace std;

void sqliteTest()
{
	int nRes = 0;
	sqlite3 * pDb = nullptr;

	const char* dbPath = "f:\\pics.db";

	int batchSize = 1000;
	int numRows = 300000;
	int imageDataSize = 60 * 1024;
	void *imageData = malloc(imageDataSize); // 60KB

	char *errorMessage = "";
	char *cmdCreatBolbTable = "create table pics(id integer PRIMARY KEY, pic blob)";

	remove(dbPath);
	if (nRes = sqlite3_open(dbPath, &pDb), nRes != 0) {
	// if (nRes = sqlite3_open(":memory", &pDb), nRes != 0) {
		cout << "sqlite3_open falied." << endl;
		return;
	}

	sqlite3_exec(pDb, "PRAGMA synchronous = OFF; ", 0, 0, 0);

	if (nRes = sqlite3_exec(pDb, cmdCreatBolbTable, nullptr, nullptr, &errorMessage), nRes != 0) {
		cout << "sqlite3_exec falied." << endl;
		return;
	}

	long timeStart = GetTickCount();

	sqlite3_stmt *stmt = nullptr;

	const char* sql = "insert into pics values(?,?)";
	if (nRes = sqlite3_prepare(pDb, sql, strlen(sql), &stmt, nullptr), nRes != 0) {
		cout << "sqlite3_prepare failed" <<endl;
		return;
	}

	// 开启事务
	if (nRes = sqlite3_exec(pDb, "BEGIN", nullptr, nullptr, &errorMessage), nRes != 0) {
		cout << "sqlite3_exec begin failed, error = " << errorMessage << endl;
		return;
	}

	for (int r = 0; r < numRows; r++) {

		if (nRes = sqlite3_bind_int(stmt, 1, r), nRes != 0) {
			cout << "sqlite3_bind_int failed, r = " << r << endl;
			return;
		}

		if (nRes = sqlite3_bind_blob(stmt, 2, imageData, imageDataSize, nullptr), nRes != 0) {
			cout << "sqlite3_bind_blob failed, r = " << r << endl;
			return;
		}

		if (nRes = sqlite3_step(stmt), nRes != SQLITE_DONE) {
			cout << "sqlite3_step failed, r = " << r << endl;
			return;
		}

		// 提交事务
		if (r % batchSize == 0 && r != 0) {
			if (nRes = sqlite3_exec(pDb, "COMMIT", nullptr, nullptr, &errorMessage), nRes != 0) {
				cout << "sqlite3_exec begin failed, error = " << errorMessage << endl;
				return;
			}

			if (nRes = sqlite3_exec(pDb, "BEGIN", nullptr, nullptr, &errorMessage), nRes != 0) {
				cout << "sqlite3_exec begin failed, error = " << errorMessage << endl;
				return;
			}
		}

		if (r % 1000 == 0)
		{
			cout << r << ": " << GetTickCount() - timeStart << endl;
		}

		sqlite3_reset(stmt);
	}

	// 最后提交一次
	if (nRes = sqlite3_exec(pDb, "COMMIT", nullptr, nullptr, &errorMessage), nRes != 0) {
		cout << "sqlite3_exec begin failed, error = " << errorMessage << endl;
		return;
	}

	sqlite3_finalize(stmt);

	sqlite3_close(pDb);

	long timeSpend = GetTickCount() - timeStart;

	cout << "time spend " << timeSpend << " ms" << endl;
	cin.ignore();

}