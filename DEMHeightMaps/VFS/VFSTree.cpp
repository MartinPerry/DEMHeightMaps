#include "./VFS.h"

#include <stdlib.h>
#include <stdio.h>

VFSTree::VFSTree()
{
	VFS_DIR * rootDir = new VFS_DIR;
	rootDir->name = "/";
	rootDir->numDirs = 0;
	rootDir->numFiles = 0;
	rootDir->parent = NULL;
	
	this->root = rootDir;

}

VFSTree::~VFSTree()
{
	this->Release();
}

void VFSTree::Release()
{
	if (this->root == nullptr)
	{
		return;
	}
	printf("Releaseing vfs tree");
	this->Release(this->root);

	delete this->root;
	this->root = nullptr;
}

void VFSTree::Release(VFS_DIR * node)
{
	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{
		this->Release((*it));
				
		delete[] (*it)->name;		
		delete (*it);
		
	}

	std::vector<VFS_FILE *>::const_iterator jt;
	for (jt = node->files.begin(); jt != node->files.end(); jt++)
	{
		this->ReleaseFile((*jt));	
	}

	node->files.clear();

	this->cachedFiles.clear();
}

void VFSTree::ReleaseFile(VFS_FILE * file)
{
	if (file == NULL) 
	{			
		return;
	}
						
	delete file->archiveInfo;
	delete[] file->ext;
	delete[] file->filePath;
	//delete (*jt)->filePtr;
	delete[] file->fullName;
	delete[] file->name;
	delete file;	
}

std::vector<VFS_FILE *> VFSTree::GetAllFiles(bool withFilesFromArchives)
{
	std::vector<VFS_FILE *> tmp;
	this->GetAllFiles(this->root, 1, withFilesFromArchives, tmp);
	return tmp;
}

void VFSTree::GetAllFiles(VFS_DIR * node, int depth, bool withFilesFromArchives, std::vector<VFS_FILE *> & output)
{
	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{		
		this->GetAllFiles((*it), depth + 1, withFilesFromArchives, output);
	}

	std::vector<VFS_FILE *>::const_iterator jt;
	for (jt = node->files.begin(); jt != node->files.end(); jt++)
	{
		if ((*jt) == NULL)
		{			
			continue;
		}	
		if (withFilesFromArchives)
		{
			output.push_back((*jt));
		}
		else 
		{
			bool canAdd = true;
			for (size_t i = 0; i < output.size(); i++)
			{
				if (output[i]->filePath == (*jt)->filePath)
				{
					canAdd = false;
					break;
				}
			}

			if (canAdd)
			{
				output.push_back((*jt));
			}
		}
		
	}
	
}

/*-----------------------------------------------------------
Function:	PrintStructure

Print out VFS structure
-------------------------------------------------------------*/
void VFSTree::PrintStructure()
{
	this->PrintStructure(this->root, 1);
}

/*-----------------------------------------------------------
Function:	PrintStructure
Parameters:	
	[in] node - start node
	[in] depth - actual depth in file system


Print out actual node and iterate to next node
-------------------------------------------------------------*/
void VFSTree::PrintStructure(VFS_DIR * node, int depth)
{
	char * zanoreni = new char[depth * 2 + 1];
	memset(zanoreni, ' ', sizeof(char) * depth * 2);
	zanoreni[depth * 2 ] = 0; 

	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{
		printf("%s[DIR] %s (dirs: %d, files: %d)\n", zanoreni, (*it)->name, (*it)->numDirs, (*it)->numFiles);
		
		this->PrintStructure((*it), depth + 1);
	}

	std::vector<VFS_FILE *>::const_iterator jt;
	for (jt = node->files.begin(); jt != node->files.end(); jt++)
	{
		if ((*jt) == NULL) 
		{
			printf("%s[FILE] <unknown NULL VFS_FILE>\n", zanoreni);
			continue;
		}
		printf("%s[FILE] %s\n", zanoreni, (*jt)->fullName);
	}

	delete[] zanoreni;
}

/*-----------------------------------------------------------
Function:	AddFile
Parameters:	
	[in] path - path to file. Must start with "/"
Returns:
	true if OK, false if file already exist

Add new file to VFS. Every file can be added only once. 
If file is added second time, return false and file is not inserted
Input path is splitted into tokens, each token is one directory
New file must be created first from filePath...
new created file has only name, no other info provided. Should
be used for real files, not for files in archives
-------------------------------------------------------------*/
bool VFSTree::AddFile(std::string &filePath)
{
	/*
	VFS_FILE * file = new VFS_FILE;
	file->name = "<to do>";
	file->ext = "<to do>";
	file->fileSize = -1;
	file->archiveInfo = NULL;

	char * str;
	fileName.FillString(str);
	file->fullName = strdup(str);
	delete[] str;
	*/

	return this->AddFile(filePath, NULL);
	
}

