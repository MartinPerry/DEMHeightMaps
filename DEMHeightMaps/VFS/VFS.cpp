#include "./VFS.h"

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <unordered_map>

#include "./minizip/unzip.h"

#ifdef _WIN32
	#include "./win_dirent.h"
#else 
    #include <sys/stat.h>
	#include <dirent.h>
#endif

#include "./OSUtils.h"

#ifdef __ANDROID_API__
    #include <android/asset_manager.h>
	#include "../../OS_Android/AndroidUtils.h"
#endif

//singleton instance of VFS
VFS * VFS::single = nullptr;

/*-----------------------------------------------------------
Function:	ctor

VFS Singleton private ctor
-------------------------------------------------------------*/
VFS::VFS()
{
	this->fileSystem = new VFSTree();	
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
	delete this->fileSystem;
	this->fileSystem = nullptr;	
}

void VFS::InitializeEmpty()
{
	single = new VFS();	
}

void VFS::InitializeRaw(const MyStringAnsi &dir)
{
	single = new VFS();	
	single->initDirs.push_back(dir);
}

/*-----------------------------------------------------------
Function:	Initialize
Parametrs:
	[in] dir - default directory for VFS mapping
	[in] mode - VFS mode (default: RELEASE_MODE)

Create new VFS singleton class
-------------------------------------------------------------*/
void VFS::InitializeFull(const MyStringAnsi &dir)
{
	single = new VFS();
	single->AddDirectory(dir);
}

