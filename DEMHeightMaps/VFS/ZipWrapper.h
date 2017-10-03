#ifndef ZIP_WRAPPER_H
#define ZIP_WRAPPER_H

#define BUFFER_SIZE 100


#ifdef _MSC_VER
#ifdef _WIN64
#pragma comment(lib, "zlibstat_x64.lib")				
#else
#pragma comment(lib, "zlibstat.lib")			
#endif
#endif


#include <string>
#include <time.h>
#include <unordered_map>

#include "VFSUtils.h"

tm GetActualTime();

typedef enum OpenMode { WRITE = 0, WRITE_APPEND = 1, READ = 2 } OpenMode;



struct ZipFile 
{	
	const std::string & GetFileName();

	bool AddFile(const std::string & fileName, const std::string & fileNameInZip);
	bool AddFile(const void * buffer, unsigned int bufferSize, const std::string & fileNameInZip);

	void OpenFile(const std::string & fileNameInZip, void ** buffer, int * bufferSize);
	bool FileExist(const std::string & fileNameInZip);

	unsigned int GetFileSize(const std::string & fileNameInZip);
	unsigned int GetArchiveFilesSize();

	friend class ZipWrapper;

	private:	
		typedef struct FileInfo 
		{
			std::string fileName;
			unsigned int size;
		} FileInfo;

		void * zipPtr;		
		bool writeMode;
		std::string fileName;

		std::unordered_map<std::string, FileInfo> files;
		

		ZipFile(const std::string & fileName);
		void LoadContent();
};

class ZipWrapper 
{
	public:
		ZipWrapper();
		~ZipWrapper();

		ZipFile * OpenArchive(const std::string & fileName, OpenMode mode);
		void CloseArchive(ZipFile * zf);

	private:		

};

#endif