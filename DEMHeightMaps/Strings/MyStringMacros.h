#ifndef _MY_STRING_MACROS_H_
#define _MY_STRING_MACROS_H_


//#define CONSTEXPR_DISABLE 


#ifndef SWAP
	//http://www.cprogramming.com/tutorial/cpreprocessor.html
	#define SWAP(a, b)  do { a ^= b; b ^= a; a ^= b; } while ( 0 )
#endif







#ifdef _MSC_VER
	#ifndef my_fopen 
		#define my_fopen(a, b, c) fopen_s(a, b, c)	
	#endif

	#ifndef my_localtime
		#define my_localtime(a, b) localtime_s(a, b)	
	#endif

	#ifndef FORCE_INLINE
		#define FORCE_INLINE	__forceinline
	#endif

#else

	#ifndef my_fopen 
		#define my_fopen(a, b, c) (*a = fopen(b, c))
	#endif

	#ifndef my_localtime
		#define my_localtime(a, b) (a = localtime(b))	
	#endif

	#ifndef FORCE_INLINE
		#define	FORCE_INLINE inline __attribute__((always_inline))
	#endif
#endif




#ifdef _MSC_VER
#define my_strncpy(a, b, c, d) strncpy_s(a, b, c, d)
#define my_strlwr(a, b) _strlwr_s(a, b)
#define my_strupr(a, b) _strupr_s(a, b)
#define my_strtok(a, b, c) strtok_s(a, b, c)
#define my_sprintf(a, b, c, d) sprintf_s(a, b, c, d)
#define my_vsnprintf(a, b, c, e, d) vsnprintf_s(a, b, c, e, d)
#define my_mbstowcs(a, b, c, e, d) mbstowcs_s(a, b, c, e, d)
#define my_swprintf(a, b, c, d) swprintf_s(a, b, c, d)
#define my_strdup(a) _strdup(a)

#define my_wcsdup(a) _wcsdup(a)
#define my_wcstok(a, b, c)  wcstok(a, b)
#define my_wcslwr(a) _wcslwr(a)
#define my_wcsupr(a) _wcsupr(a)

#else

#include <cctype>
#include <cwctype>

#define my_strncpy(a, b, c, d) strncpy(a, c, d)
#define my_strlwr(a, b) do {char * p = a; while ((*p = tolower( *p ))) p++;} while( 0 );
#define my_strupr(a, b) do {char * p = a; while ((*p = toupper( *p ))) p++;} while( 0 );
#define my_strtok(a, b, c) strtok(a, b)
#define my_sprintf(a, b, c, d) sprintf(a, c, d)

#define my_vsnprintf(a, b, c, e, d) vsnprintf(a, c, e, d)
#define my_mbstowcs(a, b, c, e, d) mbstowcs(b, e, d)

#define my_strdup(a) strdup(a)
#define my_swprintf(a, b, c, d) swprintf(a, b, c, d)

#define my_wcsdup(a) wcsdup(a)
#define my_wcstok(a, b, c)  wcstok(a, b, c)

#define my_wcslwr(a) a=my_wcslwr_func(a)
#define my_wcsupr(a) a=my_wcsupr_func(a)



inline wchar_t* my_wcslwr_func(wchar_t* str) { wchar_t* it = str; while (*it != 0) { *it = towlower(*it); ++it; } return str; }

inline wchar_t* my_wcsupr_func(wchar_t* str) { wchar_t* it = str; while (*it != 0) { *it = towupper(*it); ++it; } return str; }

#endif

#define SAFE_STRDUP(s) (s != nullptr) ? my_strdup(s) : my_strdup("")

//https://stackoverflow.com/questions/13842468/comma-in-c-c-macro

#define UNPACK_ENABLE_COND( ... ) __VA_ARGS__

#define RET_VAL_STR(ret_type, enable_cond) \
	typename std::enable_if<UNPACK_ENABLE_COND enable_cond, ret_type>::type
#endif
