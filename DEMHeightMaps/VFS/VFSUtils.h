#ifndef VFS_UTILS_H
#define VFS_UTILS_H


#include <string>
#include <sstream>
#include <vector>
#include <iterator>


#ifdef _MSC_VER
#ifndef my_fopen 
#define my_fopen(a, b, c) fopen_s(a, b, c)	
#endif
#ifndef my_fseek 
#define my_fseek(a, b, c) _fseeki64(a, b, c)	
#endif
#ifndef my_ftell
#define my_ftell(a) _ftelli64(a)	
#endif
#ifndef my_localtime
#define my_localtime(a, b) localtime_s(a, b)	
#endif
#ifndef my_gmtime
#define my_gmtime(a, b) gmtime_s(a, b)		
#endif
#ifndef my_strdup
#define my_strdup(a) _strdup(a)
#endif
#else
#ifndef my_fopen 
#define my_fopen(a, b, c) (*a = fopen(b, c))
#endif
#ifndef my_fseek 
#define my_fseek(a, b, c) fseeko64(a, b, c)	
#endif
#ifndef my_ftell
#define my_ftell(a) ftello64(a)	
#endif
#ifndef my_localtime
#define my_localtime(a, b) (a = localtime(b))	
#endif
#ifndef my_gmtime
#define my_gmtime(a, b) {struct tm * tmp = gmtime(b); memcpy(a, tmp, sizeof(struct tm));}
#endif
#ifndef my_strdup
#define my_strdup(a) strdup(a)
#endif
#endif



template<typename Out>
void split(const std::string &s, char delim, Out result) 
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) 
	{
		*(result++) = item;
	}
}


inline std::vector<std::string> split(const std::string &s, char delim) 
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

inline void replaceAll(std::string &s, const std::string &search, const std::string &replace) 
{
	for (size_t pos = 0; ; pos += replace.length()) 
	{
		// Locate the substring to replace
		pos = s.find(search, pos);
		if (pos == std::string::npos) break;
		// Replace by erasing and inserting
		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}


inline std::string loadFromFile(const std::string & fileName)
{
	FILE *f = NULL;  //pointer to file we will read in
	my_fopen(&f, fileName.c_str(), "rb");
	if (f == NULL)
	{
		printf("Failed to open file: \"%s\"\n", fileName.c_str());
		return "";
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char * data = new char[size + 1];
	fread(data, sizeof(char), size, f);
	fclose(f);

	data[size] = 0;
	std::string tmp = std::string(data);
	delete[] (data);

	return tmp;

}


#endif
