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
	return 0;
}

int UMDLibFS::FileRead(int fd, string buffer, int size)
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
			if (filePointer == WorkingDisk[nodeSector][nodeOffset])
			{
				return numBytesWritten;
			}

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
	return 0;
}

int UMDLibFS::FileClose(int fd)
{
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
		if (OpenFileTable[i] == nodeName)
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
	for (int i = 3; i < 6; i++)
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

	for (int i = 0; i < 6; i++)
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
	for (int i = 0; i < 6; i++)
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
	int numNodes = SplitFilePath(splitPath, path);

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

					//If it's the file
					if (i == (numNodes - 1))
					{
						nodeSector = k;
						nodeOffset = (offset + 28);
						return 1;
					}

					int dataBlockWPointers = WorkingDisk[k][offset + 2];
					//Iterate through the data block that holds file pointers for that Inode
					for (int h = 0; h < 512; h++)
					{
						if (WorkingDisk[dataBlockWPointers][h] != 0)
						{
							//If one of the Inode names matches the next node name
							if (GetInodeName(WorkingDisk[dataBlockWPointers][h]) == splitPath[i])
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