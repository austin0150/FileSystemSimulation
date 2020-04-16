#include <iostream>
#include "UMDLibFS.h"
#include "Logging.h"

int main()
{
	UMDLibFS lib;

	lib.FileSystemUnavailible = false;

	//lib.INIT();

	Logging::WriteToLogFile("test");


	string path = "/Test/";
	string buffer = "";
	int size = 50;
	int result = lib.DirCreate(path);

	path = "/Test/SubTest/";
	result = lib.DirCreate(path);

	path = "/Test/SubTest2/";
	result = lib.DirCreate(path);

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