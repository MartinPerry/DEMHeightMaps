#include "./ZipWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

/*-----------------------------------------------------------
Function:	GetActualTime
Returns:
	time info

Obtain actual time info from system clock
-------------------------------------------------------------*/
tm GetActualTime()
{
	time_t rawTime;
	
	#ifdef _MSC_VER
		struct tm timeinfo;
	
		time (&rawTime);
		my_localtime(&timeinfo, &rawTime);
		return timeinfo;
	#else 
		struct tm * timeinfo = NULL;
		
		time (&rawTime);
		my_localtime(timeinfo, &rawTime);
		return *timeinfo;
	#endif
}

//============================================================================
//=========================== Zip File == ====================================
//============================================================================

ZipFile::ZipFile(const std::string & fileName)
{
	this->fileName = fileName;
}

/*-----------------------------------------------------------
Function:	GetFileName
Returns:
	file name of archive

Get fileName of archive
-------------------------------------------------------------*/
const std::string & ZipFile::GetFileName()
{
	return this->fileName;
}

/*-----------------------------------------------------------
Function:	AddFile
Parameters:	
	[in] fileName - path to physical file.
	[in] fileNameInZip - path to zipped file in archive
Returns:
	true if OK, false if packing failed

Add new file to archive.
-------------------------------------------------------------*/
bool ZipFile::AddFile(const std::string & fileName, const std::string & fileNameInZip)
{
	zip_fileinfo zi;
	memset(&zi, 0, sizeof(zip_fileinfo));

	tm timeInfo = GetActualTime();
	zi.tmz_date.tm_hour = timeInfo.tm_hour;
	zi.tmz_date.tm_mday = timeInfo.tm_mday;
	zi.tmz_date.tm_min = timeInfo.tm_min;
	zi.tmz_date.tm_mon = timeInfo.tm_mon;
	zi.tmz_date.tm_sec = timeInfo.tm_sec;
	zi.tmz_date.tm_year = timeInfo.tm_year;


	zipOpenNewFileInZip(this->zipPtr, fileNameInZip.c_str(), &zi, NULL, 0, NULL, 0, NULL, 0, 0);

	FILE * fileInZip = NULL;
	my_fopen(&fileInZip, fileName.c_str(), "rb");
	if (fileInZip == NULL)
	{	
		 return false;
	}

	int err;

	char * buf = (char *)malloc(BUFFER_SIZE * sizeof(char));
	int size_read = 0;
	int fileSize = 0;
	do
    {
		err = ZIP_OK;
        size_read = (int)fread(buf, 1, BUFFER_SIZE, fileInZip);
		fileSize += size_read;
		if (size_read < BUFFER_SIZE)
		{
			if (feof(fileInZip) == 0)
			{				
				err = ZIP_ERRNO;
			}
		}

        if (size_read > 0)
		{
			err = zipWriteInFileInZip (this->zipPtr, buf, size_read);            

        }
	} while ((err == ZIP_OK) && (size_read>0));

	fclose(fileInZip);

	zipCloseFileInZip(this->zipPtr);

	free(buf);

	if (err != ZIP_OK) 
	{
		return false;
	}
	
	FileInfo fi;
	fi.fileName = fileNameInZip;
	fi.size = fileSize;

	this->files[fileNameInZip] = fi;	

	return true;


}

bool ZipFile::AddFile(const void * buffer, unsigned int bufferSize, const std::string & fileNameInZip)
{
	zip_fileinfo zi;
	memset(&zi, 0, sizeof(zip_fileinfo));

	tm timeInfo = GetActualTime();
	zi.tmz_date.tm_hour = timeInfo.tm_hour;
	zi.tmz_date.tm_mday = timeInfo.tm_mday;
	zi.tmz_date.tm_min = timeInfo.tm_min;
	zi.tmz_date.tm_mon = timeInfo.tm_mon;
	zi.tmz_date.tm_sec = timeInfo.tm_sec;
	zi.tmz_date.tm_year = timeInfo.tm_year;


	zipOpenNewFileInZip(this->zipPtr, fileNameInZip.c_str(), &zi, NULL, 0, NULL, 0, NULL, 0, 0);

	int err = zipWriteInFileInZip (this->zipPtr, buffer, bufferSize);    

	zipCloseFileInZip(this->zipPtr);
	
	if (err != ZIP_OK) 
	{
		return false;
	}
	
	FileInfo fi;
	fi.fileName = fileNameInZip;
	fi.size = bufferSize;

	this->files[fileNameInZip] = fi;	

	return true;


}

unsigned int ZipFile::GetFileSize(const std::string & fileNameInZip)
{	
	FileInfo fi = this->files[fileNameInZip];
	return fi.size;
}

