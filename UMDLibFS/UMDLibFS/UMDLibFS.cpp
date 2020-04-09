#include "UMDLibFS.h"

//int WorkingDisk[];
//int ExternalDisk[];

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
	int length = sizeof(WorkingDisk) / sizeof(*WorkingDisk);
	for (int i = 0; i < length; i++)
	{
		ExternalDisk[i] = WorkingDisk[i];
	}
	return 0;
}

int UMDLibFS::DiskWrite(int sector, string buffer)
{
	if (sector < 0 || (sector >= NUM_SECTORS))
	{
		osErrMsg = "E_WRITE_INVALID_PARAM";
		return -1;
	}
	if (buffer.empty)
	{
		osErrMsg = "E_WRITE_INVALID_PARAM";
		return -1;
	}

	int offsetIndex = sector * 1000;

	int length = sizeof(WorkingDisk) / sizeof(*WorkingDisk);
	for (int i = 0; i < length; i++)
	{
		WorkingDisk[i + offsetIndex] = buffer[i];
	}

	return 0;
}

int UMDLibFS::DiskRead(int sector, string& buffer)
{
	if (sector < 0 || (sector >= NUM_SECTORS))
	{
		osErrMsg = "E_READ_INVALID_PARAM";
		return -1;
	}
	if (buffer.empty)
	{
		osErrMsg = "E_READ_INVALID_PARAM";
		return -1;
	}

	int offsetIndex = sector * 1000;
	int length = sizeof(WorkingDisk) / sizeof(*WorkingDisk);
	for (int i = 0; i < length; i++)
	{
		buffer[i] = WorkingDisk[i + offsetIndex];
	}

	return 0;
}
