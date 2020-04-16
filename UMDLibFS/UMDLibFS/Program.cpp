#include <iostream>
#include "UMDLibFS.h"
#include "Logging.h"

void Logit(string);

int main()
{
	UMDLibFS lib;
	char TempString[256];
	int result = 0;

	result = lib.FSBoot();
	sprintf_s(TempString, "FS boot with result: %d", result);
	Logit(TempString);
	lib.DumpLocalDisk();
	lib.DumpRemoteDisk();

	string path = "/Test/";
	string buffer = "";
	int size = 50;
	result = lib.DirCreate(path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", path, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/SubTest/";
	result = lib.DirCreate(path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", path, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/SubTest2/";
	result = lib.DirCreate(path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", path, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/";
	result = lib.DirSize(path);

	path = "/Test/";
	result = lib.DirRead(path, buffer, size);

	path = "/Test/dumb.txt";
	result = lib.FileCreate(path);

	lib.OpenFileTable[0] = path;
	lib.CurrentFilePointerTable[0] = 0;
	buffer = "This is some test text";
	result = lib.FileWrite(0, buffer, buffer.length());

	lib.CurrentFilePointerTable[0] = 0;
	buffer = "";
	result = lib.FileRead(0, buffer, 22);

	path = "/Test/SubTest2/";
	result = lib.DirUnlink(path);
	lib.DumpLocalDisk();

	result = lib.FileUnlink(path);

	//result = lib.FileRead()

	return result;
}

void Logit(string line)
{
	Logging::WriteToLogFile(line);
	Logging::WriteToMemDumpLog(line);
	
}