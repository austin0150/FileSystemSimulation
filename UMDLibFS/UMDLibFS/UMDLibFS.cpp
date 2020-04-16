#include "UMDLibFS.h"
#include <list>

//[0][] SuperBlock (put the magic number in index 0)
	//[1][] reserved for Inode Bitmap (Use InodeMap)
	//[2][] reserved for datablock bitmap (Use DataBlockMap)
	//[3 - 8][] reserved for Inodes ([3][0-29]) is root Inode
static int WorkingDisk[1000][512];
static int ExternalDisk[1000][512];

//For Inodes
// [0] = size
// [1] = type (0 = directory, 1 = file)
// [2 - 11] = data block pointers
// if(directory)
//		2 = datablock where child nodes are kept
//		[2][0 - 512] = each int holds inode number of file in the directory
// [12-27] = file name
// [28] = Inode Number
// [29] = Parent directory
// -- Inode takes 30 ints, max of 100 inodes, so inodes need 300 ints 3 blocks

void UMDLibFS()
{

}

void UMDLibFS::INIT()
{
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			WorkingDisk[i][j] = 0;
		}
	}

	for (int i = 0; i < 910; i++)
	{
		DataBlockMap[i] = false;
	}
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			InodeMap[i][j] = false;
		}
	}

	//Add root dir
	NumInodes++;
	InodeMap[0][0] = true;
	WorkingDisk[3][0] = 0;
	WorkingDisk[3][1] = 0;
	WorkingDisk[3][2] = 10;
	DataBlockMap[10] = true;
	WorkingDisk[3][12] = '/';
	WorkingDisk[3][28] = 0;
	WorkingDisk[3][29] = 0;
}

int UMDLibFS::FSBoot() //complete
{
	//Check if the external disk exists
	if (ExternalDisk[0][0] != SUPERBLOCK_NUMBER)
	{
		DiskInit();
	}

	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 512; j++)

		{
			WorkingDisk[i][j] = ExternalDisk[i][j];
		}
	}

	for (int i = 0; i < 990; i++)
	{
		DataBlockMap[i] = ExternalDataBlockMap[i];
	}

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			InodeMap[i][j] = ExternalInodeMap[i][j];
		}
	}

	if (WorkingDisk[0][0] != SUPERBLOCK_NUMBER)
	{
		osErrMsg = "E_FILE_BOOT";
		return -1;
	}

	return 0;
}

int UMDLibFS::FSSync() //complete
{
	if (FileSystemUnavailible) // if file system is unavailable
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 512; j++)

		{
			ExternalDisk[i][j] = WorkingDisk[i][j];
		}
	}

	return 0;
}

int UMDLibFS::FSReset() //complete
{
	if (FileSystemUnavailible) // if file system is unavailable
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int result;
	result = FSSync();

	if (result == -1)
	{
		osErrMsg = "E_FILE_RESET";
		return -1;
	}

	FileSystemUnavailible = true ; //setting file system unavailable
}

int UMDLibFS::FileCreate(string file)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int temp1, temp2;
	if (GetNodeLocation(file, temp1,temp2) != -1)
	{
		osErrMsg = "E_FILE_CREATE";
		return -1;
	}

	int nodeNum = NavigateToDir(file);
	if (nodeNum == -1)
	{
		return -1;
	}

	int offset;
	int nodeSector;
	int nodeOffset;

	string splitPath[256];
	int lastNode = SplitFilePath(splitPath, file) - 1;
	string nodeName = splitPath[lastNode];
	int nameLength = nodeName.length();

	//Get the offset of the parent directory
	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (WorkingDisk[i][offset + 28] == nodeNum)
			{
				nodeSector = i;
				nodeOffset = offset;

				//break the loop
				i = 10;
				j = 20;
			}
		}
	}

	//Get the location for the new Inode
	int newNodeSector;
	int newNodeOffset;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			if (InodeMap[i][j] == false)
			{
				InodeMap[i][j] = true;
				newNodeSector = i + 3;
				newNodeOffset = j * 30;

				//End the loop
				i = 6;
				j = 47;
			}
		}
	}

	//Add new Inode to parent directory
	int nodeWPointers = WorkingDisk[nodeSector][nodeOffset + 2];
	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[nodeWPointers][i] == 0)
		{
			WorkingDisk[nodeWPointers][i] = NumInodes;
			NumInodes++;
			break;
		}
	}
	WorkingDisk[nodeSector][nodeOffset] ++; //Update directory size

	//Creat the new Inode on the disk
	WorkingDisk[newNodeSector][newNodeOffset] = 0; //node size
	WorkingDisk[newNodeSector][newNodeOffset + 1] = 1; // node type
	WorkingDisk[newNodeSector][newNodeOffset + 2] = AllocDataBlock();
	for (int i = 0; i < nameLength; i++) //node name
	{
		WorkingDisk[newNodeSector][newNodeOffset + i + 12] = nodeName[i];
	}
	WorkingDisk[newNodeSector][newNodeOffset + 28] = NumInodes - 1; //NodeNumber
	WorkingDisk[newNodeSector][newNodeOffset + 29] = WorkingDisk[nodeSector][nodeOffset + 28]; //node parent

	return 0;

}

