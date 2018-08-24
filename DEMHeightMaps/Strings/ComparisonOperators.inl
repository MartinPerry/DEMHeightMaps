#ifndef _COMPARISON_OPERATORS_H_
#define _COMPARISON_OPERATORS_H_

#include "./MyStringAnsi.h"
#include "./MySmallStringAnsi.h"

#include <cstring>

//================================================================

//define forward declaration
#define GENERATE_FORWARD_OPERATORS(p1, p2) \
	bool operator ==(const p1 & str1, const p2 & str2); \
	bool operator !=(const p1 & str1, const p2 & str2); \
	bool operator >(const p1 & str1, const p2 & str2); \
	bool operator <(const p1 & str1, const p2 & str2);

#define GENERATE_FORWARD_OPERATORS_2(p1, p2) \
	bool operator ==(const p1 str1, const p2 str2); \
	bool operator !=(const p1 str1, const p2 str2); \
	bool operator >(const p1 str1, const p2 str2); \
	bool operator <(const p1 str1, const p2 str2);

#define GENERATE_BODY_OPERATORS(p1, p2) \
	inline bool operator ==(const p1 & str1, const p2 & str2){ \
		if (str1.length() != str2.length()) return false; \
		return (strcmp(str1.c_str(), str2.c_str()) == 0); \
	}; \
	inline bool operator !=(const p1 & str1, const p2 & str2){ \
		if (str1.length() == str2.length()) return false; \
			return (strcmp(str1.c_str(), str2.c_str()) != 0); \
	}; \
	inline bool operator >(const p1 & str1, const p2 & str2){ \
		return (strcmp(str1.c_str(), str2.c_str()) > 0); \
	}; \
	inline bool operator <(const p1 & str1, const p2 & str2) { \
			return (strcmp(str1.c_str(), str2.c_str()) < 0); \
	};

//================================================================

GENERATE_FORWARD_OPERATORS(MyStringAnsi, MyStringAnsi);
GENERATE_FORWARD_OPERATORS(MySmallStringAnsi, MySmallStringAnsi);
GENERATE_FORWARD_OPERATORS(MyStringAnsi, MySmallStringAnsi);
GENERATE_FORWARD_OPERATORS(MySmallStringAnsi, MyStringAnsi);
GENERATE_FORWARD_OPERATORS(MyStringAnsi, std::string);
GENERATE_FORWARD_OPERATORS(std::string, MyStringAnsi);
GENERATE_FORWARD_OPERATORS(std::string, MySmallStringAnsi);
GENERATE_FORWARD_OPERATORS(MySmallStringAnsi, std::string);
GENERATE_FORWARD_OPERATORS_2(char *, MyStringAnsi &);
GENERATE_FORWARD_OPERATORS_2(char *, MySmallStringAnsi &);
GENERATE_FORWARD_OPERATORS_2(MyStringAnsi &, char *);
GENERATE_FORWARD_OPERATORS_2(MySmallStringAnsi &, char *);

//=====================================================================

GENERATE_BODY_OPERATORS(MyStringAnsi, MyStringAnsi);
GENERATE_BODY_OPERATORS(MySmallStringAnsi, MySmallStringAnsi);
GENERATE_BODY_OPERATORS(MyStringAnsi, MySmallStringAnsi);
GENERATE_BODY_OPERATORS(MySmallStringAnsi, MyStringAnsi);
GENERATE_BODY_OPERATORS(MyStringAnsi, std::string);
GENERATE_BODY_OPERATORS(std::string, MyStringAnsi);
GENERATE_BODY_OPERATORS(std::string, MySmallStringAnsi);
GENERATE_BODY_OPERATORS(MySmallStringAnsi, std::string);

//==========================================================



inline bool operator ==(const char * str1, const MyStringAnsi & str2)
{
	if (str1 == nullptr) return false;
	return (strcmp(str1, str2.c_str()) == 0);
};

inline bool operator ==(const MyStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return false;
	return (strcmp(str1.c_str(), str2) == 0);
};


inline bool operator ==(const char * str1, const MySmallStringAnsi & str2)
{
	if (str1 == nullptr) return false;
	return (strcmp(str1, str2.c_str()) == 0);
};

inline bool operator ==(const MySmallStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return false;
	return (strcmp(str1.c_str(), str2) == 0);
};



inline bool operator !=(const char * str1, const MyStringAnsi &str2)
{
	if (str1 == nullptr) return true;
	return (strcmp(str1, str2.c_str()) != 0);
};

inline bool operator !=(const  MyStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return true;
	return (strcmp(str1.c_str(), str2) != 0);
};

inline bool operator !=(const char * str1, const MySmallStringAnsi &str2)
{
	if (str1 == nullptr) return true;
	return (strcmp(str1, str2.c_str()) != 0);
};

inline bool operator !=(const  MySmallStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return true;
	return (strcmp(str1.c_str(), str2) != 0);
};


inline bool operator <(const char * str1, const MyStringAnsi & str2)
{
	if (str1 == nullptr) return true;
	return (strcmp(str1, str2.c_str()) < 0);
};

inline bool operator <(const MyStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return false;
	return (strcmp(str1.c_str(), str2) < 0);
};

inline bool operator <(const char * str1, const MySmallStringAnsi & str2)
{	
	if (str1 == nullptr) return true;
	return (strcmp(str1, str2.c_str()) < 0);
};

inline bool operator <(const MySmallStringAnsi &str1, const char * str2)
{	
	if (str2 == nullptr) return false;
	return (strcmp(str1.c_str(), str2) < 0);
};


inline bool operator >(const char * str1, const MyStringAnsi & str2)
{
	if (str1 == nullptr) return false;
	return (strcmp(str1, str2.c_str()) > 0);
};

inline bool operator >(const MyStringAnsi &str1, const char * str2)
{
	if (str1 == nullptr) return true;
	return (strcmp(str1.c_str(), str2) > 0);
};

inline bool operator >(const char * str1, const MySmallStringAnsi & str2)
{
	if (str2 == nullptr) return false;
	return (strcmp(str1, str2.c_str()) > 0);
};

inline bool operator >(const MySmallStringAnsi &str1, const char * str2)
{
	if (str2 == nullptr) return true;
	return (strcmp(str1.c_str(), str2) > 0);
};

//================================
//undef defined values

#ifdef GENERATE_FORWARD_OPERATORS
	#undef GENERATE_FORWARD_OPERATORS
#endif

#ifdef GENERATE_BODY_OPERATORS
	#undef GENERATE_BODY_OPERATORS
#endif

#endif