#pragma once
#include <string>
#include<fstream>

using namespace std;

static class Logging
{
public:
	string LogFileName;
	string LogFileNameWMemDump;

	static void WriteToLogFile(string line);
	static void WriteToMemDumpLog(string line);
};

