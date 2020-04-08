#pragma once
#include<list>
#include <string>

using namespace std;

class Inode
{
	int size;
	bool fileType; // true if file, false is directory
	int DataBlockLocations[10];
	char Name[16];
	Inode* Self;
	Inode* Parent;
	list<Inode*> nodes;
};