int UMDLibFS::FileOpen(string file)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int intOne, intTwo;
	if (GetNodeLocation(file, intOne,intTwo) == -1) //checks if file exists as path

	{
		osErrMsg = "E_NO_SUCH_FILE";
		return -1;
	}

	int numOpenFiles = 0;
	for (int i = 0; i < 10; i++)
	{
		if (OpenFileTable[i] != "")
		{
			numOpenFiles++;
		}
	}

	if(numOpenFiles == 10)
	{
		osErrMsg = "E_TOO_MANY_OPEN_FILES";
		return -1;
	}

	int fd = 0;

	//Add file to open file table
	for (int i = 0; i < 10; i++)
	{
		if (OpenFileTable[i] == "")
		{
			OpenFileTable[i] = file;
			CurrentFilePointerTable[i] = 0;
			fd = i;
			break;
		}
	}

	return fd;
}

int UMDLibFS::FileRead(int fd, string& buffer, int size)
{

	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	if (OpenFileTable[fd] == "")
	{
		osErrMsg = "E_READ_BAD_FD";
		return -1;
	}

	int nodeSector;
	int nodeOffset;

	if (GetNodeLocation(OpenFileTable[fd], nodeSector, nodeOffset) == -1)
	{
		osErrMsg = "E_READ_FILE";
		return -1;
	}

	int numBytesRead = 0;
	int filePointer = CurrentFilePointerTable[fd];
	int currentDatablock = filePointer / 512;
	int currentOffset = filePointer - (currentDatablock * 512);
	int currentSector = WorkingDisk[nodeSector][nodeOffset + 2 + currentDatablock];

	try
	{
		//iterate through file
		for (int i = 0; i < size; i++)
		{
			if (filePointer == WorkingDisk[nodeSector][nodeOffset])
			{
				return numBytesRead;
			}

			if (currentOffset == 511)
			{
				if (filePointer == 5110)
				{
					return numBytesRead;
				}

				int nextDataBlockNum = (filePointer / 512) + 1;

				currentSector = WorkingDisk[nodeSector][nodeOffset + 2 + nextDataBlockNum];
				currentOffset = 0;

				if (WorkingDisk[nodeSector][nodeOffset + 2 + nextDataBlockNum] == 0)
				{
					return numBytesRead;
				}

			}

			buffer.append(1, (char)WorkingDisk[currentSector][currentOffset]);

			currentOffset++;
			filePointer++;
			numBytesRead++;
			CurrentFilePointerTable[fd] = filePointer;
		}
	}
	catch (exception e)
	{
		osErrMsg = "E_READ_FILE";
		return -1;
	}

	return numBytesRead;
}

