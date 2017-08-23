#ifndef VFS_H
#define VFS_H

#ifdef _MSC_VER
	#if defined(DEBUG)|defined(_DEBUG)
		#pragma comment(lib, "zlibstat_debug.lib")		
	#else
		#pragma comment(lib, "zlibstat_release.lib")		
	#endif	
#endif

#include "VFSUtils.h"

#include <vector>
#include <map>


/*====================================

VFS v. 1.0
Martin Perry 
8.7.2011 

=====================================*/

struct Archived;
struct VFS_FILE;

/*-----------------------------------------------------------
Struct:	Archived

Info about file in archive
-------------------------------------------------------------*/
typedef struct Archived 
{
	char * filePath;	//full path to archive file ... C:/DATA/archive.zip
	void * ptr;				//pointer to the archived file
	unsigned long offset;	//ofset within archived file to the actual file
	int method;				//archivation method of file - unused, for future use
	int compressedSize;		//compressed size

	VFS_FILE * parent;

} Archived;

/*-----------------------------------------------------------
Struct:	VFS_FILE

File within VFS
-------------------------------------------------------------*/
typedef struct VFS_FILE 
{
	char * filePath;	//full path to file in VFS .... C:/DATA/sample.txt (real file) or Sample/Sample.txt (file in archive)
	char * fullName;	//full name ... sample.txt
	char * name;		//name only ... sample
	char * ext;		// extension only ... txt
	Archived * archiveInfo; //archived info, if file not in archive - NULL
	void * filePtr;			//pointer to the opened file

	int fileSize;			//raw size of file
	

} VFS_FILE;

/*-----------------------------------------------------------
Struct:	VFS_DIR

Directory within VFS
-------------------------------------------------------------*/
typedef struct VFS_DIR 
{
	VFS_DIR * parent;				//parent directory
	std::vector<VFS_DIR *> dirs;	//all sub dirs
	std::vector<VFS_FILE *> files;	//all files within dir

	char * name;				//directory name ... SAMPLE
	int numFiles;					//total number of files in directory and all its subdirs
	int numDirs;					//total number of dirs and its subdirs

} VFS_DIR;


/*-----------------------------------------------------------
Typedef: VFS_MODE

Mode of VFS
If mode is DEBUG_MODE, than VFS can also access files 
within real FS (eg: C:/Program Files/...)
-------------------------------------------------------------*/
typedef enum VFS_MODE {DEBUG_MODE = 0, RELEASE_MODE = 1} VFS_MODE;

/*-----------------------------------------------------------
Class:	VFSTree

FileSystem Tree
All files withinf VFS are stored in this tree
Files from archives are stored as if archive will be
unarchived
Eg: VFS/example.zip (file: a.txt, b.txt)
Is stored as: VFS_FILE - a.txt
			  VFS_FILE - b.txt
example.zip file is not stored in tree
-------------------------------------------------------------*/
class VFSTree 
{
	public:
		VFSTree();
		~VFSTree();

		void Release();
		
		bool AddFile(std::string &filePath);
		bool AddFile(std::string &filePath, VFS_FILE * file);
		VFS_FILE * GetFile(const std::string &path);
		VFS_DIR * GetDir(const std::string &path);

		std::vector<VFS_FILE *> GetAllFiles(bool withFilesFromArchives);

		void PrintStructure();

	private:
		std::map<std::string, VFS_FILE *> cachedFiles;
		VFS_DIR * root;

		VFS_DIR * AddDir(VFS_DIR * node, const std::string &dirName);
		VFS_DIR * GetDir(VFS_DIR * node, const std::string &dirName);

		void Release(VFS_DIR * node);
		void ReleaseFile(VFS_FILE * file);

		void GetAllFiles(VFS_DIR * node, int depth, bool withFilesFromArchives, std::vector<VFS_FILE *> & output);
		void PrintStructure(VFS_DIR * node, int depth);
};

/*-----------------------------------------------------------
Class:	VFS

VFS main class
-------------------------------------------------------------*/
class VFS
{
	public:
		static void Initialize(const std::string &dir, VFS_MODE mode = RELEASE_MODE);				//init from default dir
		static void Initialize(VFS_MODE mode = RELEASE_MODE);				//init from default dir
		
		static void Destroy();
		static VFS * GetInstance();

		bool ExistFile(const std::string & fileName);

		std::vector<VFS_FILE *> GetAllFiles();
		std::vector<VFS_FILE *> GetMainFiles();
		void PrintStructure();
		void SetWorkingDir(const std::string & dir);
		const std::string & GetWorkingDir();

		void AddDirectory(const std::string &dir);	//add root directory to the file-system
		
		VFS_FILE * OpenFile(const std::string &path);
		char * GetFileContent(const std::string &path, int * fileSize);
		std::string GetFileString(const std::string &path);
		void CloseFile(VFS_FILE * file);

		void RefreshFile(const std::string &path);

		int Read(void * buffer, size_t elementSize, size_t bytesCount, VFS_FILE * file);
		int ReadString(char * buffer, size_t bytesCount, VFS_FILE * file);
		int ReadEntireFile(void * buffer, VFS_FILE * file);

	private:
		static VFS * single;
		VFSTree * fileSystem;		
		std::vector<VFS_FILE *> debugModeFiles;
		VFS_MODE mode;
		std::string workingDir;

		

		VFS();
		~VFS();

		void Release();

		void AddDirectory(const std::string &dir, const std::string &startDirName);
		void ScanArchive(const std::string &fileName);
		bool FileInfo(const std::string &fileName, bool &archived, int &fileSize);
		void CreateVFSFile(std::string vfsPath, std::string fullPath);
		VFS_FILE * CreateDebugFile(std::string fullPath);
		
		
};

//void vfs_init(const char * dir); //init with default dir
//void vfs_add_directory(const char * dir); //projde cely adresar a vytvori strom
//VFS_FILE * vfs_open(const char * path); //lokalizovat soubor ve stromu a vratit ukazatel na "nodu" stromu
//int vfs_read(void * buffer, int bytesCount, VFS_FILE * file); //podle ukazatele na "nodu" otevrit skutecny soubor
//void vfs_seek(VFS_FILE * file, int seekOrigin, int offset);
//void vfs_close(VFS_FILE * file);

#undef _CRT_SECURE_NO_WARNINGS	//for MSVC - disable MSVC warnings on C functions
#undef _CRT_SECURE_NO_DEPRECATE

#endif