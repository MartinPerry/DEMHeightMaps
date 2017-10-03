#include "./VFS.h"

#include <cstdio>
#include <cstdlib>
#include "./minizip/unzip.h"
#include <fstream>
#include <string>
#include <iostream>

#include "VFSUtils.h"

#ifdef _WIN32
	#include "./win_dirent.h"
#else 
	#include <dirent.h>
#endif

//singleton instance of VFS
VFS * VFS::single = NULL;

/*-----------------------------------------------------------
Function:	ctor

VFS Singleton private ctor
-------------------------------------------------------------*/
VFS::VFS()
{
	this->fileSystem = new VFSTree();	
	this->workingDir = "";
}

/*-----------------------------------------------------------
Function:	dtor

VFS Singleton private dtor
-------------------------------------------------------------*/
VFS::~VFS()
{
	this->Release();
}

/*-----------------------------------------------------------
Function:	Release

Release VFS from memory
-------------------------------------------------------------*/
void VFS::Release()
{
	
	std::vector<VFS_FILE *>::iterator jt;
	for (jt = this->debugModeFiles.begin(); jt != this->debugModeFiles.end(); jt++)
	{
		if (*jt != NULL)
		{	
			if ((*jt)->archiveInfo != NULL) 
			{
				free((*jt)->archiveInfo->filePath);
			}
			delete (*jt)->archiveInfo;										
			free((*jt)->ext);
			free((*jt)->filePath);
			free((*jt)->fullName);
			free((*jt)->name);
		}
		delete (*jt);
	}
	this->debugModeFiles.clear();

	delete this->fileSystem;
	this->fileSystem = NULL;
	
}

/*-----------------------------------------------------------
Function:	Initialize
Parametrs:
	[in] dir - default directory for VFS mapping
	[in] mode - VFS mode (default: RELEASE_MODE)

Create new VFS singleton class
-------------------------------------------------------------*/
void VFS::Initialize(const std::string &dir, VFS_MODE mode)
{
	single = new VFS();	
	single->AddDirectory(dir);	
	single->mode = mode;
}

void VFS::Initialize(VFS_MODE mode)
{
	single = new VFS();	
	single->mode = mode;
}


/*-----------------------------------------------------------
Function:	Destroy

Destroy VFS singleton
-------------------------------------------------------------*/
void VFS::Destroy()
{
	delete single;
	single = NULL;
}

/*-----------------------------------------------------------
Function:	GetInstance
Returns:
	instance of VFS singleton class

Get VFS singleton class pointer.
If VFS singleton not Initialized, NULL pointer returned
-------------------------------------------------------------*/
VFS * VFS::GetInstance()
{
	return single;
}


void VFS::PrintStructure()
{
	this->fileSystem->PrintStructure();
}

std::vector<VFS_FILE *> VFS::GetAllFiles()
{
	return this->fileSystem->GetAllFiles(true);
}

std::vector<VFS_FILE *> VFS::GetMainFiles()
{
	return this->fileSystem->GetAllFiles(false);
}

void VFS::SetWorkingDir(const std::string & dir)
{
	this->workingDir = dir;
}

const std::string & VFS::GetWorkingDir()
{
	return this->workingDir;
}

bool VFS::ExistFile(const std::string & fileName)
{
	VFS_FILE * file = this->fileSystem->GetFile(fileName);

	if (file == NULL)
	{
		return false;
	}

	return true;
}

bool VFS::IsFileInArchive(const std::string & fileName)
{
	VFS_FILE * file = this->fileSystem->GetFile(fileName);
	return file->archiveInfo != nullptr;
}

VFS_FILE * VFS::OpenFile(const std::string &path)
{
	std::string workingPath;
	VFS_FILE * file = NULL;
	
	if (this->workingDir.length() == 0) 
	{
		file = this->fileSystem->GetFile(path);
	}
	else 
	{
		workingPath = this->workingDir;
		if (path.c_str()[0] == '\\')
		{
			workingPath += path.substr(1, path.length() - 1);
		}
		else 
		{
			workingPath += path;
		}
		file = this->fileSystem->GetFile(workingPath);
	}

	if (file == NULL)
	{
		if (this->mode == RELEASE_MODE)
		{
			printf("[Error] File %s not found.\n", path.c_str());
			printf("[Error] Opening files out of VFS is permited only in DEBUG_MODE.\n");
			return NULL;
		}
		else if (this->mode == DEBUG_MODE)
		{
			if (this->workingDir.length() == 0) 
			{
				file = this->CreateDebugFile(path);
			}
			else 
			{
				file = this->CreateDebugFile(workingPath);
			}
		}
	}
	
	if (file == NULL)
	{
		return NULL;
	}

	if (file->archiveInfo == NULL)
	{
		FILE * tmpFile = NULL;
		my_fopen(&tmpFile, file->filePath, "rb");
		file->filePtr = tmpFile;
	}
	else 
	{
		file->archiveInfo->ptr = unzOpen(file->archiveInfo->filePath);

		unzSetOffset(file->archiveInfo->ptr, file->archiveInfo->offset);
		unzOpenCurrentFile(file->archiveInfo->ptr);
	}

	return file;
}

