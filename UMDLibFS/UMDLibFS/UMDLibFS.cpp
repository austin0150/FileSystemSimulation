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
			WorkingDisk[i][j] = -1;
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

int UMDLibFS::FSBoot() //issue on get superblock
{
	if (ExternalDisk == NULL)
	{
		ExternalDisk = DiskImage;
	}
	
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 512; j++)

		{
			WorkingDisk[i][j] = ExternalDisk[i][j];
		}
	}
	
	if (WorkingDisk.GetSuperBlock != CorrectSuperBlock) //need to fix this line
	{
	   	 osErrMsg = "E_FILE_BOOT";
		 return 0;
	}

	else
	{
		return -1;
	}
}

int UMDLibFS::FSSync() //complete
{
	if (FileSystemUnavailable == true) // if file system is unavailable
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
	if (FileSystemUnavailable == true) // if file system is unavailable
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
	
	FileSystemUnavailable = true ; //setting file system unavailable
}

int UMDLibFS::FileCreate(string file)
{
	return 0;
}

int UMDLibFS::FileOpen(string file) //in progress
{
	if (FileSystemUnavailable == true)
	{
		osErrMsg = “E_INVALID_ACCESS_ATTEMPT”;
		return -1;
	}
	
	if(File does not exist at path) //need to fix this line
	{
		set osErrMsg = “E_NO_SUCH_FILE”;
		return -1;
	}
	
	if(num files open  = MAXFILESOPEN) //need to fix this line
	{
		set osErrMsg = “E_TOO_MANY_OPEN_FILES”;
		return -1;
	}

	//Add inode pointer to OpenFileTable and return its position in the table

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

int UMDLibFS::FileClose(int fd) //in progress
{
	if (FileSystemUnavailable == true)
	{
		osErrMsg = "";
		return -1;
	}
	
	if (fd not present in OpenFileTable) //need to fix this line
	{	
		osErrMsg = “E_CLOSE_ BAD_FD”;
		return -1;
	}	
	
	//remove fd from the OpenFileTable
	
	return 0;
}

int UMDLibFS::FileUnlink(string file)
{
	return 0;
}

int UMDLibFS::DirCreate(string path)
{
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
		if (WorkingDisk[nodeWPointers][i] == -1)
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

int UMDLibFS::DiskInit() //complete
{
	for (int i = 0; i < 512; i++)
	{
		WorkingDisk[sector][i] = 0;
	}
	
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
	
	return 0;
}

int UMDLibFS::DiskSave()
{
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
						if (WorkingDisk[dataBlockWPointers][h] != -1)
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
		if (WorkingDisk[nodeSector][nodeOffset + 12 + o] != -1)
		{
			nodeName.append(1, (char)WorkingDisk[nodeSector][nodeOffset + 12 + o]);
		}
	}

	return nodeName;
}