int UMDLibFS::FileWrite(int fd, string buffer, int size)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	if (OpenFileTable[fd] == "")
	{
		osErrMsg = "E_WRITE_BAD_FD";
		return -1;
	}

	int nodeSector;
	int nodeOffset;

	if (GetNodeLocation(OpenFileTable[fd], nodeSector, nodeOffset) == -1)
	{
		osErrMsg = "E_WRITE_FILE";
		return -1;
	}

	int numBytesWritten = 0;
	int filePointer = CurrentFilePointerTable[fd];
	int currentDatablock = filePointer / 512;
	int currentOffset = filePointer - (currentDatablock * 512);
	int currentSector = WorkingDisk[nodeSector][nodeOffset + 2 + currentDatablock];

	try
	{
		//iterate through file
		for (int i = 0; i < size; i++)
		{

			if (currentOffset == 511)
			{
				if (filePointer == 5110)
				{
					return numBytesWritten;
				}

				int nextDataBlockNum = (filePointer / 512) + 1;

				currentSector = WorkingDisk[nodeSector][nodeOffset + 2 + nextDataBlockNum];
				currentOffset = 0;

				if (WorkingDisk[nodeSector][nodeOffset + 2 + nextDataBlockNum] == 0)
				{
					WorkingDisk[nodeSector][nodeOffset + 2 + nextDataBlockNum] = AllocDataBlock();
				}

			}

			WorkingDisk[currentSector][currentOffset] = buffer[i];

			currentOffset++;
			filePointer++;
			numBytesWritten++;
			CurrentFilePointerTable[fd] = filePointer;
			WorkingDisk[nodeSector][nodeOffset]++;
		}
	}
	catch (exception e)
	{
		osErrMsg = "E_WRITE_FILE";
		return -1;
	}

	return numBytesWritten;
}

int UMDLibFS::FileSeek(int fd, int offset)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	if (OpenFileTable[fd] == "")
	{
		osErrMsg = "E_SEEK_BAD_FD";
		return -1;
	}

	string filePath = OpenFileTable[fd];

	int nodeSector;
	int nodeOffset;

	GetNodeLocation(filePath, nodeSector, nodeOffset);

	int fileSize = WorkingDisk[nodeOffset][nodeOffset];

	if (offset > fileSize || offset < 0)
	{
		osErrMsg = "E_SEEK_OUT_OF_BOUNDS";
		return -1;
	}

	CurrentFilePointerTable[fd] += offset;
	return CurrentFilePointerTable[fd];

}

int UMDLibFS::FileClose(int fd) 
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	if (OpenFileTable[fd] == "") //if fd does not exist in open file table
	{
		osErrMsg = "E_CLOSE_ BAD_FD";
		return -1;
	}

	//remove fd from the OpenFileTable
	CurrentFilePointerTable[fd] = 0;
	OpenFileTable[fd] = "";

	return 0;
}

int UMDLibFS::FileUnlink(string file)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int temp1, temp2;
	if (GetNodeLocation(file, temp1, temp2) == -1)
	{
		osErrMsg = "E_NO_SUCH_FILE";
		return -1;
	}

	string splitPath[256];
	int lastNode = SplitFilePath(splitPath, file) - 1;
	string nodeName = splitPath[lastNode];

	for (int i = 0; i < 10; i++)
	{
		if (OpenFileTable[i] == file)
		{
			osErrMsg = "E_FILE_IN_USE";
			return -1;
		}
	}

	int nodeSector;
	int nodeOffet;
	int result = GetNodeLocation(file, nodeSector, nodeOffet);
	if (result == -1)
	{
		return -1;
	}

	//Update Parent dir
	int parentNodeNum = WorkingDisk[nodeSector][nodeOffet + 29];
	int parentSector = 0;
	int parentOffset = 0;
	int offset;
	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (WorkingDisk[i][offset + 28] == parentNodeNum)
			{
				parentSector = i;
				parentOffset = offset;
			}
		}
	}

	if (parentSector == 0)
	{
		osErrMsg = "E_FILE_UNLINK";
		return -1;
	}

	//Reduce DirSize
	WorkingDisk[parentSector][parentOffset] = (WorkingDisk[parentSector][parentOffset] - 1);

	int blockWParentPointers = WorkingDisk[parentSector][parentOffset + 2];

	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[blockWParentPointers][i] == WorkingDisk[nodeSector][nodeOffet + 28])
		{
			WorkingDisk[blockWParentPointers][i] = 0;
			break;
		}
	}

	//Clear up datablocks
	for (int i = 0; i < 10; i++)
	{
		int dataBlock = WorkingDisk[nodeSector][nodeOffet + 2 + i];
		if (dataBlock != 0)
		{
			DataBlockMap[dataBlock] = false;
			for (int j = 0; j < 512; j++)
			{
				WorkingDisk[dataBlock][j] = 0;
			}
		}
	}

	for (int i = 0; i < 30; i++)
	{
		WorkingDisk[nodeSector][nodeOffet + i] = 0;
		InodeMap[nodeSector - 3][nodeOffet / 30] = false;
	}

	return 0;
}