/*-----------------------------------------------------------
Function:	GetFileContent
Parametrs:
	[in] path - file path within VFS
	[out] fileSize - size of opened file in bytes
Returns:
	char * - byte array of data

Open file and return openeded data in buffer
buffer must be dealocated manually via delete[]
If file open failed, returns NULL
-------------------------------------------------------------*/
char * VFS::GetFileContent(const std::string &path, int * fileSize)
{
	VFS_FILE * f = this->OpenFile(path);
	if (f == NULL)
	{
		return NULL;
	}
	char * buf = new char[f->fileSize];
	this->Read(buf, sizeof(char), f->fileSize, f);
	*fileSize = f->fileSize;

	this->CloseFile(f);

	return buf;
}



/*-----------------------------------------------------------
Function:	GetFileString
Parametrs:
	[in] path - file path within VFS
Returns:
	loaded string

Open file and return openeded data as string
If file open failed, returns ""
-------------------------------------------------------------*/
std::string VFS::GetFileString(const std::string &path)
{
	VFS_FILE * f = this->OpenFile(path);
	if (f == NULL)
	{
		return "";
	}
	char * buf = new char[f->fileSize + 1];
	this->Read(buf, sizeof(char), f->fileSize, f);
	buf[f->fileSize] = 0;
	std::string str(buf);
	

	
	delete[] buf;

	this->CloseFile(f);

	
	return str;
}

int VFS::Read(void * buffer, size_t elementSize, size_t bytesCount, VFS_FILE * file)
{	
	if (file->archiveInfo == NULL) 
	{
		return fread(buffer, elementSize, bytesCount, static_cast<FILE *>(file->filePtr));
	}
	return unzReadCurrentFile(file->archiveInfo->ptr, buffer, elementSize * bytesCount);
}

int VFS::ReadString(char * buffer, size_t bytesCount, VFS_FILE * file)
{
	if (file == NULL)
	{
		return -1;
	}

	int read = 0;
	if (file->archiveInfo == NULL) 
	{
		read = fread(buffer, sizeof(char), bytesCount, static_cast<FILE *>(file->filePtr));
	}
	else 
	{ 
		read = unzReadCurrentFile(file->archiveInfo->ptr, buffer, bytesCount);
	}

	buffer[bytesCount] = 0;
	return read;
}

/*-----------------------------------------------------------
Function:	ReadEntireFile
Parametrs:
	[out] buffer - loaded file
	[in] file - file to be read
Returns:
	number of read bytes

Read entire file and fill it to buffer
Buffer must be dealocated with free(buffer) !!!
-------------------------------------------------------------*/
int VFS::ReadEntireFile(void * buffer, VFS_FILE * file)
{
	buffer = malloc(file->fileSize);

	if (file->archiveInfo == NULL) 
	{
		return fread(buffer, 1, file->fileSize, static_cast<FILE *>(file->filePtr));
	}
	
	return unzReadCurrentFile(file->archiveInfo->ptr, buffer, file->fileSize);
}

void VFS::CloseFile(VFS_FILE * file)
{
	
	if (file == NULL)
	{
		return;
	}

	if (file->archiveInfo == NULL)
	{
		fclose(static_cast<FILE *>(file->filePtr));
		file->filePtr = NULL;
	}
	else 
	{
		unzCloseCurrentFile(file->archiveInfo->ptr);
		unzClose(file->archiveInfo->ptr);
	}


	if (this->mode == DEBUG_MODE)
	{		
		for (unsigned int i = 0; i < this->debugModeFiles.size(); i++)
		{
			if (this->debugModeFiles[i] == file)
			{
				delete[] this->debugModeFiles[i]->name;
				delete[] this->debugModeFiles[i]->fullName;
				delete[] this->debugModeFiles[i]->filePath;
				delete[] this->debugModeFiles[i]->ext;
				
				
				delete this->debugModeFiles[i];
				this->debugModeFiles[i] = NULL;
			}
		}			
	}
}

