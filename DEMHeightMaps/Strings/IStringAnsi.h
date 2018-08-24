#ifndef INTERFACE_STRING_ANSI_H
#define INTERFACE_STRING_ANSI_H

class MyStringAnsi;
class MySmallStringAnsi;

#include <limits>
#include <vector>
#include <string> //std::string
#include <cstring> //strlen and other
#include <functional> //lambdas
#include <stdarg.h> //var args
#include <algorithm> //std::find

#include "./MyStringUtils.h"
#include "./MyStringMacros.h"


static const char* const conversions[] = {
	"00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
	"60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
	"70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
	"80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
	"90", "91", "92", "93", "94", "95", "96", "97", "98", "99"
};

typedef enum SearchAlgorithm { BM = 0, KMP = 1, BF = 2, C_LIB = 3 } SearchAlgorithm;
typedef enum StringConstants { REPLACE_ALL = -1 } StringConstants;

template <typename Type>
class IStringAnsi
{
public:
		
	static const int npos = -1;

	IStringAnsi();
	IStringAnsi(char * str);
	IStringAnsi(const char * str);
	IStringAnsi(const std::string & str);

	IStringAnsi(size_t bufferSize);

	virtual ~IStringAnsi();

	template <typename RetVal = Type>
	static RetVal LoadFromFile(const char * fileName);

	template <typename RetVal = Type>
	static RetVal CreateFormated(const char * str, ...);
		
	//======================================================================
	void Release();
	bool SaveToFile(const char * fileName) const;


	uint32_t GetHashCode() const;
	uint32_t GetRawHashCode() const
	{
		return this->hashCode;
	}

	char GetLastChar() const
	{
		size_t length = static_cast<const Type *>(this)->length();
		return this->operator[](length - 1);
	}


	void Trim();
	void Reverse();
	void RemoveMultipleChars(char t);
	void PopBack();

	template <typename RetVal = Type>
	std::vector<RetVal> Split(char delimeter, bool keepEmptyValues = false) const;

	template <typename RetVal = Type>
	std::vector<RetVal> Split(const std::vector<char> & delimeters, bool keepEmptyValues = false) const;

	size_t Count(const char str) const;


	int Find(const Type & str, SearchAlgorithm algo = C_LIB) const;
	int Find(const char * str, SearchAlgorithm algo = C_LIB) const;
	int Find(const char * str, int offset) const;
	std::vector<int> FindAll(const char * str) const;



	void Append(const char * str, size_t len = 0);

	template<typename... Args>
	void AppendFormat(const char * str, Args... args);
	
	void Replace(const Type & oldValue, const Type & newValue);
	void Replace(const char * oldValue, const char * newValue);
	void Replace(const char * oldValue, const char * newValue, int replaceOffset);
	void Replace(const char * oldValue, const char * newValue, const std::vector<int> & searchStartPos);	
	Type CreateReplaced(const char * src, const char * dest) const;

	Type SubString(int start) const;
	Type SubString(int start, size_t length) const;
	void CopySubstring(int start, char ** destination) const;
	void CopySubstring(int start, size_t length, char ** destination) const;


	std::vector<double> GetAllNumbers() const;

	bool IsNumber() const;
	bool IsIntNumber() const;
	bool IsFloatNumber() const;

	void Transform(std::function<char(char)> t);

	template <typename T>	
	RET_VAL_STR(void, (std::is_integral<T>::value && std::is_signed<T>::value)) operator += (T number);

	template <typename T>	
	RET_VAL_STR(void, (std::is_unsigned<T>::value)) operator += (T number);

	template <typename T>
	RET_VAL_STR(void, (std::is_floating_point<T>::value)) operator += (T number);

