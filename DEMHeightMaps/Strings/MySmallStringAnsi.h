#ifndef MY_SMALL_STRING_ANSI_H
#define MY_SMALL_STRING_ANSI_H

#include <string.h>
#include <vector>
#include <limits>

#include "./MyStringUtils.h"
#include "./MyStringMacros.h"

#include "./IStringAnsi.h"

//#define BUFFER_SIZE 19



class MySmallStringAnsi : public IStringAnsi<MySmallStringAnsi>
{
public:
	static const size_t BUFFER_SIZE = 19;

	using IStringAnsi<MySmallStringAnsi>::IStringAnsi;
	using IStringAnsi<MySmallStringAnsi>::operator=;

	MySmallStringAnsi()		
	{
		this->CtorInternal(nullptr);
	}

	MySmallStringAnsi(const char * newStr, size_t length)
	{
		memset(local, 0, sizeof(local));
		size_t bufferSize = length + 1;

		char * str = local;
		if (bufferSize > BUFFER_SIZE)
		{
			str = new char[bufferSize];
			this->SetBufferSizeInternal(bufferSize);
		}

		memcpy(str, newStr, length);
		str[length] = 0;

		this->SetStrInternal(str);
		this->SetLengthInternal(length);
	}

	MySmallStringAnsi(const MySmallStringAnsi &other)
	{
		memcpy(local, other.local, sizeof(local));
		if (this->IsLocal() == false)
		{
			this->CtorInternal(other.c_str());
		}
		this->hashCode = other.hashCode;
	};

	MySmallStringAnsi(MySmallStringAnsi && other)  /* noexcept needed to enable optimizations in containers */
	{
		//http://blog.smartbear.com/c-plus-plus/c11-tutorial-introducing-the-move-constructor-and-the-move-assignment-operator/
		memcpy(local, other.local, sizeof(local));
		hashCode = other.hashCode;

		memset(other.local, 0, sizeof(other.local));
		other.hashCode = std::numeric_limits<uint32_t>::max();
	};

	~MySmallStringAnsi() = default;



	const char * c_str() const
	{
		if (this->IsLocal()) return local;

		uintptr_t addr = 0;
		memcpy(&addr, local + 8, sizeof(uintptr_t));
		return reinterpret_cast<char *>(addr);
	};


	size_t length() const
	{
		if (this->IsLocal()) return local[BUFFER_SIZE];
		return (uint8_t(local[7]) << 24) +
			(uint8_t(local[6]) << 16) +
			(uint8_t(local[5]) << 8) +
			uint8_t(local[4]);
	};

	size_t capacity() const
	{
		if (this->IsLocal()) return BUFFER_SIZE;

		return (uint8_t(local[3]) << 24) +
			(uint8_t(local[2]) << 16) +
			(uint8_t(local[1]) << 8) +
			uint8_t(local[0]);
	};


	MySmallStringAnsi & operator = (const MySmallStringAnsi & str)
	{
		return IStringAnsi<MySmallStringAnsi>::operator=(str);
	};

	friend class IStringAnsi<MySmallStringAnsi>;

protected:

	//minimal size must be: 
	//uint32_t | uint32_t | uintptr_t | uint8_t
	//capacity | length (if "heap") | address (if "heap") | length (if local), < 0 (if "heap")  
	char local[BUFFER_SIZE + 1];



	void ReleaseInternal()
	{
		char * str = this->str();
		if (str != this->local)
		{
			delete[] str;
			str = nullptr;
		}

		memset(this->local, 0, sizeof(local));
		this->hashCode = std::numeric_limits<uint32_t>::max();		
	};

	void DefaultInit()
	{
		memset(this->local, 0, sizeof(local));		
	}

	void CtorInternal(const char * newStr)
	{
		memset(local, 0, sizeof(local));
		if (newStr == nullptr)
		{
			this->local[0] = 0;
			this->SetLengthInternal(0);
			return;
		}

		size_t strLength = strlen(newStr);

		//if strlen < 16 -> use local buffer, [15] = 0
		//if strlen >= 16 -> use "heap"

		char * str = this->local;

		if (strLength >= BUFFER_SIZE)
		{
			size_t bufferSize = strLength + 1;
			this->SetBufferSizeInternal(bufferSize);

			str = new char[bufferSize];
		}
		this->SetStrInternal(str);

		memcpy(str, newStr, strLength + 1);	//copy with "null-termination"		
		this->SetLengthInternal(strLength);
	};


	char * str()
	{
		if (this->IsLocal()) return local;

		uintptr_t addr = 0;
		memcpy(&addr, local + 8, sizeof(uintptr_t));
		return reinterpret_cast<char *>(addr);
		//return this->strp;
	};

	bool IsLocal() const
	{
		return local[BUFFER_SIZE] >= 0;
	};

	void SetBufferSizeInternal(size_t s)
	{
		uint32_t tmp = static_cast<uint32_t>(s);
		memcpy(this->local, &tmp, sizeof(uint32_t));
	};

	void SetLengthInternal(size_t s)
	{
		if (this->IsLocal())
		{
			local[BUFFER_SIZE] = static_cast<char>(s);
		}
		else
		{
			uint32_t tmp = static_cast<uint32_t>(s);
			memcpy(this->local + 4, &tmp, sizeof(uint32_t));
		}
	};

	void SetStrInternal(char * s)
	{
		if (s == local) return;

		uintptr_t addr = reinterpret_cast<uintptr_t>(s);
		memcpy(local + 8, &addr, sizeof(uintptr_t));

		local[BUFFER_SIZE] = -1;
	};
};

//For use in std::unordered_map
//http://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std
{
	template <>
	struct hash<MySmallStringAnsi>
	{
		std::size_t operator()(const MySmallStringAnsi & k) const
		{
			return k.GetHashCode();
		};
	};
};



#endif