int UMDLibFS::DirCreate(string path)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int nodeNum = NavigateToDir(path);
	if (nodeNum == -1)
	{
		return -1;
	}

	int offset;
	int nodeSector;
	int nodeOffset;

	string splitPath[256];
	int lastNode = SplitFilePath(splitPath, path) - 1;
	string nodeName = splitPath[lastNode];
	int nameLength = nodeName.length();

	//Get the offset of the parent directory
	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (WorkingDisk[i][offset + 28] == nodeNum)
			{
				nodeSector = i;
				nodeOffset = offset;

				//break the loop
				i = 10;
				j = 20;
			}
		}
	}

	//Get the location for the new Inode
	int newNodeSector;
	int newNodeOffset;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			if (InodeMap[i][j] == false)
			{
				InodeMap[i][j] = true;
				newNodeSector = i + 3;
				newNodeOffset = j * 30;

				//End the loop
				i = 6;
				j = 47;
			}
		}
	}

	//Add new Inode to parent directory
	int nodeWPointers = WorkingDisk[nodeSector][nodeOffset + 2];
	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[nodeWPointers][i] == 0)
		{
			WorkingDisk[nodeWPointers][i] = NumInodes;
			NumInodes++;
			break;
		}
	}
	WorkingDisk[nodeSector][nodeOffset] ++; //Update directory size

	//determine block to hold pointers to directory children
	int blockPointerSector = 0;
	for (int i = 10; i < 990; i++)
	{
		if (DataBlockMap[i] == false)
		{
			DataBlockMap[i] = true;
			blockPointerSector = i;
			break;
		}
	}

	//Creat the new Inode on the disk
	WorkingDisk[newNodeSector][newNodeOffset] = 0; //node size
	WorkingDisk[newNodeSector][newNodeOffset + 1] = 0; // node type
	WorkingDisk[newNodeSector][newNodeOffset + 2] = blockPointerSector; //sector with pointer to child nodes
	for (int i = 0; i < nameLength; i++) //node name
	{
		WorkingDisk[newNodeSector][newNodeOffset + i + 12] = nodeName[i];
	}
	WorkingDisk[newNodeSector][newNodeOffset + 28] = NumInodes - 1; //NodeNumber
	WorkingDisk[newNodeSector][newNodeOffset + 29] = WorkingDisk[nodeSector][nodeOffset + 28]; //node parent

	return 0;
}

int UMDLibFS::DirSize(string path)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}
	int result = NavigateToDir(path);
	int offset;

	string splitPath[256];
	int lastNode = SplitFilePath(splitPath, path) - 1;
	string nodeName = splitPath[lastNode];

	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (GetInodeName(WorkingDisk[i][offset + 28]) == nodeName)
			{
				return WorkingDisk[i][offset];
			}
		}
	}

	return 0;
}

int UMDLibFS::DirRead(string path, string &buffer, int size)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}
	int LastInodeParent = NavigateToDir(path);

	string splitPath[256];
	int lastNode = SplitFilePath(splitPath, path) - 1;
	string nodeName = splitPath[lastNode];

	int dirSector;
	int dirOffset;
	int offset;
	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (GetInodeName(WorkingDisk[i][offset + 28]) == nodeName)
			{
				dirSector = i;
				dirOffset = offset;
			}
		}
	}

	int SectorWBlockPointers = WorkingDisk[dirSector][dirOffset + 2];

	int numChildren = 0;
	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[SectorWBlockPointers][i] != 0)
		{
			if (((numChildren+1) * 17) < size)
			{
				numChildren++;
				string nodeName = GetInodeName(WorkingDisk[SectorWBlockPointers][i]);
				for (int j = 0; j < 16; j++)
				{
					if (j < nodeName.length())
					{
						buffer.append(1, nodeName[j]);
					}

				}
				buffer.append(1,(char)WorkingDisk[SectorWBlockPointers][i]);
			}
			else
			{
				osErrMsg = "E_BUFFER_TOO_SMALL";
				return -1;
			}


		}
		else
		{
			break;
		}
	}

	return 0;
}

