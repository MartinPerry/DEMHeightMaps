#ifndef _MEMORY_CACHE_H_
#define _MEMORY_CACHE_H_

#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <string.h>
#include <cstdio>
#include <cstdint>
#include <thread>
#include <mutex>
#include <random>
#include <list>

//======================================================
//======= MemoryCache Cache management =================
//======================================================

template <typename Key, typename Value, typename CacheControl>
class MemoryCache
{
public:

	struct InsertInfo
	{
		bool itemInserted;
		bool itemRemoved;
		std::vector<Value> removedValue;

	};

	MemoryCache(size_t size, const CacheControl & type);
	~MemoryCache() = default;

	void SetMaxSize(size_t size);
	size_t GetItemsCount() const;
	
	typename MemoryCache<Key, Value, CacheControl>::InsertInfo Insert(const Key & key, const Value & value, size_t valueSize = sizeof(Value));
	typename MemoryCache<Key, Value, CacheControl>::InsertInfo InsertWithValidTime(const Key & key, const Value & value, uint32_t lifeTimeSeconds, size_t valueSize = sizeof(Value));
	Value * Get(const Key & key);


    bool Remove(const Key & key);
    
private:

	struct ValueInfo
	{
		Value value;
		size_t size;
		time_t validSince;
	};

	size_t maxSize;
	size_t currentSize;
	CacheControl type;
	std::unordered_map<Key, ValueInfo> values;
	size_t insertedWithoutValidTime;

	std::mutex memCacheLock;

	bool RemoveInvalidTime(typename MemoryCache<Key, Value, CacheControl>::InsertInfo & info);
};

//======================================================
//============== Implementation ========================
//======================================================

/// <summary>
/// ctor
/// </summary>
/// <param name="size">max cache size in bytes</param>
/// <param name="type">cache control type</param>
template <typename Key, typename Value, typename CacheControl>
MemoryCache<Key, Value, CacheControl>::MemoryCache(size_t size, const CacheControl & type)
	: maxSize(size), currentSize(0), type(type), insertedWithoutValidTime(0)
{
}


/// <summary>
/// Change max size
/// </summary>
/// <param name="size">new maximal size</param>
template <typename Key, typename Value, typename CacheControl>
void MemoryCache<Key, Value, CacheControl>::SetMaxSize(size_t size)
{
	this->maxSize = size;
}

/// <summary>
/// Get number of items in cache
/// </summary>
/// <returns></returns>
template <typename Key, typename Value, typename CacheControl>
size_t MemoryCache<Key, Value, CacheControl>::GetItemsCount() const
{
	return this->values.size();
}

/// <summary>
/// Insert new element
/// </summary>
/// <param name="key"></param>
/// <param name="value"></param>
/// <returns>info about cache control</returns>
template <typename Key, typename Value, typename CacheControl>
typename MemoryCache<Key, Value, CacheControl>::InsertInfo
MemoryCache<Key, Value, CacheControl>::Insert(const Key & key, const Value & value, size_t valueSize)
{
	return this->InsertWithValidTime(key, value, 0, valueSize);
}

/// <summary>
/// Insert new element with expiration time
/// </summary>
/// <param name="key"></param>
/// <param name="value"></param>
/// <param name="lifeTimeSeconds"></param>
/// <returns>info about cache control</returns>
template <typename Key, typename Value, typename CacheControl>
typename MemoryCache<Key, Value, CacheControl>::InsertInfo
MemoryCache<Key, Value, CacheControl>::InsertWithValidTime(const Key & key, const Value & value, uint32_t lifeTimeSeconds, size_t valueSize)
{
	std::lock_guard<std::mutex> lock(memCacheLock);

	InsertInfo info;
	info.itemRemoved = false;
	info.itemInserted = false;
	

	auto it = this->values.find(key);

	if (it == this->values.end())
	{
		if (this->type.GetItemsCount() != 0)
		{
			while ((this->currentSize + valueSize) > this->maxSize)
			{
				if (this->RemoveInvalidTime(info) == false)
				{
					break;
				}
			}
			
			while ((this->currentSize + valueSize) > this->maxSize)
			{
				auto deletedKey = this->type.GetLeastKey();

				//available space exhausted... delete from cache
				if (this->type.Erase())
				{
					auto it = this->values.find(deletedKey);

					info.itemRemoved = true;
					info.removedValue.push_back(it->second.value);
					this->currentSize -= it->second.size;

					this->values.erase(it);
				}				
			}
		}

		ValueInfo vi;
		vi.size = valueSize;
		vi.value = value;
		if (lifeTimeSeconds == 0)
		{
			insertedWithoutValidTime++;
			vi.validSince = 0;
		}
		else 
		{
			time_t now;
			time(&now);		

			vi.validSince = now + lifeTimeSeconds;
		}

		this->values[key] = vi;//.insert(std::make_pair<Key, Value>(key, value));
		this->currentSize += vi.size;

		this->type.Insert(key);

		info.itemInserted = true;
	}
	else
	{
		//same key exist
	}

	return info;
}


template <typename Key, typename Value, typename CacheControl>
bool MemoryCache<Key, Value, CacheControl>::Remove(const Key & key)
{
    auto deletedKey = this->values.find(key);
    if (deletedKey == this->values.end())
    {
        return false;
    }
    
	if (deletedKey->second.validSince == 0)
	{
		insertedWithoutValidTime--;
	}

    this->currentSize -= deletedKey->second.size;
    
    this->values.erase(deletedKey);
    
    return true;
}

/// <summary>
/// Remove keys with invalid time
/// </summary>
/// <param name="info"></param>
template <typename Key, typename Value, typename CacheControl>
bool MemoryCache<Key, Value, CacheControl>::RemoveInvalidTime(typename MemoryCache<Key, Value, CacheControl>::InsertInfo & info)
{	
	if (insertedWithoutValidTime = this->values.size())
	{
		//all items were inserted without time
		return false;
	}

	time_t now;	
	time(&now);  /* get current time; same as: now = time(NULL)  */

	std::list<Key> removed;

	for (auto v : this->values)
	{				
		if (v.second.validSince == 0)
		{
			continue;
		}

		double seconds = difftime(v.second.validSince, now);

		if (seconds < 0)
		{
			removed.push_back(v.first);
		}
	}
	

	for (auto & deletedKey : removed)
	{
		if (this->type.Erase(deletedKey))
		{
			auto it = this->values.find(deletedKey);

			if (it->second.validSince == 0)
			{
				insertedWithoutValidTime--;
			}

			info.itemRemoved = true;
			info.removedValue.push_back(it->second.value);
			this->currentSize -= it->second.size;

			this->values.erase(it);
		}
	}

	return removed.size() != 0;
}

/// <summary>
/// Get value from cache by its key and update its "use"
/// </summary>
/// <param name="key"></param>
/// <returns></returns>
template <typename Key, typename Value, typename CacheControl>
Value * MemoryCache<Key, Value, CacheControl>::Get(const Key & key)
{
	std::lock_guard<std::mutex> lock(memCacheLock);

	auto it = this->values.find(key);
	
	if (it == this->values.end())
	{
		return nullptr;
	}

	this->type.Update(key);

	return &(it->second.value);
}



#endif