unsigned int ZipFile::GetArchiveFilesSize()
{
	unsigned int size = 0;
	std::unordered_map<std::string, FileInfo>::iterator i;
	for(i = this->files.begin() ; i != this->files.end(); ++i )	
	{
		size += i->second.size;		
	}

	return size;

}

/*-----------------------------------------------------------
Function:	OpenFile
Parameters:	
	[in] fileNameInZip - path to file in archive
	[in / out] buffer - pointer to file buffer.
	[out] bufferSize - size of buffer

Open and extract file from archive. File is extracted to 
buffer. Buffer must be freed extrenally by "free(buffer)"
-------------------------------------------------------------*/
void ZipFile::OpenFile(const std::string & fileNameInZip, void ** buffer, int * bufferSize)
{
	*buffer = NULL;

	if (this->writeMode)
	{		
		return;
	}

		
	
	int res = unzLocateFile(this->zipPtr, fileNameInZip.c_str(), 0);

	if (res != UNZ_OK)
	{
		return;
	}

	FileInfo fi = this->files[fileNameInZip];
	if (bufferSize != NULL)
	{ 
		*bufferSize = fi.size;
	}
	*buffer = malloc(fi.size);

	res = unzOpenCurrentFile(this->zipPtr);
	if (res != UNZ_OK)
	{
		return;
	}

	unzReadCurrentFile(this->zipPtr, *buffer, fi.size);

	unzCloseCurrentFile(this->zipPtr);
}

bool ZipFile::FileExist(const std::string & fileNameInZip)
{
	return !(this->files.find(fileNameInZip) == this->files.end()); // true == does not exist 
}

/*-----------------------------------------------------------
Function:	LoadContent

Load content of archive. All files are iterated and stored in
std::unordered_map
-------------------------------------------------------------*/
void ZipFile::LoadContent()
{
	if (this->writeMode)
	{
		return;
	}

	unz_file_info info;
			
	char * fileNameInArchive = new char[255];
	int res = 0;

	res = unzGoToFirstFile(this->zipPtr);
	if (res == UNZ_END_OF_LIST_OF_FILE) return;

	while(true)
	{

		unzGetCurrentFileInfo(this->zipPtr, &info, fileNameInArchive, 255, NULL, 0, NULL, 0);
				

		if (fileNameInArchive[info.size_filename - 1] != '/')
		{
			FileInfo fi;
			fi.fileName = fileNameInArchive;
			fi.size = static_cast<uint32_t>(info.uncompressed_size);

			this->files[fileNameInArchive] = fi;	
		}

		res = unzGoToNextFile(this->zipPtr);
		if (res == UNZ_END_OF_LIST_OF_FILE) break;
	}

	delete[] fileNameInArchive;
}

//============================================================================
//=========================== Zip Wrapper ====================================
//============================================================================

/*-----------------------------------------------------------
Function:	ctor

ZipWrapper ctor
-------------------------------------------------------------*/
ZipWrapper::ZipWrapper()
{
}

/*-----------------------------------------------------------
Function:	dtor

ZipWrapper dtor
-------------------------------------------------------------*/
ZipWrapper::~ZipWrapper()
{
}

/*-----------------------------------------------------------
Function:	CloseArchive
Parameters:	
	[in] zf - pointer to opened ZipFile

Close opened arhive
-------------------------------------------------------------*/
void ZipWrapper::CloseArchive(ZipFile * zf)
{
	if (zf == NULL)
	{
		return;
	}

	if (zf->writeMode)
	{
		zipClose(zf->zipPtr, NULL);
	}
	else 
	{
		unzClose(zf->zipPtr);
	}

	delete zf;
	zf = NULL;
}

/*-----------------------------------------------------------
Function:	OpenArchive
Parameters:	
	[in] fileName - archive file name
	[in] mode - open mode (READ / WRITE / WRITE_APPEND)
Returns:
	opened / created ZipFile; NULL if nothing is opened

Create or open existing archive
mode can be: 
READ - open only for reading (uses unzip.h)
WRITE - create new archive (old one with same name will be overwritten) (uses zip.h)
WRITE_APPEND - add files to existing archive (uses zip.h)
-------------------------------------------------------------*/
ZipFile * ZipWrapper::OpenArchive(const std::string & fileName, OpenMode mode)
{
	ZipFile * archFile = new ZipFile(fileName);

	if (archFile == NULL)
	{

		return NULL;
	}

	void * zf = NULL;
	if (mode == READ)
	{
		zf = unzOpen(fileName.c_str());
	}
	else 
	{
		int status = 0;
		if (mode == WRITE_APPEND)
		{
			status = APPEND_STATUS_ADDINZIP;
		}

		zf = zipOpen(fileName.c_str(), status);	
	}
	archFile->zipPtr = zf;	
	archFile->writeMode = true;
	if (mode == READ)
	{
		archFile->writeMode = false;
		archFile->LoadContent();
	}
	
		 
	return archFile;
}