/*-----------------------------------------------------------
Function:	Destroy

Destroy VFS singleton
-------------------------------------------------------------*/
void VFS::Destroy()
{
	delete single;
	single = nullptr;
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


void VFS::PrintStructure() const
{
	this->fileSystem->PrintStructure();
}

std::vector<VFS_FILE *> VFS::GetAllFiles() const
{
	return this->fileSystem->GetAllFiles(true);
}

std::vector<VFS_FILE *> VFS::GetMainFiles() const
{
	return this->fileSystem->GetAllFiles(false);
}


const char * VFS::GetFileName(VFS_FILE * f) const
{
	return f->name;
}

const char * VFS::GetFileExt(VFS_FILE * f) const
{
	const char * n = f->name;
	int i = strlen(n) - 1;
	while ((i > 0) && (n[i] != '.') && (n[i] != '/') && (n[i] != '\\'))
	{
		i--;
	}
		
	return n + i + 1;
}

MyStringAnsi VFS::GetFilePath(VFS_FILE * f) const
{
	return this->fileSystem->GetFilePath(f);
}



void VFS::SaveDirStructure(const MyStringAnsi & fileName) const
{
	VFS_DIR * d = this->fileSystem->GetDir("");
	MyStringAnsi content = "";
	MyStringAnsi baseDirPath = d->name;
	//baseDirPath += '/';
	this->SaveDirStructure(d, baseDirPath, content);

	content.SaveToFile(fileName.c_str());
}

void VFS::SaveDirStructure(VFS_DIR * d, const MyStringAnsi & dirPath, MyStringAnsi & data) const
{
	data += dirPath.SubString(0, dirPath.length() - 1); //we dont want "final" / to be appended
	data += '\n';

	for (auto sd : d->dirs)
	{
		MyStringAnsi tmpDirPath = dirPath;
		tmpDirPath += sd->name;
		tmpDirPath += '/';

		this->SaveDirStructure(sd, tmpDirPath, data);
	}

	
}




bool VFS::CopySingleFile(const MyStringAnsi & src, const MyStringAnsi & dest) const
{	
	FILE * destFile = nullptr;
	my_fopen(&destFile, dest.c_str(), "wb");
	if (destFile == nullptr)
	{
		return false;
	}

	size_t bufSize = 0;
	char * buf = this->GetFileContent(src, &bufSize);

	fwrite(buf, sizeof(char), bufSize, destFile);
	fclose(destFile);

	delete[] buf;

	return true;
}

#ifdef __ANDROID_API__
bool VFS::CopySingleFileAndroid(const MyStringAnsi & src, const MyStringAnsi & dest) const
{
	FILE * srcFile = AndroidUtils::AssetFopen(src.c_str(), "rb");
	if (srcFile == nullptr)
	{
		return false;
	}


	FILE * destFile = nullptr;
	my_fopen(&destFile, dest.c_str(), "wb");
	if (destFile == nullptr)
	{
		fclose(srcFile);
		return false;
	}

	
	fseek(srcFile, 0L, SEEK_END);
	size_t fileSize = ftell(srcFile);
	fseek(srcFile, 0L, SEEK_SET);

	char * buffer = new char[fileSize];
	fread(buffer, sizeof(char), fileSize, srcFile);
	
	fwrite(buffer, sizeof(char), fileSize, destFile);
	
	fclose(srcFile);
	fclose(destFile);

	SAFE_DELETE_ARRAY(buffer);

	return true;
}
#endif

//copy all files to destination dir
int VFS::CopyAllFilesFromDir(const MyStringAnsi & dirPath, const MyStringAnsi & destPath) const
{
	int c = 0;
	VFS_DIR * d = this->fileSystem->GetDir(dirPath);
	
	MyStringAnsi tmpDestPath = destPath;
	if (tmpDestPath.GetLastChar() != '/')
	{
		tmpDestPath += '/';
	}

	if (d != nullptr)
	{
		return this->CopyAllFilesFromDir(d, tmpDestPath);
	}
	else
	{
		return this->CopyAllFilesFromRawDir(dirPath, tmpDestPath);
	}
}

int VFS::CopyAllFilesFromDir(VFS_DIR * d, const MyStringAnsi & destPath) const
{	
	int c = 0;
	
	for (auto sd : d->dirs)
	{
		MyStringAnsi tmpDestPath = destPath;		
		tmpDestPath += sd->name;
		tmpDestPath += '/';

		c += this->CopyAllFilesFromDir(sd, tmpDestPath);
	}
	
	OSUtils::Instance()->CreatePath(destPath);
	
	VFS_FILE tmp;
	for (auto ff : d->files)
	{		
		VFS_FILE * f = this->OpenFile(this->fileSystem->GetFilePath(ff), &tmp);
		
		void * buf = nullptr;
		int bufSize = this->ReadEntireFile(&buf, f);
		this->CloseFile(f);

		MyStringAnsi finalDestPath = destPath;

		if (f == &tmp)
		{
			//file is opened in RAW mode - name is same as from VFS file
			finalDestPath += this->GetFileName(ff);
		}
		else
		{
			finalDestPath += this->GetFileName(f);
		}
		
		FILE * destFile = nullptr;
		my_fopen(&destFile, finalDestPath.c_str(), "wb");
		if (destFile == nullptr)
		{
			free(buf);
			continue;
		}

		fwrite(buf, sizeof(char), bufSize, destFile);
		fclose(destFile);
		free(buf);

		c++;
	}

	return c;
}

int VFS::CopyAllFilesFromRawDir(const MyStringAnsi & dirPath, const MyStringAnsi & destPath) const
{

	if (DIR * dir = opendir(dirPath.c_str()))
	{
		struct dirent *ent;
		
		while ((ent = readdir(dir)) != nullptr)
		{
			if (ent->d_name[0] == '.')
			{
				continue;
			}
			if (ent->d_type == DT_REG)
			{			
				//printf ("%s (file)\n", ent->d_name);				
				MyStringAnsi fullPathDest = destPath;
#ifdef _MSC_VER
				MyStringAnsi fullPathSrc = dir->patt; //full path using Windows dirent
				fullPathSrc = fullPathSrc.SubString(0, fullPathSrc.length() - 1);
#else
				MyStringAnsi fullPathSrc = dirPath;
				if (fullPathSrc.GetLastChar() != '/')
				{
					fullPathSrc += '/';
				}

				if (fullPathDest.GetLastChar() != '/')
				{
					fullPathDest += '/';
				}
#endif
				fullPathSrc += ent->d_name;
				fullPathDest += ent->d_name;

				this->CopySingleFile(fullPathSrc, fullPathDest);							
			}
		}
		closedir(dir);
	}

#ifdef __ANDROID_API__
	if (AAssetDir* assetDir = AAssetManager_openDir(AndroidUtils::android_asset_manager, dirPath.c_str()))
	{
		const char* filename = (const char*)NULL;
		while ((filename = AAssetDir_getNextFileName(assetDir)) != nullptr)
		{
			if (filename[0] == '.')
			{
				continue;
			}

			MyStringAnsi fullPathSrc = dirPath;
			if (fullPathSrc.GetLastChar() != '/')
			{
				fullPathSrc += '/';
			}

			fullPathSrc += filename;

			MyStringAnsi fullPathDest = destPath;
			if (fullPathDest.GetLastChar() != '/')
			{
				fullPathDest += '/';
			}
			fullPathDest += filename;

			if (this->CopySingleFileAndroid(fullPathSrc, fullPathDest) == false)
			{
				MY_LOG_ERROR("Copy %s to %s failed", fullPathSrc.c_str(), fullPathDest.c_str());
			}
		}
		AAssetDir_close(assetDir);
	}
	
#endif

	return 0;
}

void VFS::PackStructure(const MyStringAnsi & outputFile) const
{	
	auto allFiles = this->GetAllFiles();


	typedef struct FileInfo
	{
		size_t dataSize;
		char * data;
	} FileInfo;

	std::unordered_map<MyStringAnsi, FileInfo> data;
	for (auto vf : allFiles)
	{
		MyStringAnsi path = this->GetFilePath(vf);
		FileInfo fi;
		fi.data = this->GetFileContent(path, &fi.dataSize);

		data[path] = fi;
	}

	//=====================================================

	//file size - uint32 nebo uint16 podle max velikosti

	uint32_t HEADER_SIZE = 0; //spocitat velikost "hlavicky"

	HEADER_SIZE += 2 * sizeof(char); //header identification as "VD"

	HEADER_SIZE += sizeof(uint32_t); //count of files


	for (auto vf : allFiles)
	{
		MyStringAnsi tmp = this->GetFilePath(vf);

		HEADER_SIZE += sizeof(uint32_t); //file size
		HEADER_SIZE += sizeof(uint32_t); //data offset
		HEADER_SIZE += sizeof(uint16_t); //file name length

		HEADER_SIZE += tmp.length();
		//HEADER_SIZE += sizeof(char); //ending block

	}

	//===================================================

	FILE * f = nullptr;
	my_fopen(&f, outputFile.c_str(), "wb");

	//header
	fwrite("V", sizeof(char), 1, f);
	fwrite("D", sizeof(char), 1, f);

	uint32_t s = static_cast<uint32_t>(allFiles.size());
	fwrite(&s, sizeof(uint32_t), 1, f);

	uint32_t packedOffset = HEADER_SIZE;
	for (auto vf : allFiles)
	{
		MyStringAnsi tmp = this->GetFilePath(vf);

		const FileInfo & fi = data[tmp];
        uint32_t dataSize = static_cast<uint32_t>(fi.dataSize);
		fwrite(&dataSize, sizeof(uint32_t), 1, f);
		fwrite(&packedOffset, sizeof(uint32_t), 1, f);
		packedOffset += fi.dataSize;

		uint16_t fl = static_cast<uint16_t>(tmp.length());
		fwrite(&fl, sizeof(uint16_t), 1, f);
		fwrite(tmp.c_str(), sizeof(char), tmp.length(), f);
	}

	for (auto vf : allFiles)
	{
		MyStringAnsi tmp = this->GetFilePath(vf);

		const FileInfo & fi = data[tmp];

		fwrite(fi.data, sizeof(uint8_t), fi.dataSize, f);
	}


	fclose(f);
}


bool VFS::ExistFile(const MyStringAnsi & fileName) const
{
	VFS_FILE * file = this->fileSystem->GetFile(fileName);

	if (file == nullptr)
	{
		return false;
	}

	return true;
}

bool VFS::IsFileInArchive(const MyStringAnsi & fileName) const
{
	VFS_FILE * file = this->fileSystem->GetFile(fileName);
	if (file == nullptr)
	{
		return false;
	}

	return file->archiveType != 0;
}

FILE * VFS::GetRawFile(const MyStringAnsi &path) const
{
#ifdef __ANDROID_API__
	std::vector<MyStringAnsi> paths; //(this->initDirs.size());
#endif

	struct stat sb;

	for (MyStringAnsi p : this->initDirs)
	{
		p += '/';
		p += path;
		
		if (stat(p.c_str(), &sb) == 0)
		{			
			FILE * tmpFile = nullptr;
			my_fopen(&tmpFile, p.c_str(), "rb");

			if (tmpFile)
			{
				return tmpFile;
			}
		}

#ifdef __ANDROID_API__
		paths.push_back(p);
#endif
	}

    //try open file directly
    //this allows us to use full file paths within VFS
    //like: C:/dir/other_dir/file.txt
    if (stat(path.c_str(), &sb) == 0)
    {
        FILE * tmpFile = nullptr;
        my_fopen(&tmpFile, path.c_str(), "rb");

        if (tmpFile)
        {
            return tmpFile;
        }
    }
#ifdef __ANDROID_API__
    paths.push_back(path);
#endif

	
#ifdef __ANDROID_API__
	for (auto p : paths)
	{
        FILE * tmpFile = AndroidUtils::AssetFopen(p.c_str(), "rb");
        if (tmpFile)
        {
            return tmpFile;
        }
	}
#endif
			
	return nullptr;
}

MyStringAnsi VFS::GetRawFileFullPath(const MyStringAnsi &path) const
{
	for (auto p : this->initDirs)
	{
		p += '/';
		p += path;

		struct stat sb;
		if (stat(p.c_str(), &sb) == 0)
		{
			return p;
		}
	}


#ifdef __ANDROID_API__
	for (auto p : this->initDirs)
	{
		p += '/';
		p += path;

		FILE * tmpFile = AndroidUtils::AssetFopen(p.c_str(), "rb");
		if (tmpFile)
		{
			fclose(tmpFile);
			return p;
		}
	}
#endif
	
	return "";
}


VFS_FILE * VFS::OpenFile(const MyStringAnsi &path, VFS_FILE * temporary) const
{
	VFS_FILE * f = nullptr;
	if (FILE * ff = this->GetRawFile(path))
	{
		//open file directly from OS file system

		temporary->archiveFileIndex = std::numeric_limits<uint16_t>::max();
		temporary->archiveType = VFS_ARCHIVE_TYPE::NONE;
		temporary->filePtr = ff;

		fseek(ff, 0L, SEEK_END);
		temporary->fileSize = static_cast<size_t>(ftell(ff));
		fseek(ff, 0L, SEEK_SET);
		
		//can directly return		
		return temporary;
	}
	else
	{
		//failed to open directly from OS file system
		//find file in VFS tree
		//this is probably file in archive
		f = this->fileSystem->GetFile(path);
	}

	if (f == nullptr)
	{
		//file not found
		return nullptr;
	}


	if (f->archiveFileIndex == std::numeric_limits<uint16_t>::max())
	{
		printf("Problem - should not happed. This file should already be opened by OS file system");
		/*
		FILE * tmpFile = nullptr;
		my_fopen(&tmpFile, f->filePath, "rb");
#ifdef __ANDROID_API__
        if (tmpFile == nullptr)
		{
			tmpFile = AndroidUtils::AssetFopen(f->filePath, "rb");
		}
#endif
		f->filePtr = tmpFile;
		*/
	}
	else 
	{
		//file is zipped archive

		if (f->archiveType == VFS_ARCHIVE_TYPE::ZIP)
		{
			f->filePtr = unzOpen(this->archiveFiles[f->archiveFileIndex].c_str());

			unzSetOffset(f->filePtr, f->archiveOffset);
			int res = unzOpenCurrentFile(f->filePtr);
			if (res != UNZ_OK)
			{
				printf("Failed to open zipped file: %i\n", res);
				return nullptr;
			}
		}
		else if (f->archiveType == VFS_ARCHIVE_TYPE::PACKED_FS)
		{
			FILE * tmpFile = nullptr;
			my_fopen(&tmpFile, this->archiveFiles[f->archiveFileIndex].c_str(), "rb");

			if (tmpFile == nullptr)
			{
				return nullptr;
			}

			fseek(tmpFile, f->archiveOffset, SEEK_SET);

			f->filePtr = tmpFile;
		}
	}

	return f;
}

void VFS::CloseFile(VFS_FILE * file) const
{
	if ((file == nullptr) && (file->filePtr == nullptr))
	{
		return;
	}

	if (file->archiveFileIndex == std::numeric_limits<uint16_t>::max())
	{
		fclose(static_cast<FILE *>(file->filePtr));
	}
	else 
	{
		if (file->archiveType == VFS_ARCHIVE_TYPE::ZIP)
		{
			unzCloseCurrentFile(file->filePtr);
			unzClose(file->filePtr);
		}
		else if (file->archiveType == VFS_ARCHIVE_TYPE::PACKED_FS)
		{
			fclose(static_cast<FILE *>(file->filePtr));
		}
	}

	file->filePtr = nullptr;
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
char * VFS::GetFileContent(const MyStringAnsi &path, size_t * fileSize) const
{
	VFS_FILE tmp;
	VFS_FILE * f = this->OpenFile(path, &tmp);

	if (f == nullptr)
	{
		return nullptr;
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
MyStringAnsi VFS::GetFileString(const MyStringAnsi &path) const
{
	VFS_FILE tmp;
	VFS_FILE * f = this->OpenFile(path, &tmp);

	if (f == nullptr)
	{
		return "";
	}


	char * buf = new char[f->fileSize + 1];
	int ret = this->Read(buf, sizeof(char), f->fileSize, f);
	buf[f->fileSize] = 0;
	MyStringAnsi str = MyStringAnsi::CreateFromMoveMemory(buf, f->fileSize + 1, f->fileSize);

	//MyStringAnsi str(buf);	
	//delete[] buf;

	this->CloseFile(f);

	return str;
}

int VFS::Read(void * buffer, size_t elementSize, size_t bytesCount, VFS_FILE * file) const
{
	if ((file == nullptr) && (file->filePtr == nullptr))
	{
		return -1;
	}

	if (file->archiveType != VFS_ARCHIVE_TYPE::ZIP)
	{
		return static_cast<int>(fread(buffer, elementSize, bytesCount, static_cast<FILE *>(file->filePtr)));
	}
	return unzReadCurrentFile(file->filePtr, buffer, static_cast<unsigned>(elementSize * bytesCount));
}

int VFS::ReadString(char * buffer, size_t bytesCount, VFS_FILE * file) const
{
	if ((file == nullptr) && (file->filePtr == nullptr))
	{
		return -1;
	}

	int read = 0;
	if (file->archiveType != VFS_ARCHIVE_TYPE::ZIP)
	{
		read = static_cast<int>(fread(buffer, sizeof(char), bytesCount, static_cast<FILE *>(file->filePtr)));
	}
	else 
	{ 
		read = unzReadCurrentFile(file->filePtr, buffer, static_cast<unsigned>(bytesCount));
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
int VFS::ReadEntireFile(void ** buffer, VFS_FILE * file) const
{
	*buffer = malloc(file->fileSize);

	if (file->archiveType != VFS_ARCHIVE_TYPE::ZIP)
	{
		return static_cast<int>(fread(*buffer, 1, file->fileSize, static_cast<FILE *>(file->filePtr)));
	}

	return unzReadCurrentFile(file->filePtr, *buffer, static_cast<unsigned>(file->fileSize));
}


void VFS::RefreshFile(const MyStringAnsi &path)
{
	VFS_FILE tmp;
	if (VFS_FILE * f = this->OpenFile(path, &tmp))
	{
		if (f == &tmp)
		{
			//do not refresh, file is not in VFS -
			//opened file is "raw" file from OS file system
			return;
		}

		if (f->archiveFileIndex == std::numeric_limits<uint16_t>::max())
		{
			VFS_ARCHIVE_TYPE arch;
			size_t fs = 0;
			this->FileInfo(this->fileSystem->GetFilePath(f), arch, fs);

			f->fileSize = fs;
		}

		this->CloseFile(f);
	}
}

void VFS::AddDirectory(const MyStringAnsi &dirName)
{
    bool failed = true;

	if (DIR * dir = opendir(dirName.c_str()))
    {
        MyStringAnsi startDirName = dirName;
#ifdef _MSC_VER
        startDirName = dir->patt; //full path using Windows dirent
        startDirName = startDirName.SubString(0, startDirName.length() - 2);
#endif
        closedir(dir);

        this->initDirs.push_back(startDirName);

        this->AddDirectory(dirName, startDirName);
        failed = false;
    }
#ifdef __ANDROID_API__

    if (AAssetDir* dir = AAssetManager_openDir(AndroidUtils::android_asset_manager, dirName.c_str()))
    {
        bool dirExist = AAssetDir_getNextFileName(dir) != NULL;
        AAssetDir_close(dir);

        if (dirExist)
        {
            MyStringAnsi startDirName = dirName;

            this->initDirs.push_back(startDirName);

            this->AddDirectoryAndroid(dirName, startDirName);
            failed = false;
        }
    }
#endif

    if (failed)
    {
        printf("[VFS Error] Directory %s not found.\n", dirName.c_str());
    }
	//printf("\n========\n");
	//this->fileSystem->PrintStructure();
}

void VFS::AddHighPriorityRawDirectory(const MyStringAnsi &dirName)
{
	this->AddRawDirectory(dirName, 0);
}

void VFS::AddRawDirectory(const MyStringAnsi &dirName, int priority)
{
	if (DIR * dir = opendir(dirName.c_str()))
    {
        MyStringAnsi startDirName = dirName;
#ifdef _MSC_VER
        startDirName = dir->patt; //full path using Windows dirent
        startDirName = startDirName.SubString(0, startDirName.length() - 2);
#endif
        closedir(dir);

		if (priority == 0)
		{
			this->initDirs.insert(this->initDirs.begin(), startDirName);
		}
		else if (priority > 0)
		{
			this->initDirs.insert(this->initDirs.begin() + priority, startDirName);
		}
		else
		{
			this->initDirs.push_back(startDirName);
		}

		return;
    }


#ifdef __ANDROID_API__


	if (priority == 0)
	{
		this->initDirs.insert(this->initDirs.begin(), dirName);
	}
	else if (priority > 0)
	{
		this->initDirs.insert(this->initDirs.begin() + priority, dirName);
	}
	else
	{
		this->initDirs.push_back(dirName);
	}

	//do not test for android
	/*
    if (AAssetDir* dir = AAssetManager_openDir(AndroidUtils::android_asset_manager, dirName.c_str()))
    {
        bool dirExist = AAssetDir_getNextFileName(dir) != NULL;
        AAssetDir_close(dir);

        if (dirExist)
        {
            MyStringAnsi startDirName = dirName;

			if (priority == 0)
			{
				this->initDirs.insert(this->initDirs.begin(), startDirName);
			}
			else
			{
				this->initDirs.push_back(startDirName);
			}
        }
    }
	*/
#endif

}


void VFS::AddDirectory(const MyStringAnsi &dirName, const MyStringAnsi &startDirName)
{
	
	DIR * dir = opendir(dirName.c_str());
	if (dir == nullptr)
	{
		printf("[VFS Error] Failed to open dir %s\n", dirName.c_str());
		return;
	}

	
	struct dirent *ent;
	MyStringAnsi newDirName;
	MyStringAnsi fullPath;
	MyStringAnsi vfsPath;



	/* print all the files and directories within directory */
	while ((ent = readdir(dir)) != nullptr)
	{
		if (ent->d_name[0] == '.')
		{
			continue;
		}

		switch (ent->d_type)
		{
		case DT_REG:

			//printf ("%s (file)\n", ent->d_name);                    
#ifdef _MSC_VER
			fullPath = dir->patt; //full path using Windows dirent
			fullPath = fullPath.SubString(0, fullPath.length() - 1);
#else
			fullPath = dirName;
			if (fullPath.GetLastChar() != '/')
			{
				fullPath += '/';
			}
#endif
			fullPath += ent->d_name;

			fullPath.Replace("\\", "/");

			vfsPath = fullPath;
			vfsPath = vfsPath.SubString(startDirName.length(),
				fullPath.length() - startDirName.length());


			//printf("Full file path: %s\n", fullPath.c_str());
			//printf("VFS file path: %s\n", vfsPath.c_str());

			this->CreateVFSFile(vfsPath, fullPath);

			break;

		case DT_DIR:
			//printf ("%s (dir)\n", ent->d_name);

			newDirName = dirName;
			if (newDirName.GetLastChar() != '/')
			{
				newDirName += '/';
			}
			newDirName += ent->d_name;
			this->AddDirectory(newDirName, startDirName);

			break;

		default:
			//printf ("%s:\n", ent->d_name);
			break;
		}
	}


	closedir(dir);
   
	
}

#ifdef __ANDROID_API__
void VFS::AddDirectoryAndroid(const MyStringAnsi &dirName, const MyStringAnsi &startDirName)
{
    AAssetDir* assetDir = AAssetManager_openDir(AndroidUtils::android_asset_manager, dirName.c_str());
    const char* filename = (const char*)NULL;
    while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL)
    {
        if (filename[0] == '.')
        {
            continue;
        }

        MyStringAnsi fullPath = dirName;
        if (fullPath[fullPath.length() - 1] != '/')
        {
			fullPath += '/';
        }

        fullPath += filename;

        MyStringAnsi vfsPath = fullPath;
        vfsPath = vfsPath.SubString(startDirName.length(), fullPath.length() - startDirName.length());



        this->CreateVFSFile(vfsPath, fullPath);

    }
    AAssetDir_close(assetDir);
}
#endif

bool VFS::FileInfo(const MyStringAnsi &fileName, VFS_ARCHIVE_TYPE &archiveType, size_t &fileSize) const
{
	FILE * file = nullptr;
	my_fopen(&file, fileName.c_str(), "rb");
	if (file == nullptr)
	{
#ifdef __ANDROID_API__
		file = file = AndroidUtils::AssetFopen(fileName.c_str(), "rb");
		if (file == nullptr)
		{
			return false;
		}
#else
		return false;
#endif
	}

	fseek(file, 0, SEEK_END);
	fileSize = static_cast<size_t>(ftell(file));
	fseek(file, 0, SEEK_SET);

	archiveType = VFS_ARCHIVE_TYPE::NONE;

	if (fileSize > 10)
	{
		char buf[10];
		fread(buf, sizeof(char), 10, file);				
		if ((buf[0] == 'P') && (buf[1] == 'K'))
		{
			archiveType = VFS_ARCHIVE_TYPE::ZIP;
		}
		else if ((buf[0] == 'V') && (buf[1] == 'D'))
		{
			archiveType = VFS_ARCHIVE_TYPE::PACKED_FS;
		}
	}

	fclose(file);

	return true;
}

void VFS::ScanPackedFS(const MyStringAnsi & vfsPath, const MyStringAnsi &fullPath)
{
	this->archiveFiles.push_back(fullPath);

	int i = vfsPath.length() - 1;
	while ((i > 0) && (vfsPath[i] != '/') && (vfsPath[i] != '\\'))
	{
		i--;
	}

	MyStringAnsi archiveVfsDir = vfsPath;
	archiveVfsDir[i + 1] = 0; //cut-off file name

	FILE * f = nullptr;
	my_fopen(&f, fullPath.c_str(), "rb");
	if (f == nullptr)
	{
		return;
	}

	char header[2];
	fread(header, sizeof(char), 2, f);
	
	uint32_t fileCount = 0;
	fread(&fileCount, sizeof(uint32_t), 1, f);

	for (uint32_t fi = 0; fi < fileCount; fi++)
	{
		uint32_t fileSize = 0;
		uint32_t dataOffset = 0;
		uint16_t nameLength = 0;

		fread(&fileSize, sizeof(uint32_t), 1, f);
		fread(&dataOffset, sizeof(uint32_t), 1, f);
		fread(&nameLength, sizeof(uint16_t), 1, f);

		char * n = new char[nameLength + 1];
		fread(n, sizeof(char), nameLength, f);
		n[nameLength] = 0;

		MyStringAnsi path = MyStringAnsi::CreateFromMoveMemory(n, nameLength + 1, nameLength);

		VFS_FILE * vfsFile = new VFS_FILE;
		vfsFile->fileSize = static_cast<size_t>(fileSize);
		vfsFile->archiveOffset = dataOffset;
		vfsFile->archiveFileIndex = static_cast<uint16_t>(this->archiveFiles.size() - 1);
		vfsFile->filePtr = nullptr;
		vfsFile->archiveType = VFS_ARCHIVE_TYPE::PACKED_FS;
		//vfsFile->filePath = my_strdup(path.c_str()); //NEED RELEASE !

		//arch->parent = file;


		int i = path.length() - 1;
		while ((i > 0) && (path[i] != '/') && (path[i] != '\\'))
		{
			i--;
		}
		vfsFile->name = my_strdup(path.c_str() + i + 1);

		this->fileSystem->AddFile(path, vfsFile);
	}

	fclose(f);
}

void VFS::ScanZipArchive(const MyStringAnsi & vfsPath, const MyStringAnsi &fullPath)
{
	this->archiveFiles.push_back(fullPath);

	int i = vfsPath.length() - 1;
	while ((i > 0) && (vfsPath[i] != '/') && (vfsPath[i] != '\\'))
	{
		i--;
	}
	
	MyStringAnsi archiveVfsDir = vfsPath;
	archiveVfsDir[i + 1] = 0; //cut-off file name


	unzFile zipFile = unzOpen(fullPath.c_str());
	

	int res = unzGoToFirstFile(zipFile);

	unz_file_info info;
	char fileNameInArchive[255 + 1];
	while(true)
	{
		unzGetCurrentFileInfo(zipFile, &info, fileNameInArchive, 255, nullptr, 0, nullptr, 0);
		//unzGetFilePos(file, &pos);
		
		if (fileNameInArchive[info.size_filename - 1] != '/')
		{
			MyStringAnsi path = archiveVfsDir;

			path += fileNameInArchive;
			
			
			VFS_FILE * vfsFile = new VFS_FILE;
			vfsFile->fileSize = static_cast<size_t>(info.uncompressed_size);
			vfsFile->archiveOffset = unzGetOffset(zipFile);
			vfsFile->archiveFileIndex = static_cast<uint16_t>(this->archiveFiles.size() - 1);
			vfsFile->filePtr = nullptr;
			vfsFile->archiveType = VFS_ARCHIVE_TYPE::ZIP;
			//vfsFile->filePath = my_strdup(path.c_str()); //NEED RELEASE !
			
			//arch->parent = file;

			
			int i = path.length() - 1;
			while ((i > 0) && (path[i] != '/') && (path[i] != '\\'))
			{
				i--;
			}
			vfsFile->name = my_strdup(path.c_str() + i + 1);

			this->fileSystem->AddFile(path, vfsFile);
		}
			
		res = unzGoToNextFile(zipFile);
		if (res == UNZ_END_OF_LIST_OF_FILE) break;
	}

	unzClose(zipFile);
}

void VFS::CreateVFSFile(MyStringAnsi & vfsPath, const MyStringAnsi & fullPath)
{
	VFS_ARCHIVE_TYPE archiveType;
	size_t fileSize;
	if (this->FileInfo(fullPath, archiveType, fileSize) == false)
    {
        return;
    }

	if (archiveType == VFS_ARCHIVE_TYPE::ZIP)
	{
		this->ScanZipArchive(vfsPath, fullPath);
		return;
	}

	if (archiveType == VFS_ARCHIVE_TYPE::PACKED_FS)
	{
		this->ScanPackedFS(vfsPath, fullPath);
		return;
	}
	
	VFS_FILE * vfsFile = new VFS_FILE;
	vfsFile->fileSize = fileSize;
	vfsFile->archiveFileIndex = std::numeric_limits<uint16_t>::max();
	vfsFile->archiveOffset = std::numeric_limits<unsigned long>::max();
	vfsFile->filePtr = nullptr;	
	vfsFile->archiveType = VFS_ARCHIVE_TYPE::NONE;
	//vfsFile->filePath = my_strdup(vfsPath.c_str()); //NEED RELEASE !

	int i = vfsPath.length() - 1;
	while ((i > 0) && (vfsPath[i] != '/') && (vfsPath[i] != '\\'))
	{
		i--;
	} 
	vfsFile->name = my_strdup(vfsPath.c_str() + i + 1);
	
	this->fileSystem->AddFile(vfsPath, vfsFile);
	
}