	template <typename T>
	RET_VAL_STR(void, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
    operator += (const T & str);
	void operator += (const std::string & str);
	void operator += (const char * str);
	void operator += (const char letter);

	template <typename T>
	RET_VAL_STR(T, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
    operator + (const T & str) const;
	Type operator + (const char * str) const;

	template <typename T>
	RET_VAL_STR(Type &, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
    operator = (const T & str);
	Type & operator = (const char * str);
	Type & operator = (const std::string & str);
	Type & operator = (Type && other);


	template <typename T>
	RET_VAL_STR(char, (std::is_integral<T>::value))
    operator [](const T index) const;
    
    template <typename T>
	RET_VAL_STR(char&, (std::is_integral<T>::value))
    operator [](const T index);


	//template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>	
	//IStringAnsi operator + (T number) const;

	//operator uint32_t() const { return GetHashCode(); }


protected:

	mutable uint32_t hashCode;

	size_t CalcNewBufferSize(size_t forLen) const
	{
		return CalcNewBufferSize(static_cast<const Type *>(this)->capacity(), forLen);
	};

	size_t CalcNewBufferSize(size_t curSize, size_t forLen) const
	{
		size_t newSize = curSize + static_cast<size_t>(curSize * 0.6);
		if (newSize < forLen + 1)
		{
			newSize = forLen + 1;
		}

		return newSize;
	};



	void ResizeBuffer(size_t bufferSize);
	void CreateNew(const char * str, size_t length);

	int CLib(const char * str, size_t start = 0) const;
	int BruteForce(const char * str, size_t start = 0) const;
	int BoyerMoore(const char * str, int * &lookUp, size_t start = 0) const;
	int KnuthMorisPrat(const char * str, int * &lookUp, size_t start = 0) const;


};



//====================================================================
// Static methods
//====================================================================

/// <summary>
/// Load string from file "fileName"
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
template <typename Type>
template <typename RetVal>
RetVal IStringAnsi<Type>::LoadFromFile(const char * fileName)
{
	FILE *f = nullptr;  //pointer to file we will read in
	my_fopen(&f, fileName, "rb");
	if (f == nullptr)
	{
		printf("Failed to open file: \"%s\"\n", fileName);
		return "";
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char * data = new char[size + 1];
	fread(data, sizeof(char), size, f);
	fclose(f);

	data[size] = 0;
	RetVal tmp = RetVal(data);
	delete[] data;

	return tmp;

}

/// <summary>
/// Create new string based on formatted string
/// ("%s %i", "kuk", 145)
/// This is quite slow
/// </summary>
/// <param name="str"></param>
/// <param name=""></param>
/// <returns></returns>
template <typename Type>
template <typename RetVal>
RetVal IStringAnsi<Type>::CreateFormated(const char * newStrFormat, ...)
{
	if (newStrFormat == nullptr)
	{
		return RetVal("");
	}

	va_list vl;

	//calculate length of new string
	std::vector<char> localBuffer;
	int appendLength = -1;
	while (appendLength < 0)
	{
		va_start(vl, newStrFormat);
		localBuffer.resize(localBuffer.size() + 256);
		appendLength = my_vsnprintf(&localBuffer[0], localBuffer.size(), localBuffer.size() - 1, newStrFormat, vl);
		va_end(vl);
	}


	//always store in heap
	size_t bufferSize = appendLength + 16;
	RetVal newStr = RetVal(bufferSize);

	char * str = newStr.str();

	va_start(vl, newStrFormat);
	int written = my_vsnprintf(str, bufferSize, bufferSize - 1, newStrFormat, vl);
	va_end(vl);

	if (written == -1)
	{
		return "";
	}


	size_t strLength = strlen(str);
	str[strLength] = 0;

	newStr.hashCode = std::numeric_limits<uint32_t>::max();
	newStr.SetLengthInternal(strLength);
	return newStr;
}

/// <summary>
/// Append new formated string
/// Really SLOW !!!!- because of used vsnprintf
/// eg("Formated %d %d", 10, 20) = > "Formated 10, 20"
/// </summary>
/// <param name="appendStr"></param>
/// <param name="...args"></param>
template <typename Type>
template<typename... Args>
void IStringAnsi<Type>::AppendFormat(const char * appendStr, Args... args)
{
	Type tmp = IStringAnsi<Type>::CreateFormated(appendStr, args...);
	this->Append(tmp.c_str());
}

//====================================================================
// Operators
//====================================================================


/// <summary>
/// Append signed number
/// </summary>
template <typename Type>
template <typename T>
RET_VAL_STR(void, (std::is_integral<T>::value && std::is_signed<T>::value)) IStringAnsi<Type>::operator += (T number)
{
	//uint64_t input = number;
	typename std::make_unsigned<T>::type input = number;

	int sign = 0;
	if (number < 0)
	{
		sign = 1;
		input = -number;
	}

	size_t len = MyStringUtils::GetNumDigits(input) + sign;
	//sizeof(T) = 1 => 4 (3 + sign)
	//sizeof(T) = 2 => 6 (5 + sign)
	//sizeof(T) = 4 => 11 (10 + sign)
	//sizeof(T) = 8 => 21 (20 + sign)

	size_t curSize = static_cast<Type *>(this)->capacity();
	size_t strLength = static_cast<Type *>(this)->length();
	if (curSize <= strLength + len)
	{
		curSize = CalcNewBufferSize(curSize, strLength + len);
		this->ResizeBuffer(curSize);
	}

	char * str = static_cast<Type *>(this)->str();

	if (len == 1)
	{
		str[strLength++] = static_cast<char>(number + '0');
		str[strLength] = 0;
		static_cast<Type *>(this)->SetLengthInternal(strLength);
		this->hashCode = std::numeric_limits<uint32_t>::max();
		return;
	}


	size_t i = strLength + len - 1;
	while (input > 9)
	{
		typename std::make_unsigned<T>::type x = input / 100;
		const char * t = conversions[input - 100 * x];
		str[i--] = t[1];
		str[i--] = t[0];
		input = x;
	};

	if (input != 0)
	{
		str[i--] = static_cast<char>(input + '0');
	}
	if (sign)
	{
		str[i] = '-';
	}

	strLength += len;

	str[strLength] = 0;
	static_cast<Type *>(this)->SetLengthInternal(strLength);
	this->hashCode = std::numeric_limits<uint32_t>::max();
};

/// <summary>
/// Append unsigned number
/// </summary>
template <typename Type>
template <typename T>
RET_VAL_STR(void, (std::is_unsigned<T>::value)) IStringAnsi<Type>::operator += (T number)
{
	size_t len = MyStringUtils::GetNumDigits(number);
	//sizeof(T) = 1 => 4 (3 + sign)
	//sizeof(T) = 2 => 6 (5 + sign)
	//sizeof(T) = 4 => 11 (10 + sign)
	//sizeof(T) = 8 => 21 (20 + sign)

	size_t curSize = static_cast<Type *>(this)->capacity();
	size_t strLength = static_cast<Type *>(this)->length();

	if (curSize <= strLength + len)
	{
		curSize = CalcNewBufferSize(curSize, strLength + len);
		this->ResizeBuffer(curSize);
	}

	char * str = static_cast<Type *>(this)->str();

	if (len == 1)
	{
		str[strLength++] = static_cast<char>(number + '0');
		str[strLength] = 0;
		static_cast<Type *>(this)->SetLengthInternal(strLength);
		this->hashCode = std::numeric_limits<uint32_t>::max();
		return;
	}

	size_t i = strLength + len - 1;
	while (number > 9)
	{
		T x = number / 100;
		const char * t = conversions[number - 100 * x];
		str[i--] = t[1];
		str[i--] = t[0];
		number = x;
	};

	if (number != 0)
	{
		str[i] = static_cast<char>(number + '0');
	}

	strLength += len;

	str[strLength] = 0;
	static_cast<Type *>(this)->SetLengthInternal(strLength);
	this->hashCode = std::numeric_limits<uint32_t>::max();
};


/// <summary>
/// Append float number
/// </summary>
template <typename Type>
template <typename T>
RET_VAL_STR(void, (std::is_floating_point<T>::value)) IStringAnsi<Type>::operator += (T number)
{
	//to do - create manuall solution that correspond to MyStringUtils::ToNumber instead of this
	std::string tmp = std::to_string(number);
	this->Append(tmp.c_str(), tmp.length());
};


/*
template <typename Type>
template <typename T>
IStringAnsi<Type> IStringAnsi<Type>::operator + (T number) const
{
IStringAnsi newStr = *this;
newStr += number;
return newStr;
}
*/

template <typename Type>
template <typename T>
RET_VAL_STR(void, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
IStringAnsi<Type>::operator += (const T & str)
{
	this->Append(str.c_str(), str.length());
};

template <typename Type>
inline void IStringAnsi<Type>::operator += (const std::string & str)
{
	this->Append(str.c_str(), str.length());
};

template <typename Type>
inline void IStringAnsi<Type>::operator+= (const char * str)
{
	if (str == nullptr)
	{
		return;
	}
	this->Append(str);
}


/// <summary>
/// Add single char to string
/// </summary>
/// <param name="singleChar"></param>
template <typename Type>
inline void IStringAnsi<Type>::operator+= (const char singleChar)
{
	size_t curSize = static_cast<Type *>(this)->capacity();
	size_t strLength = static_cast<Type *>(this)->length();

	if (curSize <= strLength + 1)
	{
		curSize = CalcNewBufferSize(curSize, strLength + 1);
		this->ResizeBuffer(curSize);
	}

	char * str = static_cast<Type *>(this)->str();

	str[strLength++] = singleChar;
	str[strLength] = 0;
	static_cast<Type *>(this)->SetLengthInternal(strLength);

	this->hashCode = std::numeric_limits<uint32_t>::max();
}


template <typename Type>
template <typename T>
RET_VAL_STR(T, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
IStringAnsi<Type>::operator + (const T & str) const
{
	T newStr = T(static_cast<const Type *>(this)->c_str(), 
		static_cast<const Type *>(this)->length());
	newStr.Append(str.c_str(), str.length());
	
	return newStr;
};


template <typename Type>
inline Type IStringAnsi<Type>::operator + (const char * str) const
{
	Type newStr = Type(static_cast<const Type *>(this)->c_str(), 
		static_cast<const Type *>(this)->length());
	newStr.Append(str);

	return newStr;
}

template <typename Type>
template <typename T>
RET_VAL_STR(char, (std::is_integral<T>::value))
IStringAnsi<Type>::operator [](const T index) const
{
	return static_cast<const Type *>(this)->c_str()[index];
}

template <typename Type>
template <typename T>
RET_VAL_STR(char&, (std::is_integral<T>::value))
IStringAnsi<Type>::operator [](const T index)
{
	this->hashCode = std::numeric_limits<uint32_t>::max();
	return static_cast<Type *>(this)->str()[index];
}

template <typename Type>
template <typename T>
RET_VAL_STR(Type&, (std::is_same<T, MyStringAnsi>::value || std::is_same<T, MySmallStringAnsi>::value))
IStringAnsi<Type>::operator = (const T & str)
{
	/*
	if (this == &str) //they are same
	{
		return *static_cast<Type *>(this);
	}
	*/

	this->CreateNew(str.c_str(), str.length());
	return *static_cast<Type *>(this);
};


template <typename Type>
inline Type & IStringAnsi<Type>::operator =(const char * str)
{
	this->CreateNew(str, 0);
	return *static_cast<Type *>(this);
};

template <typename Type>
inline Type & IStringAnsi<Type>::operator = (const std::string & str)
{
	this->CreateNew(str.c_str(), str.length());
	return *static_cast<Type *>(this);
};

template <typename Type>
inline Type & IStringAnsi<Type>::operator = (Type && other)
{
	static_cast<Type *>(this)->ReleaseInternal();
	
	static_cast<Type *>(this)->SetLengthInternal(other.length());
	static_cast<Type *>(this)->SetStrInternal(other.str());
	static_cast<Type *>(this)->SetBufferSizeInternal(other.capacity());

	this->hashCode = other.hashCode;

	// reset other
	other.SetLengthInternal(0);
	other.SetStrInternal(nullptr);
	other.SetBufferSizeInternal(0);
	other.hashCode = std::numeric_limits<uint32_t>::max();

	return *static_cast<Type *>(this);
};

//====================================================================
// Other methods
//====================================================================


/// <summary>
/// Split string by a char delimeter
/// Eg: 11  11  22 -> ' ' => {11,11,22}
/// Eg: 11  11  22 -> ' ', keepEmptyValues => {11,,11,,22}
/// 
/// </summary>
/// <param name="delimeter"></param>
/// <param name="keepEmptyValues">if we want to keep empty values (default: false)</param>
/// <returns></returns>
template <typename Type>
template <typename RetVal>
std::vector<RetVal> IStringAnsi<Type>::Split(char delimeter, bool keepEmptyValues) const
{
	return this->Split<RetVal>(std::vector<char>({ delimeter }), keepEmptyValues);
}

/// <summary>
/// Split string by a char delimeters
/// Eg: 11  11 - 22 -> ' ', '-' => {11,11,22}
/// Eg: 11  11 - 22 -> ' ', '-' keepEmptyValues => {11,,11,,22}
/// 
/// </summary>
/// <param name="delimeters"></param>
/// <param name="keepEmptyValues">if we want to keep empty values (default: false)</param>
/// <returns></returns>
template <typename Type>
template <typename RetVal>
std::vector<RetVal> IStringAnsi<Type>::Split(const std::vector<char> & delimeters, bool keepEmptyValues) const
{
	std::vector<RetVal> splited;

	const char * end = static_cast<const Type *>(this)->c_str();
	const char * start = end;
	char c = 0;
	while ((c = *end) != 0)
	{
		//if (c == delimeter)
		if (std::find(delimeters.begin(), delimeters.end(), c) != delimeters.end())
		{
			//splitting
			size_t len = end - start;
			if (len == 0)
			{
				if (keepEmptyValues)
				{					
					splited.push_back("");
				}
			}
			else
			{				
				splited.push_back(RetVal(start, len));
			}

			start = end + 1;
		}

		end++;
	}


	size_t len = end - start;
	if (len == 0)
	{
		if (keepEmptyValues)
		{			
			splited.push_back("");
		}
	}
	else
	{		
		splited.push_back(RetVal(start, len));
	}

	return splited;
}




#endif
