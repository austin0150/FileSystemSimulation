#include <iostream>
#include "UMDLibFS.h"

int main()
{
	UMDLibFS lib;
	string path = "/CIS450/prog3/UMDLibFS.cpp";
	string arr[256];

	int result = lib.SplitFilePath(arr, path);
	return result;
}