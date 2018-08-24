#include "./OSUtils.h"



#ifdef _WIN32
#include "./win_dirent.h"
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif

#ifdef _WIN32
#include "./WinUtils.h"
#elif __APPLE__
#include "./OS_iOS/IOSUtils.h"
#elif __ANDROID_API__
#include "./OS_Android/AndroidUtils.h"
#endif

#include "./VFS.h"

std::shared_ptr<OSUtils> OSUtils::instance = nullptr;

void OSUtils::Init(const OSInfo & info)
{
	if (OSUtils::instance == nullptr)
	{
#ifdef _WIN32
		OSUtils::instance = std::make_shared<WinUtils>();
#elif __APPLE__ 
		OSUtils::instance = std::make_shared<IOSUtils>();
#elif __ANDROID_API__
		OSUtils::instance = std::make_shared<AndroidUtils>();
#endif 

		OSUtils::instance->info = info;

	}
}


std::shared_ptr<OSUtils> OSUtils::Instance()
{
	return OSUtils::instance;
}

//pridat metodu mkdir, ktera vyrobi pouze 1 adresar a neresi cestu
// - pro VFS copy rychlejsi, akorat zmenit poradi ve vfs.dirs

int OSUtils::CreateDir(const MyStringAnsi & path, mode_t mode)
{
	int res = 0;
#if defined _WIN32
	res = _mkdir(path.c_str());
#else
	res = mkdir(path.c_str(), mode);
#endif
	return res;
}

//https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
int OSUtils::CreatePath(MyStringAnsi path, mode_t mode)
{	
	if (path.GetLastChar() != '/')
	{
		path += '/';
	}
	char * file_path = my_strdup(path.c_str());
	

	bool created = false;
	
	int res = 0;
	for (char * p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/'))
	{
		*p = '\0';

	#if defined _WIN32	
			res = _mkdir(file_path);											
	#else
			res = mkdir(file_path, mode);
	#endif
		/*
		if ((res == -1) && (errno != EEXIST))
		{
			*p = '/';
			free(file_path);
			return -1;
		}		
		*/
		if (res == 0)
		{
			created = true;
		}


		*p = '/';
	}

	free(file_path);

	if (created)
	{
		return 1;
	}

	return 0;
}

int OSUtils::RemoveDir(const MyStringAnsi & path)
{
	DIR *dir = opendir(path.c_str());
	if (dir == nullptr)
	{
		return 1;
	}

	struct dirent *entry = nullptr;
	while ((entry = readdir(dir)))
	{				
		if (entry->d_name[0] != '.')
		{
			//sprintf(abs_path, "%s/%s", path, entry->d_name);
			MyStringAnsi abs_path = path;
			abs_path += '/';
			abs_path += entry->d_name;

			DIR *sub_dir = nullptr;
			if ((sub_dir = opendir(abs_path.c_str())))
			{
				closedir(sub_dir);
				this->RemoveDir(abs_path);
			}
			else
			{
				this->RemoveFile(abs_path);				
			}
		}
	}
	rmdir(path.c_str());

	return 0;
}

int OSUtils::RemoveFile(const MyStringAnsi & path)
{
	FILE *file = nullptr;
	my_fopen(&file, path.c_str(), "r");
	if (file != nullptr)
	{
		fclose(file);
		remove(path.c_str());
	}

	return 0;
}
