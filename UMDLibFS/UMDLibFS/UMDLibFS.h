#pragma once
#include <string>
#include <cstddef>

using namespace std;

class UMDLibFS
{
public:
	bool FileSystemUnavailible;
	int NUM_SECTORS = 512;
	int SECTOR_SIZE = 1000;
	int MAX_FILES = 100;
	int SUPERBLOCK_NUMBER = 15;
	string osErrMsg;

	//[0 - 999] SuperBlock (put the magic number in index 0)
	//[1000 - 1999] reserved for Inode Bitmap (Use InodeMap)
	//[2000 - 2999] reserved for datablock bitmap (Use DataBlockMap)
	int WorkingDisk[512000];
	int ExternalDisk[512000];

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
	int DiskRead(int sector, string &buffer);

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
};

