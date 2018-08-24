#ifndef _OS_UTILS_H_
#define _OS_UTILS_H_

#include <memory>

#include "../Strings/MyStringAnsi.h"


#ifdef _WIN32
	typedef uint32_t mode_t;
#endif

class OSUtils
{
public:

	typedef struct OSInfo
	{
		MyStringAnsi installPath;
		MyStringAnsi documentPath;

		int screenDPI;
		int baseFontSize;

	} OSInfo;

    virtual ~OSUtils() = default;
    
	static void Init(const OSInfo & info);
	static std::shared_ptr<OSUtils> Instance();
	
	virtual MyStringAnsi GetDocumentsDataPath() const = 0;
	virtual MyStringAnsi GetDocumentsFilePath(const MyStringAnsi & file) const = 0;

    virtual MyStringAnsi GetInstallDataPath() const = 0;
    virtual MyStringAnsi GetInstallFilePath(const MyStringAnsi & file) const = 0;


	virtual int GetScreenDPI() const = 0;
	virtual int GetBaseFontSize() const = 0;

	virtual void OpenURL(const MyStringAnsi & url) const = 0;

	int CreateDir(const MyStringAnsi & path, mode_t mode = 0777);
	int CreatePath(MyStringAnsi path, mode_t mode = 0777);
	int RemoveDir(const MyStringAnsi & path);
	int RemoveFile(const MyStringAnsi & path);


protected:
	static std::shared_ptr<OSUtils> instance;

	OSInfo info;
	
};

#endif
