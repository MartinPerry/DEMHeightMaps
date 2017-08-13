#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <cstdio>
#include <errno.h>
#include <string>
#include <vector>

#define DEG_TO_RAD(x) ((x) * 0.0174532925)
#define RAD_TO_DEG(x) ((x) * 57.2957795)

class Utils 
{
	public:
		
		template <typename T>
		static T * LoadFile(const std::string & fileName, long * dataSize);

		template <typename T>
		static T * LoadFile(const std::string & fileName);

		template <typename T>
		static void SaveToFile(T * data, long dataSize, const std::string & fileName);

		template <typename T>
		static void SaveToFile(std::vector<T> & data, const std::string & fileName);
		
		static double MapRange(double a1, double a2, double b1, double b2, double s);
};


template <typename T>
void Utils::SaveToFile(T * data, long dataSize, const std::string & fileName)
{
	if (data == NULL)
	{
		return;
	}	
	FILE * f = NULL;
	my_fopen(&f, fileName.c_str(), "wb");
	if (f == NULL)
	{		
		printf("Failed to open file %s (%s)", fileName.c_str(), strerror(errno));
		return;
	}
	fwrite(data, sizeof(T), dataSize, f);
	fclose(f);
};

template <typename T>
void Utils::SaveToFile(std::vector<T> & data, const std::string & fileName)
{
	Utils::SaveToFile(&data[0], data.size(), fileName);
}

template <typename T>
T * Utils::LoadFile(const std::string & fileName)
{
	return Utils::LoadFile<T>(fileName, NULL);
};

template <typename T>
T * Utils::LoadFile(const std::string & fileName, long * dataSize)
{
	FILE * f = NULL;
	my_fopen(&f, fileName.c_str(), "rb");
	if (f == NULL)
	{
		printf("Failed to open file %s (%s)", fileName.c_str(), strerror(errno));
		return NULL;
	}
	fseek(f, 0L, SEEK_END);
	long tmpSize = ftell(f);
	fseek(f, 0L, SEEK_SET);	

	tmpSize /= sizeof(T);
	T * origData = new T[tmpSize];
	fread(origData, sizeof(T), tmpSize, f);
	fclose(f);		

	if (dataSize != NULL)
	{
		*dataSize = tmpSize;
	}

	return origData;
};



#endif
