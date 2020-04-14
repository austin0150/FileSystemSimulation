#pragma once
#include <string>
#include <cstddef>

using namespace std;

class UMDLibFS
{
public:
	bool FileSystemUnavailible;
	int NUM_SECTORS = 1000;
	int SECTOR_SIZE = 512;
	int MAX_FILES = 100;
	int SUPERBLOCK_NUMBER = 15;
	string osErrMsg;
	int NumInodes = 0;

	bool InodeMap[6][17];
	bool DataBlockMap[990];

	string OpenFileTable[10];
	int CurrentFilePointerTable[10];
	
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
	int DirRead(string path, string &buffer, int size);
	int DirUnlink(string path);

	int SplitFilePath(string splitPath[], string path);
	int NavigateToDir(string path);
	string GetInodeName(int nodeNumber);
	int GetNodeLocation(string path, int& nodeSector, int& nodeOffset);
	void INIT();
};

