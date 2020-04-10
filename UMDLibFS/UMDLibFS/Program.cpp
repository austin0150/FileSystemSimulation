#include <iostream>
#include "UMDLibFS.h"

int main()
{
	UMDLibFS lib;

	lib.INIT();
	//string path = "/CIS450/prog3/UMDLibFS.cpp";
	string path = "/Test/";

	//int result = lib.SplitFilePath(arr, path);
	int result = lib.DirCreate(path);

	path = "/Test/SubTest/";
	result = lib.DirCreate(path);

	return result;
}