int UMDLibFS::DirUnlink(string path)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}

	int nodeSector;
	int nodeOffset;

	int result = GetNodeLocation(path, nodeSector,nodeOffset);

	int blockWData = WorkingDisk[nodeSector][nodeOffset + 2];
	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[blockWData][i] != 0)
		{
			osErrMsg = "E_DIR_NOT_EMPTY";
			return -1;
		}
	}

	//Update Parent dir
	int parentNodeNum = WorkingDisk[nodeSector][nodeOffset + 29];
	int parentSector = 0;
	int parentOffset = 0;
	int offset;
	for (int i = 3; i < 9; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			offset = j * 30;
			if (WorkingDisk[i][offset + 28] == parentNodeNum)
			{
				parentSector = i;
				parentOffset = offset;
			}
		}
	}

	if (parentSector == 0)
	{
		osErrMsg = "E_DIR_UNLINK";
		return -1;
	}

	//Reduce DirSize
	WorkingDisk[parentSector][parentOffset] = (WorkingDisk[parentSector][parentOffset] - 1);

	int blockWParentPointers = WorkingDisk[parentSector][parentOffset + 2];

	for (int i = 0; i < 512; i++)
	{
		if (WorkingDisk[blockWParentPointers][i] == WorkingDisk[nodeSector][nodeOffset + 28])
		{
			WorkingDisk[blockWParentPointers][i] = 0;
			break;
		}
	}

	//Clear up datablocks

	int nodeWData = WorkingDisk[nodeSector][nodeOffset + 2];
	DataBlockMap[nodeWData] = false;

	for (int i = 0; i < 512; i++)
	{
		WorkingDisk[blockWData][i] = 0;
	}

	for (int i = 0; i < 30; i++)
	{
		WorkingDisk[nodeSector][nodeOffset + i] = 0;
		InodeMap[nodeSector - 3][nodeOffset / 30] = false;
	}

	return 0;
}

int UMDLibFS::DiskInit() //complete
{
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			ExternalDisk[i][j] = 0;
		}
	}

	ExternalDisk[0][0] = SUPERBLOCK_NUMBER;

	//Init the datablock map
	for (int i = 0; i < 990; i++)
	{
		ExternalDataBlockMap[i] = false;
	}

	//init inode map
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 17; j++)
		{
			ExternalInodeMap[i][j] = false;
		}
	}

	//Add root dir
	NumInodes++;
	ExternalInodeMap[0][0] = true;
    ExternalDisk[3][0] = 0;
	ExternalDisk[3][1] = 0;
	ExternalDisk[3][2] = 10;
	ExternalDataBlockMap[10] = true;
	ExternalDisk[3][12] = '/';
	ExternalDisk[3][28] = 0;
	ExternalDisk[3][29] = 0;

	return 0;
}

int UMDLibFS::DiskLoad() //complete
{
	for (int i = 0; i <1000; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			WorkingDisk[i][j] = ExternalDisk[i][j];
		}
	}

	for (int i = 0; i < 990; i++)
	{
		DataBlockMap[i] = ExternalDataBlockMap[i];
	}

	for (int i = 0; i < 6; i++)
	{
		for (int k = 0; k < 17; k++)
		{
			InodeMap[i][k] = ExternalInodeMap[i][k];
		}
	}

	NumInodes = ExternalNumInodes;

	return 0;
}

int UMDLibFS::DiskSave()
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}
	for (int i = 0; i <1000; i++)
	{
		for (int j = 0; j < 512; j++)
		{
			ExternalDisk[i][j] = WorkingDisk[i][j];
		}
	}

	for (int i = 0; i < 990; i++)
	{
		ExternalDataBlockMap[i] = DataBlockMap[i];
	}

	for (int i = 0; i < 6; i++)
	{
		for (int k = 0; k < 17; k++)
		{
			ExternalInodeMap[i][k] = InodeMap[i][k];
		}
	}

	ExternalNumInodes = NumInodes;
	
	return 0;
}

int UMDLibFS::DiskWrite(int sector, string buffer)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}
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

	for (int i = 0; i < 512; i++)
	{
		WorkingDisk[sector][i] = buffer[i];
	}

	return 0;
}

