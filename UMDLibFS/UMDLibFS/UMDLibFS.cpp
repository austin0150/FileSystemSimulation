#include "UMDLibFS.h"

int WorkingDisk[];
int ExternalDisk[];

void UMDLibFS()
{

}

int UMDLibFS::FSBoot()
{
	return 0;
}

int UMDLibFS::FSSync()
{
	return 0;
}

int UMDLibFS::FSReset()
{
	return 0;
}

int UMDLibFS::FileCreate(string file)
{
	return 0;
}

int UMDLibFS::FileOpen(string file)
{
	return 0;
}

int UMDLibFS::FileRead(int fd, string buffer, int size)
{
	return 0;
}

int UMDLibFS::FileWrite(int fd, string buffer, int size)
{
	return 0;
}

int UMDLibFS::FileSeek(int fd, int offset)
{
	return 0;
}

int UMDLibFS::FileClose(int fd)
{
	return 0;
}

int UMDLibFS::FileUnlink(string file)
{
	return 0;
}

int UMDLibFS::DirCreate(string path)
{
	return 0;
}

int UMDLibFS::DirSize(string path)
{
	return 0;
}

int UMDLibFS::DirRead(string path, string buffer, int size)
{
	return 0;
}

int UMDLibFS::DirUnlink(string path)
{
	return 0;
}

int UMDLibFS::DiskInit()
{
	return 0;
}

int UMDLibFS::DiskLoad()
{
	return 0;
}

int UMDLibFS::DiskSave()
{
	return 0;
}

int UMDLibFS::DiskWrite(int sector, string buffer)
{
	return 0;
}

int UMDLibFS::DiskRead(int sector, string buffer)
{
	return 0;
}
