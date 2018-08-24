#ifndef _MY_STRING_ID_H_
#define _MY_STRING_ID_H_

//ID_DEBUG - in this mode, string is stored together with ID
//-> this mode can not be used together with constexpr !!!
//#define ID_DEBUG



//================================
//includes

#ifdef ID_DEBUG
#include <unordered_map>
#include <set>
#include <cassert>
#endif

#include "./MyStringMacros.h"

#include "./MurmurHash3.h"

#include "./MyStringAnsi.h"
#include "./MySmallStringAnsi.h"


//================================
//class declaration

class MyStringID 
{
	public:
#if defined(ID_DEBUG) || defined(CONSTEXPR_DISABLE)
		MyStringID(const char * key);
#else
		constexpr MyStringID(const char * key);
#endif
		MyStringID(const MyStringAnsi & key);
		MyStringID(const MySmallStringAnsi & key);
		MyStringID(const MyStringID & key);
		MyStringID(const uint32_t & key);

		inline uint32_t GetHashID() const { return this->hashID; };
		inline uint32_t GetHashCode() const { return this->hashID; };
		inline operator uint32_t() { return this->hashID; }
		inline operator uint32_t() const { return this->hashID; }
		
		
	private:
		#ifdef ID_DEBUG
			MyStringAnsi debugKey;

			static void Check(const MyStringAnsi & key, uint32_t hash);
						
		#endif
			const uint32_t hashID;
		
		

};

//================================
// add operator() to std namepespace
//this will enabled to use our ID in hash_map

namespace std
{
	template <>
	struct hash < MyStringID >
	{
		size_t operator()(const MyStringID & k) const
		{
			return k.GetHashID();
		};
	};
};



//================================
//declaration of inline ctor
//this is used mainly for constexpr

#if defined(ID_DEBUG) || defined(CONSTEXPR_DISABLE)
	inline MyStringID::MyStringID(const char * key)
		: hashID(CALC_HASH(key))
	{
#ifdef ID_DEBUG
		MyStringAnsi debugKey = key;
		MyStringID::Check(debugKey, hashID);
#endif
	}
#else
	constexpr inline MyStringID::MyStringID(const char * key)
		: hashID(CALC_HASH(key))
	{
		
	}
#endif

//other ctors
//those are not constexpr

	inline MyStringID::MyStringID(const MyStringAnsi & key)
		: hashID(key.GetHashCode())
	{
#ifdef ID_DEBUG
		this->debugKey = key;
		MyStringID::Check(debugKey, hashID);		
#endif	
	}
	
	inline MyStringID::MyStringID(const MySmallStringAnsi & key)
		: hashID(key.GetHashCode())
	{
#ifdef ID_DEBUG
		this->debugKey = key;
		MyStringID::Check(debugKey, hashID);
#endif	
	}
	

	inline MyStringID::MyStringID(const MyStringID & key)
		: hashID(key.GetHashID())
	{
#ifdef ID_DEBUG
		this->debugKey = key.debugKey;
		MyStringID::Check(debugKey, hashID);
#endif	
	}

	inline MyStringID::MyStringID(const uint32_t & key)
		: hashID(key)
	{
#ifdef ID_DEBUG
		this->debugKey = "direct_hash_value_";
		this->debugKey += key;
		MyStringID::Check(debugKey, hashID);
#endif

	}


#ifdef ID_DEBUG
	inline void MyStringID::Check(const MyStringAnsi & key, uint32_t hashID)
	{
		static std::unordered_map<uint32_t, std::set<MyStringAnsi> > collisions;
		
		if (collisions.find(hashID) == collisions.end())
		{
			//hashID not exist yet
			collisions[hashID].insert(key);
			return;
		}

		
		if (collisions[hashID].find(key) == collisions[hashID].end())
		{
			//hashID already exist... only one key can be set for this hashID
			assert(collisions[hashID].size() == 0);
		}

		
		collisions[hashID].insert(key);

	}
#endif
//================================

//define forward declaration
#define GENERATE_OPERATOR_FORWRAD(p1, p2, op) \
	bool operator op(const p1 &str1, const p2 &id2);


//
//same has id can be for different string - possibly test here, 
//but that is not needed for common words or if there is no prior problem
//
#define GENERATE_BODY(p1, p2, op) \
	inline bool operator op(const p1 &str1, const p2 &id2){ \
		return (str1.GetHashCode() op id2.GetHashCode());  \
	}; 

#define GENERATE_BODY2(p1, p2, op) \
	inline bool operator op(const p1 &id1, const p2 &id2){ \
		return (id1 op id2.GetHashCode());  \
	}; 

#define GENERATE_BODY3(p1, p2, op) \
	inline bool operator op(const p1 &id1, const p2 &id2){ \
		return (id1.GetHashCode() op id2);  \
	}; 