int UMDLibFS::DiskRead(int sector, string buffer)
{
	if (FileSystemUnavailible)
	{
		osErrMsg = "E_INVALID_ACCESS_ATTEMPT";
		return -1;
	}
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

	for (int i = 0; i < 512; i++)
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
				inName = false;
				i--;
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
			pathCounter++;
		}
	}

	return pathCounter;
}

int UMDLibFS::NavigateToDir(string path)
{
	string pathSplit[256];
	int splitResult = (SplitFilePath(pathSplit, path) - 1);
	int numNodes = splitResult;

	if (NumInodes < numNodes)
	{
		osErrMsg = "E_DIR_CREATE";
		return -1;
	}

	int nextInodeToSearch = 0;
	int offset = 0;
	int nodesFound = 0;
	string nodeName;
	bool foundNextNode = false;
	int LastFoundAddr = 0;

	//iterate through the items of the path
	for (int i = 0; i < numNodes; i++)
	{
		//iterate through the inodes on the disk
		for (int k = 3; k < 9; k++)
		{
			//iterate through all Inodes in that sector
			for (int j = 0; j < 17; j++)
			{
				offset = (j * 30);

				//If the Inode is the one we are looking for
				if (WorkingDisk[k][offset + 28] == nextInodeToSearch)
				{
					nodesFound++;
					LastFoundAddr = nextInodeToSearch;

					int dataBlockWPointers = WorkingDisk[k][offset + 2];
					//Iterate through the data block that holds file pointers for that Inode
					for (int h = 0; h < 512; h++)
					{
						if (WorkingDisk[dataBlockWPointers][h] != 0)
						{
							//If one of the Inode names matches the next node name
							if (GetInodeName(WorkingDisk[dataBlockWPointers][h]) == pathSplit[i])
							{
								nextInodeToSearch = WorkingDisk[dataBlockWPointers][h];
								foundNextNode = true;
							}
						}
						else
						{
							break;
						}

					}

				}

			}
		}

	}

	if ((nodesFound) < numNodes)
	{
		osErrMsg = "E_DIR_CREATE";
		return -1;
	}

	return LastFoundAddr;
}

string UMDLibFS::GetInodeName(int nodeNumber)
{
	int offset;
	int nodeSector, nodeOffset;

	//iterate through the inodes on the disk
	for (int k = 3; k < 9; k++)
	{
		//iterate through all Inodes in that sector
		for (int j = 0; j < 17; j++)
		{
			offset = (j * 30);

			//If the Inode is the one we are looking for
			if (WorkingDisk[k][offset + 28] == nodeNumber)
			{
				nodeSector = k;
				nodeOffset = offset;
			}
		}
	}

	string nodeName = "";
	for (int o = 0; o < 16; o++)
	{
		if (WorkingDisk[nodeSector][nodeOffset + 12 + o] != 0)
		{
			nodeName.append(1, (char)WorkingDisk[nodeSector][nodeOffset + 12 + o]);
		}
	}

	return nodeName;
}

int UMDLibFS::GetNodeLocation(string path, int& nodeSector, int& nodeOffset)
{
	string splitPath[256];
	int numNodes = SplitFilePath(splitPath, path) ;

	int nextInodeToSearch = 0;
	int offset = 0;
	int nodesFound = 0;
	string nodeName = splitPath[numNodes - 1];
	bool foundNextNode = false;
	int LastFoundAddr = 0;

	int foundNodeSector;
	int foundNodeOffset;
	int pathNodes[256];

	int foundNodeNums = 0;

	//Get all node numbers
	for (int i = 0; i < numNodes; i++)
	{
		//Find the node number
		for (int j = 3; j < 9; j++)
		{
			int offset;
			for (int k = 0; k < 17; k++)
			{
				offset = k * 30;
				if (GetInodeName(WorkingDisk[j][offset + 28]) == splitPath[i])
				{
					pathNodes[i] = WorkingDisk[j][offset + 28];
					foundNodeNums++;
				}
			}
		}
	}

	if (foundNodeNums < numNodes)
	{
		osErrMsg = "E_FILE_PATH";
		return -1;
	}


	bool childNodeExists = false;
	//iterate through the path
	for (int i = 0; i < numNodes; i++)
	{
		for (int j = 3; j < 9; j++)
		{
			int offset;
			for (int k = 0; k < 17; k++)
			{
				offset = k * 30;
				if (WorkingDisk[j][offset + 28] == pathNodes[i])
				{
					//if the node we are looking at is the last one in the path
					if (i == (numNodes - 1))
					{
						nodeOffset = offset;
						nodeSector = j;
						return 0;
					}

					int SectorWData = WorkingDisk[j][offset + 2];
					for (int l = 0; l < 512; l++)
					{
						if (WorkingDisk[SectorWData][l] == pathNodes[i + 1])
						{
							childNodeExists = true;
						}
					}

				}
			}
		}

		if (!childNodeExists)
		{
			osErrMsg = "E_FILE_PATH";
			return -1;
		}
	}

	return 1;
}

