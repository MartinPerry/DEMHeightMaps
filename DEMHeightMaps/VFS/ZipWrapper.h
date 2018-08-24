#ifndef ZIP_WRAPPER_H
#define ZIP_WRAPPER_H

#define ZIP_BUFFER_SIZE 100

#ifdef _MSC_VER

	#if defined(DEBUG)|defined(_DEBUG)
		#pragma comment(lib, "zlib.lib")		
	#else
		#pragma comment(lib, "zlib.lib")		
	#endif	
#endif

#include <time.h>
#include <unordered_map>

#include "../Strings/MyString.h"
//#include "../../Macros.h"

tm GetActualTime();

typedef enum OpenMode { WRITE = 0, WRITE_APPEND = 1, READ = 2 } OpenMode;



struct ZipFile 
{	
	const MyStringAnsi & GetFileName();

	bool AddFile(const MyStringAnsi & fileName, const MyStringAnsi & fileNameInZip);
	bool AddFile(const void * buffer, unsigned int bufferSize, const MyStringAnsi & fileNameInZip);

	void OpenFile(const MyStringAnsi & fileNameInZip, void ** buffer, int * bufferSize);
	bool FileExist(const MyStringAnsi & fileNameInZip);

	unsigned int GetFileSize(const MyStringAnsi & fileNameInZip);
	unsigned int GetArchiveFilesSize();

	friend class ZipWrapper;

	private:	
		typedef struct FileInfo 
		{
			MyStringAnsi fileName;
			size_t size;
		} FileInfo;

		void * zipPtr;		
		bool writeMode;
		MyStringAnsi fileName;

		std::unordered_map<MyStringAnsi, FileInfo> files;
		

		ZipFile(const MyStringAnsi & fileName);
		void LoadContent();
};

class ZipWrapper 
{
	public:
		ZipWrapper();
		~ZipWrapper();

		ZipFile * OpenArchive(const MyStringAnsi & fileName, OpenMode mode);
		void CloseArchive(ZipFile * zf);

	private:		

};

#endif