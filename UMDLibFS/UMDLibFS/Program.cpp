#include <iostream>
#include "UMDLibFS.h"
#include "Logging.h"

void Logit(string);
void CovertString(char arr[], string line);
int RunTests();

int main()
{
	RunTests();
}

void Logit(string line)
{
	Logging::WriteToLogFile(line);
	Logging::WriteToMemDumpLog(line);
	cout << line << endl;
}

void CovertString(char arr[], string line)
{
	for (int i = 0; i < line.length(); i++)
	{
		arr[i] = line[i];

		if (i == (line.length() - 1))
		{
			arr[i + 1] = NULL;
		}
	}
}

int RunTests()
{
	UMDLibFS lib;
	char TempString[256];
	char TempBuffer[256];
	int result = 0;
	int size = 50;
	string path;

	result = lib.FSBoot();
	sprintf_s(TempString, "FS boot with result: %d", result);
	Logit(TempString);
	lib.DumpLocalDisk();
	lib.DumpRemoteDisk();

	path = "/Test/";
	string buffer = "";
	result = lib.DirCreate(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", TempBuffer, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/SubTest/";
	result = lib.DirCreate(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", TempBuffer, result);
	Logit(TempString);
	Logit(lib.osErrMsg);
	lib.DumpLocalDisk();

	path = "/Test/SubTest2/";
	result = lib.DirCreate(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Create Dir at: %s , Result = %d", TempBuffer, result);
	Logit(TempString);
	Logit(lib.osErrMsg);
	lib.DumpLocalDisk();

	path = "/Test/SubTest2/";
	result = lib.DirUnlink(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Dir Unlink at : %s, Result = %d", TempBuffer, result);
	Logit(TempString);

	path = "/Test/";
	result = lib.DirSize(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Dir Size at : %s, Result = %d", TempBuffer, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/dumb.txt";
	result = lib.FileCreate(path);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "File Create at : %s, Result = %d", TempBuffer, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	result = lib.FileOpen(path);
	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Tried to open file at %s, with result: %d", TempBuffer, result);
	Logit(TempString);
	lib.DumpLocalDisk();


	lib.OpenFileTable[0] = path;
	lib.CurrentFilePointerTable[0] = 0;
	buffer = "This is some test text";
	result = lib.FileWrite(0, buffer, buffer.length());

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Wrote to File at %s, with result %d",TempBuffer, result);
	Logit(TempString);
	CovertString(TempBuffer, buffer);
	sprintf_s(TempString, "Wrote: %s", TempBuffer);
	Logit(TempString);
	lib.DumpLocalDisk();

	lib.CurrentFilePointerTable[0] = 0;
	buffer = "";
	result = lib.FileRead(0, buffer, 22);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Read file at %s, with result %d", TempBuffer, result);
	Logit(TempString);
	CovertString(TempBuffer, buffer);
	sprintf_s(TempString, "Wrote: %s", TempBuffer);
	Logit(TempString);
	result = lib.FileClose(0);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Tried to close file with FD: %d, with result: %d", 0, result);
	Logit(TempString);
	lib.DumpLocalDisk();

	path = "/Test/";
	result = lib.DirRead(path, buffer, size);

	CovertString(TempBuffer, path);
	sprintf_s(TempString, "Read Dir at %s, with result %d", TempBuffer, result);
	Logit(TempString);
	CovertString(TempBuffer, buffer);
	sprintf_s(TempString, "Read %s From Dir", TempBuffer);
	Logit(TempString);
	lib.DumpLocalDisk();

	result = lib.FSSync();
	sprintf_s(TempString, "FS sync with result %d", result);
	Logit(TempString);
	lib.DumpLocalDisk();
	lib.DumpRemoteDisk();

	return result;
}