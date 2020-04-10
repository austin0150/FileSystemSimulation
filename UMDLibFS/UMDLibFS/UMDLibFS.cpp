#include "UMDLibFS.h"
#include <list>

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
	string pathSplit[256];
	//int splitResult = (SplitFilePath(pathSplit, path) -1);

	//for (int i = 0; i < splitResult; i++)
	{

	}

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
	for (int i = 0; i <1000; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			ExternalDisk[i][j] = WorkingDisk[i][j];
		}
		
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
	if (buffer == "")
	{
		osErrMsg = "E_WRITE_INVALID_PARAM";
		return -1;
	}

	for (int i = 0; i < 128; i++)
	{
		WorkingDisk[sector][i] = buffer[i];
	}

	return 0;
}

int UMDLibFS::DiskRead(int sector, string buffer)
{
	if (sector < 0 || (sector >= NUM_SECTORS))
	{
		osErrMsg = "E_READ_INVALID_PARAM";
		return -1;
	}
	if (buffer == "")
	{
		osErrMsg = "E_READ_INVALID_PARAM";
		return -1;
	}

	for (int i = 0; i < 128; i++)
	{
		buffer[i] = WorkingDisk[sector][i];
	}

	return 0;
}

int UMDLibFS::SplitFilePath(string splitPath[], string path)
{
	int startName = 0;
	//int endName = 0;
	int pathCounter = 0;
	bool inName = false;
	bool endName = false;
	int length = path.length();
	for (int i = 0; i < length; i++)
	{
		if (path[i] == '/')
		{
			if (inName)
			{
				splitPath[pathCounter] = path.substr(startName, (i - startName));
				pathCounter++;
				startName = i + 1;
			}
			else
			{
				startName = i + 1;
				inName = true;
			}
		}
		else if (path[i] == '.')
		{
			endName = true;
		}
		
		if (i == length - 1 && endName)
		{
			splitPath[pathCounter] = path.substr(startName, (i - (startName - 1)));
		}
	}

	return pathCounter;
}