//================================

//declare friend operators to compare our ID with different
//classes and structures directly
GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringID, == );
GENERATE_OPERATOR_FORWRAD(MyStringAnsi, MyStringID, == );
GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringAnsi, == );
GENERATE_OPERATOR_FORWRAD(MySmallStringAnsi, MyStringID, == );
GENERATE_OPERATOR_FORWRAD(MyStringID, MySmallStringAnsi, == );
GENERATE_OPERATOR_FORWRAD(MyStringID, uint32_t, == );
GENERATE_OPERATOR_FORWRAD(uint32_t, MyStringID, == );

GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringID, != );
GENERATE_OPERATOR_FORWRAD(MyStringAnsi, MyStringID, != );
GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringAnsi, != );
GENERATE_OPERATOR_FORWRAD(MySmallStringAnsi, MyStringID, != );
GENERATE_OPERATOR_FORWRAD(MyStringID, MySmallStringAnsi, != );
GENERATE_OPERATOR_FORWRAD(MyStringID, uint32_t, != );
GENERATE_OPERATOR_FORWRAD(uint32_t, MyStringID, != );

GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringID, >);
GENERATE_OPERATOR_FORWRAD(MyStringAnsi, MyStringID, >);
GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringAnsi, >);
GENERATE_OPERATOR_FORWRAD(MySmallStringAnsi, MyStringID, >);
GENERATE_OPERATOR_FORWRAD(MyStringID, MySmallStringAnsi, >);
GENERATE_OPERATOR_FORWRAD(MyStringID, uint32_t, >);
GENERATE_OPERATOR_FORWRAD(uint32_t, MyStringID, >);


GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringID, <);
GENERATE_OPERATOR_FORWRAD(MyStringAnsi, MyStringID, <);
GENERATE_OPERATOR_FORWRAD(MyStringID, MyStringAnsi, <);
GENERATE_OPERATOR_FORWRAD(MySmallStringAnsi, MyStringID, <);
GENERATE_OPERATOR_FORWRAD(MyStringID, MySmallStringAnsi, <);
GENERATE_OPERATOR_FORWRAD(MyStringID, uint32_t, <);
GENERATE_OPERATOR_FORWRAD(uint32_t, MyStringID, <);

//================================


GENERATE_BODY(MyStringAnsi, MyStringID, == );
GENERATE_BODY(MyStringID, MyStringAnsi, == );
GENERATE_BODY(MySmallStringAnsi, MyStringID, == );
GENERATE_BODY(MyStringID, MySmallStringAnsi, == );
GENERATE_BODY(MyStringID, MyStringID, == );
GENERATE_BODY2(uint32_t, MyStringID, == );
GENERATE_BODY3(MyStringID, uint32_t, == );


GENERATE_BODY(MyStringAnsi, MyStringID, != );
GENERATE_BODY(MyStringID, MyStringAnsi, != );
GENERATE_BODY(MySmallStringAnsi, MyStringID, != );
GENERATE_BODY(MyStringID, MySmallStringAnsi, != );
GENERATE_BODY(MyStringID, MyStringID, != );
GENERATE_BODY2(uint32_t, MyStringID, != );
GENERATE_BODY3(MyStringID, uint32_t, != );


GENERATE_BODY(MyStringAnsi, MyStringID, < );
GENERATE_BODY(MyStringID, MyStringAnsi, < );
GENERATE_BODY(MySmallStringAnsi, MyStringID, <);
GENERATE_BODY(MyStringID, MySmallStringAnsi, <);
GENERATE_BODY(MyStringID, MyStringID, < );
GENERATE_BODY2(uint32_t, MyStringID, < );
GENERATE_BODY3(MyStringID, uint32_t, < );


GENERATE_BODY(MyStringAnsi, MyStringID, > );
GENERATE_BODY(MyStringID, MyStringAnsi, > );
GENERATE_BODY(MySmallStringAnsi, MyStringID, >);
GENERATE_BODY(MyStringID, MySmallStringAnsi, >);
GENERATE_BODY(MyStringID, MyStringID, > );
GENERATE_BODY2(uint32_t, MyStringID, > );
GENERATE_BODY3(MyStringID, uint32_t, > );


//================================
//undef defined values

#ifdef GENERATE_OPERATOR_FORWRAD
	#undef GENERATE_OPERATOR_FORWRAD
#endif

#ifdef GENERATE_BODY
	#undef GENERATE_BODY
#endif
#ifdef GENERATE_BODY2
	#undef GENERATE_BODY2
#endif
#ifdef GENERATE_BODY3
	#undef GENERATE_BODY3
#endif

#ifdef ID_DEBUG
	#undef ID_DEBUG
#endif



#endif //_MY_ID_H_