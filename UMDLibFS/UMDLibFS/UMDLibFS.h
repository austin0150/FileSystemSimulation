#pragma once
#include <string>
#include <cstddef>

using namespace std;

class UMDLibFS
{
public:
	bool FileSystemUnavailible;
	int NUM_SECTORS = 1000;
	int SECTOR_SIZE = 128;
	int MAX_FILES = 100;
	int SUPERBLOCK_NUMBER = 15;
	string osErrMsg;

	//[0 - 128] SuperBlock (put the magic number in index 0)
	//[129 - 256] reserved for Inode Bitmap (Use InodeMap)
	//[257 - 384] reserved for datablock bitmap (Use DataBlockMap)
	//[385 - 768] reserved for Inodes (385 - 415) is root Inode
	int WorkingDisk[128000];
	int ExternalDisk[128000];

	//For Inodes
	// [0] = size
	// [1] = type
	// [2 - 11] = data block pointers
	// if(directory)
	//		2 = datablock where child nodes are kept
	//		[2][0 - 999] = each int holds inode number of file in the directory
	// [12-27] = file name
	// [28] = Inode Number
	// [29] = Parent directory
	// -- Inode takes 30 ints, max of 100 inodes, so inodes need 300 ints 3 blocks

	bool InodeMap[4000];
	bool DataBlockMap[4096];

	string OpenFileTable[10];
	
	int FSBoot();
	int FSSync();
	int FSReset();

	int DiskInit();
	int DiskLoad();
	int DiskSave();
	int DiskWrite(int sector, string buffer);
	int DiskRead(int sector, string buffer);

	int FileCreate(string file);
	int FileOpen(string file);
	int FileRead(int fd, string buffer, int size);
	int FileWrite(int fd, string buffer, int size);
	int FileSeek(int fd, int offset);
	int FileClose(int fd);
	int FileUnlink(string file);

	int DirCreate(string path);
	int DirSize(string path);
	int DirRead(string path, string buffer, int size);
	int DirUnlink(string path);

	int SplitFilePath(string splitPath[], string path);
};