int UMDLibFS::AllocDataBlock()
{
	for(int i = 10; i < 990; i++)
	{
		if (DataBlockMap[i] == false)
		{
			DataBlockMap[i] = true;
			return i;
		}
	}
}

int UMDLibFS::DumpLocalDisk()
{
	Logging::WriteToMemDumpLog("Local Disk Dump -----------------------");

	Logging::WriteToMemDumpLog(("SuperBlock: " + WorkingDisk[0][0]));

	//Write the datablock map
	Logging::WriteToMemDumpLog("DataBlock Map");
	for (int i = 0; i < 990; i += 200)
	{
		string buffer = "";
		for (int j = 0; j < 200; j++)
		{
			if (DataBlockMap[i + j])
			{
				buffer.append(1, '1');
			}
			else
			{
				buffer.append(1, '0');
			}
			buffer.append(1, ' ');
		}

		Logging::WriteToMemDumpLog(buffer);
	}

	Logging::WriteToMemDumpLog("Data");

	//Write Inodes
	for (int i = 3; i < 6; i++)
	{
		string buffer = "";
		for (int j = 0; j < 512; j++)
		{
			char temp[256];
			sprintf_s(temp, "%d", WorkingDisk[i][j]);

			buffer.append(1, temp[0]);
			buffer.append(1, ' ');
		}
		Logging::WriteToMemDumpLog(buffer);
	}

	//Write rest of disk
	for (int i = 9; i < 1000; i++)
	{
		string buffer = "";
		if (DataBlockMap[i] == true)
		{
			for (int j = 0; j < 512; j++)
			{
				char temp [256];
				sprintf_s(temp, "%d", WorkingDisk[i][j]);

				buffer.append(1, temp[0]);
				buffer.append(1, ' ');
			}
		}
		Logging::WriteToMemDumpLog(buffer);

	}

	Logging::WriteToMemDumpLog("End Disk Dump -------------------------");
	return 0;
}

int UMDLibFS::DumpRemoteDisk()
{
	Logging::WriteToMemDumpLog("External Disk Dump -----------------------");

	Logging::WriteToMemDumpLog(("SuperBlock: " + ExternalDisk[0][0]));

	//Write the datablock map
	Logging::WriteToMemDumpLog("DataBlock Map");
	for (int i = 0; i < 990; i += 200)
	{
		string buffer = "";
		for (int j = 0; j < 200; j++)
		{
			if (ExternalDataBlockMap[i + j])
			{
				buffer.append(1, '1');
			}
			else
			{
				buffer.append(1, '0');
			}
			buffer.append(1, ' ');
		}

		Logging::WriteToMemDumpLog(buffer);
	}

	Logging::WriteToMemDumpLog("Data");

	//Write Inodes
	for (int i = 3; i < 6; i++)
	{
		string buffer = "";
		for (int j = 0; j < 512; j++)
		{
			char temp[256];
			sprintf_s(temp, "%d", ExternalDisk[i][j]);

			buffer.append(1, temp[0]);
			buffer.append(1, ' ');
		}
		Logging::WriteToMemDumpLog(buffer);
	}

	//Write rest of disk
	for (int i = 9; i < 1000; i++)
	{
		string buffer = "";
		if (ExternalDataBlockMap[i] == true)
		{
			for (int j = 0; j < 512; j++)
			{
				char temp[256];
				sprintf_s(temp, "%d", ExternalDisk[i][j]);

				buffer.append(1, temp[0]);
				buffer.append(1, ' ');
			}
		}
		Logging::WriteToMemDumpLog(buffer);

	}

	Logging::WriteToMemDumpLog("End Disk Dump -------------------------");
	return 0;
}
