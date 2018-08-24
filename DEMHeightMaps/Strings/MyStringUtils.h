#ifndef MY_STRING_UTILS
#define MY_STRING_UTILS


#include <stdint.h>
#include <type_traits>

#include "./MyStringMacros.h"


struct MyStringUtils 
{
		
	//https://tombarta.wordpress.com/2008/04/23/specializing-atoi/

	/// <summary>
	/// Convert string to floating point number
	/// Not working same for "e" notation 10e125
	/// -> result differ fropm std::strtod
	/// </summary>
	/// <param name="str">input string</param>
	/// <param name="ret">out: if not null, pointer where we ended</param>
	/// <returns>converte number</returns>
	template <typename T>
	static RET_VAL_STR(T, (std::is_floating_point<T>::value)) ToNumber(const char * str, const char ** ret = nullptr)
	{
		//skip leading whitespace
		while ((*str <= ' ') && (*str != 0))
		{
			++str;
		}

		//check sign
		int sign = 1;
		if (*str == '-') // handle negative
		{
			sign = -1;
			++str;
		}

		uint64_t value = 0;
		while ((*str >= '0') && (*str <= '9'))
		{
			value = value * 10 + (*str - '0');
			++str;
		}

		if ((*str != '.') && (*str != 'e'))
		{
			if (ret) *ret = str;
			return static_cast<T>(value) * sign;
		}

		uint64_t fractValue = 0;
		uint64_t fractLen = 1;
		int64_t expon = 0;
		int exponSign = 1;

		T val = static_cast<T>(value) * sign;

		if (*str == '.')
		{
			++str;
			
			while ((*str >= '0') && (*str <= '9'))
			{
				fractValue = fractValue * 10 + (*str - '0');
				fractLen *= 10;
				++str;
			}

			val = (static_cast<T>(value) + static_cast<T>(fractValue) / fractLen) * sign;
		}
		
		if (*str == 'e')
		{
			//not fully working the same as std::strtod

			++str;			
			if (*str == '-') // handle negative
			{
				exponSign = -1;
				++str;
			}
			else if (*str == '+') // handle positive
			{
				exponSign = +1;
				++str;
			}

			while ((*str >= '0') && (*str <= '9'))
			{
				expon = expon * 10 + (*str - '0');
				++str;
			}
						
			
			//pow 10^exp
			if (exponSign == -1)
			{
				while (expon--)
				{
					val /= 10.0;
				}
			}
			else 
			{
				while (expon--)
				{
					val *= 10.0;
				}
			}			
		}
		
		if (ret) *ret = str;
		return val;
	}

	/// <summary>
	/// Convert string to integral number
	/// </summary>
	/// <param name="str">input string</param>
	/// <param name="ret">out: if not null, pointer where we ended</param>
	/// <returns>converte number</returns>
	template <typename T>
	static RET_VAL_STR(T, (std::is_integral<T>::value)) ToNumber(const char * str, const char * ret = nullptr)
	{
		//skip leading whitespace
		while ((*str <= ' ') && (*str != 0))
		{
			++str;
		}
		
		//check sign
		int sign = 1;
		if (*str == '-') // handle negative
		{
			sign = -1;
			++str;
		}

		uint64_t value = 0;
				
		while ((*str >= '0') && (*str <= '9'))
		{
			value = value * 10 + (*str - '0');
			++str;
		}

		if (ret) ret = str;
		return static_cast<T>(value) * sign;
	};


	/// <summary>
	/// Universal fallback for number of digits in number
	/// </summary>
	/// <param name="number"></param>
	/// <returns></returns>
	template <typename T>	
	static int GetNumDigits(T number)
	{
		int digits = 0;
		if (number < 0) digits = 1; // remove this line if '-' counts as a digit
		while (number) {
			number /= 10;
			digits++;
		}
		return digits;
	}		
};


/// <summary>
/// Calculate number of digits in uint8_t
/// </summary>
/// <param name="number"></param>
/// <returns></returns>
template <>
inline int MyStringUtils::GetNumDigits(uint8_t x)
{
	if (x >= 10U)
	{
		if (x >= 100U) return 3;
		return 2;
	}
	return 1;
}

/// <summary>
/// Calculate number of digits in uint16_t
/// </summary>
/// <param name="number"></param>
/// <returns></returns>
template <>
inline int MyStringUtils::GetNumDigits(uint16_t x)
{
	if (x >= 1000U)
	{
		if (x >= 10000U) return 5;
		return 4;
	}

	if (x >= 10U)
	{
		if (x >= 100U) return 3;
		return 2;
	}

	return 1;
}

/// <summary>
/// Calculate number of digits in uint32_t
/// </summary>
/// <param name="number"></param>
/// <returns></returns>
template <>
inline int MyStringUtils::GetNumDigits(uint32_t x)
{
	if (x >= 10000U) {
		if (x >= 10000000U) {
			if (x >= 100000000U) {
				if (x >= 1000000000U) return 10;
				return 9;
			}
			return 8;
		}
		if (x >= 100000U) {
			if (x >= 1000000U) return 7;
			return 6;
		}
		return 5;
	}
	if (x >= 100U) {
		if (x >= 1000U) return 4;
		return 3;
	}
	if (x >= 10U) return 2;
	return 1;
}

/// <summary>
/// Calculate number of digits in uint64_t
/// </summary>
/// <param name="number"></param>
/// <returns></returns>
template <>
inline int MyStringUtils::GetNumDigits(uint64_t x)
{
	if (x >= 10000000000U) { //11
		if (x >= 100000000000000U) { //15
			if (x >= 100000000000000000U) { //18
				if (x >= 1000000000000000000U) { //19
					if (x >= 10000000000000000000U) return 20;
					return 19;
				}
				return 18;
			}
			if (x >= 1000000000000000U) { //16
				if (x >= 10000000000000000U) return 17;
				return 16;
			}
			return 15;
		}
		if (x >= 1000000000000U) { //13
			if (x >= 10000000000000U) return 14;
			return 13;
		}
		if (x >= 100000000000U) return 12;
		return 11;
	}

	if (x >= 10000U) { //5
		if (x >= 10000000U) { //8
			if (x >= 100000000U) { //9
				if (x >= 1000000000U) return 10;
				return 9;
			}
			return 8;
		}
		if (x >= 100000U) { //6
			if (x >= 1000000U) return 7;
			return 6;
		}
		return 5;
	}
	if (x >= 100U) { //3
		if (x >= 1000U) return 4;
		return 3;
	}
	if (x >= 10U) return 2;
	return 1;
}

#endif
