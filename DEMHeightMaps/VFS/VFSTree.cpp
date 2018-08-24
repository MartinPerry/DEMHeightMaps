#include "./VFS.h"

#include <stdlib.h>
#include <stdio.h>
#include <stack>

VFSTree::VFSTree()
{
	VFS_DIR * rootDir = new VFS_DIR;
	rootDir->name = my_strdup("");
	//rootDir->numDirs = 0;
	//rootDir->numFiles = 0;
	rootDir->parent = nullptr;
	
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

	free((char *)this->root->name);
	delete this->root;
	this->root = nullptr;
}

void VFSTree::Release(VFS_DIR * node)
{	
	for (auto it : node->dirs)
	{
		this->Release(it);
				
		free((char *)it->name);
		delete it;		
	}
	node->dirs.clear();

	
	for (auto jt : node->files)
	{
		this->ReleaseFile(jt);	
	}
	node->files.clear();
}

void VFSTree::ReleaseFile(VFS_FILE * file)
{
	if (file == nullptr)
	{			
		return;
	}
								
	free((char *)file->name);
	delete file;	
}

std::vector<VFS_FILE *> VFSTree::GetAllFiles(bool withFilesFromArchives) const
{
	std::vector<VFS_FILE *> tmp;
	this->GetAllFiles(this->root, 1, withFilesFromArchives, tmp);
	return tmp;
}

void VFSTree::GetAllFiles(VFS_DIR * node, int depth, bool withFilesFromArchives, std::vector<VFS_FILE *> & output) const
{
	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{		
		this->GetAllFiles((*it), depth + 1, withFilesFromArchives, output);
	}

	std::vector<VFS_FILE *>::const_iterator jt;
	for (jt = node->files.begin(); jt != node->files.end(); jt++)
	{
		if ((*jt) == nullptr)
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
				MyStringAnsi f1 = this->GetFilePath(output[i]);
				MyStringAnsi f2 = this->GetFilePath(*jt);
				if (f1 == f2)
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
void VFSTree::PrintStructure() const
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
void VFSTree::PrintStructure(VFS_DIR * node, int depth) const
{
	char * zanoreni = new char[depth * 2 + 1];
	memset(zanoreni, ' ', sizeof(char) * depth * 2);
	zanoreni[depth * 2 ] = 0; 

	std::vector<VFS_DIR *>::const_iterator it;
	for (it = node->dirs.begin(); it != node->dirs.end(); it++)
	{
		printf("%s[DIR] %s (dirs: %d, files: %d)\n", zanoreni, 
			(*it)->name, (*it)->dirs.size(), (*it)->files.size());
		
		this->PrintStructure((*it), depth + 1);
	}

	for (auto f : node->files)
	{
		if (f == nullptr)
		{
			printf("%s[FILE] <unknown NULL VFS_FILE>\n", zanoreni);
			continue;
		}
		printf("%s[FILE] %s\n", zanoreni, f->name);
	}

	delete[] zanoreni;
}

MyStringAnsi VFSTree::GetFilePath(VFS_FILE * f) const
{
	
	std::stack<const char *> names;
	names.push(f->name);

	VFS_DIR * d = f->dir;
	while ((d != nullptr))
	{
		names.push(d->name);
		d = d->parent;
	}

	MyStringAnsi path = names.top();
	names.pop();
	while (names.empty() == false)
	{
		path += '/';
		path += names.top();
		names.pop();
	}


	return path;
}

/*-----------------------------------------------------------
Function:	AddFile
Parameters:	
	[in] file - file to be inserted
Returns:
	true if OK, false if file already exist

Add new file to VFS. Every file can be added only once. 
If file is added second time, return false and file is not inserted
Input path is splitted into tokens, each token is one directory
-------------------------------------------------------------*/
bool VFSTree::AddFile(MyStringAnsi & vfsPath, VFS_FILE * file)
{
	if (this->GetFile(vfsPath) != nullptr)
	{
		this->ReleaseFile(file);
		printf("[Error] File \"%s\" already exist.\n", vfsPath.c_str());
		return false;
	}
	
	char * tmpPath = &vfsPath[0];

	VFS_DIR * node = this->root;

	size_t i = 1;
	size_t start = 1;
	while(tmpPath[i] != 0)
	{
		if (tmpPath[i] == '/')
		{						
			char * dirName = tmpPath + start;
			char tmp = dirName[i - start];

			dirName[i - start] = 0; //create string cut-off			
			node = this->AddDir(node, dirName);
			dirName[i - start] = tmp; //restore original path

			start = i + 1;
		}
		
		i++;		
	}	

	file->dir = node;
	node->files.push_back(file);	
	
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
VFS_DIR * VFSTree::AddDir(VFS_DIR * node, const char * dirName)
{
	//iterate all sub dirs from current node
	VFS_DIR * res = this->GetDir(node, dirName);
	if (res != nullptr)
	{
		return res;
	}
		
	//create new dir
	VFS_DIR * newDir = new VFS_DIR;		
	newDir->name = my_strdup(dirName);	
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
VFS_DIR * VFSTree::GetDir(VFS_DIR * node, const char * dirName) const
{	
	for (auto it : node->dirs)
	{		
		if (strcmp(it->name, dirName) == 0)
		{
			//directory with input name exist in sub-dirs
			return it;
		}
		
	}
	return nullptr;
}

/*-----------------------------------------------------------
Function:	GetFile
Parameters:	
	[in] path - path to file. Must start with "/"
Returns:
	VFS_FILE pointer

Get file by its path
-------------------------------------------------------------*/
VFS_FILE * VFSTree::GetFile(const MyStringAnsi &path) const
{
		
	VFS_DIR * node = this->root;
	char * pathStr = my_strdup(path.c_str()); //create copy of path, because we will modify this


	size_t i = 1;
	size_t start = 1;
	while (pathStr[i] != 0)
	{
		if (pathStr[i] == '/')
		{			
			char * dirName = pathStr + start;
			char tmp = dirName[i - start];

			dirName[i - start] = 0; //create string cut-off
			node = this->GetDir(node, dirName);
			dirName[i - start] = tmp; //restore original path

			if (node == nullptr)
			{
				free(pathStr);
				return nullptr;
			}

			start = i + 1;
		}

		i++;
	}
		
	const char * fileName = pathStr + start;

	for (auto f : node->files)
	{
		if (strcmp(f->name, fileName) == 0)
		{
			free(pathStr);
			return f;
		}
	}

	free(pathStr);
	return nullptr;
}

/*-----------------------------------------------------------
Function:	GetDir
Parameters:	
	[in] path - path to dir. Must start with "/"
Returns:
	VFS_DIR pointer

Get dir by its path
-------------------------------------------------------------*/
VFS_DIR * VFSTree::GetDir(const MyStringAnsi &path) const
{
	std::vector<MyStringAnsi> splited = path.Split<MyStringAnsi>({ '/', '\\' });

	int count = static_cast<int>(splited.size());
	VFS_DIR * node = this->root;
	for (int i = 0; i < count; i++)
	{
		node = this->GetDir(node, splited[i].c_str());
		if (node == nullptr)
		{
			return nullptr;
		}
	}

	
	return node;
}