void VFS::RefreshFile(const std::string &path)
{
	VFS_FILE *f = this->OpenFile(path);
	if (f->archiveInfo == NULL)
	{
		bool arch;
		int fs;
		this->FileInfo(f->filePath, arch, fs);
	

		f->fileSize = fs;
	}



	this->CloseFile(f);
}

void VFS::AddDirectory(const std::string &dirName)
{
	DIR * dir = opendir(dirName.c_str());
	if (dir == NULL)
	{
		printf("[Error] Directory %s not found.\n", dirName.c_str());		
		return;
	}
	std::string startDirName = dirName;
	#ifdef _MSC_VER
		startDirName = dir->patt; //full path using Windows dirent
		startDirName = startDirName.substr(0, startDirName.length() - 2);
	#endif	
	closedir(dir);

	this->AddDirectory(dirName, startDirName);

	
	//printf("\n========\n");
	//this->fileSystem->PrintStructure();
}

void VFS::AddDirectory(const std::string &dirName, const std::string &startDirName)
{
	
	DIR * dir = opendir(dirName.c_str());
	
    if (dir == NULL)
    {
        printf("Failed to open dir %s\n", dirName.c_str());
        return;
    }
    
	struct dirent * ent;
	std::string newDirName;
	std::string fullPath;
	std::string vfsPath;
	
   
    
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) 
	{
		if ((strcmp(ent->d_name, ".") == 0)||(strcmp(ent->d_name, "..") == 0))
		{
			continue;
		}
        switch (ent->d_type) 
		{
			case DT_REG:
			
				//printf ("%s (file)\n", ent->d_name);
				fullPath = dirName;
				#ifdef _MSC_VER
					fullPath = dir->patt; //full path using Windows dirent
					fullPath = fullPath.substr(0, fullPath.length() - 1);
				#else
                    if (fullPath[fullPath.GetLength() - 1] != '/')
                    {
                        fullPath += "/";
                    }
                #endif				
				fullPath += ent->d_name;
				
				vfsPath = fullPath;
				vfsPath = vfsPath.substr(startDirName.length(), fullPath.length() - startDirName.length());


				//printf("Full file path: %s\n", fullPath.GetConstString());
				//printf("VFS file path: %s\n", vfsPath.GetConstString());
				
				this->CreateVFSFile(vfsPath, fullPath);
				
				break;

			case DT_DIR:
				//printf ("%s (dir)\n", ent->d_name);
				
				newDirName = dirName;
                if (newDirName[newDirName.length() - 1] != '/')
                {
                    newDirName += "/";
                }
				newDirName += ent->d_name;
				this->AddDirectory(newDirName, startDirName);
				
				break;

			default:
				//printf ("%s:\n", ent->d_name);
				break;
        }
    }

	
    closedir (dir); 
	
}

bool VFS::FileInfo(const std::string &fileName, bool &archived, int &fileSize)
{
	FILE * file = NULL;
	my_fopen(&file, fileName.c_str(), "rb");
	if (file == NULL)
	{
		printf("Failed to open file %s with: %d\n", fileName.c_str(), errno);
		return false;
	}

	fseek(file, 0, SEEK_END);
	fileSize = static_cast<int>(ftell(file));
	fseek(file, 0, SEEK_SET);

	char * buf = (char *)malloc(10 * sizeof(char));
	fread(buf, sizeof(char), 10, file);


	fclose(file);

	archived = false;
	if ((buf[0] == 'P')&&(buf[1] == 'K')) 
	{
		archived  = true;
	}

	free(buf);

	return true;
}

void VFS::ScanArchive(const std::string &fileName)
{
	unzFile file = unzOpen(fileName.c_str());
	

	int res = unzGoToFirstFile(file);

	unz_file_info info;
	unz_file_pos pos;
	char * fileNameInArchive = new char[255 + 1];
	while(1)
	{

		unzGetCurrentFileInfo(file, &info, fileNameInArchive, 255, NULL, 0, NULL, 0);
		unzGetFilePos(file, &pos);

		if (fileNameInArchive[info.size_filename - 1] != '/')
		{
			std::string path = fileNameInArchive;

			//printf("%s\n", fileNameInArchive);

			Archived * arch = new Archived;
			arch->compressedSize = static_cast<int>(info.compressed_size);
			arch->method = 0;
			arch->ptr = NULL;
			arch->offset = pos.pos_in_zip_directory;			
			arch->filePath = my_strdup(fileName.c_str());

			VFS_FILE * file = new VFS_FILE;
			file->fileSize = static_cast<int>(info.uncompressed_size);
			file->archiveInfo = arch;
			file->filePtr = NULL;			
			file->filePath = my_strdup(path.c_str());

			arch->parent = file;

			replaceAll(path, "\\", "/");
			std::vector<std::string> splited = split(path, '/');
			std::string name = splited[splited.size() - 1];			
			file->fullName = my_strdup(name.c_str());

			splited = split(name, '.');
			std::string ext = "";
            if (splited.size() > 1)
            {
                ext = splited[splited.size() - 1];
            }			
			file->ext = my_strdup(ext.c_str());

			name = name.substr(0, name.length() - ext.length() - 1);			
			file->name = my_strdup(name.c_str());

			this->fileSystem->AddFile(path, file);
		}
			
		res = unzGoToNextFile(file);
		if (res == UNZ_END_OF_LIST_OF_FILE) break;
	}

	//keep zip file opened
	unzClose(file);
	delete[] fileNameInArchive;
}

