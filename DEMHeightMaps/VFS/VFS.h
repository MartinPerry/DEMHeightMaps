#ifndef VFS_H
#define VFS_H

#ifdef _MSC_VER
	#if defined(DEBUG)|defined(_DEBUG)
		#pragma comment(lib, "zlib.lib")		
	#else
		#pragma comment(lib, "zlib.lib")		
	#endif	
#endif

#ifdef _MSC_VER	
	#ifndef my_fopen 
		#define my_fopen(a, b, c) fopen_s(a, b, c)	
	#endif
	#ifndef my_strdup
		#define my_strdup(a) _strdup(a)
	#endif
#else	
	#ifndef my_fopen 
		#define my_fopen(a, b, c) (*a = fopen(b, c))
	#endif
	#ifndef my_strdup
		#define my_strdup(a) strdup(a)
	#endif
#endif

#include <vector>
#include "../Strings/MyString.h"

/*====================================

VFS v. 1.0
Martin Perry 
8.7.2011 

VFS v. 2.0
Martin Perry
5.6.2018


=====================================*/

struct VFS_DIR;

typedef enum VFS_ARCHIVE_TYPE {
	NONE = 0,
	ZIP = 1,
	PACKED_FS = 2

} VFS_ARCHIVE_TYPE;

/*-----------------------------------------------------------
Struct:	VFS_FILE

File within VFS
-------------------------------------------------------------*/
typedef struct VFS_FILE 
{
	const char * name;	//file name with extension (eg. Sample.txt)
						//must be released with free()
	
	VFS_DIR * dir;		//pointer to parent directory
		
	uint16_t archiveFileIndex; //index to archive file path
	uint8_t archiveType;
	unsigned long archiveOffset; //ofset within archived file to the actual file

	void * filePtr;			//pointer to the opened file in OS file system or inside archive

	size_t fileSize;			//raw size of file

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

	const char * name;				//directory name (eg. SAMPLE)
									//must be released with free()	
} VFS_DIR;


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
		
		//bool AddFile(const MyStringAnsi &filePath);
		bool AddFile(MyStringAnsi & vfsPath, VFS_FILE * file); 
		VFS_FILE * GetFile(const MyStringAnsi &path) const;
		VFS_DIR * GetDir(const MyStringAnsi &path) const;
		
		MyStringAnsi GetFilePath(VFS_FILE * f) const;

		std::vector<VFS_FILE *> GetAllFiles(bool withFilesFromArchives) const;

		void PrintStructure() const;

	private:		
		VFS_DIR * root;

		VFS_DIR * AddDir(VFS_DIR * node, const char * dirName);
		VFS_DIR * GetDir(VFS_DIR * node, const char * dirName) const;

		void Release(VFS_DIR * node);
		void ReleaseFile(VFS_FILE * file);

		void GetAllFiles(VFS_DIR * node, int depth, bool withFilesFromArchives, std::vector<VFS_FILE *> & output) const;
		void PrintStructure(VFS_DIR * node, int depth) const;
};

/*-----------------------------------------------------------
Class:	VFS

VFS main class
-------------------------------------------------------------*/
class VFS
{
	public:
		static void InitializeEmpty();
		static void InitializeRaw(const MyStringAnsi &dir);
		static void InitializeFull(const MyStringAnsi &dir);				//init from default dir
		
		static void Destroy();
		static VFS * GetInstance();

		bool ExistFile(const MyStringAnsi & fileName) const;
		bool IsFileInArchive(const MyStringAnsi & fileName) const;

		std::vector<VFS_FILE *> GetAllFiles() const;
		std::vector<VFS_FILE *> GetMainFiles() const;
		void PrintStructure() const;
		void SaveDirStructure(const MyStringAnsi & fileName) const;
		const char * GetFileName(VFS_FILE * f) const;
		const char * GetFileExt(VFS_FILE * f) const;
		MyStringAnsi GetFilePath(VFS_FILE * f) const;

		void AddDirectory(const MyStringAnsi &dir);	//add root directory to the file-system
		void AddHighPriorityRawDirectory(const MyStringAnsi &dir);	//add root directory to the file-system - data are not loaded from this
		void AddRawDirectory(const MyStringAnsi &dir, int priority = -1);	//add root directory to the file-system - data are not loaded from this

		MyStringAnsi GetRawFileFullPath(const MyStringAnsi &path) const;

		FILE * GetRawFile(const MyStringAnsi &path) const;
		
		char * GetFileContent(const MyStringAnsi &path, size_t * fileSize) const;
		MyStringAnsi GetFileString(const MyStringAnsi &path) const;
		void CloseFile(VFS_FILE * file) const;

		bool CopySingleFile(const MyStringAnsi & src, const MyStringAnsi & dest) const;
		int CopyAllFilesFromDir(const MyStringAnsi & dirPath, const MyStringAnsi & destPath) const;
		void PackStructure(const MyStringAnsi & outputFile) const;

		void RefreshFile(const MyStringAnsi &path);

		int Read(void * buffer, size_t elementSize, size_t bytesCount, VFS_FILE * file) const;
		int ReadString(char * buffer, size_t bytesCount, VFS_FILE * file) const;
		int ReadEntireFile(void ** buffer, VFS_FILE * file) const;

	private:
		static VFS * single;
		VFSTree * fileSystem;		
		//std::vector<VFS_FILE *> debugModeFiles;
		
		std::vector<MyStringAnsi> initDirs; //store all directories that are used as VFS init dir
										    //VFS can be created from multiple dirs and one VFS is created
											//so you can obtain file within VFS

		std::vector<MyStringAnsi> archiveFiles; //store all "archive" full OS file paths that are used in VFS

		
	

		VFS();
		~VFS();

		void Release();

		void SaveDirStructure(VFS_DIR * d, const MyStringAnsi & dirPath, MyStringAnsi & data) const;

		int CopyAllFilesFromDir(VFS_DIR * d, const MyStringAnsi & destPath) const;
		int CopyAllFilesFromRawDir(const MyStringAnsi & dirPath, const MyStringAnsi & destPath) const;
		

		VFS_FILE * OpenFile(const MyStringAnsi &path, VFS_FILE * temporary) const;

		void AddDirectory(const MyStringAnsi &dir, const MyStringAnsi &startDirName);
		
		void ScanZipArchive(const MyStringAnsi & vfsPath, const MyStringAnsi &fullPath);
		void ScanPackedFS(const MyStringAnsi & vfsPath, const MyStringAnsi &fullPath);
		
		bool FileInfo(const MyStringAnsi &fileName, VFS_ARCHIVE_TYPE &archiveType, size_t &fileSize) const;
		void CreateVFSFile(MyStringAnsi & vfsPath, const MyStringAnsi & fullPath);
		

#ifdef __ANDROID_API__
		void AddDirectoryAndroid(const MyStringAnsi &dir, const MyStringAnsi &startDirName);
		bool CopySingleFileAndroid(const MyStringAnsi & src, const MyStringAnsi & dest) const;
#endif
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
