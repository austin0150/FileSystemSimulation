#include <iostream>
#include "UMDLibFS.h"

int main()
{
	UMDLibFS lib;

	lib.INIT();
	//string path = "/CIS450/prog3/UMDLibFS.cpp";
	string path = "/Test/";
	string buffer = "";
	int size = 50;

	//int result = lib.SplitFilePath(arr, path);
	int result = lib.DirCreate(path);

	path = "/Test/SubTest/";
	result = lib.DirCreate(path);

	path = "/Test/SubTest2/";
	result = lib.DirCreate(path);

	path = "/Test/";
	result = lib.DirSize(path);

	path = "/Test/";
	result = lib.DirRead(path, buffer, size);

	return result;
}