void VFS::CreateVFSFile(std::string vfsPath, std::string fullPath)
{
	bool archived;
	int fileSize;
	if (this->FileInfo(fullPath, archived, fileSize) == false)
    {
        return;
    }

	if (archived)
	{
		this->ScanArchive(fullPath);
		return;
	}
	
	VFS_FILE * file = new VFS_FILE;
	file->fileSize = fileSize;
	file->archiveInfo = NULL;
	file->filePtr = NULL;	
	file->filePath = my_strdup(fullPath.c_str());


	replaceAll(vfsPath, "\\", "/");
	std::vector<std::string> splited = split(vfsPath, '/');
	std::string name = splited[splited.size() - 1];	
	file->fullName = my_strdup(name.c_str());

	splited = split(name, '.');
	std::string ext = "";
    if (splited.size() > 1)
    {
        ext = splited[splited.size() - 1];
    }	
	file->ext = my_strdup(ext.c_str());
	name = name.substr(0, name.length() - ext.length() - 1);	
	file->name = my_strdup(name.c_str());

	this->fileSystem->AddFile(vfsPath, file);
	
}

/*-----------------------------------------------------------
Function:	CreateDebugFile
Parameters:	
	[in] path - absolute path to file in real File System
Returns:
	newly created VFS_FILE pointer 

Create VFS_FILE pointer and fill structure for file, that is
opened from real FileSystem (eg.: C:\\Windows\\example.txt)
Supported only in VFS DEBUG_MODE
-------------------------------------------------------------*/
VFS_FILE * VFS::CreateDebugFile(std::string path)
{
	bool archived;
	int fileSize;
	bool res = this->FileInfo(path, archived, fileSize);
	if (!res)
	{
		printf("[Error] File %s not found.\n", path.c_str());
		return NULL;
	}

	VFS_FILE * file = new VFS_FILE;
	file->fileSize = fileSize;
	file->archiveInfo = NULL;
	file->filePtr = NULL;	
	file->filePath = my_strdup(path.c_str());
	

	std::string tmp = path;
	replaceAll(tmp, "\\", "/");
	std::vector<std::string> splited = split(tmp, '/');
	std::string name = splited[splited.size() - 1];
	file->fullName = my_strdup(name.c_str());

	splited = split(name, '.');
	std::string ext = "";
    if (splited.size() > 1)
    {
        ext = splited[splited.size() - 1];
    }	
	file->ext = my_strdup(ext.c_str());

	name = name.substr(0, name.length() - ext.length() - 1);	
	file->name = my_strdup(name.c_str());

	this->debugModeFiles.push_back(file);

	return file;
}


void VFS::ExportStructure(const std::string & fileName)
{
	std::string all = "";

	auto fs = this->GetAllFiles();
	for (auto & f : fs)
	{
		std::string fi = "";

		fi += f->ext;
		fi += ";";
		fi += f->filePath;
		fi += ";";
		fi += std::to_string(f->fileSize);
		fi += ";";
		fi += f->fullName;
		fi += ";";
		fi += f->name;
		fi += ";";

		if (f->archiveInfo != nullptr)
		{
			fi += std::to_string(f->archiveInfo->compressedSize);
			fi += ";";
			fi += f->archiveInfo->filePath;
			fi += ";";
			fi += std::to_string(f->archiveInfo->method);
			fi += ";";
			fi += std::to_string(f->archiveInfo->offset);
			fi += ";";
		}

		all += fi;
		all += "\n";
	}

	
	std::cin >> all;
	std::ofstream out(fileName);
	out << all;
	out.close();
}

/*
void VFS::ImportStructure()
{
	//this->fileSystem->AddFile(vfsPath, file);
}
*/