/*-----------------------------------------------------------
Function:	AddFile
Parameters:	
	[in] path - path to file. Must start with "/"
	[in] file - file to be inserted
Returns:
	true if OK, false if file already exist

Add new file to VFS. Every file can be added only once. 
If file is added second time, return false and file is not inserted
Input path is splitted into tokens, each token is one directory
-------------------------------------------------------------*/
bool VFSTree::AddFile(std::string &filePath, VFS_FILE * file)
{
	if (this->GetFile(filePath) != NULL)
	{
		this->ReleaseFile(file);
		printf("[Error] File \"%s\" already exist.\n", filePath.c_str());
		return false;
	}

	replaceAll(filePath, "\\", "/");
	std::vector<std::string> splited = split(filePath, '/');
	
	int count = static_cast<int>(splited.size());
	VFS_DIR * node = this->root;
	for (int i = 0; i < count - 1; i++)
	{
		node = this->AddDir(node, splited[i]);
	}
	node->files.push_back(file);

	splited.clear();

	//Caching files in tree....
	//filePath is altered to start with "\"
	//rest of filePath is taken from input filePath and "/" are replaced with "\"

	std::string cachedPath;
	if (filePath[0] != '/')
	{
		cachedPath = "/";
	}
	cachedPath += filePath;	
	replaceAll(cachedPath, "\\", "/");

	this->cachedFiles.insert( std::pair<std::string, VFS_FILE *>(cachedPath, file) );


	return true;
}

/*-----------------------------------------------------------
Function:	AddDir
Parameters:	
	[in] node - actual node
	[in] dirName - new directory to input
Returns:
	new created node

Iterate through all child nodes in actual node. 
If dirName == actually iteratd dir, go one level deeper and return 
Else create new dir, insert into structure and return. Parent dirs
are updated on number of deeper dirs
-------------------------------------------------------------*/
VFS_DIR * VFSTree::AddDir(VFS_DIR * node, const std::string &dirName)
{
	//iterate all sub dirs from current node
	VFS_DIR * res = this->GetDir(node, dirName);
	if (res != NULL)
	{
		res->numFiles++;
		return res;
	}
	
	//update parent number of subdirs
	VFS_DIR * tmp = node;
	while(true)
	{
		tmp->numDirs++;
		tmp = tmp->parent;
		if (tmp == NULL) break;
	}

	//create new dir
	VFS_DIR * newDir = new VFS_DIR;
	
	//char * str;	
	newDir->name = my_strdup(dirName.c_str());
	//newDir->name = my_strdup(str);
	//delete[] str;

	newDir->numDirs = 0;
	newDir->numFiles = 1;
	newDir->parent = node;
	node->dirs.push_back(newDir);


	return newDir;
}

/*-----------------------------------------------------------
Function:	GetDir
Parameters:	
	[in] node - actual node
	[in] dirName - name of directory
Returns:
	node with desired DIR

Obtain directory by its name
-------------------------------------------------------------*/
VFS_DIR * VFSTree::GetDir(VFS_DIR * node, const std::string &dirName)
{
	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{
		
		if (strcmp((*it)->name, dirName.c_str()) == 0)
		{
			//directory with input name exist in sub-dirs
			return (*it);
		}
		
	}
	return NULL;
}

/*-----------------------------------------------------------
Function:	GetFile
Parameters:	
	[in] path - path to file. Must start with "/"
Returns:
	VFS_FILE pointer

Get file by its path
-------------------------------------------------------------*/
VFS_FILE * VFSTree::GetFile(const std::string &path)
{
	if (this->cachedFiles.find(path) != this->cachedFiles.end())
	{
		return this->cachedFiles[path];
	}
	
	std::string tmp = path;
	replaceAll(tmp, "\\", "/");
	std::vector<std::string> splited = split(tmp, '/');
	

	VFS_DIR * node = this->root;
	for (int i = 0; i < static_cast<int>(splited.size()) - 1; i++)
	{
		node = this->GetDir(node, splited[i]);
		if (node == NULL)
		{
			return NULL;
		}
	}

	std::vector<VFS_FILE *>::const_iterator jt;
	for (jt = node->files.begin(); jt != node->files.end(); jt++)
	{
		if (strcmp(splited.back().c_str(), (*jt)->fullName) == 0)
		{
			return (*jt);
		}
	}

	return NULL;
}

/*-----------------------------------------------------------
Function:	GetDir
Parameters:	
	[in] path - path to dir. Must start with "/"
Returns:
	VFS_DIR pointer

Get dir by its path
-------------------------------------------------------------*/
VFS_DIR * VFSTree::GetDir(const std::string &path)
{
	std::string tmp = path;
	replaceAll(tmp, "\\", "/");
	std::vector<std::string> splited = split(tmp, '/');

	int count = static_cast<int>(splited.size());
	VFS_DIR * node = this->root;
	for (int i = 0; i < count; i++)
	{
		node = this->GetDir(node, splited[i]);
		if (node == NULL)
		{
			return NULL;
		}
	}

	
	return node;
}