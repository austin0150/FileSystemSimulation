#pragma once
#include <string>

using namespace std;

class Logging
{
public:
	string LogFileName;
	string LogFileNameWMemDump;

	void WriteToLogFile(string line);
	void WriteToMemDumpLog(string line);
};

