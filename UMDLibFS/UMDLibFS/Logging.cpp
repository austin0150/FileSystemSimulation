#include "Logging.h"


void Logging::WriteToLogFile(string line)
{
	ofstream file;
	file.open("log.txt", ios::app);
	file << line << endl;

	file.close();
	return;
}

void Logging::WriteToMemDumpLog(string line)
{
	ofstream file;
	file.open("DetailLog.txt", ios::app);
	file << line << endl;

	file.close();
	return;
}
